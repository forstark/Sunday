#include <WiFi.h>

#define BAUDRATE 115200

const String nom = "IOT_Master";

const int MASTER_PORT = 4000;
WiFiServer server(MASTER_PORT);

WiFiClient calculationUnit;
IPAddress calculationUnitAddress(192, 168, 1, 24); // To check !
const int calculationUnitPort = 5000;

WiFiClient slave1;
IPAddress slave1Address(192, 168, 1, 101);
const int slave1Port = 5001;

const char* ssid = "Freebox-53DBE7";
const char* password = "cd5=GVnbAEW)";
IPAddress ip(192, 168, 1, 100);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

const int pinButton = 4;
#define DEBOUNCE_TIME 250
volatile uint32_t DebounceTimer = 0;

void setup(void)
{
  Serial.begin(BAUDRATE);

  WiFi.config(ip, gateway, subnet);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
   delay(500);
   Serial.print(".");
  }

  Serial.print(F(" connected to Wifi! IP address : "));
  Serial.println(WiFi.localIP());

  server.begin();

  pinMode(pinButton, INPUT);
}

void loop(void)
{  
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
