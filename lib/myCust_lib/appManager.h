#ifndef __APP_MANAGER_H__
#define __APP_MANAGER_H__

#include "app_config.h"
#include "connectionManager.h"
#include "HX711.h"

/*Application Manager's attributes*/

typedef struct {

     connectionManager* conManager;
//   energyMonitoringManager eManager;
     float current_accomulated;
     int switch_val;
     float waterLevel; 
     HX711 scale;

} appManager;

void appManager_ctor(appManager * const me, int sw_val); // constructor

void initBoard();   
void LED_allOn();
void LED_allOff();
HX711 setLoadCell(appManager*);
void broadcast_appMgr(appManager*);
void check_WT(appManager*);
int  checkTouchDetected(appManager*);
void checkWaterLevel_and_indicators(appManager*);
void checkConnections_and_reconnect(void * pvParameters);

// functions to set LEDs as per status

/*

HEARTBEAT_LED             // Red
WIFI_LED                  // Blue
BLE_LED                   // Green
*/

#endif