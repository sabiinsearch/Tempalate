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

String sub_topic = SUB_TOPIC;
String pub_topic = PUB_TOPIC;
char server[20] = SERVER;
char mqttUser[20] = MQTT_USER;
char mqttPassword[20] = MQTT_PASSWORD;



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

  //   // Initiate Preferences for fetching few configurations 
  // preferences.begin("app_config",true);

  // String org_local = preferences.getString("ORG","");
  // String board_type_local = preferences.getString("BOARD_TYPE","");
  
  // String sub_topic_local = preferences.getString("SUB_TOPIC","");
  // String pub_topic_local = preferences.getString("PUB_TOPIC","");
  // String server_local = preferences.getString("SERVER","");
  // String mqttUser_local = preferences.getString("MQTT_USER","");
  // String mqttPassword_local = preferences.getString("MQTT_PWD","");

  // preferences.end();

  
  if(con->Wifi_status){
    if(BOARD_ID == ""){
      BOARD_ID = "HB_" +String(getBoard_ID());  
    }
    // BOARD_ID = "HB_2552610648";
    
    String clientId = "d:" ORG ":" BOARD_TYPE ":" +BOARD_ID;
    Serial.print("Connecting MQTT client: ");
    Serial.println(clientId);
    // mqttConnected = client.connect((char*) clientId.c_str(), token, "");
  //  pub_sub_client.username_pw_set(mqttUser, mqttPassword);
    pub_sub_client.setServer(server, 1883);
    pub_sub_client.setCallback(mqttCallback);
    con->mqtt_status = pub_sub_client.connect((char*) clientId.c_str(), mqttUser, mqttPassword);
    Serial.println("MQTT Status: >>>> ");
    Serial.print(pub_sub_client.state());
          
    if(con->mqtt_status){
      digitalWrite(MQTT_LED,LOW);   
      pub_sub_client.subscribe(sub_topic.c_str());
      Serial.print("Subscribed to : >>  ");
      Serial.println(sub_topic);
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

void saveConfig_str(const char* key, const char* value) {

   preferences.begin("app_config", false);
   preferences.remove(key);
   preferences.putString(key, value);
//   Serial.println(F(" value saved.."));
   preferences.end();


}

void saveConfig_long(const char* key, long value) {

   preferences.begin("app_config", false);

   preferences.remove(key);
   preferences.putLong64(key, value);
//   Serial.println(F(" value saved.."));
   preferences.end();

}

void saveConfig_bool(const char* key, bool value) {

   preferences.begin("app_config", false);

   preferences.remove(key);
   preferences.putBool(key, value);
//   Serial.println(F(" value saved.."));
   preferences.end();

}

void saveConfig_float(const char* key, float value) {

   preferences.begin("app_config", false);

   preferences.remove(key);
   preferences.putFloat(key, value);
//   Serial.println(F(" value saved.."));
   preferences.end();

}

//  mqtt methods
void mqttCallback(char* topic, byte* payload, unsigned int length) {

   
   StaticJsonDocument<200> jsonData;
   DeserializationError error = deserializeJson(jsonData, payload);

  // Serial.print(F(" Received JSON: "));
  // serializeJson(jsonData, Serial);

    if(BOARD_ID == ""){
      BOARD_ID = "HB_" +String(getBoard_ID());  
    }

    String uniqueID = String(getBoard_ID());

    if((jsonData["uniqueID"].as<String>() == uniqueID)) {

        int i=0;   // for iteration of keys

        String action = jsonData["action"].as<String>();        
//        Serial.print("Message arrived to ");

        if(strcmp(action.c_str(),"SHOW_CONFIG")==0) {
              showPreferences();    // print all config
        }
     
        if(strcmp(action.c_str(),"UPDATE")==0) {
//           Serial.println(action);                 
           
           JsonObject configuration = jsonData["config"].as<JsonObject>();
            
            const char* keys[configuration.size()];  

                for (JsonPair kv : configuration) {
                  
                   keys[i] = kv.key().c_str();

                  if((strcmp(keys[i],"TANK_CAPACITY")==0) ||
                     (strcmp(keys[i],"CAL_FACT")==0) || 
                     (strcmp(keys[i],"PUBLISH_ON")==0) || 
                     (strcmp(keys[i],"PUBLISH_OFF")==0) ||
                     (strcmp(keys[i],"VOLTAGE_IN")==0) ||
                     (strcmp(keys[i],"SENSTIVITY")==0) ||
                     (strcmp(keys[i],"PF")==0)) {

                    Serial.print(F(" Updating "));
                    Serial.print(keys[i]);
                    Serial.print(F(" : "));
                    Serial.println(kv.value().as<long>());

                    saveConfig_long(keys[i], kv.value().as<long>());
                  }

                  if(strcmp(keys[i],"VCC")==0) {
                    
                    Serial.print(F(" Updating "));
                    Serial.print(keys[i]);
                    Serial.print(F(" : "));
                    Serial.println(kv.value().as<float>());
                    saveConfig_float(keys[i], kv.value().as<float>());
                  }


                  if((strcmp(keys[i],"RADIO_AVAIL")==0) ||
                     (strcmp(keys[i],"BLE_AVAIL")==0) || 
                     (strcmp(keys[i],"WIFI_AVAIL")==0) ||                      
                     (strcmp(keys[i],"MQTT_AVAIL")==0)) {
                    
                    Serial.print(F(" Updating "));
                    Serial.print(keys[i]);
                    Serial.print(F(" : "));
                    Serial.println(kv.value().as<bool>());
                    saveConfig_bool(keys[i], long(kv.value().as<bool>()));
                  }

                  if((strcmp(keys[i],"ORG")==0) ||
                     (strcmp(keys[i],"BOARD_TYPE")==0) || 
                     (strcmp(keys[i],"TOKEN")==0) ||
                     (strcmp(keys[i],"SERVER")==0) ||
                     (strcmp(keys[i],"PUB_TOPIC")==0) ||
                     (strcmp(keys[i],"SUB_TOPIC")==0) ||
                     (strcmp(keys[i],"MQTT_USER")==0) ||
                     (strcmp(keys[i],"MQTT_PWD")==0) ||                      
                     (strcmp(keys[i],"MQTT_PWD")==0)) {
                    
                    Serial.print(F(" Can not Update.. "));
                    Serial.print(keys[i]);
                    Serial.print(F(" : "));
                    Serial.println(kv.value().as<String>());
//                    saveConfig_str(keys[i], kv.value().as<const char*>());
                  }
          
                  // keys[i] = kv.key().c_str();
                  // Serial.print(keys[i]);
                  // Serial.println(kv.value().as<long>());
                  i++;
//                  keys[i++] = (kv.key().c_str());
//                    Serial.print(keys[i]);
               }

                // for(int j=0; j<sizeof(keys); j++) {
                //    Serial.print(keys[j]);
                //    Serial.print(F("\t"));
                // }

              }              
//           serializeJson(configuration, Serial);           
         
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

  bool mqtt_availabilty_local;

  // Initiate Preferences for fetching few config parameters 
  preferences.begin("app_config",true);
  
  mqtt_availabilty_local = preferences.getBool("MQTT_AVAIL","");

  preferences.end();


   bool published = false;
   
     if(pub_sub_client.publish(pub_topic.c_str(), (char*) data.c_str())){
       Serial.print("Published payload to Topic[");
       Serial.print(pub_topic);
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
     


void print_communication() {
     Serial.println("from Communication_lib");
  }