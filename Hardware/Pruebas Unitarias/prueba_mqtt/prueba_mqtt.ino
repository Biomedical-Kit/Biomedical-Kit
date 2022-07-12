#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>



// Replace the next variables with your SSID/Password combination
const char* ssid =  "";
const char* password =  "";


const char* mqtt_server = "";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;


float temperature = 10;
float humidity = 20;
const char* user = "user1";

// LED Pin
//const int mqttledPin = 33;

void setup() {
  //pinMode(mqttledPin, OUTPUT);
  Serial.begin(115200);
  // default settings
  // (you can also pass in a Wire library object like &Wire2)
  //status = bme.begin();  
  setup_wifi();
  client.setServer(mqtt_server, 13023);
  client.setCallback(callback);

  //pinMode(ledPin, OUTPUT);
}

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
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  if (String(topic) == "esp32/output") {
    Serial.print("Changing output to ");
    if(messageTemp == "on"){
      Serial.println("on");
      //digitalWrite(ledPin, HIGH);
    }
    else if(messageTemp == "off"){
      Serial.println("off");
      //digitalWrite(ledPin, LOW);
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/output");
      //digitalWrite(mqttledPin, HIGH);
    } else {
      Serial.print("failed, code=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void jsonFormat(const char* topic, float value) {
  char string[8];
  dtostrf(value, 1, 2, string);
  char sensorData[1024] ;
  strcpy(sensorData,"{\"iduser\":\"");
  strcat(sensorData, user);
  strcat(sensorData, "\",\"data\":");
  strcat(sensorData, string);
  strcat(sensorData, "}");
    //const char* humString= sensorData;
  Serial.println(value);
  client.publish(topic, sensorData);
}


void loop() {
  if (!client.connected()) {
    //digitalWrite(mqttledPin, LOW);
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 5000) { // bucle cada 5 segundos
    lastMsg = now;


    Serial.print("Humidity: ");
    temperature += 1;
    humidity += 1;
    jsonFormat("humidity", humidity);
    Serial.print("Temperature: ");
    jsonFormat("temperature",temperature);
    //Envío de un valor en formato JSON para relacion de user con el dato
    /*char tempString[8];
    temperature += 1;
    dtostrf(temperature, 1, 2, tempString); // convierte valor en char
    Serial.print("Temperature: ");
    char tempData[1024] ;
    strcpy(tempData,"{\"iduser\":\"");
    strcat(tempData, user);
    strcat(tempData, "\",\"data\":");
    strcat(tempData, tempString);
    strcat(tempData, "}");
    Serial.println(temperature);
    client.publish("temperature", tempData);


    char humString[8];
    humidity += 1;
    dtostrf(humidity, 1, 2, humString);
    Serial.print("Humidity: ");
    char humData[1024] ;
    strcpy(humData,"{\"iduser\":\"");
    strcat(humData, user);
    strcat(humData, "\",\"data\":");
    strcat(humData, humString);
    strcat(humData, "}");
    //const char* humString= sensorData;
    Serial.println(humidity);
    client.publish("humidity", humData);*/

  }
}
