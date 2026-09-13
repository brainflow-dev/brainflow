.. _installation-label:

Installation Instructions
==========================

Precompiled libraries in package managers(Nuget, PYPI, etc)
-------------------------------------------------------------

Core part of BrainFlow is written in C/C++ and distributed as dynamic libraries, for some programming languages we publish packages with precompiled libraries to package managers like Nuget or PYPI.

C/C++ code should be compiled for each CPU architecture and for each OS and we cannot cover all possible cases, as of right now we support:

- x64 libraries for Windows starting from 8.1, for some devices newer version of Windows can be required
- x64 libraries for Linux, they are compiled inside manylinux docker container
- x64/ARM libraries for MacOS, they are universal binaries

If your CPU and OS is not listed above(e.g. Raspberry Pi or Windows with ARM)  you still can use BrainFlow, but you need to compile it by youself first. See :ref:`compilation-label` for details.

Python
-------

.. compound::

    Please, make sure to use Python 3+. Next, install the latest release from PYPI with the following command in terminal ::

        python -m pip install brainflow

.. compound::

    If you want to install it from source files or build unreleased version from Github, you should first compile the core module (:ref:`compilation-label`). Then run ::

        cd python_package
        python -m pip install -U .

C#
----

**Windows(Visual Studio)**

You are able to install the latest release from `Nuget <https://www.nuget.org/packages/brainflow/>`_ or build it yourself:

- Compile BrainFlow's core module
- Open Visual Studio Solution
- Build it using Visual Studio
- **Make sure that unmanaged(C++) libraries exist in search path** - set PATH env variable or copy them to correct folder

**Unix(Mono)**

- Compile BrainFlow's core module
- Install Mono and other dependcies on your system
- Build it using dotnet command
- **Make sure that unmanaged(C++) libraries exist in search path** - set LD_LIBRARY_PATH env variable or copy them to correct folder

.. compound::

    Example for Fedora: ::

        # compile c++ code
        python tools/build.py
        # install dependencies
        sudo dnf install nuget
        sudo dnf install mono-devel
        sudo dnf install mono-complete
        sudo dnf install monodevelop
        sudo dnf install dotnet-sdk-6.0
        sudo dnf install dotnet-runtime-6.0
        sudo dnf install dotnet-sdk-3.1
        sudo dnf install dotnet-runtime-3.1
        # build solution
        dotnet build csharp_package/brainflow/brainflow.sln
        # run tests
        export LD_LIBRARY_PATH=/home/andreyparfenov/brainflow/installed/lib/
        mono csharp_package/brainflow/examples/denoising/bin/Debug/denoising.exe

R
-----

R binding is based on `reticulate <https://rstudio.github.io/reticulate/>`_ package and calls Python 
, so you need to install Python binding first, make sure that reticulate uses correct virtual environment, after that you will be able to build R package from command line or using R Studio, install it and run samples.

Java
-----

You are able to download jar files directly from `release page <https://github.com/brainflow-dev/brainflow/releases>`_

.. compound::

    If you want to install it from source files or build unreleased version from github you should compile core module first (:ref:`compilation-label`) and run ::

        cd java_package
        cd brainflow
        mvn package

Also, you can use `GitHub Package <https://github.com/brainflow-dev/brainflow/packages/450100>`_ and download BrainFlow using Maven or Gradle.
To use Github packages you need to `change Maven settings <https://help.github.com/en/packages/using-github-packages-with-your-projects-ecosystem/configuring-apache-maven-for-use-with-github-packages>`_. `Example file <https://github.com/brainflow-dev/brainflow/blob/master/java_package/brainflow/settings.xml>`_  here you need to change OWNER and TOKEN by Github username and token with an access to Github Packages.

Matlab
--------

Steps to setup Matlab binding for BrainFlow:

