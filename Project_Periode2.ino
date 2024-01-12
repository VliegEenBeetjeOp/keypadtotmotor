#include <Keypad.h>
#include <WiFi.h>
#include <ArduinoMqttClient.h>

char ssid[] = "IoTatelierF2122";    // 22 tot 44 
char pass[] = "IoTatelier";
WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);
const char broker[] = "192.168.122.1";  //22 tot 44
const int port = 1883;
const char publishTopic[] = "sanderhavelaar/deur"; 
const char subscribeTopic[] = "Toufik/plantvochtigheid"; 
const char* unsubscribedTopic = subscribeTopic;
long count = 0;
const long interval = 1000;   //analog read interval
unsigned long previousMillis = 0;
unsigned long mqttPollMillis = 0;

#define ROW_NUM     4 // four rows
#define COLUMN_NUM  3 // three columns

char keys[ROW_NUM][COLUMN_NUM] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

byte pin_rows[ROW_NUM] = {33, 25, 26, 27}; // GPIO18, GPIO5, GPIO17, GPIO16 connect to the row pins
byte pin_column[COLUMN_NUM] = {14, 12, 13};  // GPIO4, GPIO0, GPIO2 connect to the column pins

Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );
const String password = "6969"; // change your password here
String input_password;
String deur = "gesloten";

//motor code
int motor1Pin1 = 32; 
int motor1Pin2 = 35; 

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
 
  // Perform actions based on the received message (if needed)
}
// MQTT connect
void connecttoMQTT(){
    Serial.println("Wifi connecting");
  
  WiFi.useStaticBuffers(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
    Serial.println("MQTT connecting");
    bool MQTTconnected = false;
       while (!MQTTconnected) {
      if (!mqttClient.connect(broker, port))
        delay(1000);
      else
        MQTTconnected = true;
    }
    mqttClient.onMessage(onMqttMessage);
    mqttClient.subscribe(subscribeTopic);
    Serial.println("Setup complete");
}
// Send message to MQTT
void sendMessagetoMQTT(){
    mqttClient.poll();
    
    unsigned long currentMillis = millis();
    
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      
      Serial.print("Sending message to topic: ");
      Serial.println(publishTopic);
      Serial.print(deur);
      
      mqttClient.beginMessage(publishTopic,true,0);
      mqttClient.print(deur);
      mqttClient.endMessage();
  }
  delay(1);
}
// Receive message from MQTT
void onMqttMessage(int messageSize) {
  Serial.print("Received a message with topic '");
  Serial.println(mqttClient.messageTopic());

  String message = "";
  while (mqttClient.available()) {
    message.concat((char)mqttClient.read());
  }
  Serial.print("Subscribed is ");
  Serial.println(message);
  mqttClient.unsubscribe(unsubscribedTopic);
}
void openDeur(){
  deur = "Open";
  Serial.println("Moving Forward");
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, HIGH); 
  delay(2000);

  // Stop the DC motor
  Serial.println("Motor stopped");
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, LOW);
  delay(2000);
}
void sluitDeur(){
  deur = "Gesloten";
  Serial.println("Moving Backwards");
  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW); 
  delay(2000);

  // Stop the DC motor
  Serial.println("Motor stopped");
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, LOW);
  delay(1000);
}


void setup() {
  Serial.begin(115200);
  //Connect to MQTT
  connecttoMQTT();

  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);

  input_password.reserve(32); // maximum input characters is 33, change if needed  
}

void loop() {
  char key = keypad.getKey();
  mqttClient.poll();
  if (millis() - mqttPollMillis >= 9001) {
    mqttClient.subscribe(unsubscribedTopic);
    mqttPollMillis = millis(); // Bijwerken van de tijd
  }

  if (key) {
    Serial.print(key);

        if (key == '*') {
      input_password = ""; // clear input password
      Serial.println();
    } else if (key == '#') {
      if (password == input_password) {
        Serial.println();
        Serial.println("The password is correct, ACCESS GRANTED!");
        openDeur();
        sendMessagetoMQTT();
        sluitDeur();
        sendMessagetoMQTT();

      } else {
        Serial.println();
        Serial.println("The password is incorrect, ACCESS DENIED!");
      }

      input_password = ""; // clear input password
    } else {
      input_password += key; // append new character to input password string
    }
  }
}
