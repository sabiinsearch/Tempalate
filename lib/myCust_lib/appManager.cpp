#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ArduinoJson.h>

// Custom Libraries
#include "app_config.h"
#include "appManager.h"
#include "connectionManager.h"
#include "EnergyMonitoring.h"
#include "receiverBoard.h"

connectionManager conManagerr;

/* constructor implementation */

void appManager_ctor(appManager * const me, int sw_val) {
  initBoard();
  Serial.println("Board Initialized..");
  me->conManager = connectionManager_ctor(&conManagerr);
  Serial.println("Connection Manager set with App Manager");
  me->switch_val = sw_val;
  me->waterLevel = analogRead(WT_sensor);
  Serial.println("Water_Level indicators set...");
  // broadcast_appMgr(me);
  Serial.println("AppManager set.. ");
}

/* Function Implementation */

// Setting the Tank LEDs accordingly
void LED_allOff() {
   digitalWrite(LED1_U,HIGH);
   digitalWrite(LED1_D,HIGH);
   digitalWrite(LED2_U,HIGH);
   digitalWrite(LED2_D,HIGH);
   digitalWrite(LED3_U,HIGH);
   digitalWrite(LED3_D,HIGH);
   digitalWrite(LED4_U,HIGH);
   digitalWrite(LED4_D,HIGH);
   digitalWrite(LED5_U,HIGH);
   digitalWrite(LED5_D,HIGH);
}

void LED_allOn() {
   digitalWrite(LED1_U,LOW);
   digitalWrite(LED1_D,LOW);
   digitalWrite(LED2_U,LOW);
   digitalWrite(LED2_D,LOW);
   digitalWrite(LED3_U,LOW);
   digitalWrite(LED3_D,LOW);
   digitalWrite(LED4_U,LOW);
   digitalWrite(LED4_D,LOW);
   digitalWrite(LED5_U,LOW);
   digitalWrite(LED5_D,LOW);
}

void initRGB(){
  pinMode(HEARTBEAT_LED, OUTPUT);
  pinMode(WIFI_LED, OUTPUT);
  pinMode(MQTT_LED, OUTPUT);
  
  digitalWrite(HEARTBEAT_LED,HIGH);
  digitalWrite(WIFI_LED,HIGH);
  digitalWrite(MQTT_LED,HIGH);

  //Serial.println("InitRGB : appManager.cpp");
 }

 void initBoard() {
  
  // Configuring Board pins
  initRGB();
  pinMode(SW_pin, OUTPUT);
  pinMode(touch1, INPUT);
  pinMode(WT_sensor, INPUT);
  pinMode(A0,INPUT);
  pinMode(ACS_pin,INPUT);

  // setting Tank level LEDs
  pinMode(LED1_U,OUTPUT);
  pinMode(LED1_D,OUTPUT);
  pinMode(LED2_U,OUTPUT);
  pinMode(LED2_D,OUTPUT);
  pinMode(LED3_U,OUTPUT);
  pinMode(LED3_D,OUTPUT);
  pinMode(LED4_U,OUTPUT);
  pinMode(LED4_D,OUTPUT);
  pinMode(LED5_U,OUTPUT);
  pinMode(LED5_D,OUTPUT);

 }
 
 void broadcast_appMgr(appManager * appMgr) {
  
  String payload;
  StaticJsonBuffer<200> dataJsonBuffer;
  JsonObject& root = dataJsonBuffer.createObject();
  JsonObject& data = root.createNestedObject("d");

  //char* boardID = getBoard_ID();
  
  root["type"] = BOARD_TYPE;
  root["uniqueId"] = getBoard_ID();
  data["switch"] = appMgr->switch_val;
  data["level"] = appMgr->waterLevel;
  data["energy"] = appMgr->current_accomulated;
  appMgr->current_accomulated = 0;
  data["timestamp"] = millis();

  // Convert JSON object into a string
  root.printTo(payload);
  Serial.print(payload);
  Serial.println("\n"); 
  publishData(payload,appMgr->conManager);
  
  dataJsonBuffer.clear();
  vTaskDelay(10);
 }

 void check_WT(appManager * appMgr) {

  uint32_t raw; 
  uint32_t Vin = 3.3;
  float Vout = 0.0;
  float buffer = 0;
  raw = analogRead(WT_sensor);
//  if(raw){
  buffer = raw * Vin;
  Vout = (buffer)/1024.0;
  // Serial.print("Raw: ");
  // Serial.print(raw);
  // Serial.print("\t");
  // Serial.print("Vout: ");
  // Serial.print(Vout);
  String volt_level = "Tank_Level";
  volt_level += Vout;
  // Serial.print("\t");
  // Serial.println(volt_level);
  //publishData(volt_level,appMgr->conManager);
  //broadcast_appMgr(appMgr);

// Set the water levels as per the input received 

    if(Vout <= 0 ) {
        appMgr->waterLevel = 0;  
    }
    if(0 < Vout < 10) {
        appMgr->waterLevel = 1;  
    }
    if(10 < Vout < 20) {
        appMgr->waterLevel = 2;  
    }
    if(20 < Vout < 30) {
        appMgr->waterLevel = 3;  
    }
    if(30 < Vout < 40) {
        appMgr->waterLevel = 4;  
    }
    if(40 < Vout < 50) {
        appMgr->waterLevel = 5;  
    }
 }


 int checkTouchDetected(appManager* appMgr) {
  if(digitalRead(touch1) == HIGH){
        long press_start = millis();
        long press_end = press_start;
        int count_press = 0;

        while (digitalRead(touch1) == HIGH) {
          press_end = millis();
          count_press = press_end-press_start;
           if((count_press>3000) && (WIFI_AVAILABILITY)) {
            Serial.println("Wifi Resetting.."); 
            digitalWrite(WIFI_LED,HIGH);
            digitalWrite(HEARTBEAT_LED,LOW);
            resetWifi(appMgr->conManager); // reset settings - wipe stored credentials for testing, these are stored by the esp library
            connectWiFi(appMgr->conManager);
            break;
           }  
        } 
        

        if(count_press<2500) {
          check_WT(appMgr);
          if (appMgr->switch_val == 1){
            Serial.println("Energy Monitoring Off..");
            digitalWrite(SW_pin, 1);
            //LED_allOff();
            appMgr->switch_val= 0;
          } else {
            Serial.println("Energy Monitoring On..");
            digitalWrite(SW_pin, 0);
            //LED_allOn();
            appMgr->switch_val = 1;
          }
          delay(100);             
        } 
        broadcast_appMgr(appMgr);
  }
  return appMgr->switch_val;
 }