- Compile Core Module, using the instructions in :ref:`compilation-label`. If you don't want to compile C++ code you can download Matlab package with precompiled libs from `Release page <https://github.com/brainflow-dev/brainflow/releases>`_
- Open Matlab IDE and open brainflow/matlab_package/brainflow folder there
- Add folders lib and inc to Matlab path
- If you want to run Matlab scripts from folders different than brainflow/matlab_package/brainflow you need to add it to your Matlab path too
- If you see errors you may need to configure Matlab to use C++ compiler instead C, install Visual Studio 2017 or newer(for Windows) and run this command in Matlab terminal :code:`mex -setup cpp`, you need to select Visual Studio Compiler from the list. More info can be found `here <https://www.mathworks.com/help/matlab/matlab_external/choose-c-or-c-compilers.html>`_.

Julia
--------

BrainFlow is a registered package in the Julia general registry, so it can be installed via the Pkg manager:

.. compound::

    Example: ::

        import Pkg
        Pkg.add("BrainFlow")
        
When using BrainFlow for the first time in Julia, the BrainFlow artifact containing the compiled BrainFlow libraries will be downloaded from release page automatically.

If you compile BrainFlow from source local libraries will take precedence over the artifact.

Typescript
-----------

.. compound::

    You can install BrainFlow using next command without compilation ::

        npm install brainflow

.. compound::

    If you want to install it from source files or build unreleased version from Github, you should first compile the core module (:ref:`compilation-label`). Then run ::

        cd nodejs_package
        npm install


Rust
-------

.. compound::

    You can build Rust binding locally using commands below, but you need to compile C/C++ code first ::

        cd rust_package
        cd brainflow
        cargo build --features generate_binding

Swift
-------

You can build Swift bindings for BrainFlow with Swift Package Manager or Xcode. Before running examples or tests you need to compile C/C++ code :ref:`compilation-label` and ensure that native libraries are available to the Swift runtime loader.

Local build example:

.. code-block:: bash

    python3 tools/build.py
    cd swift_package
    BRAINFLOW_LIB_DIR=../installed/lib swift build
    BRAINFLOW_LIB_DIR=../installed/lib swift test
    BRAINFLOW_LIB_DIR=../installed/lib swift run brainflow-swift-cli
    BRAINFLOW_LIB_DIR=../installed/lib swift run swift-brainflow-get-data

The Swift package intentionally does not vendor BrainFlow native binaries. Like source builds for other bindings, it dynamically loads native libraries built from this repository. The loader searches :code:`BRAINFLOW_LIB_DIR`, system library paths, :code:`installed/lib`, and app bundle resource/framework directories for :code:`libBoardController`, :code:`libDataHandler`, and :code:`libMLModule`.

For production iOS and macOS applications, use Apple XCFramework artifacts and the generated Swift binary package. Regenerate Apple artifacts with:

.. code-block:: bash

    tools/apple/regenerate_artifacts.sh
    tools/apple/verify_xcframeworks.sh build/apple_xcframeworks

The default generated artifact directory is :code:`build/apple_xcframeworks`. It contains :code:`XCFrameworks`, :code:`BrainFlowSwiftBinaryPackage`, :code:`BrainFlowSwiftPackageRemote`, :code:`SwiftPMArtifacts/*.xcframework.zip`, :code:`BrainFlowAppleXCFrameworks.zip`, and checksum files. These generated headers and binaries are release/CI artifacts, not source files committed to the repository.

The generated :code:`BrainFlowSwiftBinaryPackage` contains the Swift API and binary targets for :code:`BoardController.xcframework`, :code:`DataHandler.xcframework`, and :code:`MLModule.xcframework`. Add this package to an app through Xcode or Swift Package Manager so embedded frameworks are handled by standard Apple build, embed, and signing flows.

For public Swift Package distribution, publish the individual zips from :code:`SwiftPMArtifacts` and use :code:`BrainFlowSwiftPackageRemote`, which declares URL-based binary targets with checksums generated by :code:`swift package compute-checksum`. Set :code:`BRAINFLOW_APPLE_RELEASE_BASE_URL` before regeneration if release assets are hosted outside the default GitHub Release tag URL.

The macOS demo can be built with:

.. code-block:: bash

    cd swift_package
    BRAINFLOW_LIB_DIR=../installed/lib swift run BrainFlowMacDemo

