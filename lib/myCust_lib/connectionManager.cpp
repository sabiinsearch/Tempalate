#include <ArduinoJson.h>
#include "connectionManager.h"
#include "Preferences.h"
#include "appManager.h"

// for WiFi, LoRa and mqtt

#include <WiFiManager.h> 
#include "WiFi.h"
#include "WiFiGeneric.h"
#include "WiFiSTA.h"


#include <LoRa.h>
#include <PubSubClient.h>   // for Mqtt

#include "app_config.h"     // for Custom Configration
#include "receiverBoard.h"


#define BAND    433E6
#define SCK     5
#define MISO    19
#define MOSI    27
#define CS      18

#define SS      18
#define RST     14
#define DI0     26

/*  */

Preferences preferences;

String BOARD_ID;
WiFiClient wifiClient;
// const char* mqttServer = SERVER;
// PubSubClient pub_sub_client(mqttServer, 1883, NULL, wifiClient);
PubSubClient pub_sub_client(wifiClient);

WiFiManager wm; // WiFi Manager 




/* constructor implementation */

connectionManager * const connectionManager_ctor(connectionManager * const me ) {

  //  me->config.mqtt_user = MQTT_USER;
  //  me->config.mqtt_pwd = MQTT_PASSWORD;
   initConfig( me);
   return me;
}

/* Function implementation */

void mqtt_loop(){
  pub_sub_client.loop();
}

void initWiFi() {
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP  
  //wm.setWiFiAutoReconnect(true);
}

void print_default_config();  // Implementation below

void showPreferences();   // For Troubleshooting..

bool getWiFi_Availability(connectionManager* conMgr) {
     
     bool availability;   
  // Initiate Preferences for fetching few configurations 
  preferences.begin("app_config",false);

     availability = preferences.getBool("WIFI_AVAILIABILITY");
     
            return availability;
}

/**
 * Connect to MQTT Server
 */
 bool connectMQTT(connectionManager * con) {

    // Initiate Preferences for fetching few configurations 
  preferences.begin("app_config",true);

  String org_local = preferences.getString("ORG","");
  String board_type_local = preferences.getString("BOARD_TYPE","");
  
  String sub_topic_local = preferences.getString("SUB_TOPIC","");
  String pub_topic_local = preferences.getString("PUB_TOPIC","");
  String server_local = preferences.getString("SERVER","");
  String mqttUser_local = preferences.getString("MQTT_USER","");
  String mqttPassword_local = preferences.getString("MQTT_PWD","");

  preferences.end();

  
  if(con->Wifi_status){
    if(BOARD_ID == ""){
      BOARD_ID = "HB_" +String(getBoard_ID());  
    }
    // BOARD_ID = "HB_2552610648";
    
    String clientId = "d:"+org_local+":"+board_type_local+":" +BOARD_ID;
    Serial.print("Connecting MQTT client: ");
    Serial.println(clientId);
    // mqttConnected = client.connect((char*) clientId.c_str(), token, "");
  //  pub_sub_client.username_pw_set(mqttUser, mqttPassword);
    pub_sub_client.setServer(server_local.c_str(), 1883);
    pub_sub_client.setCallback(mqttCallback);
    con->mqtt_status = pub_sub_client.connect((char*) clientId.c_str(), mqttUser_local.c_str(), mqttPassword_local.c_str());
    // Serial.println("MQTT Status: >>>> ");
    // Serial.print(pub_sub_client.state());
          
    if(con->mqtt_status){
      digitalWrite(MQTT_LED,LOW);   
      pub_sub_client.subscribe(sub_topic_local.c_str());
      Serial.print("Subscribed to : >>>>  ");
      Serial.println(sub_topic_local);
    }else {
      digitalWrite(MQTT_LED,HIGH);
      Serial.print("Error connecting to MQTT, state: ");
      Serial.println(pub_sub_client.state());
      // delay(5000);
    }
     
     con->mqtt_status = true;
     // Serial.println(mqttConnected);
  }else{
    digitalWrite(MQTT_LED,HIGH);
    Serial.println("Cannot connect to MQTT as WiFi is not Connected !!");
  }
  return con->mqtt_status;
}

