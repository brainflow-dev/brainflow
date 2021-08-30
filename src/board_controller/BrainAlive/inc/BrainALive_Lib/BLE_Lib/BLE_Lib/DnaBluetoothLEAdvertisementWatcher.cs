using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using Windows.Devices.Bluetooth;
using Windows.Devices.Bluetooth.Advertisement;
using Windows.Devices.Bluetooth.GenericAttributeProfile;
using Windows.Devices.Enumeration;
using Windows.Storage.Streams;

namespace BLE_Lib
{
    public class DnaBluetoothLEAdvertisementWatcher
    {
        Windows.Devices.Bluetooth.GenericAttributeProfile.GattCharacteristicProperties mProperties;
        Windows.Devices.Bluetooth.GenericAttributeProfile.GattCharacteristicsResult mCharacteristics;
        public GattCharacteristic mNotifychar, mWritechar, mOTA, mOTA_Rebbot, mOTA_File_upload, mOTA_File_upload_cnfrm;
        public bool mDevicepaired = false;
        public bool mDeviceunpaired = false;
        public byte[] mReceive = new byte[38];
        public BluetoothLEDevice mBledevice;
        GattDeviceServicesResult mGatt_2;
        //GattCharacteristic mCharacteristics_map;

        #region Private Members

        /// <summary>
        /// The underlying bluetooth watcher class
        /// </summary>
        private readonly BluetoothLEAdvertisementWatcher mWatcher;

        /// <summary>
        /// A list of discovered devices
        /// </summary>
        private readonly Dictionary<string, DnaBluetoothLEDevice> mDiscoveredDevices = new Dictionary<string, DnaBluetoothLEDevice>();

        /// <summary>
        /// The details about GATT services
        /// </summary>
        private readonly GattServiceIds mGattServiceIds;

        /// <summary>
        /// A thread lock object for this class 
        /// </summary>
        private readonly object mThreadLock = new object();

        #endregion

        #region Public Properties

        /// <summary>
        /// Indicates if this watcher is listening for advertisements
        /// </summary>
        public bool Listening => mWatcher.Status == BluetoothLEAdvertisementWatcherStatus.Started;

        /// <summary>
        /// A list of discovered devices
        /// </summary>
        public IReadOnlyCollection<DnaBluetoothLEDevice> DiscoveredDevices
        {
            get
            {
                // Clean up any timeouts
                CleanupTimeouts();

                // Practice thread-safety kids!
                lock (mThreadLock)
                {
                    // Convert to read-only list
                    return mDiscoveredDevices.Values.ToList().AsReadOnly();
                }
            }
        }

        /// <summary>
        /// The timeout in seconds that a device is removed from the <see cref="DiscoveredDevices"/>
        /// list if it is not re-advertised within this time
        /// </summary>
        public int HeartbeatTimeout { get; set; } = 100;

        #endregion

        #region Public Events

        /// <summary>
        /// Fired when the bluetooth watcher stops listening
        /// </summary>

        public event Action StoppedListening = () => { };

        /// <summary>
        /// Fired when the bluetooth watcher starts listening
        /// </summary>
        public event Action StartedListening = () => { };

        /// <summary>
        /// Fired when a device is discovered
        /// </summary>
        public event Action<DnaBluetoothLEDevice> DeviceDiscovered = (device) => { };

        /// <summary>
        /// Fired when a new device is discovered
        /// </summary>
        public event Action<DnaBluetoothLEDevice> NewDeviceDiscovered = (device) => { };

        /// <summary>
        /// Fired when a device name changes
        /// </summary>
        public event Action<DnaBluetoothLEDevice> DeviceNameChanged = (device) => { };

        /// <summary>
        /// Fired when a device is removed for timing out
        /// </summary>
        public event Action<DnaBluetoothLEDevice> DeviceTimeout = (device) => { };

        #endregion

        #region Constructor

