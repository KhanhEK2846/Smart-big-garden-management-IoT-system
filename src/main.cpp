#include <Arduino.h>
#include <DHT.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <PubSubClient.h>
#include  <FirebaseESP32.h>
#include "addons/RTDBHelper.h"
#include <time.h>
#include <esp_wifi.h>
#include <HTTPClient.h>
#include "Transmit.h"
#include "CommandCode.h"
#include "Plant.h"
#include "URL.h"
#include "html.h"
//Port GPIOs
#define DHTPIN 1 //Read DHT22 Sensor
#define LDR 3 //Read Light Sensor
#define Soil_Moisture 2 //Read Soild Sensor
#define Pumps 4 // Control Pump
#define Light 5 //Control Light
//DHT11 Variable
#define DHTTYPE DHT22 
DHT dht(DHTPIN, DHTTYPE);
float Humidity = 0; 
float Temperature = 0;
//Light Sensor Variable
int lumen = 0; //Store value from LDR
//Soild Sensor Variable
int soilMoist = 0; //Store value from Moisure
//Sensor Check
boolean Soil_Err = false;
boolean Err = false;//Catch Error Sensor
boolean DHT_Err = false;
boolean LDR_Err = false;
//Plant
Plant Tree;
//Days
unsigned long Time_Passed = 0;
const unsigned long A_Day_milis = 86400000;//24 hours
const unsigned long A_Day_timestamp = 86400;//24 hours
//Pump
unsigned long Times_Pumps=0;
const unsigned long Next_Pump = 43200000; //12 hours
unsigned long Still_Pumps = 60000; //Water in 1 minute
boolean PumpsStatus = false; //Current Status Pump
//Light
boolean LightStatus = false; //Current Status Light
//WIFI Variable
// String sta_ssid = "ESP32_Server"; 
// String sta_password = "123456789" ;
// String ap_ssid = "ESP32_Client";
// String ap_password = "123456789";
String sta_ssid = "Sieu Viet 1"; 
String sta_password = "02838474844" ;
String ap_ssid = "ESP32_Server";
String ap_password = "123456789";
const unsigned long Network_TimeOut = 5000;// Wait 5 seconds to Connect Wifi
String Contingency_sta_ssid = ""; 
String Contingency_sta_password = "";
int disconnected_wifi_count = -1;
//Ping
WiFiClient PingClient;
const unsigned long time_delay_to_ping = 300000; // 5 minutes/ping
unsigned long Last_ping_time = 0;
boolean ping_flag = false; //Result Ping
//NTP
unsigned long timestamp;
const long  gmtOffset_sec = 7 * 60 * 60; // UTC +7
const int daylightOffset_sec = 0; //Daylight saving time
//IPAdress AP
IPAddress NMask(255, 255, 255, 0);
IPAddress ApIP(192,168,1,1);
String DeliveryIP = "";
//List Device Connected
wifi_sta_list_t wifi_sta_list; //List of sta connect include MAC
tcpip_adapter_sta_list_t adapter_sta_list; // List of Mac and IP
String KnownIP[4] = {"","","",""};//[0] is gateway, [1-2] is original node, [3] is reserved node 
int FailIP[4] = {0,0,0,0}; //Amount of delivery fail
int MAX_Clients = 2;
int Num_Clients = 0;
//Firebase Variable
FirebaseData firebaseData;
const unsigned long time_delay_send_datalogging = 180000; //3 minutes/Send
unsigned long Last_datalogging_time = 0;
//MQTT Variable
WiFiClient wifiClient;
PubSubClient client(wifiClient);
boolean MQTTStatus = false; //Status Connect Broker
String MQTT_Messange = "";
//Local Server Variable
AsyncWebServer server(80); //Create a web server listening on port 80
AsyncWebSocket ws("/ws");//Create a path for web socket server
AsyncWebHandler* InitWaitGateway = NULL;
int Person = 0; // Number clients access local host
String messanger;
String MessLimit;
const String Local_Path[11] = {"StatusD","StatusL","StatusM","Day","humi","temp","ligh","mois","LED","Pump","MQTT"};
//Command from client
int Command_Pump = 0; // 0: Nothing, 1:ON, 2:OFF
int Command_Light = 0; // 0: Nothing, 1:ON, 2:OFF
boolean Ig_Pump = false;// Ignore Command_Pump
boolean Ig_Led = false;// Ignore Command_Light 
//flag
boolean sta_flag = false;
boolean first_sta = true;
boolean valueChange_flag = false;
boolean contingency_flag = false;
//Type of server
int gateway_node = 0; // 0:default 1: gateway 2:node
//Own & Deliver
Transmit D_Data;
DataPackage D_Pack;
Transmit O_Data;
DataPackage O_Pack;
String O_Command;
String D_Command;
Transmit MQTT_Data;
//Task Delivery Data
TaskHandle_t DeliveryTask = NULL;
TaskHandle_t DatabaseTask = NULL;
SemaphoreHandle_t xMutex_HTTP = NULL;
QueueHandle_t Queue_Delivery = NULL;
QueueHandle_t Queue_Command = NULL;
QueueHandle_t Queue_Database = NULL;
const int Queue_Length = 10;
const unsigned long long Queue_item_delivery_size = sizeof(Transmit);
const unsigned long long Queue_item_command_size = sizeof(String);
const unsigned long long Queue_item_database_size = sizeof(DataPackage);
//Sercurity
String http_username = "admin";
String http_password = "admin";;
String auth_username = "owner";
String auth_password = "owner";
boolean sercurity_backend_key = false;
boolean tolerance_backend_key = false;
const unsigned long reset_key_time = 300000; // 5 minutes to reset
unsigned long before_reset_key = 0;
//ID
const String ID = WiFi.macAddress();
//Loop variable
int i;
// Store Recent Value
int Temp[11] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
#pragma region Common Function
unsigned long getTime() // Get Timestamp
{ 
  time_t now; 
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {return(0);} //Failed to obtain time
  time(&now);
  return now;
}
void Make_Day()//Counter Day
{
  if((unsigned long) (millis()-Time_Passed) >= A_Day_milis){
    Time_Passed = millis();
    Tree.Days ++;
  }
}
#pragma endregion
#pragma region Check Internet Connected from Wifi
void Ping()// Ping to host
{
  if(gateway_node == 1){ //If it's gateway, check Internet
    if (PingClient.connect(PINGInternet, 80)) {
      ping_flag = true;
    } else {
      ping_flag = false;
    }
    PingClient.stop();
  }else{ping_flag =  false;}
}
void Cycle_Ping()// Cycle Ping to Host // FIX:
{
  if(Last_ping_time == 0)
    Last_ping_time = millis();
  if((unsigned long)(millis()- Last_ping_time) > time_delay_to_ping){
    Last_ping_time = millis();
    Ping();
  }
}
#pragma endregion
#pragma region WebSocket Protocol
void notifyClients(const String data) //Notify all local clients with a message
{
  ws.textAll(data);
}
void notifyClient(AsyncWebSocketClient *client)//Notify only one local client
{
  String data ="/";
  data += Tree.Name;
  data += "/";
  data += Local_Path[0];
  data += " ";
  data += String(DHT_Err);
  data += "/";
  data += Local_Path[1];
  data += " ";
  data += String(LDR_Err);
  data += "/";
  data += Local_Path[2];
  data += " ";
  data += String(Soil_Err);
  data += "/";
  data += Local_Path[4];
  data += " ";
  data += String(Humidity);
  data += "/";
  data += Local_Path[5];
  data += " ";
  data += String(Temperature);
  data += "/";
  data += Local_Path[6];
  data += " ";
  data += String(lumen);
  data += "/";  
  data += Local_Path[7];
  data += " ";
  data += String(soilMoist);
  data += "/"; 
  data += Local_Path[8];
  data += " ";
  data += String(LightStatus);
  data += "/";  
  data += Local_Path[9];
  data += " ";
  data += String(PumpsStatus);
  data += "/";
  if(WiFi.status() != WL_CONNECTED)   
    data += "Wifi OFF";
  else if (ping_flag)
    data += "Wifi CONNECT";
  else   
    data += "Wifi ON";
  ws.text(client->id(),data);
  data.clear();
}
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) //Handle messange from local clients
{
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    if(String((char*)data).indexOf("Username:") >= 0){
      sta_ssid = String((char*)data).substring(String((char*)data).indexOf(' ')+1,String((char*)data).length());
    }
    if(String((char*)data).indexOf("Password:") >= 0){
      sta_password = String((char*)data).substring(String((char*)data).indexOf(' ')+1,String((char*)data).length());
      sta_flag = true;
    }
    if(String((char*)data).indexOf("Disconnect") >= 0){
      WiFi.disconnect(true,true);
      WiFi.mode(WIFI_AP);
      if(gateway_node != 0) //If it's not in dafault state
      {
        gateway_node = 0;
        server.removeHandler(InitWaitGateway);
        InitWaitGateway = NULL;
      }
      ping_flag = false;
      notifyClients("Wifi OFF");
    }
    if(String((char*)data).indexOf("Pump") >= 0){
      if(PumpsStatus)
        Command_Pump = 2;
      else
        Command_Pump = 1;
      if(WiFi.status() == WL_CONNECTED)
        Ig_Pump = true;
    }
    if(String((char*)data).indexOf("LED") >= 0){
      if(LightStatus)
        Command_Light = 2;
      else
        Command_Light = 1;
      if(WiFi.status() == WL_CONNECTED)
        Ig_Led = true;
    }
  }
}//Handle messange from local clients
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,void *arg, uint8_t *data, size_t len) //Handle event WebSocket
{
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      Person += 1;
      notifyClient(client);
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
        Person = (Person > 0)? Person-1 : 0;
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}//Handle event WebSocket
void initWebSocket() //Initialize the WebSocket protocol
{
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}
#pragma endregion
#pragma region Device Connected Manager
void List_Connected_Update() //Update the list of device connected to ap wifi
{
  memset(&wifi_sta_list, 0, sizeof(wifi_sta_list));
  memset(&adapter_sta_list, 0, sizeof(adapter_sta_list));
  esp_wifi_ap_get_sta_list(&wifi_sta_list); 
  tcpip_adapter_get_sta_list(&wifi_sta_list, &adapter_sta_list);//Get IP information for stations connected to the Wi-Fi AP interface.
}