iOS and Mac App Store sample source is available in :code:`swift_package/examples/apps`. See :ref:`apple-binary-distribution-label` and :ref:`app-store-readiness-label` for release-preparation notes. App runtime support requires matching BrainFlow native frameworks embedded and signed inside the app bundle; App Store builds should not depend on :code:`BRAINFLOW_LIB_DIR` or local development directories.

For dedicated iOS build, installation, and integration instructions, see :ref:`ios-label`.

Docker Image
--------------

There are docker images with precompiled BrainFlow. You can get them from `DockerHub <https://hub.docker.com/r/brainflow/brainflow>`_.

All bindings except Matlab are preinstalled there.

Also, there are other packages for BCI research and development:

- mne
- pyriemann
- scipy
- matplotlib
- jupyter
- pandas
- etc

If your devices uses TCP/IP to send data, you need to run docker container with :code:`--network host`. For serial port connection you need to pass serial port to docker using :code:`--device %your port here%`

.. compound::

    Example:  ::

        # pull container from DockerHub
        docker pull brainflow/brainflow:latest
        # run docker container with serial port /dev/ttyUSB0
        docker run -it --device /dev/ttyUSB0 brainflow/brainflow:latest /bin/bash
        # run docker container for boards which use networking
        docker run -it --network host brainflow/brainflow:latest /bin/bash

.. _compilation-label:

Compilation of Core Module and C++ Binding
-------------------------------------------

Windows
~~~~~~~~

- Install CMake>=3.16 you can install it from PYPI via pip or from `CMake website <https://cmake.org/>`_
- Install Visual Studio 2019(preferred) or Visual Studio 2017. Other versions may work but not tested
- In VS installer make sure you selected "Visual C++ ATL support"
- Build it as a standard CMake project, you don't need to set any options

.. compound::

    If you are not familiar with CMake you can use `build.py <https://github.com/brainflow-dev/brainflow/blob/master/tools/build.py>`_ : ::

        # install python3 and run
        python -m pip install cmake
        cd tools
        python build.py
        # to get info about args and configure your build you can run
        python build.py --help


Linux
~~~~~~

- Install CMake>=3.16 you can install it from PYPI via pip, via package managers for your OS(apt, dnf, etc) or from `CMake website <https://cmake.org/>`_
- If you are going to distribute compiled Linux libraries you HAVE to build it inside manylinux Docker container
- Build it as a standard CMake project, you don't need to set any options
- You can use any compiler but for Linux we test only GCC

.. compound::

    If you are not familiar with CMake you can use `build.py <https://github.com/brainflow-dev/brainflow/blob/master/tools/build.py>`_ : ::

        python3 -m pip install cmake
        cd tools
        python3 build.py
        # to get info about args and configure your build you can run
        python3 build.py --help

MacOS
~~~~~~~

- Install CMake>=3.16 you can install it from PYPI via pip, using :code:`brew` or from `CMake website <https://cmake.org/>`_
- Build it as a standard CMake project, you don't need to set any options
- You can use any compiler but for MacOS we test only Clang

.. compound::

    If you are not familiar with CMake you can use `build.py <https://github.com/brainflow-dev/brainflow/blob/master/tools/build.py>`_ : ::

        python3 -m pip install cmake
        cd tools
        python3 build.py
        # to get info about args and configure your build you can run
        python3 build.py --help


Android
---------

To check supported boards for Android visit :ref:`supported-boards-label`

Installation instructions
~~~~~~~~~~~~~~~~~~~~~~~~~~~

- Create an Android project in Android Studio
- Download *brainflow-android.aar* from `Release page <https://github.com/brainflow-dev/brainflow/releases>`_
- Copy *brainflow-android.aar* to *project/app/libs*
- Add it to your app dependencies ::

    dependencies {
        implementation files('libs/brainflow-android.aar')
    }

Now you can use BrainFlow SDK in your Android application!

Note: Android Studio inline compiler may show red errors but it should be compiled fine with Gradle. To fix inline compiler you can use *File > Sync Project with Gradle Files* or click at *File > Invalidate Cache/Restart > Invalidate and Restart*

Prebuilt libraries inside *brainflow-android.aar* are compiled using:

