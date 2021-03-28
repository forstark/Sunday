#include <WiFi.h>
#include <PubSubClient.h>

#define BAUDRATE 115200

const String nom = "Sunday_Master";

const int MASTER_PORT = 4000;
WiFiServer server(MASTER_PORT);

WiFiClient calculationUnit;
IPAddress calculationUnitAddress(192, 168, 1, 24); // To check !
const int calculationUnitPort = 5000;

WiFiClient slave1;
IPAddress slave1Address(192, 168, 1, 101);
const int slave1Port = 5001;

//const char *mqttServer = "iot.fr-par.scw.cloud";
IPAddress mqttServer(51, 159, 24, 174);
const int mqttPort = 1883;
const char *mqttUser = "5913f12c-bdef-48dd-a6b6-e0527d9e6f13";
const char *mqttPassword = "";
WiFiClient mqttClient;
PubSubClient mqttPubClient(mqttClient);

const char* ssid = "Freebox-53DBE7";
const char* password = "cd5=GVnbAEW)";
IPAddress ip(192, 168, 1, 100);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(192, 168, 1, 1);

const int pinButton = 4;
#define DEBOUNCE_TIME 250
volatile uint32_t DebounceTimer = 0;

void setup(void)
{
  Serial.begin(BAUDRATE);

  WiFi.config(ip, dns, gateway, subnet);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
   delay(500);
   Serial.print(".");
  }

  Serial.print(F(" connected to Wifi! IP address : "));
  Serial.println(WiFi.localIP());

  server.begin();

  Serial.println("Configuring MQTT Server !");
  mqttPubClient.setServer(mqttServer, mqttPort);
  while(!mqttPubClient.connected())
  {
    Serial.println("Connecting to MQTT Server ...");
    if(mqttPubClient.connect(nom.c_str(), mqttUser, mqttPassword))
    {
      Serial.println("Connected !");
    }

    else
    {
      Serial.print("Failed with state : ");
      Serial.println(mqttPubClient.state());
      delay(2000);
    }
  }

  pinMode(pinButton, INPUT);

  delay(1000);
  Serial.println("Ready !");
}

void loop(void)
{ 
  mqttPubClient.loop();
  clientRequest();
  
  if(digitalRead(pinButton) == HIGH)
  {
    if(millis() - DEBOUNCE_TIME >= DebounceTimer)
    {
      DebounceTimer = millis();
      
      connectCalculationUnit();
    }
  }
}

void clientRequest(void)
{
  ////Check if client connected
  WiFiClient client = server.available();
  client.setTimeout(50);
  
  if(client)
  {
    if(client.connected())
    {
      //Print client IP address
      Serial.print("Client connected ->");
      Serial.print(client.remoteIP());
      Serial.print(" : ");
      String request = client.readStringUntil('\r');
      Serial.println(request);
      
      client.stop();

      handleRequest(request);
    }
  }
}

void handleRequest(String r)
{
  Serial.println("Sending data...");

  mqttPubClient.publish("room/heater", r.substring(r.indexOf(":") + 1).c_str());
    
  if(slave1.connect(slave1Address, slave1Port))
  {
    slave1.println(r);
  }

  else
  {
    Serial.println("ERROR : Connection failed");
  }

  slave1.stop();
}

void connectCalculationUnit(void)
{
  Serial.println("Sending keyword...");
    
  if(calculationUnit.connect(calculationUnitAddress, calculationUnitPort))
  {
    calculationUnit.println("Sunday");
  }

  else
  {
    Serial.println("ERROR : Connection failed");
  }

  calculationUnit.stop();
}
