#include "EnergyMonitoring.h"
#include "Arduino.h"

#include "app_config.h"
#include "appManager.h"
#include "receiverBoard.h"      // for using cust_board pins

// for Energy Monitoring

#include <Filters.h>
#include <stdio.h>



// varialble for Energy Monitoring
volatile unsigned long total_energy_consumed;
   unsigned int Sensitivity = SENSTIVITY;    //  value from app_config.h
   float Vpp = 0; // peak-peak voltage 
   float Vrms = 0; // rms voltage
   float Irms = 0; // rms current
   float Supply_Voltage = VOLTAGE_IN;           // reading from DMM
   float Vcc = VCC;         // value from app_config.h 
   float power = 0;         // power in watt              
   unsigned long last_time =0;
   unsigned long current_time =0;
   unsigned int calibration = 100;  // V2 slider calibrates this
   unsigned int pF = PF;           // value from app_config.h


  unsigned long getEngergy(appManager* appMgr) {
    
    resetEnergy(appMgr);
    return appMgr->current_accomulated;
}
// Energy Consumption

 void eMonitorig(appManager* appMgr) {
       // when switch is ON
        broadcast_appMgr(appMgr);
        resetEnergy(appMgr);   
 }

  float getVPP() {
    float result; 
    int readValue;                
    int maxValue = 0;             
    int minValue = 4096; 
    const int Sensor_Pin = ACS_pin;           // value from app_config.h

    uint32_t start_time = millis();
    while((millis()-start_time) < 950) { //read every 0.95 Sec 
     readValue = analogRead(Sensor_Pin);    
     if (readValue > maxValue) 
     {         
         maxValue = readValue; 
     }
     if (readValue < minValue) 
     {          
         minValue = readValue;
     }
    } 
    result = ((maxValue - minValue) * Vcc) / 4096.0;  
    return result;
  }


void getACS712(appManager* appMgr) {  // for AC
   // defining local variables 
              
   volatile float Wh =0 ;             // Energy in kWh


  Vpp = getVPP();
  Vrms = (Vpp/2.0) *0.707; 
  Vrms = Vrms - (calibration / 10000.0);     // calibtrate to zero with slider
  Irms = (Vrms * 1000)/Sensitivity ;
  if((Irms > -0.015) && (Irms < 0.008)){  // remove low end chatter
    Irms = 0.0;
  }
  power= (Supply_Voltage * Irms) * (pF / 100.0); 
  last_time = current_time;
  current_time = millis();    
  Wh = Wh+  power *(( current_time -last_time) /3600000.0) ; // calculating energy in Watt-Hour   
  Wh = (float)(int)(Wh*1000000)/1000000;
  appMgr->current_accomulated += Wh;

}


 void energy_consumption(void * pvParameters) { 

    appManager* appMgr = (appManager*)pvParameters; 

   unsigned long prev_pub_time = millis();

    while( true ) {
      getACS712(appMgr);
       
        if( (appMgr->switch_val==0) && ((unsigned long)(millis() - prev_pub_time) >= PUBLISH_INTERVAL_OFF)) { 
      
             eMonitorig(appMgr); 
             prev_pub_time = millis();            
        }

        if( (appMgr->switch_val==1) && ((unsigned long)(millis() - prev_pub_time) >= PUBLISH_INTERVAL_ON)) { 
      
                eMonitorig(appMgr); 
                prev_pub_time = millis();            
        }              
     }

}

void resetEnergy(appManager* appMgr) {
  //Serial.println("Energy reset to 0");
  appMgr->current_accomulated = 0;
}