        /// <summary>
        /// The default constructor
        /// </summary>
        public DnaBluetoothLEAdvertisementWatcher(GattServiceIds gattIds)
        {
            // Null guard
            mGattServiceIds = gattIds ?? throw new ArgumentNullException(nameof(gattIds));

            // Create bluetooth listener
            mWatcher = new BluetoothLEAdvertisementWatcher
            {
                ScanningMode = BluetoothLEScanningMode.Active
            };

            // Listen out for new advertisements
            mWatcher.Received += WatcherAdvertisementReceivedAsync;

            // Listen out for when the watcher stops listening
            mWatcher.Stopped += (watcher, e) =>
            {
                // Inform listeners
                StoppedListening();
            };
        }

        #endregion

        #region Private Methods

        /// <summary>
        /// Listens out for watcher advertisements
        /// </summary>
        /// <param name="sender">The watcher</param>
        /// <param name="args">The arguments</param>
        private async void WatcherAdvertisementReceivedAsync(BluetoothLEAdvertisementWatcher sender, BluetoothLEAdvertisementReceivedEventArgs args)
        {
            // Cleanup Timeouts
            // CleanupTimeouts();


            // Get BLE device info
            var device = await GetBluetoothLEDeviceAsync(
                args.BluetoothAddress,
                args.Timestamp,
                args.RawSignalStrengthInDBm);

            // Null guard
            if (device == null)
                return;

            // Is new discovery?
            var newDiscovery = false;
            var existingName = default(string);
            var nameChanged = false;

            // Lock your doors
            lock (mThreadLock)
            {
                // Check if this is a new discovery
                newDiscovery = !mDiscoveredDevices.ContainsKey(device.DeviceId);

                // If this is not new...
                if (!newDiscovery)
                    // Store the old name
                    existingName = mDiscoveredDevices[device.DeviceId].Name;

                // Name changed?
                nameChanged =
                    // If it already exists
                    !newDiscovery &&
                    // And is not a blank  name
                    !string.IsNullOrEmpty(device.Name) &&
                    // And the name is different
                    existingName != device.Name;

                // If we are no longer listening...
                if (!Listening)
                    // Don't bother adding to the list and do nothing
                    return;

                // Add/update the device in the dictionary
                mDiscoveredDevices[device.DeviceId] = device;
            }

            // Inform listeners
            DeviceDiscovered(device);

            // If name changed...
            if (nameChanged)
                // Inform listeners
                DeviceNameChanged(device);

            // If new discovery...
            if (newDiscovery)
                // Inform listeners
                NewDeviceDiscovered(device);
        }

        /// <summary>
        /// Connects to the BLE device and extracts more information from the
        /// <see cref="https://docs.microsoft.com/en-us/uwp/api/windows.devices.bluetooth.bluetoothledevice"/>
        /// </summary>
        /// <param name="address">The BT address of the device to connect to</param>
        /// <param name="broadcastTime">The time the broadcast message was received</param>
        /// <param name="rssi">The signal strength in dB</param>
        /// <returns></returns>
        private async Task<DnaBluetoothLEDevice> GetBluetoothLEDeviceAsync(ulong address, DateTimeOffset broadcastTime, short rssi)
        {
            // Get bluetooth device info
             var device = await BluetoothLEDevice.FromBluetoothAddressAsync(address).AsTask();

            // Null guard
            if (device == null)
                return null;

            // NOTE: This can throw a System.Exception for failures
            /* // Get GATT services that are available
             var gatt = await device.GetGattServicesAsync().AsTask();
             // If we have any services...
             if (gatt.Status == GattCommunicationStatus.Success)
             {
                 // Loop each GATT service
                 foreach (var service in gatt.Services)
                 {
                     // This ID contains the GATT Profile Assigned number we want!
                     // TODO: Get more info and connect
                     var gattProfileId = service.Uuid;


                 }
             }*/

            // Return the new device information
            return new DnaBluetoothLEDevice
            (
                // Device Id
                deviceId: device.DeviceId,
                // Bluetooth Address
                address: device.BluetoothAddress,
                // Device Name
                name: device.Name,
                // Broadcast Time
                broadcastTime: broadcastTime,
                // Signal Strength
                rssi: rssi
            // Is Connected?
            /* connected: device.ConnectionStatus == BluetoothConnectionStatus.Connected,
             // Can Pair?
             canPair: device.DeviceInformation.Pairing.CanPair,
             // Is Paired?
             paired: device.DeviceInformation.Pairing.IsPaired*/
            );
        }

