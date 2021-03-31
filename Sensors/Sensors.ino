#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"
#define DHTTYPE DHT11 // DHT 11
#define DHTPIN 5 // DHT Sensor - GPIO 5 = D1 on ESP-12E NodeMCU board
#define PIRPIN 2 // PIR Sensor - GPIO 2 = D4 on ESP-12E NodeMCU board
#define BUTTON A0 // Button - GPIO A0 on ESP-12E NodeMCU board
#define LED 14 // Led for light - GPIO 14 = D5 on ESP-12E NodeMCU board
#define HEATER 4 // Led for header - GPIO 4 = D2 on ESP-12E NodeMCU board
#define LAMP 12 // Lamp - LED - GPIO 12 = D6 on ESP-12E NodeMCU board
#define BAUDRATE 115200
#define SERVER_PORT 5001
#define MASTER_PORT 4000

const String nom = "IOT_Slave1";

WiFiServer server(SERVER_PORT);

WiFiClient master;
IPAddress masterAddress(192, 168, 131, 100);

IPAddress ip(192, 168, 131, 101);
IPAddress gateway(192, 168, 131, 162);
IPAddress subnet(255, 255, 255, 0);

// Change the credentials below, so your ESP8266 connects to your router
const char* ssid = "Pixel_1621";
const char* password = "e93bc9a01bw82jfo2";
// Change the variable to your MQTT Broker, so it connects to it
IPAddress mqttServer(51, 159, 24, 174);//const char* mqttServer = "iot.fr-par.scw.cloud";
const int mqttPort = 1883;
const char* mqttUser = "5913f12c-bdef-48dd-a6b6-e0527d9e6f13";
const char* mqttPassword = "";

// Declaration of Led's class 
class Led{
  private:
    int pin; // Pin of the led
    int state; // Status of the led
  public:
    Led(int value);  // Constructor
    void setup(); // Initialization of the led
    void on(); // Switch ON the led and change the status
    void off(); // Switch OFF the led and change the status
    void refresh(); // Switch the led
    void modifState(); // Modify the status of the led
    int getState(); //Return the status of the led
};

// Declaration of Button's class 
class Button {
  private:
    int pin; // Pin of the button
    int state; // Status of the button
  public:
    Button(int value): pin(value){}; // Constructor
    void setup(); // Initialization of the led
    bool isPressed(); // Return is the button is pressed
    int getState(); //Return the status of the button
    void modifState(); // Modify the status of the button
};

//Constructor's definition
Led::Led(int value){ pin = value;}

//Initialization of the led in OUTPUT
void Led::setup(){ pinMode(pin, OUTPUT);}

//Method to switch ON the led and change the status
void Led::on(){
  state = HIGH;
  refresh();
}

//Method to switch OFF the led and change the status
void Led::off(){
  state = LOW;
  refresh();
}

//Method to switch the led
void Led::refresh(){ digitalWrite(pin, state);}

//Mehod which switches the led ON or OFF according to its status
void Led::modifState(){
  if(state == 1){
    off();
    state=0;
  }
  else{
    on();
    state=1;
  }
}

//Method thate return the status of the led
int Led::getState(){
  return state;
}

// Method tha initialize the button in OUTPUT
void Button::setup(){ pinMode(pin, INPUT);}

//Method that return is the button is pressed
bool Button::isPressed() {
  if(analogRead(pin) < 100){
    modifState();
    return true;
  }
  else{
    return false;
  }
 }

//Mehod which switches the status of the button
void Button::modifState(){
  if(state == 1){
    state=0;
  }
  else{
    state=1;
  }
}

//Method thate return the status of the button
int Button::getState(){
  return state;
}
// Initializes the espClient. You should change the espClient name if you have multiple ESPs running in your home automation system
WiFiClient espClient;
PubSubClient client(espClient);

Led light(LED); 
Led heater(HEATER);
Led lamp(LAMP);
Button interrupt(BUTTON);
DHT dht(DHTPIN, DHTTYPE);
int heaterTemp = 0;

// Timers auxiliar variables
long now = millis();
long lastMeasure = 0;

// Don't change the function below. This functions connects your ESP8266 to your router
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
}

