#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ArduinoJson.h>


// Custom Libraries
//#include "app_config.h"
#include "appManager.h"
#include "connectionManager.h"
#include "EnergyMonitoring.h"
#include "receiverBoard.h"

// Libraries for Load Cell
#include <Arduino.h> 
#include "EEPROM.h"
#include "Preferences.h"
#include "HX711.h"
#include "soc/rtc.h"
#include "esp32-hal-cpu.h"


connectionManager conManagerr;

Preferences pref;
/* constructor implementation */

void appManager_ctor(appManager * const me) {

  
  initBoard();
  Serial.println("Board Initialized..");

    // Initial setting of Switch
  setSwitchOn(me);

  // get switch update from EEPROM / cloud 
  getUpdateFrmCloud(me);

 // me->waterLevel = analogRead(WT_sensor);
  me->scale = setLoadCell(me);
  Serial.print("Scale set with appMgr.. ");
  // broadcast_appMgr(me);

  me->conManager = connectionManager_ctor(&conManagerr);
  Serial.println("Connection Manager set with App Manager");

  Serial.print("AppManager set @ Core ");
  Serial.println(xPortGetCoreID());


}

/* Function Implementation */

// function to get switch status from cloud
void getUpdateFrmCloud(appManager* appMgr) {

  // Initiate preferences   
     pref.begin("app_conf",false);

  // get switch value from cloud
      appMgr->switch_val = pref.getInt("switch_value");
      Serial.print("Preferences Switch Value: ");
      Serial.println(appMgr->switch_val);

  // get threshold from cloud
      appMgr->threshold = pref.getFloat("threshold");
      Serial.print("Preferences Threshold: ");
      Serial.println(appMgr->threshold);

      pref.end();
}


// Setting the Tank LEDs accordingly
void LED_allOff() {
   digitalWrite(LED1,HIGH);
   digitalWrite(LED2,HIGH);
   digitalWrite(LED3,HIGH);
   digitalWrite(LED4,HIGH);
   digitalWrite(LED5,HIGH);
}

void LED_allOn() {
   digitalWrite(LED1,LOW);
   digitalWrite(LED2,LOW);
   digitalWrite(LED3,LOW);
   digitalWrite(LED4,LOW);
   digitalWrite(LED5,LOW);
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
  pinMode(reset_pin,OUTPUT);

  digitalWrite(reset_pin,HIGH);
  // setting Tank level LEDs
  pinMode(LED1,OUTPUT);
  pinMode(LED2,OUTPUT);
  pinMode(LED3,OUTPUT);
  pinMode(LED4,OUTPUT);
  pinMode(LED5,OUTPUT);

  // pinMode(data_pin,INPUT);
  // pinMode(clk_pin,OUTPUT);
 }
 
 void broadcast_appMgr(appManager * appMgr) {
  
  String payload;
  char json_string[256];
  StaticJsonDocument<256> dataJsonDocument;

  JsonObject root = dataJsonDocument.createNestedObject("Root");
  JsonObject data = root.createNestedObject("d");

  //char* boardID = getBoard_ID();
  
  //root["type"] = appMgr->conManager->config->boardType;
  root["type"] = BOARD_TYPE;
  root["uniqueId"] = getBoard_ID();
  data["switch"] = appMgr->switch_val;
  data["level_%"] = appMgr->waterLevel;
//  appMgr->totalEnergy = 0.0;                 // reset Total Energy after broadcast
  data["energy"] = appMgr->totalEnergy;
  appMgr->totalEnergy = 0.0;                 // reset Total Energy after broadcast
  data["timestamp"] = millis();

  serializeJson(root, payload);
  publishData(payload,appMgr->conManager);
 }

void setSwitchOn(appManager* appMgr) {
      // Initiate Preferences to save state
      pref.begin("app_conf",false);
      digitalWrite(SW_pin, 0);
      appMgr->switch_val = 1;
      pref.putInt("switch_value", appMgr->switch_val);
      pref.end();

}

void setSwitchOff(appManager* appMgr) {
      // Initiate Preferences to save state
      pref.begin("app_conf",false);

      digitalWrite(SW_pin, 1);
      appMgr->switch_val = 0;

      pref.putInt("switch_value", appMgr->switch_val);
      pref.end();

}
// initialize the Scale
    