        /// <summary>
        /// Prune any timed out devices that we have not heard off
        /// </summary>
        private void CleanupTimeouts()
        {
            lock (mThreadLock)
            {
                // The date in time that if less than means a device has timed out
                var threshold = DateTime.UtcNow - TimeSpan.FromSeconds(HeartbeatTimeout);

                // Any devices that have not sent a new broadcast within the heartbeat time
                mDiscoveredDevices.Where(f => f.Value.BroadcastTime < threshold).ToList().ForEach(device =>
                {
                    // Remove device
                    mDiscoveredDevices.Remove(device.Key);

                    // Inform listeners
                    DeviceTimeout(device.Value);
                });
            }
        }

        #endregion

        #region Public Methods

        /// <summary>
        /// Starts listening for advertisements
        /// </summary>
        public void StartListening()
        {
            lock (mThreadLock)
            {
                // If already listening...
                if (Listening)
                    // Do nothing more
                    return;

                // Start the underlying watcher
                mWatcher.Start();
            }

            // Inform listeners
            StartedListening();
        }

        /// <summary>
        /// Stops listening for advertisements
        /// </summary>
        public void StopListening()
        {
            lock (mThreadLock)
            {
                // If we are no currently listening...
                if (!Listening)
                    // Do nothing more
                    return;

                // Stop listening
                mWatcher.Stop();

                // Clear any devices
                mDiscoveredDevices.Clear();
            }
        }
        public void InitiateConnection()
        {
            var mreceive = new DnaBluetoothLEAdvertisementWatcher(new GattServiceIds());
            // Console.WriteLine("Read start");
            mNotifychar.ValueChanged += RecieveDataAsync;

            //Console.WriteLine(" File Write Start...");


            //Block code

        }
        /// <summary>
        /// Attempts to pair to a BLE device, by ID
        /// </summary>
        /// <param name="deviceId">The BLE device ID</param>
        /// <returns></returns>
        /*  public async Task UnPairToDeviceAsync(string deviceId)
          {
              using var device = await BluetoothLEDevice.FromIdAsync(deviceId).AsTask();
              if (device == null)
                  // TODO: Localize
                  throw new ArgumentNullException("Failed to get information about the Bluetooth device");
              if ((!device.DeviceInformation.Pairing.IsPaired)==true)
                  return;
             else if (device.DeviceInformation.Pairing.IsPaired==true)
              {
                  var resu = await device.DeviceInformation.Pairing.UnpairAsync();
              }


          }*/

        public async Task Write_to_DeviceAsync(byte[] writevalue)
        {

            var writer = new DataWriter();
            writer.WriteBytes(writevalue);
            /* var valu = writer.DetachBuffer();*/
            var readvalue = await mWritechar.WriteValueAsync(writer.DetachBuffer());
            Console.WriteLine("Write Sucessfully");


        }
        public async Task Write_OTAsync(byte[] writevalue)
        {

            var writer = new DataWriter();
            writer.WriteBytes(writevalue);
            /* var valu = writer.DetachBuffer();*/
            var readvalue = await mOTA.WriteValueAsync(writer.DetachBuffer());
            Console.WriteLine("Write Sucessfully");
        }
        public async Task Write_OTA_ModeAsync(byte[] writevalue)
        {

            var writer = new DataWriter();
            writer.WriteBytes(writevalue);
            /* var valu = writer.DetachBuffer();*/
            var readvalue = await mOTA_Rebbot.WriteValueAsync(writer.DetachBuffer());
            Console.WriteLine("Write Sucessfully");
        }
        public async Task Write_upload_fileAsync(byte[] writevalue)
        {
            var length = writevalue.Length;
            var writer = new DataWriter();
            writer.WriteBytes(writevalue);
            var readvalue = await mOTA_File_upload.WriteValueAsync(writer.DetachBuffer());
            Console.WriteLine("Write Sucessfully");
        }
        public async Task Write_upload_file_cnfrmAsync(byte[] writevalue)
        {

            var writer = new DataWriter();
            writer.WriteBytes(writevalue);
            /* var valu = writer.DetachBuffer();*/
            var readvalue = await mOTA_File_upload_cnfrm.WriteValueAsync(writer.DetachBuffer());
            Console.WriteLine("Write Sucessfully");
        }

