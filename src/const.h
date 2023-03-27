#ifndef HYDROPONICS_CONST_H
#define HYDROPONICS_CONST_H

// Defaults
#define DEFAULT_CLIENT_SSID "Your_Network"
#define DEFAULT_AP_SSID "HYDROPONICS-AP"
#define DEFAULT_AP_PASS "hydro1234"


#define NUMBER_MEASUREMENTS 5 // number of measurements used to form an average
#define TEMPERATURE_INTERVAL 5 // Interval to measure temperature in seconds
#define PH_TDS_INTERVAL 300 // Interval to measure ph and tds in seconds (alternating between the sensors)
#define PH_ON_TIME 60 // Seconds waiting before taking a ph measurement after switching the sensor on
#define TDS_ON_TIME 30 // Seconds waiting before taking a tds measurement after switching the sensor on

#define TEMP_PIN 25
#define PH_PIN 32
#define PH_MOSFET_PIN 27
#define TDS_PIN 35
#define TDS_MOSFET_PIN 33

//Access point behavior
#define AP_BEHAVIOR_BOOT_NO_CONN          0     //Open AP when no connection after boot
#define AP_BEHAVIOR_NO_CONN               1     //Open when no connection (either after boot or if connection is lost)
#define AP_BEHAVIOR_ALWAYS                2     //Always open
#define AP_BEHAVIOR_BUTTON_ONLY           3     //Only when button pressed for 6 sec

#define JSON_BUFFER_SIZE 24576
#define MIN_HEAP_SIZE (8192)

#define SETTINGS_STACK_BUF_SIZE 3096

// Error modes
#define ERR_NONE         0  // All good :)
#define ERR_EEP_COMMIT   2  // Could not commit to EEPROM (wrong flash layout?)
#define ERR_NOBUF        3  // JSON buffer was not released in time, request cannot be handled at this time
#define ERR_JSON         9  // JSON parsing failed (input too large?)
#define ERR_FS_BEGIN    10  // Could not init filesystem (no partition?)
#define ERR_FS_QUOTA    11  // The FS is full or the maximum file size is reached

#endif