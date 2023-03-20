#ifndef HYDROPONICS_CONST_H
#define HYDROPONICS_CONST_H

// Defaults
#define DEFAULT_CLIENT_SSID "Your_Network"
#define DEFAULT_AP_SSID "HYDROPONICS-AP"
#define DEFAULT_AP_PASS "hydro1234"
#define DEFAULT_OTA_PASS "hydroota"

//Access point behavior
#define AP_BEHAVIOR_BOOT_NO_CONN          0     //Open AP when no connection after boot
#define AP_BEHAVIOR_NO_CONN               1     //Open when no connection (either after boot or if connection is lost)
#define AP_BEHAVIOR_ALWAYS                2     //Always open
#define AP_BEHAVIOR_BUTTON_ONLY           3     //Only when button pressed for 6 sec


#define MIN_HEAP_SIZE (8192)

#endif