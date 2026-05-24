#include <Servo.h>

Servo myServo;
const int buttonPin = 2;     // Button connected to Pin 2 and GND
int servoState = 0;          // 0 = at 0 degrees, 1 = at 180 degrees
bool lastButtonState = HIGH; // For debouncing

void setup() {
  myServo.attach(3);
  pinMode(buttonPin, INPUT_PULLUP); // Uses internal resistor to keep pin HIGH
  myServo.write(0);                 // Start at 0
  Serial.begin(9600);
  Serial.println("Button Control Initialized. Press to move!");
}

void loop() {
  int currentButtonState = digitalRead(buttonPin);

  // Check if button is pressed (it goes LOW when pressed)
  if (currentButtonState == LOW && lastButtonState == HIGH) {
    delay(50); // Small debounce delay
    
    if (servoState == 0) {
      Serial.println("Moving to 180...");
      sweepMove(0, 180);
      servoState = 1;
    } else {
      Serial.println("Moving to 0...");
      sweepMove(180, 0);
      servoState = 0;
    }
  }
  
  lastButtonState = currentButtonState;
}

// Function to move the servo smoothly to reduce heat
void sweepMove(int start, int end) {
  if (start < end) {
    for (int pos = start; pos <= end; pos++) {
      myServo.write(pos);
      delay(15); 
    }
  } else {
    for (int pos = start; pos >= end; pos--) {
      myServo.write(pos);
      delay(15);
    }
  }
}
