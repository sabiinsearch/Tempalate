## Code for Water Tank Receiver End ##
   ### Energy Calculation ###

#### Energy Calculatios ####

Watts=Joules/time    (where *watt* is rate at which electric energy is consumed and *Joule* is unit of enegy)
 

   So Power is the Rate at which Energy is consumed

-     1 Joule in 1 sec = 1 Watt

-     1 Joule in 1 millisecond = 1000 Watt = 1 kW

-     1 Joule in microsecond = 1000000W = 1MW

-     1 Watt-hour is equal to 3600 Joules (3.6kJ). One Kilowatt-Hour is the same as 1,000 Watt-hours and is the most common unit used in electrical energy consumption.

**A Watt is the amount of energy (in Joules) that an electrical device (such as a light) is burning per second that it’s running. So a 60W bulb is burning 60 Joules of energy every second you have it turned on**

**In mqtt_publish**

            -   Total current consumed in that time is accomulated and saved then published...

            -   Rest it is to be further Multiplied by Voltage (240 V) 
                Then divided by 1000 (for converting to Kilo Watts )and Multiplying with 3600 for making it in Kilo Watt hour (KWh). 

  
Energy is calculated using reading from sensor and then applying some calculations as per the constants and saved in appManager.

**Note** : try with more than 100 watt device for good result.

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




    
