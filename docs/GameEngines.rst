Integration with Game Engines
==============================

Unity
-------

Integration with Unity can be done only using C# binding. We tested it only on Windows, but it may work on Linux and MacOS too. Android and IOS are not supported.

You can build C# binding from source or download compiled package directly from `Nuget <https://www.nuget.org/packages/brainflow/>`_.

Here we will use Nuget to download and install BrainFlow.

.. compound::

    Download `nuget.exe <https://www.nuget.org/downloads>`_ and run ::

        nuget.exe install brainflow -OutputDirectory <OUTPUTDIR>

.. image:: https://live.staticflickr.com/65535/51025831037_c88425a0bd_b.jpg
    :width: 1024px
    :height: 576px

Open OUTPUTDIR, in our example it is *D:\\BrainFlowNuget*. At the moment of writing this tutorial latest BrainFlow version is 4.0.1, it is ok if you download newer version from Nuget, it does not affect the process of integration with Unity.

For BrainFlow there are *Managed(C#)* and *Unmanaged(C++)* libraries.
C++ libraries are located inside folder *D:\\BrainFlowNuget\\brainflow.4.0.1\\lib*, C# libraries are located inside folder *D:\\BrainFlowNuget\\brainflow.4.0.1\\lib\\net45*.

.. image:: https://live.staticflickr.com/65535/51025831062_a90515e1e5_b.jpg
    :width: 1024px
    :height: 576px


Open your Unity project and copy **Managed(C#)** libraries to the Assets folder, after that copy **Unmanaged(C++)** libraries to the root folder of your project.

.. image:: https://live.staticflickr.com/65535/51025001523_63cb77ed83_b.jpg
    :width: 1024px
    :height: 576px

Now, you are able to use BrainFlow API in your Unity project.

For demo we will create a simple script to read data.

Add a game object to the Scene and attach script below.

.. code-block:: csharp 

    using System;
    using System.Collections;
    using System.Collections.Generic;
    using UnityEngine;

    using brainflow;
    using brainflow.math;

    public class SimpleGetData : MonoBehaviour
    {
        private BoardShim board_shim = null;
        private int sampling_rate = 0;

        // Start is called before the first frame update
        void Start()
        {
            try
            {
                BoardShim.set_log_file("brainflow_log.txt");
                BoardShim.enable_dev_board_logger();

                BrainFlowInputParams input_params = new BrainFlowInputParams();
                int board_id = (int)BoardIds.SYNTHETIC_BOARD;
                board_shim = new BoardShim(board_id, input_params);
                board_shim.prepare_session();
                board_shim.start_stream(450000, "file://brainflow_data.csv:w");
                sampling_rate = BoardShim.get_sampling_rate(board_id);
                Debug.Log("Brainflow streaming was started");
            }
            catch (BrainFlowException e)
            {
                Debug.Log(e);
            }
        }

        // Update is called once per frame
        void Update()
        {
            if (board_shim == null)
            {
                return;
            }
            int number_of_data_points = sampling_rate * 4;
            double[,] data = board_shim.get_current_board_data(number_of_data_points);
            // check https://brainflow.readthedocs.io/en/stable/index.html for api ref and more code samples
            Debug.Log("Num elements: " + data.GetLength(1));
        }

        // you need to call release_session and ensure that all resources correctly released
        private void OnDestroy()
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


After building your game for production don't forget to copy *Unmanaged(C++)* libraries to a folder where executable is located.


Unreal Engine
--------------

We provide `Unreal Engine Plugin <https://github.com/brainflow-dev/BrainFlowUnrealPlugin>`_ with instructions how to compile and use it. Check Readme for installtion details.

This `blog post <https://unreal.blog/how-to-include-any-third-party-library>`_ can help if you want to write your own plugin or extend existing one.

CryEngine
----------

CryEngine uses CMake, build BrainFlow by yourself first and check C++ examples for instructions to integrate BrainFlow into CMake projects.

Keep in mind MSVC runtime linking, default in BrainFlow is static, you can provide :code:`-DMSVC_RUNTIME=dynamic` or :code:`-DMSVC_RUNTIME=static` to control it.
