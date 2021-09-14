using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Windows.Devices.Enumeration;
using Windows.Devices.Bluetooth;
using Windows.Devices.Bluetooth.GenericAttributeProfile;
using System.Threading;

namespace BrainAlive_Lib
{
    public class BrainAlive
    {
        // "Magic" string for all BLE devices
        private string aqs_all_ble_devices = "(System.Devices.Aep.ProtocolId:=\"{bb7bb05e-5972-42b5-94fc-76eaa7084d49}\")";
        private string[] requested_ble_properties = { "System.Devices.Aep.DeviceAddress", "System.Devices.Aep.Bluetooth.Le.IsConnectable", };

        private volatile DeviceInformation brainalive_info = null;
        private BluetoothLEDevice brainalive_device = null;
        private GattDeviceService service = null;
        private volatile bool thread_should_stop = false;
        private GattCharacteristic send_characteristic = null;
        private GattCharacteristic receive_characteristic = null;
        private GattCharacteristic disconnect_characteristic = null;
        private TimeSpan timeout = TimeSpan.FromSeconds(6);
        private Queue<BoardData> data_queue = new Queue<BoardData>();

        public int open_brainalive()
        {
            try
            {
                var task_open = open_brainalive_async("");
                task_open.Wait();
                if (task_open.Result != (int)CustomExitCodes.STATUS_OK)
                {
                    return task_open.Result;
                }
                var task_pair = pair_brainalive_async();
                task_pair.Wait();
                return task_pair.Result;
            }
            catch (Exception ex)
            {
                return (int)CustomExitCodes.GENERAL_ERROR;
            }
        }

        public int open_brainalive(string mac_addr)
        {
            try
            {
                var task_open = open_brainalive_async(mac_addr);
                task_open.Wait();
                
                if (task_open.Result != (int)CustomExitCodes.STATUS_OK)
                {
                    return task_open.Result;
                }
                var task_pair = pair_brainalive_async();
                task_pair.Wait();
                // dont check exit code for pairing because pairing works pretty strange
                return (int)CustomExitCodes.STATUS_OK;
            }
            catch (Exception ex)
            {
                return (int)CustomExitCodes.GENERAL_ERROR;
            }
        }

        public BoardData get_data()
        {
            try
            {
                BoardData data = data_queue.Dequeue();
                return data;
            }
            catch (InvalidOperationException ie)
            {
                BoardData tmp;
                tmp.data = new byte[46];
                tmp.timestamp = 0;
                tmp.exit_code = (int)CustomExitCodes.NO_DATA_ERROR;
                return tmp;
            }
            catch (Exception e)
            {
                BoardData tmp;
                tmp.data = new byte[46];
                tmp.timestamp = 0;
                tmp.exit_code = (int)CustomExitCodes.GENERAL_ERROR;
                return tmp;
            }
        }

        public int close_brainalive()
        {
            int res = (int)CustomExitCodes.STATUS_OK;
            try
            {
                var unsub_task = disable_notifications();
                unsub_task.Wait();
                var task_unpair = unpair_brainalive_async();
                task_unpair.Wait();
            }
            catch (Exception ex)
            {
                res = (int)CustomExitCodes.GENERAL_ERROR;
            }
            stop_stream();
            config_board(" ", false); // use disc characteristic instead send characteristic
            if (brainalive_device == null)
            {
                return (int)CustomExitCodes.BRAINALIVE_IS_NOT_OPEN_ERROR;
            }
            // I am not sure in Dispose but Close method is unavailable
            else
            {
                brainalive_device.Dispose();
                brainalive_device = null;
            }

            if (service != null)
            {
                service.Dispose();
                service = null;
            }

            send_characteristic = null;
            receive_characteristic = null;
            disconnect_characteristic = null;
            return res;
        }

        public int config_board(string config, bool use_send_characteristic = true)
        {
            try
            {
                var task = send_command_async(config);
                task.Wait();
                return task.Result;
            }
            catch (Exception ex)
            {
                return (int)CustomExitCodes.GENERAL_ERROR;
            }
        }

        public int stop_stream()
        {
            return config_board("0x0a4000000d");
        }

        public int start_stream()
        {
            return config_board("0x0a8000000d");
        }

        ~BrainAlive()
        {
            close_brainalive();
        }

        private async Task<int> send_command_async(string command, bool use_send_characteristic = true)
        {
            if (brainalive_device == null)
            {
                return (int)CustomExitCodes.SEND_CHARACTERISTIC_NOT_FOUND_ERROR;
            }
            if (send_characteristic == null)
            {
                return (int)CustomExitCodes.SEND_CHARACTERISTIC_NOT_FOUND_ERROR;
            }

            var writer = new Windows.Storage.Streams.DataWriter();
            writer.WriteString(command);
            try
            {
                GattWriteResult result = null;
                if (use_send_characteristic)
                {
                    result = await send_characteristic.WriteValueWithResultAsync(writer.DetachBuffer()).AsTask().TimeoutAfter(timeout);
                }
                else
                {
                    result = await disconnect_characteristic.WriteValueWithResultAsync(writer.DetachBuffer()).AsTask().TimeoutAfter(timeout);
                }
                if (result.Status == GattCommunicationStatus.Success)
                {
                    return (int)CustomExitCodes.STATUS_OK;
                }
                else
                {
                    return (int)CustomExitCodes.STOP_ERROR;
                }
            }
            catch (TimeoutException e)
            {
                return (int)CustomExitCodes.TIMEOUT_ERROR;
            }
            catch (Exception e)
            {
                return (int)CustomExitCodes.GENERAL_ERROR;
            }
            return (int)CustomExitCodes.STATUS_OK;
        }

