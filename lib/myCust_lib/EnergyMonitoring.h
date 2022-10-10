
#ifndef __ENERGYMONITORING_H__
    #define __ENERGYMONITORING_H__

    #include "Arduino.h"
    #include "appManager.h"
        
        void  energy_consumption(void * pvParameters);
        unsigned long getEngergy(appManager*);
        

#endif