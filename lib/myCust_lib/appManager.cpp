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

void appManager_ctor(appManager * const me) {
  
  initBoard();
  Serial.println("Board Initialized..");

    // Initial setting of Switch
  setSwitchOn(me);

  // get switch update from cloud
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
void getUpdateFrmCloud(appManager*) {

  // get switch value from cloud

  // get threshold from cloud

}


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
  pinMode(reset_pin,OUTPUT);

  digitalWrite(reset_pin,HIGH);
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
  data["level_%"] = appMgr->waterLevel;
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

void setSwitchOn(appManager* appMgr) {
      digitalWrite(SW_pin, 1);
      appMgr->switch_val = 1;

}

void setSwitchOff(appManager* appMgr) {
      digitalWrite(SW_pin, 0);
      appMgr->switch_val = 0;
}
// initialize the Scale
    
HX711 setLoadCell(appManager * appMgr) {
   
    HX711 scale_local;
    
    //rtc_clk_cpu_freq_set_config(RTC_CPU_FREQ_80M);   //  RTC_CPU_FREQ_80M
    setCpuFrequencyMhz(80); 
    Serial.print("Initialinzing scale... ");  
    scale_local.begin(data_pin,clk_pin);
    scale_local.set_scale(calibration_factor);
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
          if(count_press>5100) {
            break;
          }   
        }
   // Action as per time period of pressing button

     if((count_press >0) && (count_press<1500)) {
        
        bool flag = true;  //  to check if control goes to On or Off only

          if (appMgr->switch_val == 1){
            Serial.println("Energy Monitoring Off..");
            setSwitchOff(appMgr);
            flag = false;
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
         // resetWifi(appMgr->conManager);      
         // connectWiFi(appMgr->conManager);

     }

     if((count_press >3400) && (count_press<5000)) {
        setBoardWithLC(appMgr);
     }

    }
    
 }



void setBoardWithLC(appManager* appMgr) {

Serial.println("Sync Board with LC.");
  
  float reading;

  setLevel(appMgr,0);  // reset level to zero 

  reading = check_WT(appMgr);

  if(reading<0) {
     reading = reading * (-1);
  }

  appMgr->threshold = reading;
  Serial.println("Threshold set as per Load Cell..");

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



 
