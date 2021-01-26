#pragma once

// uuids from muse-lsl, which got most of them from muse-js
// the hexadecimal numbers are the bluetooth handles, sometimes used instead of uuids by drivers.
// the first one is the characteristic handle, the 2nd is the value handle, the 3rd is the
// notification handle.

// 00001800-0000-1000-8000-00805f9b34fb Generic Access 0x05-0x0b
// 00001801-0000-1000-8000-00805f9b34fb Generic Attribute 0x01-0x04
// 00002a05-0000-1000-8000-00805f9b34fb Service Changed 0x02-0x04
const char *INTERAXON_GATT_SERVICE = "0000fe8d-0000-1000-8000-00805f9b34fb";  // 0x0c-0x42
const char *MUSE_GATT_SERIAL = "273e0001-4c4d-454d-96be-f03bac821358";        // 0x0d-0x0f
const char *MUSE_GATT_AUX_LEFT = "273e0002-4c4d-454d-96be-f03bac821358";      // 0x1c-0x1e
const char *MUSE_GATT_TP9 = "273e0003-4c4d-454d-96be-f03bac821358";           // 0x1f-0x21
const char *MUSE_GATT_FP1 = "273e0004-4c4d-454d-96be-f03bac821358";           // 0x22-0x24
const char *MUSE_GATT_FP2 = "273e0005-4c4d-454d-96be-f03bac821358";           // 0x25-0x27
const char *MUSE_GATT_TP10 = "273e0006-4c4d-454d-96be-f03bac821358";          // 0x28-0x2a
const char *MUSE_GATT_AUX_RIGHT = "273e0007-4c4d-454d-96be-f03bac821358";     // 0x2b-0x2d
const char *MUSE_GATT_DRL_REF = "273e0008-4c4d-454d-96be-f03bac821358";       // 0x10-0x12
const char *MUSE_GATT_GYRO = "273e0009-4c4d-454d-96be-f03bac821358";          // 0x13-0x15
const char *MUSE_GATT_ACCELEROMETER = "273e000a-4c4d-454d-96be-f03bac821358"; // 0x16-0x18
const char *MUSE_GATT_TELEMETRY = "273e000b-4c4d-454d-96be-f03bac821358";     // 0x19-0x1b
const char *MUSE_GATT_MAGNETOMETER = "273e000c-4c4d-454d-96be-f03bac821358";  // 0x2e-0x30
const char *MUSE_GATT_PRESSURE = "273e000d-4c4d-454d-96be-f03bac821358";      // 0x31-0x33
const char *MUSE_GATT_ULTRA_VIOLET = "273e000e-4c4d-454d-96be-f03bac821358";  // 0x34-0x36
const char *MUSE_GATT_PPG_AMBIENT = "273e000f-4c4d-454d-96be-f03bac821358";   // 0x37-0x39
const char *MUSE_GATT_PPG_IR = "273e0010-4c4d-454d-96be-f03bac821358";        // 0x3a-0x3c
const char *MUSE_GATT_PPG_RED = "273e0011-4c4d-454d-96be-f03bac821358";       // 0x3d-0x3f
const char *MUSE_S_GATT_THERMISTOR = "273e0012-4c4d-454d-96be-f03bac821358";  // 0x40-0x42
