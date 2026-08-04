#include <ESP8266WiFi.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>

// WiFi credentials
const char* IO_USERNAME = "YOUR_USERNAME";
const char* IO_KEY = "YOUR_ADAFRUIT_IO_KEY";

// Adafruit IO credentials
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "YOUR_ADAFRUIT_USERNAME"
#define AIO_KEY         "YOUR_ADAFRUIT_IO_KEY"

// Motor pins
#define ENA   4
#define IN_1  0
#define IN_2  2
#define IN_3  12
#define IN_4  13
#define ENB   15
#define Light 16

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// Set up the command feed
Adafruit_MQTT_Subscribe commandFeed = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/command");

String command;
int speedCar = 150; // 0 to 255
int speed_low = 60;

void setup() {
  Serial.begin(115200);

  // Set up motor pins
  pinMode(ENA, OUTPUT); 
  pinMode(IN_1, OUTPUT);
  pinMode(IN_2, OUTPUT);
  pinMode(IN_3, OUTPUT);
  pinMode(IN_4, OUTPUT);
  pinMode(ENB, OUTPUT); 
  pinMode(Light, OUTPUT);

  // Connect to WiFi
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Connect to Adafruit IO
  mqtt.subscribe(&commandFeed);
}

void loop() {
  // Ensure the connection to Adafruit IO
  MQTT_connect();

  // Check for new messages
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &commandFeed) {
      command = (char *)commandFeed.lastread;
      handleCommand(command);
    }
  }
}

void handleCommand(String command) {
  if (command.startsWith("SPEED:")) {
    speedCar = command.substring(6).toInt();
    Serial.print("Speed set to ");
    Serial.println(speedCar);
  } else if (command == "LIGHT:TOGGLE") {
    toggleLight();
  } else {
    if (command == "F") goForward();
    else if (command == "B") goBack();
    else if (command == "L") goLeft();
    else if (command == "R") goRight();
    else if (command == "I") goForwardRight();
    else if (command == "G") goForwardLeft();
    else if (command == "J") goBackRight();
    else if (command == "H") goBackLeft();
    else if (command == "S") stopRobot();
  }
}

void goForward() {
  Serial.print("Forward ");
  digitalWrite(IN_1, HIGH);
  digitalWrite(IN_2, LOW);
  analogWrite(ENA, speedCar);
  digitalWrite(IN_3, LOW);
  digitalWrite(IN_4, HIGH);
  analogWrite(ENB, speedCar);
  Serial.println(speedCar);
}

void goBack() {
  Serial.print("Backward ");
  digitalWrite(IN_1, LOW);
  digitalWrite(IN_2, HIGH);
  analogWrite(ENA, speedCar);
  digitalWrite(IN_3, HIGH);
  digitalWrite(IN_4, LOW);
  analogWrite(ENB, speedCar);
}

void goRight() {
  Serial.print("Right ");
  digitalWrite(IN_1, LOW);
  digitalWrite(IN_2, HIGH);
  analogWrite(ENA, speedCar);
  digitalWrite(IN_3, LOW);
  digitalWrite(IN_4, HIGH);
  analogWrite(ENB, speedCar);
  Serial.println(speedCar);
}

void goLeft() {
  Serial.print("Left ");
  digitalWrite(IN_1, HIGH);
  digitalWrite(IN_2, LOW);
  analogWrite(ENA, speedCar);
  digitalWrite(IN_3, HIGH);
  digitalWrite(IN_4, LOW);
  analogWrite(ENB, speedCar);
  Serial.println(speedCar);
}

void goForwardRight() {
  Serial.print("ForwardRight ");
  digitalWrite(IN_1, HIGH);
  digitalWrite(IN_2, LOW);
  analogWrite(ENA, speedCar - speed_low);
  digitalWrite(IN_3, LOW);
  digitalWrite(IN_4, HIGH);
  analogWrite(ENB, speedCar);
  Serial.println(speedCar);
}

void goForwardLeft() {
  Serial.print("ForwardLeft ");
  digitalWrite(IN_1, HIGH);
  digitalWrite(IN_2, LOW);
  analogWrite(ENA, speedCar);
  digitalWrite(IN_3, LOW);
  digitalWrite(IN_4, HIGH);
  analogWrite(ENB, speedCar - speed_low);
  Serial.println(speedCar);
}

void goBackRight() {
  Serial.print("BackRight ");
  digitalWrite(IN_1, LOW);
  digitalWrite(IN_2, HIGH);
  analogWrite(ENA, speedCar - speed_low);
  digitalWrite(IN_3, HIGH);
  digitalWrite(IN_4, LOW);
  analogWrite(ENB, speedCar);
  Serial.println(speedCar);
}

void goBackLeft() {
  Serial.print("BackLeft ");
  digitalWrite(IN_1, LOW);
  digitalWrite(IN_2, HIGH);
  analogWrite(ENA, speedCar);
  digitalWrite(IN_3, HIGH);
  digitalWrite(IN_4, LOW);
  analogWrite(ENB, speedCar - speed_low);
  Serial.println(speedCar);
}

void stopRobot() {
  Serial.print("Stop ");
  digitalWrite(IN_1, LOW);
  digitalWrite(IN_2, LOW);
  analogWrite(ENA, speedCar);
  digitalWrite(IN_3, LOW);
  digitalWrite(IN_4, LOW);
  analogWrite(ENB, speedCar);
  Serial.println(speedCar);
}

void toggleLight() {
  static bool lightState = false;
  lightState = !lightState;
  digitalWrite(Light, lightState ? HIGH : LOW);
  Serial.println(lightState ? "Lights_On" : "Lights_Off");
}

void MQTT_connect() {
  int8_t ret;
  if (mqtt.connected()) {
    return;
  }
  
  Serial.print("Connecting to MQTT... ");
  while ((ret = mqtt.connect()) != 0) {
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying in 5 seconds...");
    mqtt.disconnect();
    delay(5000);
  }
  Serial.println("MQTT Connected!");
}