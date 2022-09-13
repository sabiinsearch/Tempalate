## Code for Water Tank Receiver End ##
   ### Energy Calculation ###
  
current consumed is saved in every reading . As the other fields are constant (Input Voltage, and other literal values ), these could be multiplied / divided to get the final consumption later.

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



    
