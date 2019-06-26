using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Accord.Math;

namespace brainflow
{
    internal class BoardData
    {
        private double[,] board_data;
        private int package_length;
        private int board_id;

        internal BoardData (int board_id, float[] raw_data, double[] ts_data)
        {
            this.board_id = board_id;
            if (board_id == (int) BoardIds.CYTON_BOARD)
            {
                this.package_length = Cyton.package_length;
            }
            else
            {
                if (board_id == (int) BoardIds.GANGLION_BOARD)
                {
                    this.package_length = Ganglion.package_length;
                }
                else
                {
                    throw new BrainFlowExceptioin((int)CustomExitCodes.UNSUPPORTED_BOARD_ERROR);
                }
            }
            List<List<double>> temp_board_data = new List<List<double>> ();
            for (int i = 0; i < ts_data.Length; i++)
            {
                List<double> temp = new List<double> ();
                for (int j = 0; j < this.package_length; j++)
                {
                    temp.Add ((double) raw_data[i * package_length + j]);
                }
                temp.Add (ts_data[i]);
                temp_board_data.Add (temp);
            }
            board_data = temp_board_data.Select (a => a.ToArray ()).ToArray ().ToMatrix ();
        }

        internal double[,] get_board_data ()
        {
            return board_data;
        }
    }
}
