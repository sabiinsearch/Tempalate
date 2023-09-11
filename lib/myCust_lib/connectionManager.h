#ifndef __CONNNECTION_MANAGER_H__
#define __CONNNECTION_MANAGER_H__

#include "Arduino.h"
#include <WiFiManager.h>

struct configuration_t{

     int     tankCapacity;
     int     calibrationFactor;

// Connectivity configrations
     bool   radioAvailability;
     bool   wifiAvailabililty;
     bool   mqttAvailability;

// Energy Monitoring configrations

    int         publishON;
    int         publishOFF;
    uint8_t     voltageIn;
    uint8_t     vcc;
    uint8_t     senstivity;
    uint8_t     powerFactor;

    // MQTT configrations
    const char*       org;
    String            boardType;
    const char*       token;
    const char*       server;
    const char*       pub_topoic;
    const char*       sub_topoic;
    String            mqtt_user;
    String              mqtt_pwd;
};


/*Connection Manager's attributes*/
typedef struct {

  WiFiManager wifi_manager;
  bool radio_status;  
  bool ble_status;
  bool Wifi_status;
  bool mqtt_status; 
  configuration_t* config; 

} connectionManager;

/*Connection Manager's operations*/
 
   connectionManager * const  connectionManager_ctor(connectionManager * const me);     // Constructor

   void print_communication(); 
   void initWiFi();
   void mqtt_loop();
   bool connectMQTT(connectionManager*);
   bool connectWiFi(connectionManager*);
   void reconnectWiFi(connectionManager*);
   void resetWifi(connectionManager*);
   void initRadio(connectionManager*);
   void checkDataOnRadio();
   void mqttCallback(char*, byte*, unsigned int);
   void publishData(String, connectionManager*);
   void publishOnRadio(String,connectionManager*);
   void publishOnMqtt(String, connectionManager*);
   char* string2char(String);

#endif