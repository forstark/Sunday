#include <WiFi.h>

#define BAUDRATE 115200
#define SERVER_PORT 5001
#define MASTER_PORT 4000

const String nom = "IOT_Slave1";
const char* ssid = "Pixel_1621";
const char* password = "e93bc9a01bw82jfo2";

WiFiServer server(SERVER_PORT);

WiFiClient master;
IPAddress masterAddress(192, 168, 131, 100);

IPAddress ip(192, 168, 131, 101);
IPAddress gateway(192, 168, 131, 162);
IPAddress subnet(255, 255, 255, 0);

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
}

void loop(void)
{ 
  clientRequest();
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
  
}

void sendData(String d)
{
  Serial.println("Sending data...");
    
  if(master.connect(masterAddress, MASTER_PORT))
  {
    master.println(d);
  }

  else
  {
    Serial.println("ERROR : Connection failed");
  }

  master.stop();
}
