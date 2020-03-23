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


#ifdef __cplusplus
}
#endif

#endif // UNICORN_H
