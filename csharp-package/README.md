# C#
* [C# binding for brainflow library(for now Windows only)](./brainflow/brainflow)
* [Test sample](./brainflow/test)

You will need to install Nuget packages to build it yourself and compile it using Microsoft Visual Studio

## Example:
```
using System;
using brainflow;

namespace test
{
    class GetBoardData
    {
        static void Main (string[] args)
        {
            BoardShim board_shim = new BoardShim ((int)Cyton.board_id, "COM3");

            board_shim.prepare_session ();
            Console.WriteLine ("Session is ready");

            board_shim.start_stream (3600);
            Console.WriteLine ("Started");
            System.Threading.Thread.Sleep (5000);

            board_shim.stop_stream ();
            Console.WriteLine ("Stopped");

            Console.WriteLine ("data count: {0}", board_shim.get_board_data_count ());
            double[,] unprocessed_data = board_shim.get_board_data ();

            // check serialization
            DataHandler dh = new DataHandler ((int) BoardIds.CYTON_BOARD, data_from_board: unprocessed_data);
            dh.save_csv ("before_processing.csv");
            dh = new DataHandler ((int) BoardIds.CYTON_BOARD, csv_file: "before_processing.csv");
            dh.save_csv ("before_preprocessing2.csv");
            // check preprocessing
            dh.remove_dc_offset ();
            dh.bandpass (1.0, 50.0);
            dh.save_csv ("after_preprocessing.csv");

            board_shim.release_session ();
            Console.WriteLine ("Released");
            Console.ReadKey ();
        }
    }
}
```
