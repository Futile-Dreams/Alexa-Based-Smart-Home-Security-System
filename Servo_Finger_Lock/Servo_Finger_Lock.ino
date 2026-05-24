#include <WiFi.h>
#include <ESP32Servo.h>
#include <Adafruit_Fingerprint.h>

// --- Configuration ---
const char* ssid = "AlexaHome";
const char* password = "123456789";

// Pins
#define SERVO_PIN 13
#define RXD2 16
#define TXD2 17

// Hardware Objects
Servo myServo;
WiFiServer server(80);
HardwareSerial mySerial(2);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// Logic Variables
unsigned long prevSensorMillis = 0;
unsigned long lockTimer = 0;
bool isUnlocked = false;
bool guestMode = false; 

// Mock Environmental Data
float t = 24.2; 
float h = 48.5;

// --- Helper Functions ---

void lockDoor() {
  myServo.write(0);
  isUnlocked = false;
  Serial.println("Action: Locked");
}

void unlockDoor() {
  myServo.write(90);
  isUnlocked = true;
  lockTimer = millis(); // Reset the timer for auto-lock
  Serial.println("Action: Unlocked");
}

void checkFingerprint() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) return; // No finger detected, exit function

  if (finger.image2Tz() == FINGERPRINT_OK) {
    if (finger.fingerFastSearch() == FINGERPRINT_OK) {
      Serial.print("Fingerprint Match! ID #"); Serial.println(finger.fingerID);
      unlockDoor(); 
    } else {
      Serial.println("Access Denied: Unknown Finger.");
    }
  }
}

// --- Main Setup ---

void setup() {
  Serial.begin(115200);
  
  // 1. Setup Servo
  ESP32PWM::allocateTimer(0);
  myServo.setPeriodHertz(50);
  myServo.attach(SERVO_PIN, 500, 2400); 
  lockDoor(); // Start locked

  // 2. Setup Fingerprint
  mySerial.begin(57600, SERIAL_8N1, RXD2, TXD2);
  if (finger.verifyPassword()) {
    Serial.println("Fingerprint sensor found!");
  } else {
    Serial.println("Fingerprint sensor NOT found. Check wiring.");
  }

  // 3. Setup WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nWiFi connected!");
  Serial.println(WiFi.localIP());
  server.begin();
}

// --- Main Loop ---

void loop() {
  // 1. Check Fingerprint Sensor (Non-blocking)
  checkFingerprint();

  // 2. Mock Sensor Logic (Updates every 5s)
  if (millis() - prevSensorMillis >= 5000) {
    prevSensorMillis = millis();
    t += (random(-10, 11) / 100.0);
    h += (random(-20, 21) / 100.0);
  }

  // 3. Auto-Relock Logic
  // Relocks if NOT in guest mode AND time has expired (7 seconds)
  if (!guestMode && isUnlocked && (millis() - lockTimer >= 7000)) {
    lockDoor();
    Serial.println("System: Auto-Locked (Timeout)");
  }

  // 4. Web Server Logic
  WiFiClient client = server.available();
  if (client) {
    String request = client.readStringUntil('\r');
    client.flush();

    bool needsRedirect = false;

    // Command Parsing
    if (request.indexOf("GET /unlock") != -1) {
      unlockDoor();
      needsRedirect = true;
    } 
    else if (request.indexOf("GET /lock") != -1) {
      lockDoor();
      needsRedirect = true;
    }
    else if (request.indexOf("GET /togGuest") != -1) {
      guestMode = !guestMode;
      needsRedirect = true;
    }

    // Response Generation
    if (needsRedirect) {
      client.println("HTTP/1.1 200 OK\nContent-Type: text/html\n\n");
      client.println("<html><script>window.location.href='/';</script></html>");
    } else {
      // Send the HTML UI (Keeping your existing CSS/Styles)
      client.println("HTTP/1.1 200 OK\nContent-Type: text/html\n\n");
      client.println("<!DOCTYPE html><html><head>");
      client.println("<meta name='viewport' content='width=device-width, initial-scale=1.0'>");
      client.println("<style>body{font-family:sans-serif; text-align:center; background:#f4f4f4; padding:10px;} .card{background:white; margin:10px auto; padding:20px; border-radius:15px; max-width:400px; box-shadow:0 4px 6px rgba(0,0,0,0.1);} .btn{display:block; width:100%; box-sizing:border-box; padding:20px; margin:10px 0; font-size:18px; color:white; border:none; border-radius:10px; cursor:pointer; text-decoration:none;} .unlock{background:#2ecc71;} .lock{background:#e74c3c;} .mode{background:#3498db;} .refresh{background:#95a5a6; font-size:14px; padding:10px;}</style></head><body>");
      client.println("<h1>Smart Lock Multi-Modal</h1>");
      client.println("<a href='/' class='btn refresh'>REFRESH STATUS</a>");
      client.print("<div class='card'><h3>Environment</h3>Temp: "); client.print(t, 1); client.print("C | Hum: "); client.print(h, 1); client.println("%</div>");
      client.print("<div class='card'><h3>Lock Mode</h3><p>Mode: <b>"); client.print(guestMode ? "GUEST (Manual)" : "PRIVATE (Auto-Lock)"); client.print("</b></p>");
      client.println("<a href='/togGuest' class='btn mode'>Switch Mode</a></div>");
      client.println("<div class='card'><h3>Security Control</h3>");
      if (isUnlocked) {
        client.println("<p style='color:orange; font-weight:bold;'>STATUS: UNLOCKED</p>");
        client.println("<a href='/lock' class='btn lock'>LOCK NOW</a>");
      } else {
        client.println("<p style='color:green; font-weight:bold;'>STATUS: SECURED</p>");
        client.println("<a href='/unlock' class='btn unlock'>REMOTE UNLOCK</a>");
      }
      client.println("</div></body></html>");
    }
    client.stop();
  }
}