// Method for setting water level indicators
void setWaterLevel_and_indicators(appManager* appMgr) {
    int level = analogRead(WT_sensor);  
    // Serial.print("Level: "); 
    // Serial.println(level);  
    if(level<=10) {
       appMgr->waterLevel = 0;      // set the water level in appManager
    }

       switch(appMgr->waterLevel) {

         case 0:
           digitalWrite(LED1_U,HIGH);
           digitalWrite(LED2_U,HIGH);
           digitalWrite(LED3_U,HIGH);
           digitalWrite(LED4_U,HIGH);
           digitalWrite(LED5_U,HIGH);

           digitalWrite(LED1_D,LOW);          
           digitalWrite(LED2_D,LOW);          
           digitalWrite(LED3_D,LOW);           
           digitalWrite(LED4_D,LOW);           
           digitalWrite(LED5_D,LOW);
            break;

         case 1:
           digitalWrite(LED1_U,LOW);
           digitalWrite(LED2_U,HIGH);
           digitalWrite(LED3_U,HIGH);
           digitalWrite(LED4_U,HIGH);
           digitalWrite(LED5_U,HIGH);

           digitalWrite(LED1_D,HIGH);          
           digitalWrite(LED2_D,LOW);          
           digitalWrite(LED3_D,LOW);           
           digitalWrite(LED4_D,LOW);           
           digitalWrite(LED5_D,LOW);
            break;

         case 2:

           digitalWrite(LED1_U,LOW);
           digitalWrite(LED2_U,LOW);
           digitalWrite(LED3_U,HIGH);
           digitalWrite(LED4_U,HIGH);
           digitalWrite(LED5_U,HIGH);

           digitalWrite(LED1_D,HIGH);          
           digitalWrite(LED2_D,HIGH);          
           digitalWrite(LED3_D,LOW);           
           digitalWrite(LED4_D,LOW);           
           digitalWrite(LED5_D,LOW);

            break;

         case 3:

           digitalWrite(LED1_U,LOW);
           digitalWrite(LED2_U,LOW);
           digitalWrite(LED3_U,LOW);
           digitalWrite(LED4_U,HIGH);
           digitalWrite(LED5_U,HIGH);

           digitalWrite(LED1_D,HIGH);          
           digitalWrite(LED2_D,HIGH);          
           digitalWrite(LED3_D,HIGH);           
           digitalWrite(LED4_D,LOW);           
           digitalWrite(LED5_D,LOW);

            break;

         case 4:

           digitalWrite(LED1_U,LOW);
           digitalWrite(LED2_U,LOW);
           digitalWrite(LED3_U,LOW);
           digitalWrite(LED4_U,LOW);
           digitalWrite(LED5_U,HIGH);

           digitalWrite(LED1_D,HIGH);          
           digitalWrite(LED2_D,HIGH);          
           digitalWrite(LED3_D,HIGH);           
           digitalWrite(LED4_D,HIGH);           
           digitalWrite(LED5_D,LOW);

            break;

         case 5:

           digitalWrite(LED1_U,LOW);
           digitalWrite(LED2_U,LOW);
           digitalWrite(LED3_U,LOW);
           digitalWrite(LED4_U,LOW);
           digitalWrite(LED5_U,LOW);

           digitalWrite(LED1_D,HIGH);          
           digitalWrite(LED2_D,HIGH);          
           digitalWrite(LED3_D,HIGH);           
           digitalWrite(LED4_D,HIGH);           
           digitalWrite(LED5_D,HIGH);

            break;

         default:

           digitalWrite(LED1_U,HIGH);
           digitalWrite(LED2_U,HIGH);
           digitalWrite(LED3_U,HIGH);
           digitalWrite(LED4_U,HIGH);
           digitalWrite(LED5_U,HIGH);

           digitalWrite(LED1_D,LOW);          
           digitalWrite(LED2_D,LOW);          
           digitalWrite(LED3_D,LOW);           
           digitalWrite(LED4_D,LOW);           
           digitalWrite(LED5_D,LOW);

            break;               

       }
 }

 

 void checkConnections_and_reconnect(appManager* managr) {
    
    // Serial.print("Task to check connections is set @ Core ");
    // Serial.println(xPortGetCoreID());

 }



 
