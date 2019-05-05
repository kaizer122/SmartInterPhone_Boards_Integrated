
#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <GxEPD2_3C.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#endif


#include <AutoConnect.h>
#include <AutoConnectCredential.h>
//new
#include "credentialhttp.h"
#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#if defined (ESP8266)
GxEPD2_3C<GxEPD2_270c, GxEPD2_270c::HEIGHT> display(GxEPD2_270c(/*CS=D8*/ 15, /*DC=D3*/ 4, /*RST=D4*/ 5, /*BUSY=D2*/ 16));
#endif

#if defined(ESP32)
GxEPD2_BW<GxEPD2_290, GxEPD2_290::HEIGHT> display(GxEPD2_290(/*CS=*/ 15, /*DC=*/ 27, /*RST=*/ 26, /*BUSY=*/ 25));
#endif

//
#if defined(ARDUINO_ARCH_ESP8266)
ESP8266WebServer Server;
#elif defined(ARDUINO_ARCH_ESP32)
WebServer Server;
#endif
AutoConnect      Portal(Server);
String viewCredential(PageArgument&);
String delCredential(PageArgument&);
String payload="";
HTTPClient http;
WiFiClient client;
IPAddress server1(34,206,37,239); // change with host ip
IPAddress myIP;       // IP address in your local wifi net



#define CREDENTIAL_OFFSET 0

PageElement elmList(html,
  {{ "SSID", viewCredential },
   { "AUTOCONNECT_MENU", [](PageArgument& args) {
                            return String(FPSTR(autoconnectMenu));} }
  });
PageBuilder rootPage("/", { elmList });

PageElement elmDel("{{DEL}}", {{ "DEL", delCredential }});
PageBuilder delPage("/del", { elmDel });


void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println();
  display.init(115200); 
  #if defined(ESP32)
  SPI.end(); 
  SPI.begin(13, 12, 14, 15);
  #endif
  rootPage.insert(Server);    // Instead of Server.on("/", ...);
  delPage.insert(Server);     // Instead of Server.on("/del", ...);

  // Set an address of the credential area.
  AutoConnectConfig Config;
  Config.boundaryOffset = CREDENTIAL_OFFSET;
  Portal.config(Config);
 displayMsg("Trying to connect" , "after 30 seconds check ap","");
  // Start
  if (Portal.begin()) {
    Serial.println("WiFi connected: " + WiFi.localIP().toString());
    displayMsg("Wifi connected" , "IP:"+ WiFi.localIP().toString(),"");
  }
  #if defined(ESP8266) 
    if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  #endif
}

void loop() {
    Portal.handleClient();
    synchronizeWithDataBase();
}

void displayMsg(String msg, String msg2 , String msg3)
{
  display.setRotation(1);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  display.setFullWindow();
  display.firstPage();
   do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(30,50);
    display.print(msg);
     display.setCursor(30,70);
    display.print(msg2);
      display.setCursor(30,90);
    display.print(msg3);
  }
  while (display.nextPage());
  delay(1000);
}
void synchronizeWithDataBase() {

client = WiFiClient(); 

   if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status

if (client.connect(server1, 80)) {
      Serial.println("connected");
      #if defined(ESP32) 
          String globalUrl = "https://smart-interphone.herokuapp.com/api/messages/4";
      #endif
      #if defined(ESP8266) 
          String globalUrl = "http://192.168.1.15:3000/api/messages/4";
      #endif
  
    Serial.println(globalUrl);
    http.begin(globalUrl);  //Specify request destination
    int httpCode = http.GET();
    Serial.println(httpCode);       
   //Send the request
   if (httpCode > 0) { //Check the returning code
      payload = http.getString();   //Get the request response payload
      #if defined(ESP32)
      const size_t capacity = (payload.length()*2);
      #endif
      #if defined (ESP8266)
       const size_t capacity = (payload.length()*3);
      #endif
      DynamicJsonDocument doc(capacity);
     DeserializationError error = deserializeJson(doc, payload);
     if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }
      Serial.print("Content :");
      int i = 0;
    
     for (i=0;i<doc.size();i++) {
    
     #if (ESP32)
       JsonObject root = doc[i];
      String root_content = root["content"]; // "kais test 2"
      String root_displayAt = root["displayAt"]; // "2019-04-29T12:00:00.000Z"
      String username = root["user"]["username"];
      const char* root_hiddenAt = root["hiddenAt"]; 
      if (username != "null") {   
     displayMsg(root_content,"By: " + username,"Starts At:"+ root_displayAt.substring(11,16));
      } else  {
        displayMsg(root_content,"","Starts At:"+ root_displayAt.substring(11,16));
      }
      #endif
      #if (ESP8266)
        String root_content = doc[i]; // "kais test 2"
         displayMsg("","",root_content);
      #endif
     }
       
   }
}
   http.end();
}
//client.stop();
delay(1000);    //Send a request every 30 seconds
  
}
