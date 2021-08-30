using System;

namespace BLE_Lib
{
    /// <summary>
    /// Information about a BLE device
    /// </summary>
    public class DnaBluetoothLEDevice
    {
        #region Public Properties

        /// <summary>
        /// The time of the broadcast advertisement message of the device
        /// </summary>
        public DateTimeOffset BroadcastTime { get; }

        /// <summary>
        /// The address of the device
        /// </summary>
        public ulong Address { get; }

        /// <summary>
        /// The name of the device
        /// </summary>
        public string Name { get; }

        /// <summary>
        /// The signal strength in dB
        /// </summary>
        public short SignalStrengthInDB { get; }

        /// <summary>
        /// Indicates if we are connected to this device
        /// </summary>
        public bool Connected { get; }

        /// <summary>
        /// Indicates if this device supports pairing
        /// </summary>
        public bool CanPair { get; }

        /// <summary>
        /// Indicates if we are currently paired to this device
        /// </summary>
        public bool Paired { get; }

        /// <summary>
        /// The permanent unique Id of this device
        /// </summary>
        public string DeviceId { get; }

        #endregion

        #region Constructor

        /// <summary>
        /// Default constructor
        /// </summary>
        /// <param name="address">The device address</param>
        /// <param name="name">The device name</param>
        /// <param name="rssi">The signal strength</param>
        /// <param name="broadcastTime">The broadcast time of the discovery</param>
        /// <param name="connected">If connected to the device</param>
        /// <param name="canPair">If we can pair to the device</param>
        /// <param name="paired">If we are paired to the device</param>
        /// <param name="deviceId">The unique ID of the device</param>
        public DnaBluetoothLEDevice(
            ulong address,
            string name,
            short rssi,
            DateTimeOffset broadcastTime,
            string deviceId
            )
        {
            Address = address;
            Name = name;
            SignalStrengthInDB = rssi;
            BroadcastTime = broadcastTime;
            DeviceId = deviceId;
        }

        #endregion

        /// <summary>
        /// User friendly ToString
        /// </summary>
        /// <returns></returns>
        public override string ToString()
        {
            return $"({SignalStrengthInDB + " dbm"}) { (string.IsNullOrEmpty(Name) ? "[No Name]" : Name)}";
        }
    }
}