void reconnectWiFi(connectionManager  * con){
  bool res;
  res = wm.autoConnect("Tank_Board"); // anonymous ap
    if(!res) {
        con->Wifi_status = false;
        digitalWrite(WIFI_LED,HIGH);
        Serial.println("Failed to connect");
        delay(3000);
      //  ESP.restart();
        delay(5000);
    } 
    else {
        //if you get here you have connected to the WiFi  
        digitalWrite(WIFI_LED,LOW);  
        con->Wifi_status = true;   
      //  Serial.println("Wifi connected...yeey :)");       
    }
}

bool connectWiFi(connectionManager * con) {
  bool res;
  digitalWrite(HEARTBEAT_LED,LOW);  
  wm.setConnectTimeout(120);
  res = wm.autoConnect("Tank"); // auto generated AP name from chipid
  
    if(res) {
      //if you get here you have connected to the WiFi         
        con->Wifi_status = true;
        digitalWrite(HEARTBEAT_LED,HIGH);
        digitalWrite(WIFI_LED,LOW);   
      //  Serial.println("Wifi connected...yeey :)");           
    }
    return res;
}

void resetWifi(connectionManager * con) {
    con->Wifi_status = false;
    wm.resetSettings(); // reset settings - wipe stored credentials for testing, these are stored by the esp library
    digitalWrite(WIFI_LED,HIGH);
}

void initRadio(connectionManager * con){
  SPI.begin(SCK, MISO, MOSI, CS);
  LoRa.setPins(SS, RST, DI0);
      delay(1000);

      int radioTryCount = 0;      
      do{
        con->radio_status = LoRa.begin(BAND);
        radioTryCount++;
        if(!con->radio_status){
          Serial.printf("Starting Radio failed!, Try Count: %d\n", radioTryCount);
          delay(3000);
        }else{
          Serial.println("Radio Initialized Successfully...");
        }
      }while(!con->radio_status && radioTryCount < 3);

}

char* string2char(String str){
  char *p;
    if(str.length()!=0) {
        p = const_cast<char*>(str.c_str());
    }
    return p;
}

 void publishOnRadio(String data, connectionManager * con){
    // bool published = false;

    // if(con->radio_status && !published){
    //     LoRa.beginPacket();

    //     LoRa.print(data);
    //     LoRa.print("\n");
    //     LoRa.endPacket();

    //     delay(1);
    //     LoRa.flush();
    // }else{
    //    Serial.print("Radio Not Available: >> ");
    // }
}

void checkDataOnRadio(){
  String receivedText;
  // try to parse packet
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
        // received a packet
        // Serial.print("Received packet '");
        // read packet
        while (LoRa.available()) {
          receivedText = (char)LoRa.read();
          Serial.print(receivedText);
        }

        // print RSSI of packet
        // Serial.print("' with RSSI ");
        //// Serial.println(LoRa.packetRssi());
    }
}

//  mqtt methods
void mqttCallback(char* topic, byte* payload, unsigned int length) {

   
   StaticJsonDocument<200> jsonData;
   DeserializationError error = deserializeJson(jsonData, payload);

   Serial.print(F(" Received JSON: "));
   serializeJson(jsonData, Serial);

    if(BOARD_ID == ""){
      BOARD_ID = "HB_" +String(getBoard_ID());  
    }

    String uniqueID = String(getBoard_ID());

    if((jsonData["uniqueID"].as<String>() == uniqueID)) {
        String action = jsonData["action"].as<String>();
        Serial.print("Message arrived to ");
     
        if(strcmp(action.c_str(),"UPDATE")==0) {
           Serial.println(action); 

           JsonObject configuration = jsonData["config"].as<JsonObject>();
           serializeJson(configuration, Serial);           
         
        }
    }
               
  // StaticJsonBuffer<200> mqttDataBuffer;
  // JsonObject& jsonData = mqttDataBuffer.parseObject(payload);
  // Serial.print(" >>> type: ");
  // Serial.print(jsonData["type"].as<String>());
  // Serial.print(", uniqueId: ");
  // Serial.print(jsonData["uniqueId"].as<String>());
  // Serial.print(", deviceIndex: ");
  // Serial.print(jsonData["deviceIndex"].as<int>());
  // Serial.print(", deviceValue: ");
  // Serial.println(jsonData["deviceValue"].as<int>());

  // if(jsonData["type"].as<String>() == board_type_local && jsonData["uniqueId"].as<String>() == BOARD_ID){
  //   Serial.println("<<<< SWITCH ACTION ON BOARD MATCHES >>>>");
  //   int deviceIndex = jsonData["deviceIndex"].as<int>();
  //   int deviceValue = jsonData["deviceValue"].as<int>();

  //   int deviceAction = 1;
  //   if(deviceValue == 1){
  //     deviceAction = 0;
  //   }

  //   switch (deviceIndex) {
  //     case 1:
  //         digitalWrite(SW_pin, deviceAction);          
  //         // switch_value = deviceAction;
  //       break;
  //     default:
  //       Serial.println("Device index not matched .... ");
  //     }
  //  }
   jsonData.clear();
}

