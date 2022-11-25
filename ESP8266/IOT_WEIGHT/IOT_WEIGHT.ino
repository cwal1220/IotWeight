#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <ESP8266mDNS.h>
#include <PubSubClient.h>
#include <Wire.h>
#include "HX711.h"


// FIX ME!!
#define WEIGHT_THRESHOLD_GRAM 40.0



#define EEPROM_LENGTH 1024

#define NOTIFY_TOPIC "WEIGHT_221126/NOTIFY"
#define WEIGHT_TOPIC "WEIGHT_221126/WEIGHT"


const char*   mqttServer = "test.mosquitto.org";
const int     mqttPort = 1883;
const char*   mqttUser = "chan";
const char*   mqttPassword = "chan";


char eRead[30];
byte len;
char ssid[30];
char password[30];

bool captive = true;

WiFiClient espClient;
PubSubClient client(espClient);
const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 1, 1);
DNSServer dnsServer;
ESP8266WebServer webServer(80);

const int LOADCELL_DOUT_PIN = 12;
const int LOADCELL_SCK_PIN = 13;
HX711 scale;

String responseHTML = ""
    "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><title>Wi-Fi Setting Page</title></head><body><center>"
    "<p>Wi-Fi Setting Page</p>"
    "<form action='/button'>"
    "<p><input type='text' name='ssid' placeholder='SSID' onblur='this.value=removeSpaces(this.value);'></p>"
    "<p><input type='text' name='password' placeholder='WLAN Password'></p>"
    "<p><input type='submit' value='Submit'></p></form>"
    "<p>Wi-Fi Setting Page</p></center></body>"
    "<script>function removeSpaces(string) {"
    "   return string.split(' ').join('');"
    "}</script></html>";


void setup() 
{
  Serial.begin(115200);
  EEPROM.begin(EEPROM_LENGTH);
  pinMode(0, INPUT_PULLUP);
  attachInterrupt(0, initDevice, FALLING);

  ReadString(0, 30);
  if (!strcmp(eRead, ""))
    setup_captive();
  else 
  {
    captive = false;
    strcpy(ssid, eRead);
    ReadString(30, 30);
    strcpy(password, eRead);
    
    scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
    scale.set_scale(-460.080645161290);
    scale.tare(); // reset the scale to 0

    setup_runtime();  
    client.setServer(mqttServer, mqttPort);
    while (!client.connected()) 
    {
      Serial.println("Connecting to MQTT...");
      if (client.connect("IOT_WEIGHT", mqttUser, mqttPassword )) 
      {
        Serial.println("connected");
      } 
      else 
      {
        Serial.print("failed with state "); Serial.println(client.state());
        ESP.restart();
      }
    }
  }
}

void loop() {
  if (captive)
  {
    dnsServer.processNextRequest();
    webServer.handleClient();
  }
  else
  {
    float weight;
    char buf[10];

    weight = getWeight();
    sprintf(buf, "%.2lf", weight);
    client.publish(WEIGHT_TOPIC, buf);
    if(weight < WEIGHT_THRESHOLD_GRAM)
    {
      client.publish(NOTIFY_TOPIC, buf);
    }
  }
  client.loop();
}


void setup_runtime() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(1000);
    Serial.print(".");
    yield;
    if(i++ > 20) 
    {
      ESP.restart();
      return; 
    }
  }
  Serial.println("");
  Serial.print("Connected to "); Serial.println(ssid);
  Serial.print("IP address: "); Serial.println(WiFi.localIP());

  if (MDNS.begin("IOT_WEIGHT")) {
   Serial.println("MDNS responder started");
  }
  
  webServer.onNotFound(handleNotFound);
  webServer.begin();
  Serial.println("HTTP server started");  
}

void setup_captive() {    
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("IOT_WEIGHT");
  
  dnsServer.start(DNS_PORT, "*", apIP);

  webServer.on("/button", button);

  webServer.onNotFound([]() {
    webServer.send(200, "text/html", responseHTML);
  });
  webServer.begin();
  Serial.println("Captive Portal Started");
}

void button()
{
  Serial.println("button pressed");
  Serial.println(webServer.arg("ssid"));
  SaveString(0, (webServer.arg("ssid")).c_str());
  SaveString(30, (webServer.arg("password")).c_str());
  webServer.send(200, "text/plain", "OK");
  ESP.restart();
}

void SaveString(int startAt, const char* id) 
{ 
  for (byte i = 0; i <= strlen(id); i++)
  {
    EEPROM.write(i + startAt, (uint8_t) id[i]);
  }
  EEPROM.commit();
}

void ReadString(byte startAt, byte bufor) 
{
  for (byte i = 0; i <= bufor; i++) {
    eRead[i] = (char)EEPROM.read(i + startAt);
  }
  len = bufor;
}

void handleNotFound()
{
  String message = "File Not Found\n";
  webServer.send(404, "text/plain", message);
}

ICACHE_RAM_ATTR void initDevice() {
    Serial.println("Flushing EEPROM....");
    SaveString(0, "");
    ESP.restart();
}

float getWeight()
{
  float ret;
  ret = scale.get_units(10);
  Serial.println(ret);
  scale.power_down();             // put the ADC in sleep mode
  delay(1000);
  scale.power_up();
  return ret;
}
