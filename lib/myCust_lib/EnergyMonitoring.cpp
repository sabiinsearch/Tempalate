#include "EnergyMonitoring.h"
#include "Arduino.h"

#include "app_config.h"
#include "appManager.h"
#include "receiverBoard.h"      // for using cust_board pins

// for Energy Monitoring
#include <Filters.h>
#include "ACS712.h"
#include <stdio.h>
#include <Preferences.h>


Preferences preff;

// varialble for Energy Monitoring
//ACS712 currentSensor(ACS712_20A, ACS_pin);

float P=0;
float V=0;
float I=0;
long dt=0;
float CulmPwh=0;
float units=0;
float lastSample=0;
unsigned long lasttime=0;

// Energy Consumption
  float getVPP() {
    
    preff.begin("app_config",true);

    float vcc = preff.getFloat("VCC");         // value from configuration

    preff.end();

    float result; 
    int readValue;                
    int maxValue = 0;             
    int minValue = 4096; 
    const int Sensor_Pin = ACS_pin;           // value from app_config.h

    uint32_t start_time = millis();
    while((millis()-start_time) < 1000) { //read every 1 Sec 

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
    result = ((maxValue - minValue) * vcc) / 4096.0;  // ESP ADC resolution 4096.0
    return result;
  }


// void getACS712(appManager* appMgr) {

//     preff.begin("app_config",true);
//     float vcc = preff.getFloat("VCC");         // value from configuration
//     float Supply_Voltage = preff.getLong64("VOLTAGE_IN");       // value from configuration
//     float senstivity = preff.getLong64("SENSTIVITY");
//     preff.end();

//      const int sensor_pin = ACS_pin; 
//      float  rawValue = 0;
//      float Vrms_raw_value = 0;  
//      float Vrms=0;
//      float Irms=0;
//      float ACSVoltage=0;
//      double power;  


//      for(int i=0; i<200;i++) {
//           rawValue = analogRead(sensor_pin);
//           Vrms_raw_value += rawValue * rawValue;
//      } 
     
//      Vrms_raw_value = sqrt((Vrms_raw_value/200));
//      Vrms_raw_value = (Vrms_raw_value/4096)* vcc; 

//     Serial.print(F("Vrms_1: "));
//     Serial.print(Vrms_raw_value); 
    


//    ACSVoltage = Vrms_raw_value;
// //   Irms = Vrms

//   Serial.print(F(" Vrms_2: "));
//   Serial.print(ACSVoltage);  

//   Vrms = (ACSVoltage/2.0) * 0.707; 
//   Serial.print(F(" Vrms Calculated: "));
//   Serial.print(Vrms);  
//   // Vrms = Vrms - (calibration / 10000.0);     // calibtrate to zero with slider
//   Irms = (Vrms * 1000)/senstivity;
//   Serial.print(F("\tIrms Calculated: "));
//   Serial.print(Irms);
//   if((Irms > -0.015) && (Irms < 0.008)){  // remove low end chatter
//     Irms = 0.0;
//   }
//   power = Irms / 3600000;
//   Serial.print(F("\tPower: "));
//   Serial.println(power);
//   appMgr->totalEnergy = Irms*Supply_Voltage;

// }





void getACS712(appManager* appMgr) {  // for AC
   
    preff.begin("app_config",true);

    unsigned int Sensitivity = preff.getLong64("SENSTIVITY");         // value from configuration
    float Supply_Voltage = preff.getLong64("VOLTAGE_IN"); 
    unsigned int pF = preff.getLong64("PF");


    preff.end();

   
   // defining local variables 

   volatile unsigned long total_energy_consumed;    
   float Vpp = 0; // peak-peak voltage 
   float Vrms = 0; // rms voltage
   float Irms = 0; // rms current             
   float power = 0;         // power in watt              
   unsigned long last_time =0;
   unsigned long current_time =0;
   unsigned int calibration = 110;  // V2 slider calibrates this            
   volatile double Wh =0 ;             // Energy in kWh

  current_time = millis();
  Vpp = getVPP();
  last_time = millis();
  Vrms = (Vpp/2.0) *0.707;    // root 2 is 0.707
  Irms = ((Vrms * 1000)/Sensitivity)-LOCAL_ERROR ;
  if((Irms < 0.2) || (Irms<0)) {
           Irms = 0.0;
  }
  
  // power = (Irms*Supply_Voltage)/0.82;   // 1.2 is local calibration factor
  //power = (power/3600000)*(last_time-current_time);   

  // Serial.print(F("VPP: "));
  // Serial.print(Vpp);
  // Serial.print(F("\t"));
  // Serial.print(F("Vrms: "));
  // Serial.print(Vrms);
  // Serial.print(F("\t"));
  // Serial.print(F("Irms: "));
  // Serial.print(Irms);
  // Serial.print(F("\n"));
  // Serial.print(F("Power: "));
  // Serial.print(power);
  // Serial.print(F("\n"));
 

//  appMgr->totalEnergy += power;  
  appMgr->totalEnergy += Irms;  
}


 void energy_consumption(void * pvParameters) { 
    appManager* appMgr = (appManager*)pvParameters; 

   
    Serial.print("Energy Monitoring task set @ Core ");
    Serial.println(xPortGetCoreID());

    for(;;) { 
      getACS712(appMgr);
     }

}

// void getACS712(appManager* appMgr) {  // for AC

// //  I = currentSensor.getCurrentAC(FREQUENCY);
//     I = currentSensor.getCurrentAC();
//   dt = micros()- lastSample;
  
//     if(I<0.15)
//       {
//         I=0;
//         CulmPwh=0;
//       }
  
//   // To calculate the power we need voltage multiplied by current
//   //P = U * I;
//   appMgr->totalEnergy += ((I/1000)*V); 
//   // CulmPwh = CulmPwh + P*(dt/3600);///uWh
//   // units= CulmPwh/1000;
//   lastSample=micros();   
    
// }

// void calibCurrent()
// { 

//    preff.begin("app_config",true);
//    V = preff.getLong64("VOLTAGE_IN");    // get the current incoming Voltage from configuration
//    preff.end();

 
//    currentSensor.setZeroPoint(ZERO_POINT);                   // 2943
//    currentSensor.setSensitivity(SENSTIVITY); 


//   long start = millis();

//   do
//   {
//     currentSensor.calibrate();  
//     Serial.print("Zero Point Current :");
//     Serial.println(currentSensor.getZeroPoint());
//     delay(500);
//   }while((millis()-start) < 1000);

// }

//  void energy_consumption(void * pvParameters) { 

//     appManager* appMgr = (appManager*)pvParameters; 
//     calibCurrent();
//     Serial.print("Energy Monitoring task set @ Core ");
//     Serial.println(xPortGetCoreID());

//     for(;;) { 
//       getACS712(appMgr);
//      }

// }










