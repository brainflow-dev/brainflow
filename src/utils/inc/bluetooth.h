#pragma once

enum BluetoothExitCodes : int
{
};

class BluetoothLE
{

public:
    class Adapter
    {
    public:
        Adapter (const char *name);
        ~Adapter ()
        {
            close ();
        }

        int open ();
        bool is_open ();

        int scan_start (std::function<void (std::string, std::string)> on_new_device);
        int scan_stop ();

        int close ();

    private:
    };

    BluetoothLE::BluetoothLE (const char *adapter_name);
    ~BluetoothLE ()
    {
        close_bt_adapter ();
    }

    int start_scanning ();
    int stop_scanning ();
};
