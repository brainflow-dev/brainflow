//
//  sensordefines.h
//  sensorprofile
//
//  Created by 叶常青 on 2023/12/20.
//

#ifndef sensordefines_h
#define sensordefines_h


typedef NS_ENUM(NSInteger, GF_RET_CODE)  {
    /// Method returns successfully.
    GF_SUCCESS/*= 0*/,

    /// Method returns with a generic error.
    GF_ERROR,

    /// Given parameters are not match required.
    GF_ERROR_BAD_PARAM,

    /// Method call is not allowed by the inner state.
    GF_ERROR_BAD_STATE,

    /// Method is not supported at this time.
    GF_ERROR_NOT_SUPPORT,

    /// Hub is busying on device scan and cannot fulfill the call.
    GF_ERROR_SCAN_BUSY,

    /// Insufficient resource to perform the call.
    GF_ERROR_NO_RESOURCE,

    /// A preset timer is expired.
    GF_ERROR_TIMEOUT,

    /// Target device is busy and cannot fulfill the call.
    GF_ERROR_DEVICE_BUSY,

    /// The retrieving data is not ready yet
    GF_ERROR_NOT_READY,
};

typedef NS_ENUM(NSInteger, ResponseResult)  {
    RSP_CODE_SUCCESS = 0x00,
    RSP_CODE_NOT_SUPPORT = 0x01,
    RSP_CODE_BAD_PARAM = 0x02,
    RSP_CODE_FAILED = 0x03,
    RSP_CODE_TIMEOUT = 0x04,
    RSP_CODE_PARTIAL_PACKET = 0xFF
};

typedef NS_ENUM(NSInteger, FeatureMaps) {
    GFD_FEAT_EMG = 0x000002000,
    GFD_FEAT_EEG = 0x000400000,
    GFD_FEAT_ECG = 0x000800000,
    GFD_FEAT_IMPEDANCE = 0x001000000,
    GFD_FEAT_IMU = 0x002000000,
    GFD_FEAT_ADS = 0x004000000,
    GFD_FEAT_BRTH = 0x008000000,
};

typedef NS_ENUM(NSInteger, DataNotifyFlags) {
    /// Data Notify All Off
    DNF_OFF = 0x00000000,

    /// Accelerate On(C.7)
    DNF_ACCELERATE = 0x00000001,

    /// Gyroscope On(C.8)
    DNF_GYROSCOPE = 0x00000002,

    /// Magnetometer On(C.9)
    DNF_MAGNETOMETER = 0x00000004,

    /// Euler Angle On(C.10)
    DNF_EULERANGLE = 0x00000008,

    /// Quaternion On(C.11)
    DNF_QUATERNION = 0x00000010,

    /// Rotation Matrix On(C.12)
    DNF_ROTATIONMATRIX = 0x00000020,

    /// EMG Gesture On(C.13)
    DNF_EMG_GESTURE = 0x00000040,

    /// EMG Raw Data On(C.14)
    DNF_EMG_RAW = 0x00000080,

    /// HID Mouse On(C.15)
    DNF_HID_MOUSE = 0x00000100,

    /// HID Joystick On(C.16)
    DNF_HID_JOYSTICK = 0x00000200,

    /// Device Status On(C.17)
    DNF_DEVICE_STATUS = 0x00000400,

    /// Device Log On
    DNF_LOG = 0x00000800,
    
    DNF_EEG = 0x00010000,
    
    DNF_ECG = 0x00020000,
    
    DNF_IMPEDANCE = 0x00040000,
    
    DNF_IMU = 0x00080000,
    
    DNF_ADS = 0x00100000,
    
    DNF_BRTH = 0x00200000,
    /// Data Notify All On
    DNF_ALL = 0xFFFFFFFF,
};


typedef NS_ENUM(NSInteger, NotifyDataType)  {
    NTF_ACC_DATA = 0x01,
    NTF_GYO_DATA,
    NTF_MAG_DATA,
    NTF_EULER_DATA,
    NTF_QUAT_FLOAT_DATA,
    NTF_ROTA_DATA,
    NTF_EMG_GEST_DATA,
    NTF_EMG_ADC_DATA,
    NTF_HID_MOUSE,
    NTF_HID_JOYSTICK,
    NTF_DEV_STATUS,
    NTF_LOG_DATA,
    NTF_MAG_ANGLE_DATA,
    NTF_MOT_CURRENT_DATA,
    NTF_NEUCIR_STATUS,
    NTF_EEG,
    NTF_ECG,
    NTF_IMPEDANCE,
    NTF_IMU,
    NTF_ADS,
    NTF_BRTH,
    NTF_DATA_TYPE_MAX,
    NTF_PARTIAL_DATA = 0xFF
};


typedef NS_ENUM(NSInteger, BLEState)
{
    BLEStateUnConnected,
    BLEStateConnecting,
    BLEStateConnected,
    BLEStateReady,
    BLEStateDisconnecting,
    BLEStateInvalid,
};

#endif /* sensordefines_h */