        private void RecieveDataAsync(GattCharacteristic sender, GattValueChangedEventArgs args)
        {
            /*var path = @"C:\Users\SARTHAK\Downloads\malik.csv";*/
            var reader = DataReader.FromBuffer(args.CharacteristicValue);

            reader.ReadBytes(mReceive);

            // Console.WriteLine(BitConverter.ToString(mReceive));
            //  File.AppendAllText(path, BitConverter.ToString(mReceive) + Environment.NewLine);


        }

        /* public async Task Read_From_DeviceAsync()
         {

             var readvalue = await Notifychar.ReadValueAsync();
             var reader = DataReader.FromBuffer(readvalue.Value);
             var input = new byte[38];
             reader.ReadBytes(input);
             Console.WriteLine(BitConverter.ToString(input));


         }*/
        public async void DisconnectDeviceAsync(string deviceId)
        {
            if (mBledevice != null)
            {

                foreach (var s in mGatt_2.Services)
                {
                    s.Dispose();

                }
                mBledevice.Dispose();
                mBledevice = null;
            }

        }

        public async void ConnectDeviceAsync(string deviceId)
        {
            // Note: BluetoothLEDevice.FromIdAsync must be called from a UI thread because it may prompt for consent.
            mBledevice = await BluetoothLEDevice.FromIdAsync(deviceId).AsTask();

            mGatt_2 = await mBledevice.GetGattServicesAsync().AsTask();
            if (mGatt_2.Status == GattCommunicationStatus.Success)
            {
                if (mBledevice.ConnectionStatus == BluetoothConnectionStatus.Connected)
                {
                    mWatcher.Stop();
                    mDevicepaired = true;
                    // var gatt_2 = await bluetoothLeDevice.GetGattServicesAsync().AsTask();

                    foreach (var service in mGatt_2.Services)
                    {
                        // This ID contains the GATT Profile Assigned number we want!
                        // TODO: Get more info and connect

                        var gattProfileId_2 = service.Uuid;
                        Console.WriteLine("Service UUID: " + gattProfileId_2);
                        mCharacteristics = await service.GetCharacteristicsAsync(BluetoothCacheMode.Cached);

                        foreach (var character in mCharacteristics.Characteristics)
                        {

                            mProperties = character.CharacteristicProperties;

                            Console.WriteLine("Char UUID: " +
                                           character.Uuid);
                            /*}*/
                            // these are other sorting flags that can be used so sort characterisics.
                            if (mProperties.HasFlag(GattCharacteristicProperties.WriteWithoutResponse))
                            {
                                if (character.Uuid == Guid.Parse("0000fe41-8e22-4541-9d4c-21edae82ed19"))
                                    mWritechar = character;
                                if (character.Uuid == Guid.Parse("0000fe11-8e22-4541-9d4c-21edae82ed19"))
                                    mOTA_Rebbot = character;
                                if (character.Uuid == Guid.Parse("0000fe22-8e22-4541-9d4c-21edae82ed19"))
                                    mOTA = character;
                                if (character.Uuid == Guid.Parse("0000fe24-8e22-4541-9d4c-21edae82ed19"))
                                    mOTA_File_upload = character;


                                //Console.Write("This characteristic supports write with out Response.");

                            }
                            if (mProperties.HasFlag(GattCharacteristicProperties.Indicate))
                            {
                                if (character.Uuid == Guid.Parse("0000fe23-8e22-4541-9d4c-21edae82ed19"))
                                    mOTA_File_upload_cnfrm = character;
                            }
                            if (mProperties.HasFlag(GattCharacteristicProperties.Notify))
                            {

                                if (character.Uuid == Guid.Parse("0000fe42-8e22-4541-9d4c-21edae82ed19"))
                                {
                                    var status = await character.WriteClientCharacteristicConfigurationDescriptorAsync(
                                      GattClientCharacteristicConfigurationDescriptorValue.Notify);
                                    mNotifychar = character;
                                }
                                //Console.Write("This characteristic supports subscribing to notifications.");

                            }
                            if (mProperties.HasFlag(GattCharacteristicProperties.Write))
                            {
                                if (character.Uuid == Guid.Parse("0000fe41-8e22-4541-9d4c-21edae82ed19"))
                                    mWritechar = character;
                                if (character.Uuid == Guid.Parse("0000fe11-8e22-4541-9d4c-21edae82ed19"))
                                    mOTA_Rebbot = character;
                                if (character.Uuid == Guid.Parse("0000fe22-8e22-4541-9d4c-21edae82ed19"))
                                    mOTA = character;

                            }

                        }

                    }

                }
            }
            else
            {
                mDeviceunpaired = true;
            }


        }
      
