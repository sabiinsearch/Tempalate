## Code for Water Tank Receiver End ##
   ### Energy Calculation ###
  
*** current consumed is saved in every reading . at final multiply / divide as per the timeas the other fields are constant thus, for final energy consumption, these constants could be calculated later. ***

*** For Reference *** 
   Total energy cost = (Power in watts/1000) × hours operating × cost per kWh

    *** measurement we get is current in Amperes *** x with input Voltage (defined in app_config.h)  = Power in Watt (for that instance)

    =>  Energy = { Input Volt * Current Measured /1000  * (1/3600)sec } Killo Watt Hours 

    =>  Energy = { Input Volt * Current Measured /1000  * (1/3600*1000)millis } Killo Watt Hours 

    ** Note ** Time between 02 readings = 4135 millis

    =>  Energy = { Input Volt * Current Measured /1000  * (1/3600*1000*4135)millis } Killo Watt Hours



    