- Android NDK 25.1.8937393
- *-DANDROID_NATIVE_API_LEVEL=android-31*

Prebuilt Android libraries require API 31 or newer. BLE support is always enabled for Android builds and the SimpleBLE bridge is packaged into the AAR.

.. compound::
    
    The AAR declares Bluetooth permissions via its manifest, but Android 12+ still requires runtime approval for :code:`BLUETOOTH_SCAN` and :code:`BLUETOOTH_CONNECT` before creating a BrainFlow BLE board. For network or storage boards, or if you do not use manifest merging, add the required permissions to your application manifest manually ::

        <uses-permission android:name="android.permission.INTERNET"></uses-permission>
        <uses-permission android:name="android.permission.ACCESS_NETWORK_STATE"></uses-permission>
        <uses-permission android:name="android.permission.READ_EXTERNAL_STORAGE"></uses-permission>
        <uses-permission android:name="android.permission.WRITE_EXTERNAL_STORAGE"></uses-permission>
        <uses-permission android:name="android.permission.BLUETOOTH" android:maxSdkVersion="30"></uses-permission>
        <uses-permission android:name="android.permission.BLUETOOTH_ADMIN" android:maxSdkVersion="30"></uses-permission>
        <uses-permission android:name="android.permission.BLUETOOTH_SCAN" android:usesPermissionFlags="neverForLocation"></uses-permission>
        <uses-permission android:name="android.permission.BLUETOOTH_CONNECT"></uses-permission>


Compilation using Android NDK
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**For BrainFlow developers**


To test your changes in BrainFlow on Android you need to build it using Android NDK manually.

Compilation instructions:

- `Download Android NDK <https://developer.android.com/ndk/downloads>`_
- `Download Ninja <https://github.com/ninja-build/ninja/releases>`_ or get one from the *tools* folder, make sure that *ninja.exe*  is in search path
- You can also try *MinGW Makefiles* instead *Ninja*, but it's not tested and may not work
- Build C++ code using cmake and *Ninja* for **all ABIs**
- Compiled libraries will be in *tools/jniLibs* folder
- Build the AAR using *tools/build_android_aar.py*

.. compound::
    
    Command line examples: ::

        # to prepare project(choose ABIs which you need)
        # for arm64-v8a
        cmake -G Ninja -DCMAKE_TOOLCHAIN_FILE=D:\workspace\android-ndk-r25b\build\cmake\android.toolchain.cmake -DANDROID_NATIVE_API_LEVEL=android-31 -DANDROID_ABI=arm64-v8a ..
        # for armeabi-v7a
        cmake -G Ninja -DCMAKE_TOOLCHAIN_FILE=D:\workspace\android-ndk-r25b\build\cmake\android.toolchain.cmake -DANDROID_NATIVE_API_LEVEL=android-31 -DANDROID_ABI=armeabi-v7a ..
        # for x86_64
        cmake -G Ninja -DCMAKE_TOOLCHAIN_FILE=D:\workspace\android-ndk-r25b\build\cmake\android.toolchain.cmake -DANDROID_NATIVE_API_LEVEL=android-31 -DANDROID_ABI=x86_64 ..
        # for x86
        cmake -G Ninja -DCMAKE_TOOLCHAIN_FILE=D:\workspace\android-ndk-r25b\build\cmake\android.toolchain.cmake -DANDROID_NATIVE_API_LEVEL=android-31 -DANDROID_ABI=x86 ..

        # to build(should be run for each ABI from previous step**
        cmake --build . --target install --config Release -j 2 --parallel 2

        # build Java wrapper, SimpleBLE bridge jar, and package single Android artifact
        python tools/build_android_aar.py

        # or build native libraries and package a local AAR in one command
        python tools/build_android_aar.py --build-native --abis arm64-v8a --allow-missing-abis


.. _ios-label:

iOS
---

To check supported boards for iOS visit :ref:`supported-boards-label`.

BrainFlow provides first-class support for iOS applications. Native C/C++ core libraries (:code:`BoardController`, :code:`DataHandler`, and :code:`MLModule`) are distributed as multi-platform Apple XCFrameworks and can be integrated into iOS apps via Swift Package Manager (SwiftPM) or direct framework embedding in Xcode.