        private void watch_thread(string mac_addr)
        {
            var watcher = DeviceInformation.CreateWatcher(aqs_all_ble_devices, requested_ble_properties, DeviceInformationKind.AssociationEndpoint);
            watcher.Added += (DeviceWatcher sender, DeviceInformation dev_info) =>
            {
                if (brainalive_info == null)
                {
                    if (dev_info.Name.ToLower().Contains(BrainAliveConstants.name_prefix.ToLower()) || dev_info.Name.ToLower().Equals(BrainAliveConstants.alt_name.ToLower()))
                    {
                        if (string.IsNullOrEmpty(mac_addr))
                        {
                            brainalive_info = dev_info;
                        }
                        else
                        {
                            // mac address != Id it's just a substring
                            if (dev_info.Id.ToLower().Contains(mac_addr.ToLower()))
                            {
                                brainalive_info = dev_info;
                            }
                        }
                    }
                }
            };
            // maybe it works without Updated filed, just put empy function for now
            watcher.Updated += (_, __) => { };
            watcher.EnumerationCompleted += (DeviceWatcher sender, object arg) => { sender.Stop(); };
            watcher.Stopped += (DeviceWatcher sender, object arg) => { sender.Start(); };
            watcher.Start();
            while (!thread_should_stop)
            {
                if (brainalive_info != null)
                    break;
                Thread.Sleep(100);
            }
            watcher.Stop();
        }

        private async Task<int> open_brainalive_async(string mac_addr)
        {
            if (brainalive_device != null)
            {
                return (int)CustomExitCodes.BRAINALIVE_ALREADY_OPEN_ERROR;
            }

            // I guess watcher is executed in another thread by itself but not 100% sure how it works, so lets create the new one
            Thread thread = new Thread(() => watch_thread(mac_addr));
            thread.Start();
            // we can use condition variable or event to avoid waiting more than required but 2s is not too much
            Thread.Sleep(2000);
            thread_should_stop = true;
            thread.Join();
            Thread.Sleep(500);

            if (brainalive_info == null)
            {
                return (int)CustomExitCodes.BRAINALIVE_NOT_FOUND_ERROR;
            }

            try
            {
                brainalive_device = await BluetoothLEDevice.FromIdAsync(brainalive_info.Id).AsTask().TimeoutAfter(timeout);
                // find service
                var result = await brainalive_device.GetGattServicesAsync(BluetoothCacheMode.Uncached).AsTask().TimeoutAfter(timeout);
                if (result.Status == GattCommunicationStatus.Success)
                {
                    for (int i = 0; i < result.Services.Count; i++)
                    {
                        if (result.Services[i].Uuid.ToString().Equals(BrainAliveConstants.service_id))
                        {
                            service = result.Services[i];
                        }
                    }
                }
                // find characteristics
                var result_char = await service.GetCharacteristicsAsync(BluetoothCacheMode.Uncached).AsTask().TimeoutAfter(timeout);
                if (result_char.Status == GattCommunicationStatus.Success)
                {
                    for (int i = 0; i < result_char.Characteristics.Count; i++)
                    {
                        if (result_char.Characteristics[i].Uuid.ToString().Equals(BrainAliveConstants.characteristic_send_id))
                        {
                            send_characteristic = result_char.Characteristics[i];
                        }
                        if (result_char.Characteristics[i].Uuid.ToString().Equals(BrainAliveConstants.characteristic_receive_id))
                        {
                            receive_characteristic = result_char.Characteristics[i];
                            // set callback for new data
                            receive_characteristic.ValueChanged += callback_value_changed;
                            GattCommunicationStatus status = await receive_characteristic.WriteClientCharacteristicConfigurationDescriptorAsync(GattClientCharacteristicConfigurationDescriptorValue.Notify);
                            if (status != GattCommunicationStatus.Success)
                            {
                                return (int)CustomExitCodes.FAILED_TO_SET_CALLBACK_ERROR;
                            }
                            // double check that everything is okay with callback, maybe it works
                            var current_descriptor_value = await receive_characteristic.ReadClientCharacteristicConfigurationDescriptorAsync();
                            if (current_descriptor_value.Status != GattCommunicationStatus.Success
                                || current_descriptor_value.ClientCharacteristicConfigurationDescriptor != GattClientCharacteristicConfigurationDescriptorValue.Notify)
                            {
                                GattCommunicationStatus status2 = await receive_characteristic.WriteClientCharacteristicConfigurationDescriptorAsync(
                                GattClientCharacteristicConfigurationDescriptorValue.Notify);
                                if (status2 != GattCommunicationStatus.Success)
                                {
                                    return (int)CustomExitCodes.FAILED_TO_SET_CALLBACK_ERROR;
                                }
                            }

                        }
                        if (result_char.Characteristics[i].Uuid.ToString().Equals(BrainAliveConstants.characteristic_disconnect_id))
                        {
                            disconnect_characteristic = result_char.Characteristics[i];
                        }
                    }
                }
            }
            catch (TimeoutException e)
            {
                return (int)CustomExitCodes.TIMEOUT_ERROR;
            }
            catch (Exception e)
            {
                return (int)CustomExitCodes.GENERAL_ERROR;
            }
            if (service == null)
            {
                return (int)CustomExitCodes.SERVICE_NOT_FOUND_ERROR;
            }
            if (receive_characteristic == null)
            {
                return (int)CustomExitCodes.RECEIVE_CHARACTERISTIC_NOT_FOUND_ERROR;
            }
            if (send_characteristic == null)
            {
                return (int)CustomExitCodes.SEND_CHARACTERISTIC_NOT_FOUND_ERROR;
            }
            if (disconnect_characteristic == null)
            {
                return (int)CustomExitCodes.DISCONNECT_CHARACTERISTIC_NOT_FOUND_ERROR;
            }
            return (int)CustomExitCodes.STATUS_OK;
        }

