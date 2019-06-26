using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

using Accord.Math;
using MathNet.Filtering;
using MathNet.Filtering.FIR;

namespace brainflow
{
    public class DataHandler
    {
        public int fs_hz;
        public int first_eeg_channel;
        public int last_eeg_channel;
        public int ts_col_num;
        public double[,] data;

        public DataHandler (int board_id, double[,] data_from_board = null, string csv_file = null)
        {
            if (board_id == (int) BoardIds.CYTON_BOARD)
            {
                fs_hz = Cyton.fs_hz;
                first_eeg_channel = Cyton.first_eeg_channel;
                last_eeg_channel = first_eeg_channel + Cyton.num_eeg_channels - 1;
                ts_col_num = Cyton.package_length; // data format - package from board plus ts column
            }
            else
            {
                if (board_id == (int)BoardIds.GANGLION_BOARD)
                {
                    fs_hz = Ganglion.fs_hz;
                    first_eeg_channel = Ganglion.first_eeg_channel;
                    last_eeg_channel = first_eeg_channel + Cyton.num_eeg_channels - 1;
                    ts_col_num = Ganglion.package_length;
                }
                else
                {
                    throw new BrainFlowExceptioin((int)CustomExitCodes.UNSUPPORTED_BOARD_ERROR);
                }
            }
            if (data_from_board != null)
            {
                data = data_from_board.Copy ();
            }
            else
            {
                if (csv_file != null)
                {
                    List<List<double>> temp_board_data = new List<List<double>> ();
                    StreamReader sr = new StreamReader (csv_file);
                    string strline = "";
                    string[] values = null;
                    while (!sr.EndOfStream)
                    {
                        List<double> temp = new List<double> ();
                        strline = sr.ReadLine ();
                        values = strline.Split (',');
                        for (int i = 0; i < values.Length; i++)
                        {
                            temp.Add (Convert.ToDouble (values[i], System.Globalization.CultureInfo.InvariantCulture));
                        }
                        temp_board_data.Add (temp);
                    }
                    sr.Close ();
                    data = temp_board_data.Select (a => a.ToArray ()).ToArray ().ToMatrix ();
                }
                else
                {
                    throw new MissingFieldException ("nor data_from_board nor csv_file was specified");
                }
            }
        }

        public void save_csv (string filename)
        {
            // accord's csvwriter and csvreader doesnt work and leads to formatexception
            using (StreamWriter writer = new StreamWriter (filename))
            {
                for (int i = 0; i < data.Rows (); i++)
                {
                    StringBuilder str_b = new StringBuilder ();
                    string line = "";
                    foreach (double val in data.GetRow (i))
                    {
                        str_b.Append (val.ToString (System.Globalization.CultureInfo.InvariantCulture)).Append (",");
                    }
                    line = str_b.ToString ();
                    line = line.Remove (line.Length - 1);
                    writer.WriteLine (line);
                    writer.Flush ();
                }
            }
        }

        public void remove_dc_offset ()
        {
            highpass (1.0);
        }

        public void lowpass (double value)
        {
            OnlineFilter lowpass = OnlineFirFilter.CreateLowpass (ImpulseResponse.Finite, fs_hz, value);
            for (int column_id = first_eeg_channel; column_id <= last_eeg_channel; column_id++)
            {
                double[] filtered = lowpass.ProcessSamples (data.GetColumn (column_id));
                data = data.SetColumn (column_id, filtered);
            }
        }

        public void highpass (double value)
        {
            OnlineFilter highpass = OnlineFirFilter.CreateHighpass (ImpulseResponse.Finite, fs_hz, value);
            for (int column_id = first_eeg_channel; column_id <= last_eeg_channel; column_id++)
            {
                double[] filtered = highpass.ProcessSamples (data.GetColumn (column_id));
                data = data.SetColumn (column_id, filtered);
            }
        }

        public void bandpass (double low_value, double high_value)
        {
            OnlineFilter bandpass = OnlineFirFilter.CreateBandpass (ImpulseResponse.Finite, fs_hz, low_value, high_value);
            for (int column_id = first_eeg_channel; column_id <= last_eeg_channel; column_id++)
            {
                double[] filtered = bandpass.ProcessSamples (data.GetColumn (column_id));
                data = data.SetColumn (column_id, filtered);
            }
        }

        public void notch_interference (double low_value, double high_value)
        {
            OnlineFilter bandstop = OnlineFirFilter.CreateBandstop (ImpulseResponse.Finite, fs_hz, low_value, high_value);
            for (int column_id = first_eeg_channel; column_id <= last_eeg_channel; column_id++)
            {
                double[] filtered = bandstop.ProcessSamples (data.GetColumn (column_id));
                data = data.SetColumn (column_id, filtered);
            }
        }

        // median
        public void denoise (int order)
        {
            OnlineFilter denoise = OnlineFirFilter.CreateDenoise (order);
            for (int column_id = first_eeg_channel; column_id <= last_eeg_channel; column_id++)
            {
                double[] filtered = denoise.ProcessSamples (data.GetColumn (column_id));
                data = data.SetColumn (column_id, filtered);
            }
        }
    }
}