/**
 * Create unique device name from MAC address
 **/
/*
void createName() {
	uint8_t baseMac[6];
	// Get MAC address for WiFi station
	esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
	// Write unique name into apName
	sprintf(apName, "SB_MICRO-%02X%02X%02X%02X%02X%02X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);

  BOARD_ID = String(apName);
  // pub_topic = PUBSUB_PREFIX + BOARD_ID +"/evt/cloud/fmt/json";
  // sub_topic = PUBSUB_PREFIX + BOARD_ID +"/cmd/device/fmt/json";
  // strcat(sub_topic, BOARD_ID.c_str() );
  // strcat(pub_topic, BOARD_ID.c_str() );
}
*/



void publishOnMqtt(String data, connectionManager * con) {
//  Serial.println(F("For publish on mqtt"));   // log
  String pub_topic_local;
  bool mqtt_availabilty_local;

  // Initiate Preferences for fetching few config parameters 
  preferences.begin("app_config",true);
  
  pub_topic_local = preferences.getString("PUB_TOPIC","");
  mqtt_availabilty_local = preferences.getBool("MQTT_AVAIL","");

  preferences.end();


   bool published = false;
   
     if(pub_sub_client.publish(pub_topic_local.c_str(), (char*) data.c_str())){
       Serial.print("Published payload to Topic[");
       Serial.print(pub_topic_local);
       Serial.print("]: ");
       Serial.println(data);
       published = true;
     }else{
       Serial.print("Publish failed: \t");
          if (!!!pub_sub_client.connected() && mqtt_availabilty_local) {
            Serial.print(" Wifi : ");
            Serial.print(con->Wifi_status);
            Serial.print("\t");
            Serial.print(" mqtt : ");
            Serial.println(con->mqtt_status);

            connectMQTT(con);
          }
       // Serial.println(data);
     }
  
}

void publishData(String data, connectionManager* con) {
 //  Serial.print(F(" con->mqtt_status "));   // log
//      Serial.println(con->mqtt_status);

   preferences.begin("app_config",false);


    if(preferences.getBool("RADIO_AVAIL",false)) {
//      Serial.println(F("For publish on radio"));   // log
      publishOnRadio(data,con);
    }
    // Serial.print(F("Mqtt_status.."));
    // Serial.println(con->mqtt_status);
    if (con->mqtt_status) {
//      Serial.println(F("Publish Data: For publish on mqtt"));   // log
       publishOnMqtt(data, con);
    } 
       
}
     
