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

// setup function
void setup() {

  Serial.begin(9800);
  while (!Serial);
  delay(1000);
	
  // Send some device info
	// Serial.println("Build: ");
  
  // Serial.print("Board ID: WT-");
  // Serial.println(getBoard_ID());

  LED_allOff();
  //digitalWrite(touch1, 0);

  // Initial setting of Switch
  digitalWrite(SW_pin, 1);

  // Initiating Manager
  Serial.println("Initializing App Manager..");
  appManager_ctor(&managr,0);

//  Task to monitor Energy
    xTaskCreatePinnedToCore(energy_consumption, "Task2", 10000, &managr, 0, NULL,  0);   

}
/**
 * Logic that runs in Loop
 */
void loop() { 
    
   // Check touch and manage switch
    managr.switch_val = checkTouchDetected(&managr);

    if( (managr.switch_val==0) && ((unsigned long)(millis() - prev_pub_time) >= PUBLISH_INTERVAL_OFF)) { 
      
             broadcast_appMgr(&managr);             
             prev_pub_time = millis();            
      }
      //vTaskDelay(5); 
     if( (managr.switch_val==1) && ((unsigned long)(millis() - prev_pub_time) >= PUBLISH_INTERVAL_ON)) { 
      
                broadcast_appMgr(&managr); 
                prev_pub_time = millis();            
      }              


    // if(RADIO_AVAILABILITY) {
    //   checkDataOnRadio();
    // }
   setWaterLevel_and_indicators(&managr);

   if(managr.conManager->mqtt_status) {
       mqtt_loop();
    }   
  
 //  checkConnections_and_reconnect(&managr);   
}