// This functions is executed when some device publishes a message to a topic that your ESP8266 is subscribed to
// Change the function below to add logic to your program, so when a device publishes a message to a topic that 
// your ESP8266 is subscribed you can actually do something
void callback(String topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message from heater: ");
  String messageTemp;
  int heaterValue;
  for (int i = 0; i < length; i++) {
    messageTemp += (char)message[i];
  }
  heaterValue = messageTemp.toInt();
  Serial.println(heaterValue);
  // If a message is received on the topic room/heater, you check if the message is either on or off. Turns the lamp GPIO according to the message
  if(topic=="room/heater"){
    if(heaterValue >= heaterTemp){
      heater.on();
      Serial.println("Heater On");
    }
    else {
      heater.off();
      Serial.println("Heater Off");
    }
  }
}

// This functions reconnects your ESP8266 to your MQTT broker
// Change the function below if you want to subscribe to more topics with your ESP8266 
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client", mqttUser, mqttPassword )) {
      Serial.println("connected");  
      // Subscribe or resubscribe to a topic
      // You can subscribe to more topics (to control more LEDs in this example)
      client.subscribe("room/heater");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
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

//Fonction qui récupère les commandes de Sunday via MQTT et les traite
void handleRequest(String r)
{
  int sep = (r.substring(r.indexOf(":") +1)).toInt();
  if(r.indexOf("light") == 0){
    Serial.println(sep);
    if(sep == 1) {
      //light.modifState();
      //light.on();
      digitalWrite(LED, HIGH);
      interrupt.modifState();
    }
    else {
      interrupt.modifState();
      digitalWrite(LED, LOW);
    }
  }
  else if(r.indexOf("heat") == 0){
    static char heat[1];
    dtostrf(sep, 1, 0, heat);
    client.publish("room/heater", heat);
  }
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

// The setup function sets your ESP GPIOs to Outputs, starts the serial communication at a baud rate of 115200
// Sets your mqtt broker and sets the callback function
// The callback function is what receives messages and actually controls the LEDs
void setup() {
  light.setup();
  heater.setup();
  interrupt.setup();
  dht.begin();
  //Serial.begin(9600);
  //setup_wifi();
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
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

// For this project, you don't need to change anything in the loop function. Basically it ensures that you ESP is connected to your broker
void loop() {

  if (!client.connected()) {
    reconnect();
  }
  if(!client.loop())
    client.connect("ESP8266Client", mqttUser, mqttPassword );
  
  clientRequest();
  //PIR & Light code----------------------------------------------------------------------------------------------------------------------------------------------------------------
  static char mouvement[1];
  if(interrupt.isPressed() == true){ // If someone presses the interrupter the light changes status
    //client.publish("room/heater", "25");
    light.modifState();
  }
  if(digitalRead(PIRPIN) == HIGH){ // If someone moves the light changes switch on
    light.on(); 
    dtostrf(1, 1, 0, mouvement);
    Serial.println("mouvement detecte");
  }
  if(interrupt.getState() && light.getState() == HIGH && digitalRead(PIRPIN) == LOW){ // If there is no movement and the status of the interrupt is low the light changes switch off
    light.modifState();
    dtostrf(0, 1, 0, mouvement);
  }

  //Heater code----------------------------------------------------------------------------------------------------------------------------------------------------------------------
  now = millis();
  // Publishes new temperature and humidity every 30 seconds
  if (now - lastMeasure > 30000) {
    lastMeasure = now;
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();
    heaterTemp = t;
    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }
    // Publication code--------------------------------------------------------------------------------------------------------------------------------------------------------------
    // Computes temperature values in Celsius
    static char temperatureTemp[7];
    dtostrf(t, 6, 2, temperatureTemp);
    static char humidityTemp[7];
    dtostrf(h, 6, 2, humidityTemp);

    // Publishes Temperature and Humidity values
    client.publish("room/temperature", temperatureTemp);
    client.publish("room/humidity", humidityTemp);
    
    Serial.print("Humidity: ");
    Serial.println(h);
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.println(" *C ");
  }
  if (now - lastMeasure > 5000) client.publish("room/mouvement", mouvement);
  delay(200);
}
