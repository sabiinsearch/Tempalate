## Code for Water Tank Receiver End ##
   ### Energy Calculation ###
  
Energy is calculated using reading from sensor and then applying some calculations as per the constants and saved in appManager.

*** For Reference *** 


   In AC circuit analysis, both voltage and current vary sinusoidally with time.

**Real Power (P)**:

This is the power used by the device to produce useful work.It is expressed in kW.

Real Power = Voltage (V) x Current (I) x cosΦ

**Reactive Power (Q)** :

This is often called imaginary power which is a measure of power oscillates between source and load, that does no useful work.It is expressed in kVAr

Reactive Power = Voltage (V) x Current (I) x sinΦ

**Apparent Power (S)** :

It is defined as the product of the Root-Mean-Square (RMS) Voltage and the RMS Current. This can also be defined as the resultant of real and reactive power. It is expressed in kVA

Apparent Power = Voltage (V) x Current (I)

**Relationship between Real, Reactive and Apparent power** :

Real Power = Apparent Power x cosΦ

Reactive Power = Apparent Power x sinΦ

(kVA)² = (kW)² + (kVAr)²

**Power Factor (pf)** :

The ratio of the real power to the apparent power in a circuit is called the power factor.

Power Factor = Real Power/Apparent Power

   **Total energy cost = (Power in watts/1000) × hours operating × cost per kWh**

    *** measurement we get is current in Amperes *** x with input Voltage (defined in app_config.h)  = Power in Watt (for that instance)

    =>  Energy = { Input Volt * Current Measured /1000  * (1/3600)sec } Killo Watt Hours 

    =>  Energy = { Input Volt * Current Measured /1000  * (1/3600*1000)millis } Killo Watt Hours 

    ** Note ** Time between 02 readings = 4135 millis

    =>  Energy = { Input Volt * Current Measured /1000  * (1/3600*1000*4135)millis } Killo Watt Hours

## MQTT commands 

  1.   To subscribe 
            mosquitto_sub -h broker.hivemq.com -p 1883 -t iot-2/evt/status/fmt/json

  1.   To update configuration: 
            mosquitto_pub -h broker.hivemq.com -t iot-2/cmd/device/fmt/json -m "{\"uniqueID\":3940033088,\"action\":\"UPDATE\",\"config\":{\"VCC\":2.7,\"ORG\":\"reqofj\",\"PUBLISH_ON\":2000}}" -q 1

  2.   To print all configurations
             mosquitto_pub -h broker.hivemq.com -t iot-2/cmd/device/fmt/json -m "{\"uniqueID\":3940033088,\"action\":\"SHOW_CONFIG\"}" -q 1




    
