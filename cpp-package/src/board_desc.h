#ifndef BOARD_DESC
#define BOARD_DESC

class BoardDesc
{
public:
    virtual int get_num_data_channels () = 0;
    virtual int get_num_eeg_channels () = 0;
    virtual int get_sampling_rate () = 0;
    // for now first elem is a package num maybe it will be changed for new boards
    virtual int get_start_eeg_channel ()
    {
        return 1;
    }
    virtual int get_total_count ()
    {
        // package num + num_eeg + num_accel + ts
        return 1 + get_num_eeg_channels () + 3 + 1;
    }
};


class GanglionDesc : public BoardDesc
{
public:
    int get_num_data_channels ()
    {
        return 8;
    }
    int get_num_eeg_channels ()
    {
        return 4;
    }
    int get_sampling_rate ()
    {
        return 200;
    }
};

class CytonDesc : public BoardDesc
{
public:
    int get_num_data_channels ()
    {
        return 12;
    }
    int get_num_eeg_channels ()
    {
        return 8;
    }
    int get_sampling_rate ()
    {
        return 250;
    }
};

#endif
