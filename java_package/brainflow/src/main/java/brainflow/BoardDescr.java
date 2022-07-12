package brainflow;

import java.util.List;

public class BoardDescr
{
    public Integer sampling_rate;
    public List<Integer> eeg_channels;
    public List<Integer> eog_channels;
    public List<Integer> exg_channels;
    public List<Integer> emg_channels;
    public List<Integer> ppg_channels;
    public List<Integer> eda_channels;
    public List<Integer> accel_channels;
    public List<Integer> gyro_channels;
    public List<Integer> temperature_channels;
    public List<Integer> resistance_channels;
    public List<Integer> other_channels;
    public Integer package_num_channel;
    public Integer batter_channel;
    public Integer timestamp_channel;
    public Integer marker_channel;
    public Integer num_rows;
    public String name;
    public String eeg_names;

    public BoardDescr ()
    {
        sampling_rate = null;
        eeg_channels = null;
        eog_channels = null;
        exg_channels = null;
        emg_channels = null;
        ppg_channels = null;
        eda_channels = null;
        accel_channels = null;
        gyro_channels = null;
        temperature_channels = null;
        resistance_channels = null;
        other_channels = null;
        package_num_channel = null;
        timestamp_channel = null;
        num_rows = null;
        name = null;
        eeg_names = null;
    }
}
