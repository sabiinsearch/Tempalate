
#ifndef __APP_CONFIG_H__
#define __APP_CONFIG_H__

#include "Arduino.h"

// Tank Specific
     #define tankCapacity_actual 5                 // for Demo
     #define calibration_factor 6550         // This is not to be changed
   
// Set flags for Communication
     #define RADIO_AVAILABILITY  false
     #define BLE_AVAILIBILITY    false
     #define WIFI_AVAILABILITY   true
     #define MQTT_AVAILABILITY   true


// Energy Monitoring configrations
    #define PUBLISH_INTERVAL_ON      2000L    // time in Minutes * sec in a min * milliseconds in sec
    #define PUBLISH_INTERVAL_OFF     10000L    // time in Minutes * sec in a min * milliseconds in sec
    #define VOLTAGE_IN               240       // Input Voltage
    #define VCC                      2.9
    #define SENSTIVITY               66       // 185mV/A for 5A, 100 mV/A for 20A and 66mV/A for 30A Module
    #define PF                       95       // Power Factor



//  Mqtt Configurations

    // IOT PLATFORM VARIABLES

    #define ORG             "rqeofj"
    #define BOARD_TYPE      "HB_Water"
    #define TOKEN           "1SatnamWaheguruJi"
    #define SERVER          ORG".messaging.internetofthings.ibmcloud.com";
    #define PUB_TOPIC       "iot-2/evt/status/fmt/json"
    #define SUB_TOPIC       "iot-2/cmd/device/fmt/json"
    #define MQTT_USER       "use-token-auth"
    #define MQTT_PASSWORD   "1SatnamWaheguruJi"    
    
#endif