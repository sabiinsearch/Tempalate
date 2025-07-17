// Default Arduino includes
#include <Arduino.h>
#include <nvs.h>
#include <nvs_flash.h>
#include <ArduinoJson.h>
#include <stdlib.h>
#include <SPI.h>
#include <Preferences.h>

// to import all my custom libraries

#include "app_config.h"
#include "receiverBoard.h"
#include "connectionManager.h"
#include "appManager.h"
#include "EnergyMonitoring.h"

                  

// my Managers
appManager managr;
unsigned long prev_pub_time=0;
Preferences prefer;

  volatile long publishTimeON;
  volatile long publishTimeOFF;

// setup function
void setup() {

 // nvs_flash_erase(); // erase the NVS partition and...

  nvs_flash_init(); // initialize the NVS partition.  
  

  Serial.begin(9600);
  while (!Serial);
  delay(1000);
	
  LED_allOff();
  //digitalWrite(touch1, 0);
 
 
  // Initiating Manager
  Serial.println("Initializing App Manager..");
  appManager_ctor(&managr);


//  Task to monitor Energy
 //   xTaskCreatePinnedToCore(energy_consumption, "Task2", 10000, &managr, 0, NULL,  0);   
//    Serial.println("first task created ");

//  Task to monitor connectivity
    xTaskCreatePinnedToCore(checkConnections_and_reconnect, "Task3", 90000, &managr, 0, NULL,  0);   
//    Serial.println("Second task created ");

}
/**
 * Logic that runs in Loop
 */
void loop() { 
    //Serial.println(F("In loop.."));
  

          checkButtonPressed(&managr);
     prefer.begin("app_config",true);
     
     publishTimeON = prefer.getLong64("PUBLISH_ON");
     publishTimeOFF = prefer.getLong64("PUBLISH_OFF");

    //  Serial.print(F(" ON: "));
    //  Serial.print(publishTimeON);
    //  Serial.print(F("\t"));

    //  Serial.print(F(" OFF: "));
    //  Serial.print(publishTimeOFF);
    //  Serial.println(F("\t"));

     prefer.end();

    //Serial.println(F("Check detection done in loop().."));
    if( (managr.switch_val==0) && ((unsigned long)(millis() - prev_pub_time) >= publishTimeOFF)) { 
      
             broadcast_appMgr(&managr);             
             prev_pub_time = millis();            
      }
      //vTaskDelay(5); 
     if( (managr.switch_val==1) && ((unsigned long)(millis() - prev_pub_time) >= publishTimeON)) { 
      
                broadcast_appMgr(&managr); 
                prev_pub_time = millis();            
      }              

  //Serial.println(F("Checking water level and setting indicators accordingly in loop.."));

   checkWaterLevel_and_indicators(&managr);

    //   Serial.println(F("Water level checked in loop.."));

   if(managr.conManager->mqtt_status) {
       mqtt_loop();
    }   
    delay(10);  
 //  checkConnections_and_reconnect(&managr);   
}
