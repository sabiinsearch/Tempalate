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



Preferences pref_main;
                  

// my Managers
appManager managr;
unsigned long prev_pub_time=0;

// setup function
void setup() {

  Serial.begin(9600);
  while (!Serial);
  delay(1000);
	
  // Send some device info
	// Serial.println("Build: ");
  
  // Serial.print("Board ID: WT-");
  // Serial.println(getBoard_ID());

  LED_allOff();
  //digitalWrite(touch1, 0);
 
 
  // Initiating Manager
  Serial.println("Initializing App Manager..");
  appManager_ctor(&managr);


//  Task to monitor Energy
    xTaskCreatePinnedToCore(energy_consumption, "Task2", 10000, &managr, 0, NULL,  0);   
//    Serial.println("first task created ");

//  Task to monitor connectivity
    xTaskCreatePinnedToCore(checkConnections_and_reconnect, "Task3", 90000, &managr, 0, NULL,  0);   
//    Serial.println("Second task created ");

}
/**
 * Logic that runs in Loop
 */
void loop() { 
//    Serial.println("In loop..");
   pref_main.begin("app_config", true);
   
   long publish_on = pref_main.getLong64("PUBLISH_ON");
   long publish_off = pref_main.getLong64("PUBLISH_OFF");

   pref_main.end();


          checkButtonPressed(&managr);
   
//    Serial.println("Check detection done in loop()..");
    if( (managr.switch_val==0) && ((unsigned long)(millis() - prev_pub_time) >= publish_off)) { 
      
             broadcast_appMgr(&managr);             
             prev_pub_time = millis();            
      }
      //vTaskDelay(5); 
     if( (managr.switch_val==1) && ((unsigned long)(millis() - prev_pub_time) >= publish_on)) { 
      
                broadcast_appMgr(&managr); 
                prev_pub_time = millis();            
      }              

 //  Serial.println("Checking water level and setting indicators accordingly in loop..");

   checkWaterLevel_and_indicators(&managr);

  //     Serial.println("Water level checked in loop..");

   if(managr.conManager->mqtt_status) {
       mqtt_loop();
    }   
  
 //  checkConnections_and_reconnect(&managr);   
}
