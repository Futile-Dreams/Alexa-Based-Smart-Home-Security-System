#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);
Servo myServo;

byte authorizedUID[] = {0x73, 0x89, 0xF5, 0x31}; // Use your actual UID here
bool doorOpen = false; // Tracks the current state of the door

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  myServo.attach(3);
  myServo.write(0); // Ensure door starts closed
  Serial.println("System Ready: Scan to Toggle Door");
}

void loop() {
  // 1. Check for a card
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // 2. Verify UID
  bool authorized = true;
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] != authorizedUID[i]) {
      authorized = false;
      break;
    }
  }

  // 3. Toggle Logic
  if (authorized) {
    if (doorOpen == false) {
      Serial.println("Authorized: Opening Door...");
      myServo.write(90); // Move to open position
      doorOpen = true;   // Update state
    } 
    else {
      Serial.println("Authorized: Closing Door...");
      myServo.write(0);  // Move to closed position
      doorOpen = false;  // Update state
    }
    
    // 4. Critical: Add a small delay so it doesn't 
    // "double-tap" from a single long scan
    delay(2000); 
    Serial.println("Ready for next scan.");
  } else {
    Serial.println("Access Denied!");
    delay(1000);
  }
  
  // Halt PICC to stop reading the same card repeatedly
  mfrc522.PICC_HaltA();
}
