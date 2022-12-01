#pragma once

#define EDA "EA\0"
#define EDL "EL\0"
#define EDR "ER\0"
#define PPG_INFRARED "PI\0"
#define PPG_RED "PR\0"
#define PPG_GREEN "PG\0"
#define SPO2 "O2\0"
#define TEMPERATURE_0 "T0\0"
#define TEMPERATURE_1 "T1\0"
#define THERMOPILE "TH\0"
#define HUMIDITY_0 "H0\0"
#define ACCELEROMETER_X "AX\0"
#define ACCELEROMETER_Y "AY\0"
#define ACCELEROMETER_Z "AZ\0"
#define GYROSCOPE_X "GX\0"
#define GYROSCOPE_Y "GY\0"
#define GYROSCOPE_Z "GZ\0"
#define MAGNETOMETER_X "MX\0"
#define MAGNETOMETER_Y "MY\0"
#define MAGNETOMETER_Z "MZ\0"
#define BATTERY_VOLTAGE "BV\0"
#define BATTERY_PERCENT "B%\0"
#define BUTTON_PRESS_SHORT "BS\0"
#define BUTTON_PRESS_LONG "BL\0"
#define DATA_CLIPPING "DC\0"
#define DATA_OVERFLOW "DO\0"
#define SD_CARD_PERCENT "SD\0"
#define RESET "RS\0" // still necessary?
#define EMOTIBIT_DEBUG "DB\0"
#define ACK "AK\0"
#define REQUEST_DATA "RD\0"
#define TIMESTAMP_EMOTIBIT "TE\0"
#define TIMESTAMP_LOCAL "TL\0"
#define TIMESTAMP_UTC "TU\0"
#define TIMESTAMP_CROSS_TIME "TX\0"
#define EMOTIBIT_MODE "EM\0"
#define EMOTIBIT_INFO "EI\0"
#define HEART_RATE "HR\0"
#define INTER_BEAT_INTERVAL "BI\0"
#define SKIN_CONDUCTANCE_RESPONSE_AMPLITUDE "SA\0"
#define SKIN_CONDUCTANCE_RESPONSE_FREQ "SF\0"
#define SKIN_CONDUCTANCE_RESPONSE_RISE_TIME "SR\0"
// Computer data TypeTags (sent over reliable channel e.g. Control)
#define GPS_LATLNG "GL\0"
#define GPS_SPEED "GS\0"
#define GPS_BEARING "GB\0"
#define GPS_ALTITUDE "GA\0"
#define USER_NOTE "UN\0"
#define LSL_MARKER "LM\0"
// Control TypeTags
#define RECORD_BEGIN "RB\0"
#define RECORD_END "RE\0"
#define MODE_NORMAL_POWER "MN\0"  // Stops sending data timestamping should be accurate
#define MODE_LOW_POWER "ML\0"     // Stops sending data timestamping should be accurate
#define MODE_MAX_LOW_POWER "MM\0" // Stops sending data timestamping accuracy drops
#define MODE_WIRELESS_OFF "MO\0"  // Stops sending data timestamping should be accurate
#define MODE_HIBERNATE "MH\0"     // Full shutdown of all operation
#define EMOTIBIT_DISCONNECT "ED\0"
#define SERIAL_DATA_ON "S+\0"
#define SERIAL_DATA_OFF "S-\0"
// Advertising TypeTags
#define PING "PN\0"
#define PONG "PO\0"
#define HELLO_EMOTIBIT "HE\0"
#define HELLO_HOST "HH\0"
#define EMOTIBIT_CONNECT "EC\0"

#define CONTROL_PORT "CP\0"
#define DATA_PORT "DP\0"
#define RECORDING_STATUS "RS\0"
#define POWER_STATUS "PS\0"

#define PACKET_DELIMITER_CSV '\n'
#define PAYLOAD_DELIMITER ','

#define HEADER_LENGTH 6
#define WIFI_ADVERTISING_PORT 3131