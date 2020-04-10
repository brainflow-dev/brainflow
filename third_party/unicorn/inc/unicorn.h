#ifndef UNICORN_H
#define UNICORN_H

#include <stdbool.h>
#include <stdint.h>

// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the UNICORN_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// UNICORN_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#if defined _WIN32 || defined __CYGWIN__
#if defined _MSC_VER // Windows && MS Visual C
#ifdef UNICORN_EXPORTS
#define UNICORN_API __declspec(dllexport)
#else
#define UNICORN_API __declspec(dllimport)
#endif
#elif defined __GNUC__ // Windows && GNU C compiler
#ifdef UNICORN_EXPORTS
#define UNICORN_API __attribute__ ((dllexport))
#else
#define UNICORN_API __attribute__ ((dllexport))
#endif
#endif
#elif defined __linux__ || __APPLE__
#if __GNUC__ >= 4 // Linux && GNU C compiler version 4.0 and higher
#ifdef UNICORN_EXPORTS
#define UNICORN_API __attribute__ ((visibility ("default")))
#else
#define UNICORN_API __attribute__ ((visibility ("default")))
#endif
#else
#define UNICORN_API
#endif
#endif

#ifdef __cplusplus
extern "C"
{
#endif

// ========================================================================================
// Type definitions
// ========================================================================================
#ifndef NULL
    //! The null pointer.
#define NULL 0
#endif

#ifndef BOOL
    //! The boolean data type, whose values can be \ref TRUE or \ref FALSE.
    typedef int32_t BOOL;
#endif

#ifndef FALSE
    //! The \ref FALSE value for the \ref BOOL type.
#define FALSE 0
#endif

#ifndef TRUE
    //! The \ref TRUE value for the \ref BOOL type.
#define TRUE 1
#endif

// ========================================================================================
// Error Codes
// ========================================================================================

//! The operation completed successfully. No error occurred.
#define UNICORN_ERROR_SUCCESS 0

//! One of the specified parameters does not contain a valid value.
#define UNICORN_ERROR_INVALID_PARAMETER 1

//! The initialization of the Bluetooth adapter failed.
#define UNICORN_ERROR_BLUETOOTH_INIT_FAILED 2

//! The operation could not be performed because the Bluetooth socket failed.
#define UNICORN_ERROR_BLUETOOTH_SOCKET_FAILED 3

//! The device could not be opened.
#define UNICORN_ERROR_OPEN_DEVICE_FAILED 4

//! The configuration is invalid.
#define UNICORN_ERROR_INVALID_CONFIGURATION 5

//! The acquisition buffer is full.
#define UNICORN_ERROR_BUFFER_OVERFLOW 6

//! The acquisition buffer is empty.
#define UNICORN_ERROR_BUFFER_UNDERFLOW 7

//! The operation is not allowed.
#define UNICORN_ERROR_OPERATION_NOT_ALLOWED 8

//! The operation could not complete because of connection problems.
#define UNICORN_ERROR_CONNECTION_PROBLEM 9

//! The device is not supported with this API (\ref UNICORN_SUPPORTED_DEVICE_VERSION)
#define UNICORN_ERROR_UNSUPPORTED_DEVICE 10

//! The specified connection handle is invalid.
#define UNICORN_ERROR_INVALID_HANDLE 0xFFFFFFFE

//! An unspecified error occurred.
#define UNICORN_ERROR_GENERAL_ERROR 0xFFFFFFFF

// ========================================================================================
// Amplifier Properties
// ========================================================================================

//! The maximum length of the serial number.
#define UNICORN_SERIAL_LENGTH_MAX 14

//! The maximum length of the device version.
#define UNICORN_DEVICE_VERSION_LENGTH_MAX 6

//! The maximum length of the firmware version.
#define UNICORN_FIRMWARE_VERSION_LENGTH_MAX 12

//! The sampling rate of the amplifier.
#define UNICORN_SAMPLING_RATE 250

//! The maximum string length.
#define UNICORN_STRING_LENGTH_MAX 255

//! The number of available EEG channels.
#define UNICORN_EEG_CHANNELS_COUNT 8

//! The number of available accelerometer channels.
#define UNICORN_ACCELEROMETER_CHANNELS_COUNT 3

//! The number of available gyroscope channels.
#define UNICORN_GYROSCOPE_CHANNELS_COUNT 3

//! The total number of available channels.
#define UNICORN_TOTAL_CHANNELS_COUNT 17

//! Index of the first EEG \ref UNICORN_AMPLIFIER_CHANNEL in the \ref
//! UNICORN_AMPLIFIER_CONFIGURATION Channels array.
#define UNICORN_EEG_CONFIG_INDEX 0

//! Index of the first Accelerometer \ref UNICORN_AMPLIFIER_CHANNEL in the \ref
//! UNICORN_AMPLIFIER_CONFIGURATION Channels array.
#define UNICORN_ACCELEROMETER_CONFIG_INDEX 8

//! Index of the first gyroscope \ref UNICORN_AMPLIFIER_CHANNEL in the \ref
//! UNICORN_AMPLIFIER_CONFIGURATION Channels array.
#define UNICORN_GYROSCOPE_CONFIG_INDEX 11

//! Index of the battery level \ref UNICORN_AMPLIFIER_CHANNEL in the \ref
//! UNICORN_AMPLIFIER_CONFIGURATION Channels array.
#define UNICORN_BATTERY_CONFIG_INDEX 14

//! Index of the counter \ref UNICORN_AMPLIFIER_CHANNEL in the \ref UNICORN_AMPLIFIER_CONFIGURATION
//! Channels array.
#define UNICORN_COUNTER_CONFIG_INDEX 15

//! Index of the validation indicator \ref UNICORN_AMPLIFIER_CHANNEL in the \ref
//! UNICORN_AMPLIFIER_CONFIGURATION Channels array.
#define UNICORN_VALIDATION_CONFIG_INDEX 16

//! The number of digital output channels.
#define UNICORN_NUMBER_OF_DIGITAL_OUTPUTS 8

    // ========================================================================================
    // Type definitions
    // ========================================================================================

    //! Type that holds the handle associated with a device.
    typedef uint64_t UNICORN_HANDLE;

    //! Type that holds device serial.
    typedef char UNICORN_DEVICE_SERIAL[UNICORN_SERIAL_LENGTH_MAX];

    //! Type that holds device version.
    typedef char UNICORN_DEVICE_VERSION[UNICORN_DEVICE_VERSION_LENGTH_MAX];

    //! Type that holds firmware version.
    typedef char UNICORN_FIRMWARE_VERSION[UNICORN_FIRMWARE_VERSION_LENGTH_MAX];

    // ========================================================================================
    // Structures
    // ========================================================================================

    //! The type containing information about a single channel of the amplifier.
    typedef struct _UNICORN_AMPLIFIER_CHANNEL
    {
        //! The channel name.
        char name[32];
        //! The channel unit.
        char unit[32];
        //! The channel range as float array. First entry min value; Second max value.
        float range[2];
        //! The channel enabled flag. \ref TRUE to enable channel; \ref FALSE to disable channel.
        BOOL enabled;
    } UNICORN_AMPLIFIER_CHANNEL;

    //! The type containing an amplifier configuration.
    typedef struct _UNICORN_AMPLIFIER_CONFIGURATION
    {

        //! The array holding a configuration for each available \ref UNICORN_AMPLIFIER_CHANNEL.
        UNICORN_AMPLIFIER_CHANNEL Channels[UNICORN_TOTAL_CHANNELS_COUNT];

    } UNICORN_AMPLIFIER_CONFIGURATION;

    //! Type that holds additional information about the device.
    typedef struct _UNICORN_DEVICE_INFORMATION
    {
        //! The number of EEG channels.
        uint16_t numberOfEegChannels;
        //! The serial number of the device.
        UNICORN_DEVICE_SERIAL serial;
        //! The firmware version number.
        UNICORN_FIRMWARE_VERSION firmwareVersion;
        //! The device version number.
        UNICORN_DEVICE_VERSION deviceVersion;
        //! The PCB version number.
        uint8_t pcbVersion[4];
        //! The enclosure version number.
        uint8_t enclosureVersion[4];
    } UNICORN_DEVICE_INFORMATION;


    // ========================================================================================
    // API Methods
    // ========================================================================================

    //! Returns the current API version.
    /*!
        \return The current API version.
    */
    UNICORN_API float UNICORN_GetApiVersion ();

    //! Returns the last error text.
    /*!
        \return The last error text.
    */
    UNICORN_API const char *UNICORN_GetLastErrorText ();

    //! Scans for available devices.
    /*!
        Discovers available paired or unpaired devices. Estimates the number of available paired or
       unpaired devices and returns information about discovered devices.

        \param availableDevices			A pointer to the beginning of an array of
                                        \ref UNICORN_DEVICE_SERIAL, which receives available
                                        devices when the method returns. If NULL is passed, the
                                        number of available devices is returned only to determine
                                        the amount of memory to allocate.
        \param availableDevicesCount	A pointer to a variable that receives the number of
                                        available devices.
        \param rescan					A flag determining whether a full device scan should be
       performed or not. A quick-scan returns the result of the last scan an is faster than a
       rescan. A rescan lasts about 10 seconds. \ref TRUE to perform a rescan. \ref FALSE to perform
       a quick-scan.

        \return		An error code is returned as integer if scanning for available devices fails.
    */
    UNICORN_API int UNICORN_GetAvailableDevices (
        UNICORN_DEVICE_SERIAL *availableDevices, uint32_t *availableDevicesCount, BOOL rescan);

    //! Opens a device.
    /*!
        Connects to a certain Unicorn device and assigns a Unicorn handle if the connection attempt
       succeeded.

        \param serial		The serial number of the device to connect to.
        \param hDevice		A pointer to a \ref UNICORN_HANDLE that receives the handle associated
       with the current session if the device could be opened successfully.

        \return		An error code is returned as integer if the device could not be opened.
    */
    UNICORN_API int UNICORN_OpenDevice (const char *serial, UNICORN_HANDLE *hDevice);

    //! Closes a device.
    /*!
        Disconnects from a device by a given session handle.

        \param hDevice		A pointer to the handle associated with the session.

        \return		An error code is returned as integer if the disconnection attempt fails.
    */
    UNICORN_API int UNICORN_CloseDevice (UNICORN_HANDLE *hDevice);

    //! Initiates a data acquisition in testsignal or measurement mode.
    /*!
        Starts data acquisition in test signal or measurement mode.

        \param hDevice				The \ref UNICORN_HANDLE associated with the session.
        \param testSignalEnabled	Enables or disables the test signal mode. \ref TRUE to start the
       data acquisition in test signal mode; \ref FALSE to start the data acquisition in measurement
       mode.

        \return		An error code is returned as integer if the data acquisition could not be
       started.
    */
    UNICORN_API int UNICORN_StartAcquisition (UNICORN_HANDLE hDevice, BOOL testSignalEnabled);

    //! Terminates a running data acquisition.
    /*!
        Stops a currently running data acquisition session.

        \param hDevice	The \ref UNICORN_HANDLE associated with the session.

        \return		An error code is returned as integer if the acquisition could not be terminated.
    */
    UNICORN_API int UNICORN_StopAcquisition (UNICORN_HANDLE hDevice);

    //! Sets an amplifier configuration.
    /*!
        Sets an amplifier configuration.

        \param hDevice				The \ref UNICORN_HANDLE associated with the session.
        \param configuration		A pointer to \ref UNICORN_AMPLIFIER_CONFIGURATION to set.

        \return		An error code is returned as integer if configuration is invalid or could not be
       set.
    */
    UNICORN_API int UNICORN_SetConfiguration (
        UNICORN_HANDLE hDevice, UNICORN_AMPLIFIER_CONFIGURATION *configuration);

    //! Gets the amplifier configuration.
    /*!
        Retrieves the current amplifier configuration from the device as \ref
       UNICORN_AMPLIFIER_CONFIGURATION.

        \param hDevice				The \ref UNICORN_HANDLE associated with the session.
        \param configuration		A pointer to a \ref UNICORN_AMPLIFIER_CONFIGURATION which stores
       the configuration of the amplifier.

        \return		An error code is returned as integer if configuration could not be read.
    */
    UNICORN_API int UNICORN_GetConfiguration (
        UNICORN_HANDLE hDevice, UNICORN_AMPLIFIER_CONFIGURATION *configuration);

    //! Reads specific number of scans to a specified destination buffer with known length.
    /*!
        Reads a specific number of scans into the specified destination buffer of known length.
       Checks whether the destination buffer is big enough to hold the requested number of scans.

        \param hDevice						The \ref UNICORN_HANDLE associated with the session.
        \param numberOfScans				The number of scans to read. The number of scans must be
                                            greater than zero. A scan consists of one 32-bit
       floating point number for each currently acquired channel.
        \param destinationBuffer			A pointer to the destination buffer that receives the
       acquired data. The destination buffer must provide enough memory to hold the requested number
       of scans multiplied by the number of acquired channels. Call \ref
       UNICORN_GetNumberOfAcquiredChannels to determine the number of acquired channels. Call \ref
       UNICORN_GetChannelIndex to determine the index of a channel within a scan. Example: The
       sample of the battery level channel in the n-th scan is: n* \ref
       UNICORN_GetNumberOfAcquiredChannels()+ \ref UNICORN_GetChannelIndex(\93Battery Level\94)
        \param destinationBufferLength		Number of floats fitting into destination buffer.

        \return		An error code is returned as integer if data could not be read.
    */
    UNICORN_API int UNICORN_GetData (UNICORN_HANDLE hDevice, uint32_t numberOfScans,
        float *destinationBuffer, uint32_t destinationBufferLength);

    //! Determines number of acquired channels.
    /*!
        Uses the currently set \ref UNICORN_AMPLIFIER_CONFIGURATION to get the number of acquired
       channels.

        \param hDevice	The \ref UNICORN_HANDLE associated with the session.
        \param numberOfAcquiredChannels A pointer to a varialbe that receives the number of acquired
       channels.

        \return		An error code is returned as integer if the number of acquired channels could
       not be determined.
    */
    UNICORN_API int UNICORN_GetNumberOfAcquiredChannels (
        UNICORN_HANDLE hDevice, uint32_t *numberOfAcquiredChannels);

    //! Determines the index of the requested channel in an acquired scan.
    /*!
        Uses the currently set \ref UNICORN_AMPLIFIER_CONFIGURATION to get the index of the
       requested channel within an acquired scan.

        \param hDevice	The \ref UNICORN_HANDLE associated with the session.
        \param name		The name of the requested channel.
        \param channelIndex A pointer to a variable that receives the channel index.

        The default names are:
                    EEG 1|2|3|4|5|6|7|8
                    Accelerometer X|Y|Z
                    Gyroscope X|Y|Z
                    Counter
                    Battery Level
                    Validation Indicator

        \return		An error code is returned as integer if the index could not be determined.
    */
    UNICORN_API int UNICORN_GetChannelIndex (
        UNICORN_HANDLE hDevice, const char *name, uint32_t *channelIndex);

    //! Reads the device information.
    /*!
        Reads the device information by a given \ref UNICORN_HANDLE.

        \param hDevice	The \ref UNICORN_HANDLE associated with the session.
        \param deviceInformation	A pointer to a \ref UNICORN_DEVICE_INFORMATION that receives
       information about the device.

        \return An error code is returned as integer if the device information could not be read.
    */
    UNICORN_API int UNICORN_GetDeviceInformation (
        UNICORN_HANDLE hDevice, UNICORN_DEVICE_INFORMATION *deviceInformation);

    //! Sets the digital outputs.
    /*!
        Sets the digital outputs to high or low.

        \param hDevice					The \ref UNICORN_HANDLE associated with the session.
        \param digitalOutputs			A pointer to a variable that receives the states of the
       digital output channels. Each bit represents one digital output channel. If a bit is set, the
       corresponding digital output channel\92s value is set to high. If a bit is cleared, the
       corresponding digital output channel\92s value is set to low. Examples (the binary
       representation of each decimal value is shown in parentheses): 0   (0b00000000) -> all
       digital outputs set to low. 170 (0b10101010) -> digital outputs 2,4,6,8 are set to high. 255
       (0b11111111) -> all digital outputs set to high.

        \return		An error code is returned as integer if the status of the digital output pin
       could not be set.
    */
    UNICORN_API int UNICORN_SetDigitalOutputs (UNICORN_HANDLE hDevice, uint8_t digitalOutputs);

    //! Reads the digital output states.
    /*!
        Reads the digital output states.

        \param hDevice					The \ref UNICORN_HANDLE associated with the session.
        \param digitalOutputs			The state of the digital output channels to set in bits.
       Each bit represents one digital output channel. Set a bit to set the corresponding digital
       output channel\92s value to high. Clear a bit to set the corresponding digital output
       channel\92s value to low. Examples (the binary representation of each decimal value is shown
       in parentheses): 0   (0b00000000) -> all digital outputs set to low. 170 (0b10101010) ->
       digital outputs 2,4,6,8 are set to high. 255 (0b11111111) -> all digital outputs set to high.

        \return		An error code is returned as integer if the status of the digital output pin
       could not be read.
    */
    UNICORN_API int UNICORN_GetDigitalOutputs (UNICORN_HANDLE hDevice, uint8_t *digitalOutputs);
#ifdef __cplusplus
}
#endif

#endif // UNICORN_H
