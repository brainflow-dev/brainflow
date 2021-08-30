using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;


namespace BLE_Lib
{
   public class Brainalive
    {
        DnaBluetoothLEAdvertisementWatcher mWatcher = new DnaBluetoothLEAdvertisementWatcher(new GattServiceIds());
        DnaBluetoothLEDevice mContourDevice;
        private string mOld_device = "";
        private bool mBLE_Connected;

        unsafe public int Scan_BLE(char* device_name)
       {
            try
            {
                mWatcher.NewDeviceDiscovered += (device) =>
                {
               
                    if (mOld_device != device.ToString().Substring(6))
                    {
                        mOld_device = device.ToString().Substring(6);

                    }
              
                };
                 mWatcher.StartListening();
                return (int)CustomExitCodes.STATUS_OK;
            }
            catch (Exception ex)
            {
                return (int)CustomExitCodes.GENERAL_ERROR;
            }
            
       }

        public int Connect_device(char device_name) // connect Button
        {
            mContourDevice = mWatcher.DiscoveredDevices.FirstOrDefault(
                    f => f.Name.Contains(device_name));
            mWatcher.StopListening();
            try
            {
                if (mContourDevice != null)
                {


                    mWatcher.ConnectDeviceAsync(mContourDevice.DeviceId);

                    if (mWatcher.mDevicepaired == false)
                    {
                        mWatcher.ConnectDeviceAsync(mContourDevice.DeviceId);
                        //Wait(3000);

                    }

                    if (mWatcher.mDevicepaired == true)
                    {

                        mWatcher.mDevicepaired = false;
                        mBLE_Connected = true;
                        // listBox1.Items.Clear();
                        //listBox1.Items.Add("Connected with " + device_name);
                        //mWatcher.mBledevice.ConnectionStatusChanged += ConnectionStatusChangeHandler;
                    }
                    else if (mWatcher.mDevicepaired == false)
                    {
                        mWatcher.StopListening();
                        mWatcher.mDeviceunpaired = false;
                    }
                }
                return (int)CustomExitCodes.STATUS_OK;
            }
            catch (Exception ex)
            {
                return (int)CustomExitCodes.GENERAL_ERROR;
            }
        }

      
   }
}
