#include <WiFi.h>
#include <WebServer.h>

WebServer server(80);

const int buttonPin = 2; // D2
const int ledPin = 3;    // D3

bool lastButtonState = HIGH;
bool blocked = false;
bool doubleTouch = false;
bool localAction = false;

unsigned long blockStart = 0;
unsigned long doubleTouchStart = 0;
unsigned long localPressTime = 0;
unsigned long remotePressTime = 0;

const char* ap_ssid = "ESP32-AP";
const char* ap_password = "12345678";

void setLed(bool on) {
  digitalWrite(ledPin, on ? HIGH : LOW);
}

void handlePressed() {
  remotePressTime = millis();
  Serial.println("Reçu : bouton distant pressé");

  if (abs((long)(remotePressTime - localPressTime)) <= 5000) {
    Serial.println("→ Double touche détectée !");
    doubleTouch = true;
    doubleTouchStart = millis();
    blocked = false;
    localAction = false;
  }

  server.send(200, "text/plain", "OK");
}

void handleBlock() {
  blocked = true;
  blockStart = millis();
  localAction = false;
  setLed(false);
  Serial.println("Reçu : blocage distant !");
  server.send(200, "text/plain", "OK");
}

void setup() {
  Serial.begin(115200);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  setLed(false);

  WiFi.softAP(ap_ssid, ap_password);
  Serial.print("ESP32 A IP : ");
  Serial.println(WiFi.softAPIP());

  server.on("/pressed", handlePressed);
  server.on("/block", handleBlock);
  server.begin();
}

void loop() {
  server.handleClient();
  unsigned long now = millis();

  if (doubleTouch) {
    if (now - doubleTouchStart < 3000) {
      setLed((now / 300) % 2);
    } else {
      doubleTouch = false;
      setLed(false);
      blocked = false;
      localAction = false;
      Serial.println("Double touche terminée");
    }
    return;
  }

  if (blocked && now - blockStart >= 5000) {
    blocked = false;
    setLed(false);
    Serial.println("Fin du blocage");
  }

  bool btnState = digitalRead(buttonPin);
  if (!blocked && lastButtonState == HIGH && btnState == LOW) {
    localPressTime = millis();
    Serial.println("Bouton A pressé !");

    WiFiClient client;
    if (client.connect("192.168.4.2", 80)) {
      client.print("GET /pressed HTTP/1.1\r\nHost: 192.168.4.2\r\nConnection: close\r\n\r\n");
      client.stop();
      Serial.println("Signal /pressed envoyé à B");
    }

    if (abs((long)(localPressTime - remotePressTime)) <= 500) {
      Serial.println("→ Double touche détectée (local) !");
      doubleTouch = true;
      doubleTouchStart = now;
      blocked = false;
      localAction = false;
      return;
    }

    blocked = true;
    localAction = true;
    blockStart = now;
    setLed(true);

    if (client.connect("192.168.4.2", 80)) {
      client.print("GET /block HTTP/1.1\r\nHost: 192.168.4.2\r\nConnection: close\r\n\r\n");
      client.stop();
      Serial.println("Signal /block envoyé à B");
    }
  }

  lastButtonState = btnState;
  delay(50);
}