Installation instructions
~~~~~~~~~~~~~~~~~~~~~~~~~

BrainFlow supports iOS 15.0 or newer.

Using Swift Package Manager (Recommended)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Swift Package Manager is the recommended way to integrate BrainFlow into modern Xcode projects.

1. **Option A: Remote Swift Package (Precompiled Release)**

   In Xcode, open your iOS project and navigate to **File > Add Package Dependencies...** (or declare it in your app's :code:`Package.swift`).

   - Add the BrainFlow remote package repository URL (or release tag URL).
   - Select the **BrainFlow** package product.
   - SwiftPM will automatically download the binary targets (:code:`BoardController.xcframework`, :code:`DataHandler.xcframework`, :code:`MLModule.xcframework`) verified with SHA-256 checksums, along with the Swift API bindings.

2. **Option B: Local Swift Binary Package**

   If building from source or using a downloaded release archive:

   - Locate or generate :code:`build/apple_xcframeworks/BrainFlowSwiftBinaryPackage`.
   - In Xcode, drag the :code:`BrainFlowSwiftBinaryPackage` folder into your project navigator, or add it via **File > Add Package Dependencies... > Add Local...**.
   - Add the :code:`BrainFlow` library to your app target's **Frameworks, Libraries, and Embedded Content**.

Direct XCFramework Integration
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Alternatively, you can embed the prebuilt XCFrameworks directly:

- Drag :code:`BoardController.xcframework`, :code:`DataHandler.xcframework`, and :code:`MLModule.xcframework` (and any required optional vendor frameworks) into your Xcode target under **General > Frameworks, Libraries, and Embedded Content**.
- Ensure each framework is set to **Embed & Sign**.
- Add the Swift source bindings from :code:`swift_package/Sources/BrainFlow` to your project.

Permissions and Configuration
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Depending on the boards used by your application, configure your app's :code:`Info.plist` and privacy manifest:

- **Bluetooth (BLE Boards such as Muse or Ganglion):**
  iOS requires explicit user authorization before scanning for or connecting to Bluetooth peripherals. Add the following keys to your application's :code:`Info.plist`:

  .. code-block:: xml

      <key>NSBluetoothAlwaysUsageDescription</key>
      <string>BrainFlow uses Bluetooth to connect to supported biosensor devices.</string>
      <key>NSBluetoothPeripheralUsageDescription</key>
      <string>BrainFlow uses Bluetooth to connect to supported biosensor devices.</string>

- **Network Boards (WiFi streaming / UDP):**
  If communicating with boards over local networks, configure App Transport Security (ATS) keys or local network privacy descriptions as required by iOS.

- **Privacy Manifest:**
  Modern iOS App Store submissions require a :code:`PrivacyInfo.xcprivacy` file in your application bundle. A reference privacy manifest is provided in :code:`swift_package/examples/apps/ios/BrainFlowiOSDemo/PrivacyInfo.xcprivacy`.

- **Synthetic Board (Testing & Review):**
  The synthetic board (:code:`BoardIds.SYNTHETIC_BOARD`) requires no hardware, Bluetooth, or network permissions, making it ideal for unit testing, simulator development, and App Review smoke testing.

Quick Start Example (Swift)
^^^^^^^^^^^^^^^^^^^^^^^^^^^

Here is a minimal iOS Swift example initializing the synthetic board, streaming data, and applying a lowpass filter:

.. code-block:: swift

    import BrainFlow
    import Foundation

    func runBrainFlow() {
        do {
            let params = BrainFlowInputParams()
            let board = try BoardShim(board_id: .SYNTHETIC_BOARD, input_params: params)

            try board.prepare_session()
            defer {
                try? board.release_session()
            }

            try board.start_stream()
            Thread.sleep(forTimeInterval: 2.0)
            try board.stop_stream()

            let data = try board.get_board_data()
            let eegChannels = try BoardShim.get_eeg_channels(board_id: .SYNTHETIC_BOARD)
            let samplingRate = try BoardShim.get_sampling_rate(board_id: .SYNTHETIC_BOARD)

            if let firstEeg = eegChannels.first {
                var channelData = data[firstEeg]
                try DataFilter.perform_lowpass(
                    data: &channelData,
                    sampling_rate: samplingRate,
                    cutoff: 30.0,
                    order: 4,
                    filter_type: .BUTTERWORTH,
                    ripple: 0.0
                )
                print("Filtered \(channelData.count) samples")
            }
        } catch {
            print("BrainFlow error: \(error)")
        }
    }

Compilation from Source
~~~~~~~~~~~~~~~~~~~~~~~

**For BrainFlow Developers**

To compile the native Apple XCFrameworks and package the Swift bindings from source on macOS:

Prerequisites:

- macOS 13.0 or newer
- Xcode 15.0+ with Command Line Tools (:code:`xcode-select --install`)
- CMake >= 3.16 (:code:`brew install cmake` or :code:`python3 -m pip install cmake`)
- Python 3

Build Instructions:

1. **Build XCFrameworks:**

   Run the Apple build script from the repository root:

   .. code-block:: bash

       tools/apple/build_xcframeworks.sh

   Or use the convenience regeneration script:

   .. code-block:: bash

       tools/apple/regenerate_artifacts.sh

2. **Verify Output Artifacts:**

   Verify the generated artifact tree:

   .. code-block:: bash

       tools/apple/verify_xcframeworks.sh build/apple_xcframeworks

   The generated artifacts in :code:`build/apple_xcframeworks` include:

   - :code:`XCFrameworks/`: Multi-platform XCFrameworks containing slices for iOS device (:code:`ios-arm64`), iOS simulator (:code:`ios-arm64_x86_64-simulator`), and macOS universal (:code:`macos-arm64_x86_64`).
   - :code:`BrainFlowSwiftBinaryPackage`: Ready-to-use local Swift package referencing the generated XCFrameworks.
   - :code:`BrainFlowSwiftPackageRemote`: Template manifest declaring URL-based binary targets with SHA-256 checksums for release hosting.
   - :code:`SwiftPMArtifacts/*.xcframework.zip`: Individual zipped XCFrameworks formatted for Swift Package Manager releases.
   - :code:`BrainFlowAppleXCFrameworks.zip`: Aggregate zip archive of all frameworks.
   - :code:`swiftpm-checksums.txt` and :code:`swiftpm-checksums.json`: Computed checksums for SwiftPM validation.

3. **Optional Native Feature Flags:**

   By default, the script compiles a lightweight core suitable for standard App Store distribution. To enable Bluetooth/BLE or ONNX support:

   .. code-block:: bash

       BRAINFLOW_APPLE_BUILD_BLE=ON \
       BRAINFLOW_APPLE_BUILD_BLUETOOTH=ON \
       BRAINFLOW_APPLE_BUILD_ONNX=ON \
       tools/apple/build_xcframeworks.sh

iOS Demo Application
~~~~~~~~~~~~~~~~~~~~

A complete iOS demo application is provided in :code:`swift_package/examples/apps/ios/BrainFlowiOSDemo`.

- Open :code:`swift_package/examples/apps/ios/BrainFlowiOSDemo/BrainFlowiOSDemo.xcodeproj` in Xcode.
- Select an iPhone or iPad simulator target.
- Build and run the `BrainFlowiOSDemo` scheme.
- The demo includes:
  - Synthetic board streaming with real-time EEG plotting.
  - Board selector supporting BLE devices (such as Muse).
  - An automated smoke test mode: pass :code:`--autorun` as a launch argument to automatically connect, acquire data, process signals, and render results.

Additional Resources
~~~~~~~~~~~~~~~~~~~~

- :ref:`apple-binary-distribution-label` — Detailed information on XCFramework architecture, slice packaging, release maintenance, and checksum validation.
- :ref:`app-store-readiness-label` — App Store submission checklist, bundle identifier requirements, code signing, and privacy requirements.
- :ref:`swift-api-parity-label` — Full API coverage comparison between Swift and Python/Java bindings.