        /*        public async Task PairToDeviceAsync(string deviceId)
                {


                    // Get bluetooth device info
                    using var device = await BluetoothLEDevice.FromIdAsync(deviceId).AsTask();

                    // Null guard
                    if (device == null)
                        // TODO: Localize
                        throw new ArgumentNullException("Failed to get information about the Bluetooth device");

                    // If we are already paired...
                    if (device.DeviceInformation.Pairing.IsPaired)
                        // Do nothing
                        return;

                    // Listen out for pairing request
                    device.DeviceInformation.Pairing.Custom.PairingRequested += (sender, args) =>
                    {
                        // Log it
                        // TODO: Remove
                        Console.WriteLine("Accepting pairing request...");
                       *//* statusStrip1.Items.Add("Connecting...");*//*

                        // Accept all attempts
                        args.Accept(); // <-- Could enter a pin in here to accept
                    };

                    // Try and pair to the device

                    var result = await device.DeviceInformation.Pairing.Custom.PairAsync(
                        // For Contour we should try Provide Pin
                        // TODO: Try different types to see if any work

                        DevicePairingKinds.ConfirmPinMatch
                        ).AsTask();

                    // Log the result
                    if (result.Status == DevicePairingResultStatus.Paired)
                    {
                        mWatcher.Stop();
                        mDevicepaired = true;
                        Console.WriteLine("Pairing successful");

                        var gatt_2 = await device.GetGattServicesAsync().AsTask();


                        if (gatt_2.Status == GattCommunicationStatus.Success)
                        {
                            // Loop each GATT service

                            foreach (var service in gatt_2.Services)
                            {
                                // This ID contains the GATT Profile Assigned number we want!
                                // TODO: Get more info and connect

                                var gattProfileId_2 = service.Uuid;
                                Console.WriteLine("Service UUID: " + gattProfileId_2);
                                mCharacteristics = await service.GetCharacteristicsAsync(BluetoothCacheMode.Cached);

                                foreach (var character in mCharacteristics.Characteristics)
                                {

                                    mProperties = character.CharacteristicProperties;

                                    Console.WriteLine("Char UUID: " +
                                                   character.Uuid);
                                    *//*}*//*
                                    // these are other sorting flags that can be used so sort characterisics.
                                    if (mProperties.HasFlag(GattCharacteristicProperties.WriteWithoutResponse))
                                    {
                                       // if (character.Uuid == Guid.Parse("0000fe41-8e22-4541-9d4c-21edae82ed19"))
                                            mWritechar = character;
                                        //Console.Write("This characteristic supports write with out Response.");

                                    }

                                    if (mProperties.HasFlag(GattCharacteristicProperties.Notify))
                                    {
                                        var status = await character.WriteClientCharacteristicConfigurationDescriptorAsync(
                                          GattClientCharacteristicConfigurationDescriptorValue.Notify);
                                        //if (character.Uuid == Guid.Parse("0000fe42-8e22-4541-9d4c-21edae82ed19"))
                                            mNotifychar = character;
                                        //Console.Write("This characteristic supports subscribing to notifications.");

                                    }
                                    if (mProperties.HasFlag(GattCharacteristicProperties.Write))
                                    {
                                      //  if (character.Uuid == Guid.Parse("0000fe41-8e22-4541-9d4c-21edae82ed19"))
                                            mWritechar = character;

                                    }
                                }

                            }

                        }





                    }
                    else
                    {
                        Console.WriteLine($"Pairing failed: {result.Status}");
                        mDeviceunpaired = true;
                    }
                }*/

        #endregion
    }
}
