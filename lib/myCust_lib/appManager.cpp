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

// Libraries for Load Cell
#include <Arduino.h> 
#include "EEPROM.h"
#include "HX711.h"
#include "soc/rtc.h"
#include "esp32-hal-cpu.h"
connectionManager conManagerr;

/* constructor implementation */

void appManager_ctor(appManager * const me, int sw_val) {
  initBoard();
  Serial.println("Board Initialized..");
  me->conManager = connectionManager_ctor(&conManagerr);
  Serial.println("Connection Manager set with App Manager");
  me->switch_val = sw_val;
 // me->waterLevel = analogRead(WT_sensor);
  me->scale = setLoadCell(me);
  Serial.print("Scale set with appMgr.. ");
  // broadcast_appMgr(me);
  Serial.print("AppManager set @ Core ");
  Serial.println(xPortGetCoreID());
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
 // pinMode(WT_sensor, INPUT);
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

  // pinMode(data_pin,INPUT);
  // pinMode(clk_pin,OUTPUT);

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

// initialize the Scale
    
    HX711 setLoadCell(appManager * appMgr) {
   
    HX711 scale_local;
    
    //rtc_clk_cpu_freq_set_config(RTC_CPU_FREQ_80M);   //  RTC_CPU_FREQ_80M
    setCpuFrequencyMhz(80); 
    scale_local.begin(data_pin,clk_pin);
    scale_local.set_scale(calibration_factor);
    Serial.print("Scale Calibrated... ");  

    if(scale_local.is_ready()) {
       Serial.print("Scale is ready..");  
    }
    
    return scale_local;
 }


 void check_WT(appManager * appMgr) {

    float threshold;
    float reading;

      // set the threshold as per the capacity
    if(tankCapacity_actual==750) {
      threshold = 140.00;
    }
    if(tankCapacity_actual ==5) {
      threshold = 64.30;
    }
   
    reading = ((appMgr->scale.get_units())-threshold);
    reading = (float)(int)(reading*1)/1;                   // add number of 'zeros' as required decimal
    appMgr->waterLevel = reading;
   // appMgr->waterLevel = appMgr->scale.get_units();
    
/*
  uint32_t raw; 
  uint32_t Vin = 3.3;
  uint32_t level;
  float Vout = 0.0;
  float buffer = 0;
  raw = analogRead(WT_sensor);
//  if(raw){
  buffer = raw * Vin;
  Vout = (buffer)/1024.0;
  level = (uint32_t)Vout;
// Set the water levels as per the input received 
   

    if(0 < level < 1) {
        appMgr->waterLevel = 0;  
    }
    else if((0 < level ) && (level< 2)) {
        appMgr->waterLevel = 1;  
    }
    else if((1 < level ) && (level < 3)) {
        appMgr->waterLevel = 2;  
    }
    else if((2 < level ) && (level < 4)) {
        appMgr->waterLevel = 3;  
    }
    else if((4 < level ) && (level < 9)) {
        appMgr->waterLevel = 4;  
    }
    else if((10 < level ) && (level < 13 )) {
        appMgr->waterLevel = 5;  
    }
*/
   // appMgr->waterLevel = level;
    
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
          //check_WT(appMgr);
          if (appMgr->switch_val == 1){
            Serial.println("Energy Monitoring Off..");
            digitalWrite(SW_pin, 0);
            //LED_allOff();
            appMgr->switch_val= 0;
          } else {
            Serial.println("Energy Monitoring On..");
            digitalWrite(SW_pin, 1);
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
void checkWaterLevel_and_indicators(appManager* appMgr) {
//      Serial.println("In checkWaterLevel_and_indicators()");
    check_WT(appMgr);  
//      Serial.println("Setting indicators..");
       switch((int)appMgr->waterLevel) {

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

 

 void checkConnections_and_reconnect(void * pvParameters) { 
    
    appManager* appMgr = (appManager*)pvParameters; 
    Serial.print("checking connection set @ Core..");
    Serial.println(xPortGetCoreID());
    // Serial.print("\t");
    // Serial.print("wifi : ");
    // Serial.println(appMgr->conManager->wifi_manager.getWLStatusString());
    for(;;) {
      //;
      
      if((WIFI_AVAILABILITY==true) && (appMgr->conManager->wifi_manager.getWLStatusString()!= "WL_CONNECTED")) {
        Serial.print("Wifi status..");
        Serial.println(appMgr->conManager->wifi_manager.getWLStatusString());
        digitalWrite(WIFI_LED,HIGH);
        appMgr->conManager->Wifi_status = connectWiFi(appMgr->conManager);
      }
      if((MQTT_AVAILABILITY) && (appMgr->conManager->Wifi_status) && !(appMgr->conManager->mqtt_status)) {
        digitalWrite(MQTT_LED,HIGH);
        appMgr->conManager->mqtt_status = connectMQTT(appMgr->conManager);
      }
    }
 }



 