void initConfig(connectionManager* conMgr) {


  // Initiate Preferences for saving 
  preferences.begin("app_config",false);

    //  int tank_cap = TANK_CAPACITY;
    //  int calibrationFactor = CALIBRATION_FACTOR;

     bool radioAvailability = RADIO_AVAILABILITY;
     bool wifiAvailibility = WIFI_AVAILABILITY;
     bool mqttAvailibility = MQTT_AVAILABILITY;
     
     if((preferences.getLong("TANK_CAPACITY",0))==0) {         
         preferences.putLong("TANK_CAPACITY",TANK_CAPACITY);         
     }
    
     
     if((preferences.getLong64("CAL_FACT",0))==0) {         
         preferences.putLong64("CAL_FACT",CALIBRATION_FACTOR);
     }

     
     if((preferences.getBool("RADIO_AVAIL",false))) {
         preferences.putBool("RADIO_AVAIL",RADIO_AVAILABILITY);
     }

     if(!(preferences.getBool("WIFI_AVAIL",false))) {         
         preferences.putBool("WIFI_AVAIL",WIFI_AVAILABILITY);
     }

    //  Serial.print(F("WiFi_Availability set as "));
    //  Serial.println(WIFI_AVAILABILITY);

     if(!(preferences.getBool("MQTT_AVAIL",false))) {
         preferences.putBool("MQTT_AVAIL",MQTT_AVAILABILITY);
     }

    //  Serial.print(F("Mqtt_Availability set as "));
    //  Serial.println(MQTT_AVAILABILITY);

     if((preferences.getLong64("PUBLISH_ON",0))==0) {         
         preferences.putLong64("PUBLISH_ON", PUBLISH_INTERVAL_ON);
     }

     if((preferences.getLong64("PUBLISH_OFF",0))==0) {
         preferences.putLong64("PUBLISH_OFF",PUBLISH_INTERVAL_OFF);
     }
     
     if((preferences.getLong("VOLTAGE_IN",0))==0) {         
         preferences.putLong("VOLTAGE_IN",VOLTAGE_IN);         
     }

     if((preferences.getFloat("VCC",0))==0) {      
        preferences.putFloat("VCC",VCC);
     }
        
     if((preferences.getLong64("SENSTIVITY",0))==0) {        
        preferences.putLong64("SENSTIVITY",SENSTIVITY);
     }
     
     if((preferences.getLong("PF",0))==0) {        
        preferences.putLong("PF", PF);
     }
       
     if((sizeof(preferences.getString("ORG","")) > 0)) {
         preferences.putString("ORG", ORG);
     }
       
    if((sizeof(preferences.getString("BOARD_TYPE",""))>0)) {         
         preferences.putString("BOARD_TYPE",BOARD_TYPE);
    }

      
    if((sizeof(preferences.getString("TOKEN",""))>0)) {         
         preferences.putString("TOKEN", TOKEN);
    }

      
    if((sizeof(preferences.getString("SERVER",""))>0)) {         
         preferences.putString("SERVER", SERVER);
    }

    if((sizeof(preferences.getString("PUB_TOPIC",""))>0)) {         
         preferences.putString("PUB_TOPIC", PUB_TOPIC);
    }

    if((sizeof(preferences.getString("SUB_TOPIC",""))>0)) {         
         preferences.putString("SUB_TOPIC", SUB_TOPIC);
    }


    if((sizeof(preferences.getString("MQTT_USER",""))>0)) {         
         preferences.putString("MQTT_USER", MQTT_USER);
    }

    if((sizeof(preferences.getString("MQTT_PWD",""))>0)) {         
         preferences.putString("MQTT_PWD", MQTT_PASSWORD);
    }
    
      preferences.end();
      Serial.println(F("Configuration set..  "));

//      showPreferences();
      
}
  
  void showPreferences() {             

     preferences.begin("app_config",false);
     
     Serial.print(F(" TANK_CAPACITY "));
     Serial.print(preferences.getLong("TANK_CAPACITY"));
     Serial.print(F(" \t"));

     Serial.print(F(" CAL_FACT "));
     Serial.print(preferences.getLong64("CAL_FACT"));
     Serial.print(F(" \t"));

     Serial.print(F(" RADIO_AVAILABILITY "));
     Serial.print(preferences.getBool("RADIO_AVAIL"));
     Serial.print(F(" \t"));

     Serial.print(F(" WIFI_AVAILIABILITY "));
     Serial.print(preferences.getBool("WIFI_AVAIL"));
     Serial.print(F(" \t"));

     Serial.print(F(" MQTT_AVAILIABILITY "));
     Serial.print(preferences.getBool("MQTT_AVAIL"));
     Serial.print(F(" \t"));

     Serial.print(F(" PUBLISH_ON "));
     Serial.print(preferences.getLong64("PUBLISH_ON"));
     Serial.print(F(" \t"));

     Serial.print(F(" PUBLISH_OFF "));
     Serial.print(preferences.getLong64("PUBLISH_OFF"));
     Serial.print(F(" \t"));

     Serial.print(F(" VOLTAGE_IN "));
     Serial.print(preferences.getLong("VOLTAGE_IN"));
     Serial.print(F(" \t"));

     Serial.print(F(" VCC "));
     Serial.print(preferences.getFloat("VCC"));
     Serial.print(F(" \t"));

     Serial.print(F(" SENSTIVITY "));
     Serial.print(preferences.getLong64("SENSTIVITY"));
     Serial.print(F(" \t"));

     Serial.print(F(" PF "));
     Serial.print(preferences.getLong("PF"));
     Serial.print(F(" \t"));

     Serial.print(F(" ORG "));
     Serial.print(preferences.getString("ORG",""));
     Serial.print(F(" \t"));

     Serial.print(F(" BOARD_TYPE "));
     Serial.print(preferences.getString("BOARD_TYPE",""));
     Serial.print(F(" \t"));

     Serial.print(F(" TOKEN "));
     Serial.print(preferences.getString("TOKEN",""));
     Serial.print(F(" \t"));
     
     Serial.print(F(" SERVER "));
     Serial.print(preferences.getString("SERVER",""));
     Serial.print(F(" \t"));

     Serial.print(F(" PUB_TOPIC "));
     Serial.print(preferences.getString("PUB_TOPIC",""));
     Serial.print(F(" \t"));

     Serial.print(F(" SUB_TOPIC "));
     Serial.print(preferences.getString("SUB_TOPIC",""));
     Serial.print(F(" \t"));


     Serial.print(F(" MQTT_USER "));
     Serial.print(preferences.getString("MQTT_USER",""));
     Serial.print(F(" \t"));

     Serial.print(F(" MQTT_PWD "));
     Serial.print(preferences.getString("MQTT_PWD",""));
     Serial.print(F(" \t"));


     preferences.end();
  }

  void print_default_config() {

      Serial.print(F("TANK_CAPACITY "));
      Serial.print(TANK_CAPACITY);
      Serial.print(F(" \n"));

      Serial.print(F("CALIBRATION_FACTOR "));
      Serial.print(CALIBRATION_FACTOR);
      Serial.print(F(" \n"));
      
  }



