
#ifndef __APP_CONFIG_H__
#define __APP_CONFIG_H__

#include "Arduino.h"

/******************************* These Values can be updated from mqtt      **************************************/
// Tank Specific
     #define TANK_CAPACITY 5                 // for Demo
     #define CALIBRATION_FACTOR  6550         // This is not to be changed
   
// Set flags for Communication
     #define RADIO_AVAILABILITY  0//false
     #define BLE_AVAILIBILITY    0//false
     #define WIFI_AVAILABILITY   1//true
     #define MQTT_AVAILABILITY   1//true


// Energy Monitoring configrations
    #define PUBLISH_INTERVAL_ON      10000//5000L    // time in Minutes * sec in a min * milliseconds in sec
    #define PUBLISH_INTERVAL_OFF     10000//5000L    // time in Minutes * sec in a min * milliseconds in sec
    #define VOLTAGE_IN               240       // Input Voltage
    #define VCC                      2.9
    #define SENSTIVITY               66       // 185mV/A for 5A, 100 mV/A for 20A and 66mV/A for 30A Module
    #define PF                       95       // Power Factor


/******************************* Do Not Edit / update these values      **************************************/
//  Mqtt Configurations

    // IOT PLATFORM VARIABLES

    #define ORG             "rqeofj"
    #define BOARD_TYPE      "HB_Water"
    #define TOKEN           "1SatnamWaheguruJi"
    #define SERVER          "broker.hivemq.com"            //    ORG".messaging.internetofthings.ibmcloud.com";
    #define PUB_TOPIC       "iot-2/evt/status/fmt/json"
    #define SUB_TOPIC       "iot-2/cmd/device/fmt/json"
    #define MQTT_USER       "use-token-auth"
    #define MQTT_PASSWORD   "1SatnamWaheguruJi"    
    
#endif