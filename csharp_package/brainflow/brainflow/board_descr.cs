using System.Runtime.Serialization;

namespace brainflow
{
    [DataContract]
    public class BoardDescr
    {
        [DataMember]
        public int sampling_rate;
        [DataMember]
        public int[] eeg_channels;
        [DataMember]
        public int[] eog_channels;
        [DataMember]
        public int[] exg_channels;
        [DataMember]
        public int[] emg_channels;
        [DataMember]
        public int[] ppg_channels;
        [DataMember]
        public int[] eda_channels;
        [DataMember]
        public int[] accel_channels;
        [DataMember]
        public int[] gyro_channels;
        [DataMember]
        public int[] temperature_channels;
        [DataMember]
        public int[] resistance_channels;
        [DataMember]
        public int[] other_channels;
        [DataMember]
        public int package_num_channel;
        [DataMember]
        public int batter_channel;
        [DataMember]
        public int timestamp_channel;
        [DataMember]
        public int marker_channel;
        [DataMember]
        public int num_rows;
        [DataMember]
        public string name;
        [DataMember]
        public string eeg_names;

        public BoardDescr ()
        {
            sampling_rate = 0;
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
            package_num_channel = -1;
            timestamp_channel = -1;
            num_rows = -1;
            name = "";
            eeg_names = null;
        }
    }
}