void checkConnections_and_reconnect(void * pvParameters) { 


  bool radio_ability_local;
  bool wifi_ability_local;
  bool mqtt_ability_local;

  // Initiate Preferences for fetching few config parameters 
  preferences.begin("app_config",false);
  
 
  radio_ability_local = preferences.getBool("RADIO_AVAIL",false);
  wifi_ability_local = preferences.getBool("WIFI_AVAIL",false);
  mqtt_ability_local = preferences.getBool("MQTT_AVAIL",false);

  preferences.end();

    
//    connectionManager* conMgr = (connectionManager*)pvParameters; 
appManager* appMgr = (appManager*)pvParameters; 
    Serial.print("checking connection set @ Core..");
    Serial.println(xPortGetCoreID());
    // Serial.print("\t");
    // Serial.print("wifi : ");
    // Serial.println(appMgr->conManager->wifi_manager.getWLStatusString());

   if(radio_ability_local){
      initRadio(appMgr->conManager);
      Serial.print(" Ready to print ");
   }
      if(wifi_ability_local) {
        initWiFi();
      }

    for(;;) {

      //;
      //Serial.println(F(" checking connection..."));
      if((wifi_ability_local) && ((appMgr->conManager->wifi_manager.getWLStatusString()) == "WL_DISCONNECTED")) {
        Serial.print("Wifi status..");
        Serial.println(appMgr->conManager->wifi_manager.getWLStatusString());
        digitalWrite(WIFI_LED,HIGH);
        appMgr->conManager->Wifi_status = connectWiFi(appMgr->conManager);
        delay(1000);
      }

      // Serial.print(F("Connection Details.. WiFi_Availabiity: "));
      // Serial.print(wifi_ability_local);
      // Serial.print(F(" Status String: "));
      // Serial.print(appMgr->conManager->wifi_manager.getWLStatusString());

      // Serial.print(F(" MQTT_Availability: "));
      // Serial.print(mqtt_ability_local);
      // Serial.print(F(" MQTT_status: "));
      // Serial.println(appMgr->conManager->mqtt_status);


      if((mqtt_ability_local) && (appMgr->conManager->Wifi_status) && !(appMgr->conManager->mqtt_status)) {
        digitalWrite(MQTT_LED,HIGH);        
        appMgr->conManager->mqtt_status = connectMQTT(appMgr->conManager);
        Serial.print(F("conMgr->mqtt_status: "));
        Serial.println(appMgr->conManager->mqtt_status);
      }
    }
 }


void print_communication() {
     Serial.println("from Communication_lib");
  }