String MACAddressCovert(uint8_t* mac)// Convert unit8_t to String MAC
{
    char macStr[18] = { 0 };
    sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0],mac[1], mac[2],mac[3],mac[4],mac[5]);
    return String(macStr);
}
void StringtoMACAddress(const String macString, uint8_t* mac)
{
  sscanf(macString.c_str(), "%02x:%02x:%02x:%02x:%02x:%02x", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
}
int IsKnown(String IP)
{
  if(IP == "")
    return -1;
  for(int i = 0; i<4;i++)
  {
    if(KnownIP[i] == IP)
      return i;
  }
  return -1;
}
String MactoIP(String Mac)
{
  for (int i = 0; i< adapter_sta_list.num; i++)
  {
    if(Mac == MACAddressCovert(adapter_sta_list.sta[i].mac))
    {
      return IPAddress(adapter_sta_list.sta[i].ip.addr).toString();
    }
  }
  return "";
}
String IPtoMAC(String IP)
{
  for (int i = 0; i< adapter_sta_list.num; i++)
  {
    if(IP == IPAddress(adapter_sta_list.sta[i].ip.addr).toString())
    {
      return MACAddressCovert(adapter_sta_list.sta[i].mac);
    }
  }
  return "";
}
void RefreshNodeIP(String locate = "0")
{
  if(locate == "0")
  {
    KnownIP[1] = "";
    KnownIP[2] = "";
    KnownIP[3] = "";
    return;
  }
  for(int index = 0;index<adapter_sta_list.num; index++)//Find where is mac 
  {
    if(locate == MACAddressCovert(adapter_sta_list.sta[index].mac)) 
    {
      for (int index1 = 1; index1 <MAX_Clients+1;index1++) //Find where is IP relative that MAC
      {
        if(KnownIP[index1] == IPAddress(adapter_sta_list.sta[index].ip.addr).toString())
        {
          KnownIP[index1] = "";
          --Num_Clients;
          break;
        }
      }
      break;
    }
  }
}
void First_Mess_To_Node(String IP)// Init client as node
{
  if(Num_Clients < MAX_Clients)
  {
    xSemaphoreTake(xMutex_HTTP,portMAX_DELAY);
    WiFiClient client;
    HTTPClient http;
    String URL = "http://";
    URL += IP;
    URL += "/YouAreNode";
    if(http.begin(client,URL))
    {
      int httpResponseCode = http.POST(Init_Node_Code);
      if(httpResponseCode == Init_Gateway_Code)
      {
        String payload = http.getString();
        if(payload.indexOf("SSID: ") >= 0 && payload.indexOf("Password: ") >= 0)//TODO: Save a table of wifi
        {
          // String temp = payload.substring(payload.indexOf("{\n") + 3, payload.lastIndexOf("\n}"));
          // Local_WiFi[0].SSID = temp.substring(temp.indexOf("SSID: ")+6,temp.indexOf("\nPassword:"));
          // Local_WiFi[0].PASSWORD = temp.substring(temp.indexOf("Password: ")+10,temp.length());
          for(int index = 1; index < MAX_Clients+1; index++)
          {
            if(KnownIP[index] == "")
            {
              KnownIP[index] = IP;
              ++Num_Clients;
              break;
            }
          }
        }

      }
    }
    http.end();
    xSemaphoreGive(xMutex_HTTP);
  }
}
void Client_Connected(WiFiEvent_t event, WiFiEventInfo_t info)
{
  Serial.println("Client connect");
}
void Client_IP(WiFiEvent_t event, WiFiEventInfo_t info)
{
  First_Mess_To_Node(IPAddress(info.wifi_ap_staipassigned.ip.addr).toString());
  List_Connected_Update();
}
void Client_Disconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{ //BUG: Can't handle power off client
  RefreshNodeIP(MACAddressCovert(info.wifi_ap_stadisconnected.mac));
  List_Connected_Update();
  Serial.println("Client disconnect");
}
void Server_Disconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
  Serial.print("Lost WiFi: ");
  Serial.println(info.wifi_sta_disconnected.reason);
  if(info.wifi_sta_disconnected.reason == wifi_err_reason_t::WIFI_REASON_BEACON_TIMEOUT) //Handle AP Server loss
  {
    disconnected_wifi_count = 0;
  }
  if(info.wifi_sta_disconnected.reason == wifi_err_reason_t::WIFI_REASON_NO_AP_FOUND) //Disconnect after 10 times reconnect
  {
    if(disconnected_wifi_count < 0) //ignore Wrong SSID
      return;
    disconnected_wifi_count += 1;
    if(disconnected_wifi_count > 9)
      WiFi.disconnect(true,true);
  }
}
void Init_WiFi_Event()
{
  WiFi.onEvent(Client_Connected,WiFiEvent_t::ARDUINO_EVENT_WIFI_AP_STACONNECTED);
  WiFi.onEvent(Client_IP, WiFiEvent_t::ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED);
  WiFi.onEvent(Client_Disconnected,WiFiEvent_t::ARDUINO_EVENT_WIFI_AP_STADISCONNECTED);
  WiFi.onEvent(Server_Disconnected,WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
}
#pragma endregion Device Connected Manager
#pragma region MQTT Protocol //FIXME Stop mqtt when connected gateway
void connect_to_broker() // Connect to the broker
{
  while (!client.connected()) {
    String clientId = "ESP32";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      client.subscribe(MQTT_Pump_TOPIC);
      client.subscribe(MQTT_LED_TOPIC);
      MQTTStatus = true;
    } else {
      MQTTStatus = false;
      break;
    }
  }
}
void callback(char* topic, byte *payload, unsigned int length)// Receive Messange From Broker //[ ] Test it
{
    MQTT_Messange = String((char*)payload).substring(String((char*)payload).indexOf("{")+1,String((char*)payload).indexOf("}"));
    String t_ID = MQTT_Messange.substring(0,MQTT_Messange.indexOf("/"));
    String t_command = MQTT_Messange.substring(MQTT_Messange.indexOf(" ")+1,MQTT_Messange.length());
    boolean flag = false;
    if(String(topic) == MQTT_Pump_TOPIC){ 
      if(t_ID == ID)
      {
        if(Ig_Pump)
          Ig_Pump = false;
        else{ 
          if(t_command == "ON")
              Command_Pump = 1;
          else if(t_command == "OFF")
              Command_Pump = 2;
        }
      }else flag = true;
    }
    if(String(topic) == MQTT_LED_TOPIC){ 
      if(t_ID == ID)
      {
        if(Ig_Led)
            Ig_Led = false;
        else{
            if(t_command == "ON")
                Command_Light = 1;
            else if(t_command == "OFF")
                Command_Light = 2;
        }
      }else flag = true;
    }
    if(flag)
    {    
      MQTT_Data.SetData(t_ID,MQTT_Messange.substring(MQTT_Messange.indexOf("/")+1,MQTT_Messange.length()),Broadcast);
      int isNode = IsKnown(MactoIP(t_ID));
      Serial.println(MactoIP(t_ID));
      if(isNode != -1)
      {
        Serial.println("Know");
        MQTT_Data.SetNextIP(KnownIP[isNode]);
        xQueueSend(Queue_Delivery,&MQTT_Data,pdMS_TO_TICKS(100));
      }else{
        Serial.println("Un-Know");
        for(int i =1; i< 4 ;i++)
        {
          if(KnownIP[i] == "")
            continue;
          MQTT_Data.SetNextIP(KnownIP[i]);
          xQueueSend(Queue_Delivery,&MQTT_Data,pdMS_TO_TICKS(100));
        }
      }
    }

}
#pragma endregion
#pragma region Hypertext Transfer Protocol
void Delivery(void * pvParameters) //Task Delivery from node to gateway and reverse
{
  Serial.println("Delivery Task");
  Transmit data;
  UBaseType_t uxHighWaterMark;
  uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
  Serial.println(uxHighWaterMark);
  WiFiClient node;
  int IP = 0;
  while(true)
  {
    xQueueReceive(Queue_Delivery,&data,portMAX_DELAY);
    xSemaphoreTake(xMutex_HTTP,portMAX_DELAY);
    HTTPClient http;
    DeliveryIP.clear();
    IP = IsKnown(data.GetNextIP());
    if(IP == -1)
      continue;
    DeliveryIP = "http://";
    DeliveryIP += KnownIP[IP];
    DeliveryIP += "/Delivery";
    if(http.begin(node,DeliveryIP))
      int httpResponseCode = http.POST(data.GetData().toString());
    else
    {
      if(FailIP[IP] < 3)
        FailIP[IP] += 1; 
      else
      {
        if(IP == 0)
        {
          WiFi.disconnect(true,true);
          gateway_node = 0;
          KnownIP[0] = "";
          continue;
        }
        uint8_t t_mac[6];
        uint16_t t_aid;
        StringtoMACAddress(IPtoMAC(KnownIP[IP]),t_mac);
        esp_wifi_ap_get_sta_aid(t_mac,&t_aid);
        esp_wifi_deauth_sta(t_aid);
      }
    }
    http.end();
    delay(10);
    uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    Serial.println(uxHighWaterMark);
    xSemaphoreGive(xMutex_HTTP);
  }
}
#pragma endregion
#pragma region Cloud Database
void DataLog(void * pvParameters)
{
  Serial.println("Database Task");
  UBaseType_t uxHighWaterMark;
  uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
  Serial.println(uxHighWaterMark);
  DataPackage data;
  FirebaseJson json_data;
  String Root;
  unsigned long time_log = 0; //Time that data was logged
  while(true)
  {
    xQueueReceive(Queue_Database,&data,portMAX_DELAY); 
    data.DataToJson(&json_data);
    json_data.set("Status/MQTT",String(MQTTStatus));
    if(data.GetMode() == LogData)
    {
      time_log = getTime();
      Root = "DataLog/";
      Root += data.GetID();
      Root += "/";
      Root += String(time_log);
      Root += "/";
      Firebase.RTDB.setJSONAsync(&firebaseData, Root, &json_data);
    }
    else
    {
      Root = "Realtime/";
      Root += data.GetID();
      Root += "/";
      Firebase.RTDB.updateNodeSilentAsync(&firebaseData, Root, &json_data);
    }
    uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    Serial.println(uxHighWaterMark);  
  }
}
void Setup_RTDB()//Initiate Realtime Database Firebase
{
  Firebase.begin(DATABASE_URL,Database_Secrets);
  Firebase.reconnectWiFi(true);
  firebaseData.setResponseSize(4096);
}
void DataLogging()//Store a record to database
{
  if(WiFi.status() != WL_CONNECTED || ((!ping_flag || first_sta) && (gateway_node != 2)))
    return;
  if((Firebase.ready() || gateway_node == 2) && ((unsigned long)(millis()- Last_datalogging_time)>time_delay_send_datalogging)||Last_datalogging_time == 0){
    Last_datalogging_time = millis();
    switch (gateway_node)
    {
    case 1: // Gateway -> Database
      O_Pack = O_Data.GetData();
      O_Pack.SetMode(LogData);
      xQueueSend(Queue_Database,&O_Pack,pdMS_TO_TICKS(100));
      break;
    case 2: //Node -> Gateway
      O_Data.SetNextIP(KnownIP[0]);
      O_Data.SetData(ID,messanger,LogData);
      xQueueSend(Queue_Delivery,&O_Data,pdMS_TO_TICKS(100));
      break;
    default:
      break;
    }
  }
}
#pragma endregion
#pragma region Network
void Setup_Server()//Initiate connection to the servers
{
  if(!first_sta)
    return;
  configTime(gmtOffset_sec, daylightOffset_sec, NTPserver1,NTPserver2,NTPserver3);
  Setup_RTDB();
  client.setServer(MQTT_SERVER, MQTT_PORT );
  client.setCallback(callback);
  connect_to_broker();
  first_sta = false;
}
void Connect_Network()//Connect to Wifi Router
{
  if(sta_ssid == "")
    return;
  InitWaitGateway = &server.on("/YouAreNode",HTTP_POST,[](AsyncWebServerRequest *request){},NULL,
  [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total){
    if(String((char*)data) == Init_Node_Code)
    {
      String dataWiFi = "{\n";
      dataWiFi += "SSID: ";
      dataWiFi += ap_ssid;
      dataWiFi += "\nPassword: ";
      dataWiFi += ap_password;
      dataWiFi += "\n}";
      request->send(Init_Gateway_Code,"text/plain",dataWiFi);
      KnownIP[0] = request->client()->remoteIP().toString();
      esp_wifi_deauth_sta(0);
      if(WiFi.localIP()[3] == 254)
        ApIP[3] = 1;
      else
        ApIP[3] = WiFi.localIP()[3] + 1;
      WiFi.softAPConfig(ApIP,ApIP,NMask);
      gateway_node = 2; //It become node
      Person = 0;
      Serial.println(WiFi.softAPIP().toString());
    }else 
      request->send(Forbidden_Code);
    });
    WiFi.begin(sta_ssid.c_str(), sta_password.c_str());
    long current = millis();
    while (WiFi.status() != WL_CONNECTED && (unsigned long) (millis()- current) < Network_TimeOut)
    {
      delay(500);
    }

  if(WiFi.status() != WL_CONNECTED)
  {
    server.removeHandler(InitWaitGateway);
    InitWaitGateway = NULL;
    WiFi.mode(WIFI_AP);
    if(Person > 0)
      notifyClients("Wifi OFF");
  }
  else
  {
    if(gateway_node != 2)//If it isn't node, so it gateway
      gateway_node = 1;
    if(Person > 0)
      notifyClients("Wifi ON");
    Ping();
    if(ping_flag)
      Setup_Server();
  }
}
void Reset_Key()
{
  if(before_reset_key == 0)
    return;
  if((unsigned long)(millis() - before_reset_key) > reset_key_time)
  {
    sercurity_backend_key = false;
    tolerance_backend_key = false;
    before_reset_key = 0;
  }
}
void Init_Server() // FIXME: Fix backend server
{
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    if(ON_STA_FILTER(request)) //Only for client from AP Mode
      return request->redirect("/NothingHereForYou");
    if(!request->authenticate(http_username.c_str(), http_password.c_str()) && !request->authenticate(auth_username.c_str(),auth_password.c_str()))
      return request->requestAuthentication();
      request->send_P(Received_Code, "text/html", main_html);
  });//Home Page Server
  server.on("/Test",HTTP_GET,[](AsyncWebServerRequest *request){
    O_Data.SetData(ID,messanger,Default);
    request->send_P(Received_Code,"text/plain",O_Data.GetData().toString().c_str());
  });// BUG: Remove it after done
  server.on("/Sercurity",HTTP_GET,[](AsyncWebServerRequest *request){
    if(ON_STA_FILTER(request) ) //Only for client from AP Mode
      return request->redirect("/NothingHereForYou");
    if(request->authenticate(auth_username.c_str(), auth_password.c_str()))
    {
      sercurity_backend_key = true;
      before_reset_key = millis();
      return request->send_P(Received_Code,"text/html",Sercurity_html);
    }
    if(request->authenticate(http_username.c_str(),http_password.c_str()))
      return request->send_P(Forbidden_Code,"text/html",Forbidden_html);
    return request->send_P(Not_Found_Code,"text/html",Error_html);

  });
  server.on("/BackEndSercure",HTTP_POST,[](AsyncWebServerRequest *request){},NULL,[](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total){
    if(!sercurity_backend_key || ON_STA_FILTER(request) || !request->authenticate(auth_username.c_str(), auth_password.c_str()))
      return request->send(Gone_Code);
    String Filter = String((char*) data).substring(String((char*) data).indexOf('{')+1,String((char*) data).indexOf('}'));
    String TmpID = Filter.substring(Filter.indexOf(" ")+1,Filter.indexOf('/'));
    String TmpPass = Filter.substring(Filter.indexOf('/')+1,Filter.length());
    if(TmpID.length() > 63 || TmpID == NULL || TmpPass.indexOf(' ') >= 0 || ( TmpPass.length() < 8 && TmpPass != NULL))
      return request->send(Bad_Request_Code);
     
    if(String((char*) data).indexOf("Authentication: ")>=0)
    {
      http_username = TmpID;
      http_password = TmpPass;
      request->send(No_Content_Code);
    }
    else if(String((char*) data).indexOf("Authorization: ")>=0)
    {
      auth_username = TmpID;
      auth_password = TmpPass;
      request->send(No_Content_Code);
    }
    else if(String((char*) data).indexOf("AP: ")>=0)
    {
      ap_ssid = TmpID;
      ap_password = TmpPass;
      request->send(Network_Authentication_Required);
      Person = 0;
      WiFi.softAP(ap_ssid.c_str(),ap_password.c_str());
    }
    else
      request->send(No_Response_Code);
    sercurity_backend_key = false;
    before_reset_key = 0;
  });
  server.on("/Tolerance",HTTP_GET,[](AsyncWebServerRequest *request){
    if(ON_STA_FILTER(request)) //Only for client from AP Mode
      return request->redirect("/NothingHereForYou");
    if(request->authenticate(auth_username.c_str(), auth_password.c_str()))
    {
      tolerance_backend_key = true;
      before_reset_key = millis();
      return request->send_P(Received_Code,"text/html",Tolerance_html);
    }
    if(request->authenticate(http_username.c_str(),http_password.c_str()))
      return request->send_P(Forbidden_Code,"text/html",Forbidden_html);
    return request->send_P(Not_Found_Code,"text/html",Error_html);
  });
  server.on("/BackEndTolerance",HTTP_POST,[](AsyncWebServerRequest *request){},NULL,[](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total){
    if(!tolerance_backend_key || ON_STA_FILTER(request) || !request->authenticate(auth_username.c_str(), auth_password.c_str()))
      return request->send(Gone_Code);
    String Filter = String((char*) data).substring(String((char*) data).indexOf('{')+1,String((char*) data).indexOf('}'));
    Tree.Danger_Temp = atof(Filter.substring(0,Filter.indexOf('/')).c_str());
    Filter = Filter.substring(Filter.indexOf('/')+1);
    Tree.Save_Temp = atof(Filter.substring(0,Filter.indexOf('/')).c_str());
    Filter = Filter.substring(Filter.indexOf('/')+1);
    Tree.Danger_Humi = atof(Filter.substring(0,Filter.indexOf('/')).c_str());
    Filter = Filter.substring(Filter.indexOf('/')+1);
    Tree.Save_Humi = atof(Filter.substring(0,Filter.indexOf('/')).c_str());
    Filter = Filter.substring(Filter.indexOf('/')+1);
    Tree.WET_SOIL = atof(Filter.substring(0,Filter.indexOf('/')).c_str());
    Filter = Filter.substring(Filter.indexOf('/')+1);
    Tree.DRY_SOIL = atof(Filter.substring(0,Filter.indexOf('/')).c_str());
    Filter = Filter.substring(Filter.indexOf('/')+1);
    Tree.DARK_LIGHT = atof(Filter.c_str());
    request->send(Received_Code);
    tolerance_backend_key = false;
    before_reset_key = 0;
  });
  server.on("/BackEndTolerance",HTTP_GET,[](AsyncWebServerRequest *request){
    if(!request->authenticate(auth_username.c_str(), auth_password.c_str()) || !tolerance_backend_key)
      return request->send_P(Forbidden_Code,"text/html",Forbidden_html);
    MessLimit = String(Tree.Danger_Temp);
    MessLimit += "/";
    MessLimit += String(Tree.Save_Temp);
    MessLimit += "/";
    MessLimit += String(Tree.Danger_Humi);
    MessLimit += "/";
    MessLimit += String(Tree.Save_Humi);
    MessLimit += "/";
    MessLimit += String(Tree.WET_SOIL);
    MessLimit += "/";
    MessLimit += String(Tree.DRY_SOIL);
    MessLimit += "/";
    MessLimit += String(Tree.DARK_LIGHT);
    MessLimit += "/";
    return request->send_P(Received_Code,"text/plain",MessLimit.c_str());
  });
  server.on("/logout",HTTP_GET,[](AsyncWebServerRequest *request){
    request->send(Unauthorized_Code);
  });
  server.on("/Delivery",HTTP_POST,[](AsyncWebServerRequest *request){ //Receive data from Node
  },NULL,[](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total){
    request->send(Received_Code);
    Transmit package;
    package.DataFromString(String((char*) data));
    //Serial.println(package.GetData().toString());
    if(gateway_node == 0)
      return;  
    if(package.GetData().GetID() == ID || package.GetData().GetMode() == Infection ) //ReceiveIP & Infection mode 
    {
      D_Command = package.GetData().GetData();
      xQueueSend(Queue_Command,&D_Command,pdMS_TO_TICKS(100));
      if(package.GetData().GetMode() != Infection)
        return;
    }
    if (package.GetData().GetMode() == Broadcast || package.GetData().GetMode() == Infection) //Broadcast & Infection mode
    {
      int t_flag = IsKnown(request->client()->remoteIP().toString());
      if(t_flag != -1)
      {
        for(int i =0; i< 4 ;i++)
        {
          if(i == t_flag || KnownIP[i] == "")
            continue;
          package.SetNextIP(KnownIP[i]);
          xQueueSend(Queue_Delivery,&package,pdMS_TO_TICKS(100));
        }
      }
    } 
    if(package.GetData().GetMode() == Default || package.GetData().GetMode() == LogData) // Default mode
    {
      if(gateway_node == 0)
      {
        Serial.println(package.GetData().toString());
        return;
      }
      if(gateway_node == 1) // If it's a gateway -> Send to Database
      {
        D_Pack = package.GetData();
        xQueueSend(Queue_Database,&D_Pack,pdMS_TO_TICKS(100));
        return;
      }
      if(gateway_node == 2)//If it's a node -> Delivery to Gateway
      {
        package.SetNextIP(KnownIP[0]);
        xQueueSend(Queue_Delivery,&package,pdMS_TO_TICKS(100));
        return;
      }
      
    }
  });
  server.onNotFound([](AsyncWebServerRequest *request){
    if(ON_STA_FILTER(request)) //Only for client from AP Mode
      return request->redirect("https://youtu.be/dQw4w9WgXcQ");
    request->send_P(Not_Found_Code,"text/html",Error_html);
  });
  // Start server
  server.begin();
}
#pragma endregion
#pragma region Send Message
void PrepareMess() //Decide what to send
{
  valueChange_flag = false;
  messanger.clear();
  messanger = "/";
  messanger += Tree.Name;
  messanger += "/";
  messanger += Local_Path[0];
  messanger += " ";
  messanger += String(DHT_Err);
  messanger += "/";
  messanger += Local_Path[1];
  messanger += " ";
  messanger += String(LDR_Err);
  messanger += "/";
  messanger += Local_Path[2];
  messanger += " ";
  messanger += String(Soil_Err);
  messanger += "/";
  messanger += Local_Path[3];
  messanger += " ";
  messanger += String(Tree.Days);
  messanger += "/";
  messanger += Local_Path[4];
  messanger += " ";
  messanger += String(Humidity);
  messanger += "/";
  messanger += Local_Path[5];
  messanger += " ";
  messanger += String(Temperature);
  messanger += "/";
  messanger += Local_Path[6];
  messanger += " ";
  messanger += String(lumen);
  messanger += "/";
  messanger += Local_Path[7];
  messanger += " ";
  messanger += String(soilMoist);
  messanger += "/";
  messanger += Local_Path[8];
  messanger += " ";
  messanger += String(LightStatus);
  messanger += "/";
  messanger += Local_Path[9];
  messanger += " ";
  messanger += String(PumpsStatus);
  messanger += "/";

  if(Temp[0] != (int)DHT_Err)
  {
    valueChange_flag = true;
    Temp[0] = (int)DHT_Err;
  }
  if(Temp[1] != (int)LDR_Err )
  {
    valueChange_flag = true;
    Temp[1] = (int)LDR_Err;
  }
  if(Temp[2] != (int)Soil_Err)
  {
    valueChange_flag = true;
    Temp[2] = (int)Soil_Err;
  }
  if(Temp[3] != (int)Tree.Days )
  {
    valueChange_flag = true;
    Temp[3] = (int)Tree.Days;
  }
  if(Temp[4] != (int)Humidity)
  {
    valueChange_flag = true;
    Temp[4] = (int)Humidity;
  }
  if(Temp[5] != (int)Temperature)
  {
    valueChange_flag = true;
    Temp[5] = (int)Temperature;
  }
  if(Temp[6] != (int)lumen)
  {
    valueChange_flag = true;
    Temp[6] = (int)lumen;
  }
  if(Temp[7] != (int)soilMoist)
  {
    valueChange_flag = true;
    Temp[7] = (int)soilMoist;
  }
  if(Temp[8] != (int)LightStatus)
  {
    valueChange_flag = true;
    Temp[8] = (int)LightStatus;
  }
  if(Temp[9] != (int)PumpsStatus)
  {
    valueChange_flag = true;
    Temp[9] = (int)PumpsStatus;
  }
  if(Temp[10] != (int)MQTTStatus )
  {
    valueChange_flag = true;
    Temp[10] = (int)MQTTStatus;
  }
  
}
void SendMess() //Send mess prepared to who
{
  if(valueChange_flag)
  {

    if(Person>0) //Send thourgh WebSocket
      notifyClients(messanger);
    O_Data.SetData(ID,messanger,Default);
    if(gateway_node == 2) //Send to Gateway only if It's a node
    {
      O_Data.SetNextIP(KnownIP[0]);
      xQueueSend(Queue_Delivery,&O_Data,pdMS_TO_TICKS(100));
    }
    if(WiFi.status() == WL_CONNECTED && ping_flag && !first_sta && Firebase.ready()) //Send to database
    {
      O_Pack = O_Data.GetData();
      xQueueSend(Queue_Database,&O_Pack,pdMS_TO_TICKS(100));
    }  
  }
}
#pragma endregion Send Message
#pragma region Sensor Data Read
int Get_Sensor(int anaPin)// Get Data From Light Sensor & Soild Sensor
{
  int value = 0;
  value = analogRead(anaPin);
  if(value > 4095)
    value = 4095;
  if(value < 1500)
    value = 1500;
  value = map(value,4095,1500,0,100);
  return value;
} 
void Check()// Check error sensor
{
  if(isnan(Humidity) || isnan(Temperature) || Humidity > 90 || Temperature > 50 ){
      DHT_Err = true;
    }
  else{
      DHT_Err = false;
    }
  if(lumen < 0 || lumen > 100){
      LDR_Err = true;
    }
  else{
      LDR_Err = false;
    }
  if(soilMoist < 0 || soilMoist > 100){
      Soil_Err = true;
    }
  else{
      Soil_Err = false;
    }
  if(DHT_Err || LDR_Err || Soil_Err)
    Err = true;
  else if(Err)
    Err = false;
}
void Read_Sensor()//Get Data from All Sensors
{
  lumen = Get_Sensor(LDR);
  delay(500);
  soilMoist = Get_Sensor(Soil_Moisture);
  delay(500);
  Humidity = dht.readHumidity();
  Temperature = dht.readTemperature();
  Check();
}
#pragma endregion
#pragma region Controlled Things
void Condition_Pump()//Check watering conditions
{
  if(((soilMoist < Tree.DRY_SOIL && !Err ||(unsigned long)(millis()-Times_Pumps) >= Next_Pump) || Command_Pump == 1) && !PumpsStatus) //Đang tắt
  {
    Times_Pumps = millis();
    PumpsStatus = true;
    if(Command_Pump == 2)
      Command_Pump = 0;
    if(Command_Pump == 1)
      return;
  }
  if((((Temperature >= Tree.Danger_Temp || Temperature <= Tree.Save_Temp || soilMoist > Tree.WET_SOIL) && !Err || ((unsigned long)(millis()- Times_Pumps) >= Still_Pumps)) || Command_Pump == 2) && PumpsStatus) //Đang bật
  {
    Times_Pumps = millis(); 
    PumpsStatus = false;
    if(Command_Pump == 1)
      Command_Pump = 0;
    if(Command_Pump ==2)
      return;
  }
}
void Pump()//Pump Choice
{
  if(Err && Command_Pump == 0)
    PumpsStatus = false;
  else
    Condition_Pump();
  if(PumpsStatus)
    digitalWrite(Pumps,HIGH);
  else 
    digitalWrite(Pumps,LOW);
}
void Condition_Light()//Check lighting up conditions
{
  if((lumen <= Tree.DARK_LIGHT && !Err ||  Command_Light == 1 )&& !LightStatus) //Đang tắt
    {
      LightStatus = true;
      if(Command_Light == 2)
        Command_Light = 0;
      if(Command_Light ==1)
        return;
    }
  if((lumen > Tree.DARK_LIGHT && !Err || Command_Light == 2) && LightStatus) // Đang bật
    {
      LightStatus = false;
      if(Command_Light == 1)
        Command_Light = 0;
      if(Command_Light ==2)
        return;
    }
}
void Light_Up()//Light up choice
{
  if(Err && Command_Light == 0)
    LightStatus = false;
  else
    Condition_Light();
  if(LightStatus)
    digitalWrite(Light,HIGH);
  else 
    digitalWrite(Light,LOW);
}
#pragma endregion
#pragma region Main System
void Solve_Command() // TODO: Slove Command
{
  if(xQueueReceive(Queue_Command,&O_Command,0) == pdPASS)
  {
    Serial.print("Receive command: ");
    Serial.println(O_Command);
    String Actuator = O_Command.substring(0,O_Command.indexOf(" "));
    String Require = O_Command.substring(O_Command.indexOf(" ")+1,O_Command.length());
    if(Actuator == "Light")
    {
      if(Require == "ON")
          Command_Light = 1;
      else if(Require == "OFF")
          Command_Light = 2;
      return;
    }
    if(Actuator == "Pump")
    {
      if(Require == "ON")
          Command_Pump = 1;
      else if(Require == "OFF")
          Command_Pump = 2;
      return;
    }
  }
  O_Command.clear();
}
void Init_Task()
{
  Queue_Delivery = xQueueCreate(Queue_Length,Queue_item_delivery_size+1);
  Queue_Command = xQueueCreate(Queue_Length,Queue_item_command_size+1);
  Queue_Database = xQueueCreate(Queue_Length,Queue_item_database_size+1);
  xMutex_HTTP = xSemaphoreCreateMutex();
  xTaskCreate(
    Delivery,
    "Delivery",
    3000, //1684B left
    NULL,
    0,
    &DeliveryTask
  );
  xTaskCreate(
    DataLog,
    "DataLog",
    8000,//3028B left
    NULL,
    0,
    &DatabaseTask
  );
}
void Network()// Netword Part
{
  ws.cleanupClients();
  PrepareMess();
  SendMess();
  DataLogging();
  if(sta_flag)
  {
    WiFi.mode(WIFI_AP_STA);
    Connect_Network();
    sta_flag = false;
  } 
  Reset_Key(); 
  if(WiFi.status() != WL_CONNECTED){
    if(Last_ping_time != 0) Last_ping_time = 0;
    if(ping_flag) ping_flag = false;
    return;
  }
  Cycle_Ping();
  if(!ping_flag)
    return;
  Setup_Server();// If it doesn't setup
  client.loop();
  if (!client.connected()) {
    connect_to_broker();
  }
}
void Auto()//Auto Part
{
  Make_Day();
  Solve_Command();
  Read_Sensor();
  Check();
  Pump();
  Light_Up();
}
#pragma endregion
#pragma region Arduino program structure
void setup() 
{
  Serial.begin(9600);
  Serial.println();
  Init_Task();
  initWebSocket();
  pinMode(Pumps,OUTPUT);
  pinMode(Light,OUTPUT);
  digitalWrite(Pumps,LOW);
  digitalWrite(Light,LOW);
  dht.begin();
  Time_Passed = millis();
  WiFi.mode(WIFI_AP_STA);
  Init_Server();
  Init_WiFi_Event();
  WiFi.softAP(ap_ssid.c_str(), ap_password.c_str());
  Connect_Network();
}
void loop() 
{
  Auto();
  Network();
}
#pragma endregion
