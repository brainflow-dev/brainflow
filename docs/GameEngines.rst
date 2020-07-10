Integration with Game Engines
==============================

Unity
-------

Integration with Unity can be done only using C# binding. And currently it works only for Windows.

You can build C# binding from source or download compiled package directly from `Nuget <https://www.nuget.org/packages/brainflow/>`_.

Here we will use Nuget to download and install BrainFlow with dependencies.

.. compound::

    Download `nuget.exe <https://www.nuget.org/downloads>`_ and run ::

        nuget.exe install brainflow -OutputDirectory <OUTPUTDIR>

.. image:: https://live.staticflickr.com/65535/50095400941_73e7915009_z.jpg
    :width: 640
    :height: 288px

Open OUTPUTDIR, you will see BrainFlow library and dependencies. For BrainFlow there are *Managed(C#)* and *Unmanaged(C++)* files. For dependencies like Accord there are only *Managed(C#)* libraries.

.. image:: https://live.staticflickr.com/65535/50095414856_fc6ca1b231_z.jpg
    :width: 640px
    :height: 304px


.. image:: https://live.staticflickr.com/65535/50095400991_d10cc177e5_z.jpg
    :width: 640px
    :height: 304px

Open your Unity project and copy **Managed(C#)** libraries from BrainFlow and **all dependencies** to the Assets folder, after that copy **Unamanaged(C++)** libraries from BrainFlow to the root folder of your project.

.. image:: https://live.staticflickr.com/65535/50095628822_14538fede0_c.jpg
    :width: 800px
    :height: 402px


Now, you are able to use BrainFlow API in your C# scripts!

For demo we will create a simple script and run *prepare_session* and *release_session* during start and end.

.. code-block:: csharp 

    using System.Collections;
    using System.Collections.Generic;
    using UnityEngine;

    using brainflow;

    public class BoardShimUnity : MonoBehaviour
    {
        public static BoardShim board_shim = null;

        [RuntimeInitializeOnLoadMethod(RuntimeInitializeLoadType.BeforeSceneLoad)]
        static void OnRuntimeMethodLoad()
        {
            if (board_shim == null)
            {
                try
                {
                    BrainFlowInputParams input_params = new BrainFlowInputParams();
                    int board_id = (int)BoardIds.SYNTHETIC_BOARD;
                    board_shim = new BoardShim(board_id, input_params);
                    board_shim.prepare_session();
                    board_shim.start_stream(450000, "file://brainflow_data.csv:w");
                    BoardShim.set_log_file("brainflow_log.txt");
                    BoardShim.enable_dev_board_logger();
                    Debug.Log("Brainflow streaming was started");
                }
                catch (BrainFlowException e)
                {
                    Debug.Log(e);
                    Application.Quit();
                }
            }
        }

        private void OnApplicationQuit()
        {
            if (board_shim != null)
            {
                try
                {
                    board_shim.release_session();
                }
                catch (BrainFlowException e)
                {
                    Debug.Log(e);
                }
                Debug.Log("Brainflow streaming was stopped");
            }
        }

    }

Now, if you start and stop scene in Unity Editor, in console you will see messages and also it will create file with data in the project root.

Let's add one more script which will read data. Create any game object and attach script below to it.

.. code-block:: csharp 

    using System;
    using System.Collections;
    using System.Collections.Generic;
    using UnityEngine;

    using Accord;
    using Accord.Math;
    using brainflow;

    public class Sphere : MonoBehaviour
    {
        private int board_id;
        private int sampling_rate;
        private int[] eeg_channels;

        // Start is called before the first frame update
        void Start()
        {
            board_id = BoardShimUnity.board_shim.board_id;
            sampling_rate = BoardShim.get_sampling_rate(board_id);
            eeg_channels = BoardShim.get_eeg_channels(board_id);
        }

        // Update is called once per frame
        void Update()
        {
            int number_of_data_points = DataFilter.get_nearest_power_of_two(sampling_rate);
            double[,] data = BoardShimUnity.board_shim.get_current_board_data(number_of_data_points);
            if (data.GetRow(0).Length < number_of_data_points)
            {
                // wait for more data
                return;
            }
            // calc bandpowers per channel
            for (int i = 0; i < eeg_channels.Length; i++)
            {
                Tuple<double[], double[]> psd = DataFilter.get_psd(data.GetRow(eeg_channels[i]), 0,
                    data.GetRow(eeg_channels[i]).Length, sampling_rate, (int)WindowFunctions.HANNING);
                double band_power_alpha = DataFilter.get_band_power(psd, 7.0, 13.0);
                double band_power_beta = DataFilter.get_band_power(psd, 14.0, 30.0);
                Debug.Log("Alpha/Beta Ratio:" + (band_power_alpha / band_power_beta));
            }
        }
    }

If everything is fine, you will see Alpa and Beta bandpower ratio per each channel in Console.