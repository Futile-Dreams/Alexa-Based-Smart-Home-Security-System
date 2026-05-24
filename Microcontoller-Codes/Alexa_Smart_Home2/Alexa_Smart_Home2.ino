#include <WiFi.h>
#include <Espalexa.h>

// --- WiFi Configuration ---
const char* ssid = "------";
const char* password = "---------;

// --- MOSFET Pin Definitions ---
#define PIN_BEDROOM 13 
#define PIN_KITCHEN 12 
#define PIN_LIVING  27 
#define PIN_BATHROOM 33 
#define PIN_FAN      32 

Espalexa espalexa;

// --- Alexa Callbacks ---
void bedroomCB(uint8_t b) { digitalWrite(PIN_BEDROOM, b > 0 ? HIGH : LOW); }
void kitchenCB(uint8_t b) { digitalWrite(PIN_KITCHEN, b > 0 ? HIGH : LOW); }
void livingCB(uint8_t b)  { digitalWrite(PIN_LIVING, b > 0 ? HIGH : LOW); }
void bathroomCB(uint8_t b) { digitalWrite(PIN_BATHROOM, b > 0 ? HIGH : LOW); }
void fanCB(uint8_t b)      { digitalWrite(PIN_FAN, b > 0 ? HIGH : LOW); }

void setup() {
  Serial.begin(115200);

  // Initialize all MOSFET pins
  int pins[] = {13, 12, 27, 33, 32};
  for(int p : pins) { 
    pinMode(p, OUTPUT); 
    digitalWrite(p, LOW); 
  }

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");

  // Add devices to Alexa
  espalexa.addDevice("Bedroom", bedroomCB);
  espalexa.addDevice("Kitchen", kitchenCB);
  espalexa.addDevice("Living Room", livingCB);
  espalexa.addDevice("Bathroom", bathroomCB);
  espalexa.addDevice("Fan", fanCB);

  espalexa.begin();
}

void loop() {
  espalexa.loop(); // IMPORTANT: Keep this running fast!
}
