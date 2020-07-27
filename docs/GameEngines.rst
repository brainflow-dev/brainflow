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

Open your Unity project and copy **Managed(C#)** libraries from BrainFlow and **all dependencies** to the Assets folder, after that copy **Unmanaged(C++)** libraries from BrainFlow to the root folder of your project.

.. image:: https://live.staticflickr.com/65535/50095628822_14538fede0_b.jpg
    :width: 1024px
    :height: 487px


Now, you are able to use BrainFlow API in your C# scripts!

For demo we will create a simple script to read data and calculate band powers.

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
        private BoardShim board_shim = null;
        private int sampling_rate = 0;
        private int[] eeg_channels = null;

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
                eeg_channels = BoardShim.get_eeg_channels(board_id);
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
            int number_of_data_points = DataFilter.get_nearest_power_of_two(sampling_rate);
            double[,] data = board_shim.get_current_board_data(number_of_data_points);
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

If everything is fine, you will see Alpa and Beta bandpower ratio per each channel in Console.

.. image:: https://live.staticflickr.com/65535/50102505902_f110fc89d8_b.jpg
    :width: 1024px
    :height: 595px

After building your game you need to copy *Unmanaged(C++)* libraries to a folder where executable is located.


Unreal Engine
--------------

First of all you need to compile BrainFlow from source. For Windows you need to specify an option to link MSVC Runtime *dynamically*. And you need to use the same version of Visual Studio as in your Unreal Project.

.. compound::
    
    Command line example for Windows and MSVC 2017: ::

        # install cmake, clone repo and run commands below
        cd brainflow
        mkdir build_dyn
        cd build_dyn
        cmake -G "Visual Studio 15 2017 Win64" -DMSVC_RUNTIME=dynamic -DCMAKE_SYSTEM_VERSION=10.0 -DCMAKE_INSTALL_PREFIX=FULL_PATH_TO_FOLDER_FOR_INSTALLATION ..
        # e.g. cmake -G "Visual Studio 15 2017 Win64" -DMSVC_RUNTIME=dynamic -DCMAKE_SYSTEM_VERSION=10.0 -DCMAKE_INSTALL_PREFIX=E:\folder\brainflow\installed_temp ..
        cmake --build . --target install --config Release -j 2 --parallel 2

Add new entry to your *PATH* environemnt variable to point to a folder *FULL_PATH_TO_FOLDER_FOR_INSTALLATION\\lib* in example above it's *E:\\folder\\brainflow\\installed_temp\\lib*. If you have Unreal Engine Editor or Visual Studio running at this point you need to restart these processes.

Open your Visual Studio Solution for your Unreal Engine project, here we created a project called *BrainFlowUnreal*.

Edit file named *ProjectName.Build.cs*, in our example this file is called *BrainFlowUnreal.Build.cs*


.. code-block:: csharp 

    using UnrealBuildTool;
    using System.IO;

    public class BrainFlowUnreal : ModuleRules
    {
        public BrainFlowUnreal(ReadOnlyTargetRules Target) : base(Target)
        {
            PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

            PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay" });

            // here I show it only for Windows, if you wanna target more OSes add similar code for them
            if (Target.Platform == UnrealTargetPlatform.Win64)
            {
                // Add the import library
                PublicLibraryPaths.Add(Path.Combine(ModuleDirectory, "x64"));
                PublicAdditionalLibraries.Add("BrainFlow.lib");
                PublicAdditionalLibraries.Add("DataHandler.lib");
                PublicAdditionalLibraries.Add("BoardController.lib");

                // add headers for static library
                PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "include"));
            }
        }
    }

After that you need to copy headers and libraries from BrainFlow installation folder to your Unreal Engine project. Here we copied a content of *E:\\folder\\brainflow\\installed_temp\\inc* to a folder *E:\\gamedev\\BrainFlowUnreal\\Source\\BrainFlowUnreal\\include*. Also, you need to copy compiled libraries from *E:\\folder\\brainflow\\installed_temp\\lib* to *E:\\gamedev\\BrainFlowUnreal\\Source\\BrainFlowUnreal\\x64*

.. image:: https://live.staticflickr.com/65535/50156604283_0ee27ace03_b.jpg
    :width: 1024px
    :height: 517px

*Note: in this example we didn't create a new plugin as described* `here <https://docs.unrealengine.com/en-US/Programming/BuildTools/UnrealBuildTool/ThirdPartyLibraries/index.html>`_. *Also we linked only static libraries and didn't link or load dynamic libraries manually. And we don't recommend to configure it as a plugin.*

Finally, you are able to use BrainFlow in your Unreal Engine project.

When you will build a project for production put C++ libraries for BrainFlow in the folder with executable.
