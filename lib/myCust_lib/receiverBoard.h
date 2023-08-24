#include <Arduino.h>

#ifndef __RECEIVER_BOARD_H__
#define __RECEIVER_BOARD_H__

// #define RGB LEDs
#define HEARTBEAT_LED       26         // Red
#define WIFI_LED            27         // Green
#define MQTT_LED            14         // Blue

// # define Level LEDs
#define LED1             19

#define LED2             18

#define LED3             5 

#define LED4             17   

#define LED5             16 


// DECLARE OTHER PARTS OF RECEIVER BOARD

#define SW_pin             21
#define ACS_pin            35      // Energy Sensor
#define touch1             4      // Pin 2 - for WT_PCB_V4.5
#define data_pin           12     // Pin 16 - for WT_PCB_V4.5
#define clk_pin            13     // Pin 4 - for WT_PCB_V4.5
#define reset_pin          22



   unsigned long int getBoard_ID();

#endif