#include <WiFi.h>
#include <WebServer.h>

WebServer server(80);

const char* id="Lance B: ";

const int buttonPin = 2; // D2
const int ledPin = 4;    // D4

const int LOCKED_TIME=2000;
const int DOUBLE_TIME=200;

enum State{IDLE,DOUBLE_TIMING,LOCKED};

bool lastButtonState = HIGH;
State state = IDLE;
unsigned long timer = 0;

const char* ap_ssid = "ESP32-AP";
const char* ap_password = "12345678";

void setLed(bool on) {
  digitalWrite(ledPin, on ? HIGH : LOW);
}

void opponent_hit() {
  Serial.println("Touche reçu!");
  state=DOUBLE_TIMING;
  timer = millis();
  // server.send(200, "text/plain", "OK");
}

void setup() {
  Serial.begin(115200);
  pinMode(buttonPin, INPUT);
  pinMode(ledPin, OUTPUT);
  setLed(false);

  WiFi.softAP(ap_ssid, ap_password);
  Serial.print(id);
  Serial.print("ESP32 A IP : "); Serial.println(WiFi.softAPIP());

  server.on("/block", opponent_hit);
  server.begin();
}

void hit(){
  WiFiClient client;
  if (client.connect("192.168.4.1", 80)) { // IP 1
    client.print("GET /block HTTP/1.1\r\nHost: 192.168.4.1\r\nConnection: close\r\n\r\n");
    client.stop();
    Serial.print(id);
    Serial.println("Signal envoyé à A.");
  } else {
    Serial.print(id);
    Serial.println("Erreur connexion A !");
  }
}
//*
void loop(){
  server.handleClient();

  switch(state){
    case IDLE:
      if(lastButtonState == HIGH){
        state=LOCKED;
        timer=millis();
        setLed(true);
        //hit();
      }
      break;
    case DOUBLE_TIMING:
      if(millis()-timer > DOUBLE_TIME){
        state=LOCKED;
      }
      if(lastButtonState == HIGH){
        state=LOCKED;
        setLed(true);
      }
      break;
    case LOCKED:
      if(millis()-timer > LOCKED_TIME){
        state=IDLE;
        setLed(false);
        Serial.print(id);
        Serial.println("Locked end");
      }
      break;
  }
  lastButtonState = digitalRead(buttonPin);
  delay(20);
}//*/

//void loop(){
//  Serial.println(digitalRead(buttonPin));
//  delay(100);
//}