HX711 setLoadCell(appManager * appMgr) {
   
    HX711 scale_local;
    
    //rtc_clk_cpu_freq_set_config(RTC_CPU_FREQ_80M);   //  RTC_CPU_FREQ_80M
    setCpuFrequencyMhz(80); 

    Serial.print("Initializing scale... ");  
    scale_local.begin(data_pin,clk_pin);
    scale_local.set_scale(CALIBRATION_FACTOR);
    Serial.print("Scale Calibrated... ");  

    if(scale_local.is_ready()) {
       Serial.print("Scale is ready..");  
    }
    
    return scale_local;
 }

  void setLevel(appManager* appMgr, float reading) {
      appMgr->waterLevel = reading;
  }

 float check_WT(appManager * appMgr) {

    float reading;

//      reading = ((appMgr->scale.get_units(10))-threshold);
      reading = ((appMgr->scale.get_units(10)));
      reading = (float)(int)(reading*1)/1;                   // add number of 'zeros' as required decimal
     // reading = (reading/tankfull_value)*100;                // calculating the percentile
     // appMgr->waterLevel = reading;
    //   appMgr->waterLevel = appMgr->scale.get_units();
    
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
    
    return reading;
 }


 void checkButtonPressed(appManager* appMgr) {
  
    if((digitalRead(reset_pin))==LOW) {   // check if the button is pressed
        long press_start = millis();
        long press_end = press_start;
        int count_press = 0;

    // count period of button pressed

        while (digitalRead(reset_pin) == LOW) {
          press_end = millis();
          count_press = press_end-press_start;  
          if(count_press>5000) {            
            break;
          }   
        }

            digitalWrite(reset_pin,HIGH); // unpress button        
            Serial.print("Button pressed for: ");
            Serial.println(count_press);
   // Action as per time period of pressing button

     if((count_press >0) && (count_press<1500)) {
        
        bool flag = true;  //  to check if control goes to On or Off only

          if (appMgr->switch_val == 1){
            Serial.println("Energy Monitoring Off..");
            setSwitchOff(appMgr);
            flag = false;
            Serial.print("Flag is set to false..");
          } 
          
          if((appMgr->switch_val == 0) && (flag==true)) {
              Serial.println("Energy Monitoring On..");
              setSwitchOn(appMgr);
            }
          delay(100);             
          broadcast_appMgr(appMgr);

      }
     
        
     if((count_press >1400) && (count_press<3500)) {    // reset settings - wipe stored credentials for testing, these are stored by the esp library

            Serial.println("Wifi Resetting.."); 
            digitalWrite(WIFI_LED,HIGH);
            digitalWrite(HEARTBEAT_LED,LOW);
            resetWifi(appMgr->conManager);      
            connectWiFi(appMgr->conManager);

     }

     if((count_press >3400) && (count_press<6000)) {

              setBoardWithLC(appMgr);
     }

   }
    
 }



void setBoardWithLC(appManager* appMgr) {
  
  pref.begin("app_conf");
  Serial.println("Sync Board with LC.");

  appMgr->scale = setLoadCell(appMgr);
  
  float reading;

  setLevel(appMgr,0);  // reset level to zero 

  reading = check_WT(appMgr);

  if(reading<0) {
     reading = reading * (-1);
  }

  
  pref.putFloat("threshold",reading);
  appMgr->threshold = reading;
  Serial.print("Threshold set in Preferences and appManager as per Load Cell..");
  Serial.println(appMgr->threshold);
  pref.end();
}



// Method for setting water level indicators
void checkWaterLevel_and_indicators(appManager* appMgr) {
//      Serial.println("In checkWaterLevel_and_indicators()");
     float reading = check_WT(appMgr);

     if (reading<0) {
       appMgr->waterLevel = reading+appMgr->threshold;
     }
     else {
      appMgr->waterLevel = reading-appMgr->threshold;
     }
      
//      Serial.println("Setting indicators..");
       switch((int)appMgr->waterLevel) {

         case 0:
           digitalWrite(LED1,HIGH);
           digitalWrite(LED2,HIGH);
           digitalWrite(LED3,HIGH);
           digitalWrite(LED4,HIGH);
           digitalWrite(LED5,HIGH);

         case 1:
           digitalWrite(LED1,LOW);
           digitalWrite(LED2,HIGH);
           digitalWrite(LED3,HIGH);
           digitalWrite(LED4,HIGH);
           digitalWrite(LED5,HIGH);
            break;

         case 2:

           digitalWrite(LED1,LOW);
           digitalWrite(LED2,LOW);
           digitalWrite(LED3,HIGH);
           digitalWrite(LED4,HIGH);
           digitalWrite(LED5,HIGH);

            break;

         case 3:

           digitalWrite(LED1,LOW);
           digitalWrite(LED2,LOW);
           digitalWrite(LED3,LOW);
           digitalWrite(LED4,HIGH);
           digitalWrite(LED5,HIGH);

            break;

         case 4:

           digitalWrite(LED1,LOW);
           digitalWrite(LED2,LOW);
           digitalWrite(LED3,LOW);
           digitalWrite(LED4,LOW);
           digitalWrite(LED5,HIGH);
            break;

         case 5:

           digitalWrite(LED1,LOW);
           digitalWrite(LED2,LOW);
           digitalWrite(LED3,LOW);
           digitalWrite(LED4,LOW);
           digitalWrite(LED5,LOW);

            break;

         default:

           digitalWrite(LED1,HIGH);
           digitalWrite(LED2,HIGH);
           digitalWrite(LED3,HIGH);
           digitalWrite(LED4,HIGH);
           digitalWrite(LED5,HIGH);

            break;               
       }
       
 }

 

//  void checkConnections_and_reconnect(void * pvParameters) { 
    
//     appManager* appMgr = (appManager*)pvParameters; 
//     Serial.print("checking connection set @ Core..");
//     Serial.println(xPortGetCoreID());
//     // Serial.print("\t");
//     // Serial.print("wifi : ");
//     // Serial.println(appMgr->conManager->wifi_manager.getWLStatusString());

//    if(RADIO_AVAILABILITY){
//       initRadio(appMgr->conManager);
//       Serial.print(" Ready to print ");
//    }
//       if(WIFI_AVAILABILITY) {
//         initWiFi();
//       }

//     for(;;) {
//       //;
      
//       if((getWiFi_Availability(appMgr->conManager)==true) && (appMgr->conManager->wifi_manager.getWLStatusString()!= "WL_CONNECTED")) {
//         Serial.print("Wifi status..");
//         Serial.println(appMgr->conManager->wifi_manager.getWLStatusString());
//         digitalWrite(WIFI_LED,HIGH);
//         appMgr->conManager->Wifi_status = connectWiFi(appMgr->conManager);
//       }
//       if((getWiFi_Availability(appMgr->conManager)) && (appMgr->conManager->Wifi_status) && !(appMgr->conManager->mqtt_status)) {
//         digitalWrite(MQTT_LED,HIGH);
//         appMgr->conManager->mqtt_status = connectMQTT(appMgr->conManager);
//       }
//     }
//  }


 
