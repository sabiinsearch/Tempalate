#include <ArduinoJson.h>
#include "connectionManager.h"
#include "Preferences.h"

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
char server[] = SERVER;
char mqttUser[] = MQTT_USER;
char mqttPassword[] = MQTT_PASSWORD;


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

bool getWiFi_Availability(connectionManager* conMgr) {

            return conMgr->config.wifiAvailabililty;
}

/**
 * Connect to MQTT Server
 */
 bool connectMQTT(connectionManager * con) {
  
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
    con->mqtt_status = pub_sub_client.connect((char*) clientId.c_str(), con->config.mqtt_user, con->config.mqtt_pwd);
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

//  mqtt methods
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived in topic [");
  Serial.print(topic);
  Serial.print("] ");


   StaticJsonDocument<200> jsonData;
   DeserializationError error = deserializeJson(jsonData, payload);

  // StaticJsonBuffer<200> mqttDataBuffer;
  // JsonObject& jsonData = mqttDataBuffer.parseObject(payload);
  Serial.print(" >>> type: ");
  Serial.print(jsonData["type"].as<String>());
  Serial.print(", uniqueId: ");
  Serial.print(jsonData["uniqueId"].as<String>());
  Serial.print(", deviceIndex: ");
  Serial.print(jsonData["deviceIndex"].as<int>());
  Serial.print(", deviceValue: ");
  Serial.println(jsonData["deviceValue"].as<int>());

  if(jsonData["type"].as<String>() == BOARD_TYPE && jsonData["uniqueId"].as<String>() == BOARD_ID){
    Serial.println("<<<< SWITCH ACTION ON BOARD MATCHES >>>>");
    int deviceIndex = jsonData["deviceIndex"].as<int>();
    int deviceValue = jsonData["deviceValue"].as<int>();

    int deviceAction = 1;
    if(deviceValue == 1){
      deviceAction = 0;
    }

    switch (deviceIndex) {
      case 1:
          digitalWrite(SW_pin, deviceAction);          
          // switch_value = deviceAction;
        break;
      default:
        Serial.println("Device index not matched .... ");
      }
   }
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

   bool published = false;
   
     if(pub_sub_client.publish(pub_topic.c_str(), (char*) data.c_str())){
       Serial.print("Published payload to Topic[");
       Serial.print(pub_topic);
       Serial.print("]: ");
       Serial.println(data);
       published = true;
     }else{
       Serial.print("Publish failed: \t");
          if (!!!pub_sub_client.connected() && MQTT_AVAILABILITY) {
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

void publishData(String data, connectionManager * con) {
 
    if(con->radio_status) {
      publishOnRadio(data,con);
    }
    Serial.print(F("Mqtt_status.."));
    Serial.println(con->mqtt_status);
    if (con->mqtt_status) {
       publishOnMqtt(data, con);
    } 
       
}
     
void initConfig(connectionManager* conMgr) {

    String       org_local;
    String       boardType_local;
    String       token_local;
    String       server_local;
    String       pub_topoic_local;
    String       sub_topoic_local;
    String       mqtt_user_local;
    String       mqtt_pwd_local;


  // Initiate Preferences for saving 
  preferences.begin("app_config",false);


     conMgr->config.tankCapacity = preferences.getInt("TANK_CAPACITY"); 

         if(conMgr->config.tankCapacity<0) {
         conMgr->config.tankCapacity = TANK_CAPACITY;
         preferences.putInt("TANK_CAPACITY",conMgr->config.tankCapacity);
         
     }
    
     
     conMgr->config.calibrationFactor = preferences.getInt("CALIBRATION_FACTOR");

     if(conMgr->config.calibrationFactor<0) {
         conMgr->config.calibrationFactor = CALIBRATION_FACTOR;
         preferences.putInt("CALIBRATION_FACTOR",conMgr->config.calibrationFactor);
     }

     conMgr->config.radioAvailability = preferences.getBool("RADIO_AVAILABILITY");

     if(conMgr->config.radioAvailability<0) {
         conMgr->config.radioAvailability = RADIO_AVAILABILITY;
         preferences.putBool("RADIO_AVAILABILITY",conMgr->config.radioAvailability);
     }

     conMgr->config.wifiAvailabililty = preferences.getBool("WIFI_AVAILABILITY"); 

     if(!(conMgr->config.wifiAvailabililty==false) || !(conMgr->config.wifiAvailabililty==true)) {
         conMgr->config.wifiAvailabililty = WIFI_AVAILABILITY;
         preferences.putBool("WIFI_AVAILABILITY",conMgr->config.wifiAvailabililty);
     }
     Serial.print(F("WiFi_Availability set as "));
     Serial.println(conMgr->config.wifiAvailabililty);

     conMgr->config.mqttAvailability = preferences.getBool("MQTT_AVAILABILITY"); 

     if(conMgr->config.mqttAvailability<=0) {
         conMgr->config.mqttAvailability = MQTT_AVAILABILITY;
         preferences.putBool("MQTT_AVAILABILITY",conMgr->config.mqttAvailability);
     }

     Serial.print(F("Mqtt_Availability set as "));
     Serial.println(conMgr->config.mqttAvailability);

     conMgr->config.publishON = preferences.getInt("PUBLISH_INTERVAL_ON"); 

     if(conMgr->config.publishON<0) {
         conMgr->config.publishON = PUBLISH_INTERVAL_ON;
         preferences.putInt("PUBLISH_INTERVAL_ON", conMgr->config.publishON);
     }

     conMgr->config.publishOFF = preferences.getInt("PUBLISH_INTERVAL_OFF"); 

     if(conMgr->config.publishOFF<0) {
         conMgr->config.publishOFF = PUBLISH_INTERVAL_OFF;
         preferences.putBool("PUBLISH_INTERVAL_OFF",conMgr->config.publishOFF);
     }

     conMgr->config.voltageIn = preferences.getInt("VOLTAGE_IN"); 

     if(conMgr->config.voltageIn<0) {
         conMgr->config.voltageIn = VOLTAGE_IN;
         preferences.putInt("VOLTAGE_IN",conMgr->config.voltageIn);
     }
     conMgr->config.vcc = preferences.getInt("VCC"); 

     if(conMgr->config.vcc<0) {
        conMgr->config.vcc = VCC;
        preferences.putInt("VCC",conMgr->config.vcc);
     }
   
     conMgr->config.senstivity = preferences.getInt("SENSTIVITY"); 

     if(conMgr->config.senstivity<0) {
        conMgr->config.senstivity = SENSTIVITY;
        preferences.putInt("SENSTIVITY",conMgr->config.senstivity);
     }

     conMgr->config.powerFactor = preferences.getInt("PF"); 

     if(conMgr->config.powerFactor<0) {
        conMgr->config.powerFactor = PF;
        preferences.putInt("PF", conMgr->config.powerFactor);
     }

       org_local = preferences.getString("ORG"); 
       conMgr->config.org = org_local.c_str() ;
       if((sizeof(conMgr->config.org) == 0)) {
         conMgr->config.org = ORG;
         preferences.putString("ORG", conMgr->config.org);
       }
 
      boardType_local = preferences.getString("BOARD_TYPE"); 
      conMgr->config.boardType = boardType_local.c_str();
      if((sizeof(conMgr->config.boardType)==0)) {
         conMgr->config.boardType = BOARD_TYPE;
         preferences.putString("BOARD_TYPE",conMgr->config.boardType);
      }

      token_local = preferences.getString("TOKEN"); 
      conMgr->config.token = token_local.c_str();
      if((sizeof(conMgr->config.token)==0)) {
         conMgr->config.token = TOKEN;
         preferences.putString("TOKEN", conMgr->config.token);
      }

      server_local = preferences.getString("SERVER"); 
      conMgr->config.server = server_local.c_str();
      if((sizeof(conMgr->config.server)==0)) {
         conMgr->config.server = SERVER;
         preferences.putString("SERVER", conMgr->config.server);
      }

      mqtt_user_local = preferences.getString("MQTT_USER"); 
      conMgr->config.mqtt_user = mqtt_user_local.c_str();
      if((sizeof(conMgr->config.mqtt_user)==0)) {
         conMgr->config.mqtt_user = MQTT_USER;
         preferences.putString("MQTT_USER", conMgr->config.mqtt_user);
      }

      mqtt_pwd_local = preferences.getString("MQTT_PASSWORD"); 
      conMgr->config.mqtt_pwd = mqtt_pwd_local.c_str();

      if((sizeof(conMgr->config.mqtt_pwd)==0)) {
         conMgr->config.mqtt_pwd= MQTT_PASSWORD;
         preferences.putString("MQTT_PASSWORD", conMgr->config.mqtt_pwd);
      }

       preferences.end();
      Serial.println(F("Configuration set.."));
}

void checkConnections_and_reconnect(void * pvParameters) { 
    
    connectionManager* conMgr = (connectionManager*)pvParameters; 
    Serial.print("checking connection set @ Core..");
    Serial.println(xPortGetCoreID());
    // Serial.print("\t");
    // Serial.print("wifi : ");
    // Serial.println(appMgr->conManager->wifi_manager.getWLStatusString());

   if(RADIO_AVAILABILITY){
      initRadio(conMgr);
      Serial.print(" Ready to print ");
   }
      if(WIFI_AVAILABILITY) {
        initWiFi();
      }

    for(;;) {
      //;
      //Serial.println(F(" checking connection..."));
      if((conMgr->config.wifiAvailabililty==true) && ((conMgr->wifi_manager.getWLStatusString()) == "WL_DISCONNECTED")) {
        Serial.print("Wifi status..");
        Serial.println(conMgr->wifi_manager.getWLStatusString());
        digitalWrite(WIFI_LED,HIGH);
        conMgr->Wifi_status = connectWiFi(conMgr);
      }

      Serial.print(F("Connection Details.. WiFi_Availabiity: "));
      Serial.print(conMgr->config.wifiAvailabililty);
      Serial.print(F(" Status String: "));
      Serial.print(conMgr->wifi_manager.getWLStatusString());

      Serial.print(F(" MQTT_Availability: "));
      Serial.print(conMgr->config.mqttAvailability);
      Serial.print(F(" MQTT_status: "));
      Serial.println(conMgr->mqtt_status);

      if((conMgr->config.mqttAvailability) && (conMgr->Wifi_status) && !(conMgr->mqtt_status)) {
        digitalWrite(MQTT_LED,HIGH);
        conMgr->mqtt_status = connectMQTT(conMgr);
      }
    }
 }


void print_communication() {
     Serial.println("from Communication_lib");
  }