        private async Task<int> pair_brainalive_async()
        {
            if (brainalive_info == null)
            {
                return (int)CustomExitCodes.BRAINALIVE_NOT_FOUND_ERROR;
            }
            if (brainalive_device == null)
            {
                return (int)CustomExitCodes.BRAINALIVE_IS_NOT_OPEN_ERROR;
            }

            DevicePairingResult result = null;
            DeviceInformationPairing pairing_information = brainalive_device.DeviceInformation.Pairing;

            try
            {
                if ((pairing_information.CanPair) && (!pairing_information.IsPaired))
                {
                    result = await brainalive_device.DeviceInformation.Pairing.PairAsync(pairing_information.ProtectionLevel).AsTask().TimeoutAfter(timeout);
                }
                else
                {
                    return (int)CustomExitCodes.BRAINALIVE_ALREADY_PAIR_ERROR;
                }
            }
            catch (TimeoutException e)
            {
                return (int)CustomExitCodes.TIMEOUT_ERROR;
            }
            catch (Exception e)
            {
                return (int)CustomExitCodes.GENERAL_ERROR;
            }
            return (int)CustomExitCodes.STATUS_OK;
        }

        private async Task<int> unpair_brainalive_async()
        {
            if (brainalive_info == null)
            {
                return (int)CustomExitCodes.BRAINALIVE_NOT_FOUND_ERROR;
            }
            if (brainalive_device == null)
            {
                return (int)CustomExitCodes.BRAINALIVE_IS_NOT_OPEN_ERROR;
            }

            DeviceUnpairingResult result = null;
            DeviceInformationPairing pairing_information = brainalive_device.DeviceInformation.Pairing;

            try
            {
                if (pairing_information.IsPaired)
                {
                    result = await brainalive_device.DeviceInformation.Pairing.UnpairAsync().AsTask().TimeoutAfter(timeout);
                }
                else
                {
                    return (int)CustomExitCodes.BRAINALIVE_IS_NOT_PAIRED_ERROR;
                }
            }
            catch (TimeoutException e)
            {
                return (int)CustomExitCodes.TIMEOUT_ERROR;
            }
            catch (Exception e)
            {
                return (int)CustomExitCodes.GENERAL_ERROR;
            }
            return (int)CustomExitCodes.STATUS_OK;
        }

        private async Task<int> disable_notifications()
        {
            if (receive_characteristic == null)
            {
                return (int)CustomExitCodes.RECEIVE_CHARACTERISTIC_NOT_FOUND_ERROR;
            }
            receive_characteristic.ValueChanged -= callback_value_changed;
            GattCommunicationStatus status = await receive_characteristic.WriteClientCharacteristicConfigurationDescriptorAsync(GattClientCharacteristicConfigurationDescriptorValue.None);
            if (status != GattCommunicationStatus.Success)
            {
                return (int)CustomExitCodes.FAILED_TO_UNSUBSCRIBE_ERROR;
            }
            return (int)CustomExitCodes.STATUS_OK;
        }

        private void callback_value_changed(GattCharacteristic sender, GattValueChangedEventArgs args)
        {
            byte[] data = new byte[args.CharacteristicValue.Length];
            Windows.Storage.Streams.DataReader.FromBuffer(args.CharacteristicValue).ReadBytes(data);
            long curTime = DateTimeOffset.UtcNow.ToUnixTimeSeconds();
            if (data.Length == 20)
            {
                BoardData board_data;
                board_data.data = data;
                board_data.timestamp = curTime;
                board_data.exit_code = (int)CustomExitCodes.STATUS_OK;
                data_queue.Enqueue(board_data);
            }
        }
    }
}
