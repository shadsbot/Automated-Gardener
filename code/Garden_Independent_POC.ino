#include <OneButton.h>

const int PUMP = 5;
const int ledPin = A6;
const int ALERT_LED = 13;
const int BUTTON_LEFT_PIN = 7;
const int BUTTON_RIGHT_PIN = 9;
const OneButton BUTTON_LEFT(A1, true);
const OneButton BUTTON_RIGHT(A2, true);
const int ONE_HOUR = 3600;

bool edit_time = false;
unsigned long delayTime = 2; // measured in seconds

enum MOISTURE {
  DRY,    // 600+
  DAMP,   // 500 - 599
  MOIST,  // 400 - 499
  WET     // 300 - 399
};

unsigned int pump_how_long = 1000;
bool button_is_held = false;
unsigned long checkTime = millis();
unsigned long oneSecondTimer = millis();

// Setup runs on boot
void setup() {
  // Instantiate Pins
  pinMode(PUMP, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(ALERT_LED, OUTPUT);
  pinMode(BUTTON_LEFT_PIN, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT_PIN, INPUT_PULLUP);

  // Pump is Active Low, but our circuit setup says otherwise
  digitalWrite(PUMP, HIGH);

  // Attach the control functions to the buttons
  BUTTON_LEFT.attachClick(decrement_pump_how_long);
  BUTTON_RIGHT.attachClick(increment_pump_how_long);

  BUTTON_LEFT.attachLongPressStart( [](){ edit_time = true; } );
  BUTTON_RIGHT.attachLongPressStart( [](){ edit_time = false; } );
  
  // Start logging
  Serial.begin(9600);
  Serial.print("Startup complete. Howdy!\n");
}

// Loop is the main application loop
void loop() {
  // Keep watch on the buttons
  BUTTON_LEFT.tick();
  BUTTON_RIGHT.tick();

  // We're in the time editing mode, blink to let us know
  if (edit_time) {
    if (millis() % 200 > 100) {
      digitalWrite(ALERT_LED, HIGH);
    } else {
      digitalWrite(ALERT_LED, LOW);
    }
  } else {
    digitalWrite(ALERT_LED, LOW);
  }

  // Handle LEDs
  digitalWrite(ledPin, HIGH);
  
  // Handle pump
  if (millis() - checkTime > (delayTime * 1000)) {
    MOISTURE moistureSensor = ReadMoisture();
    Serial.print("Moisture Sensor: ");
    Serial.print(moistureSensor);
    Serial.print(" (");
    Serial.print(analogRead(A5));
    Serial.println(")");
    if (moistureSensor == DRY) {
      pump_on();
      delay(pump_how_long);
      pump_off();
    }
    checkTime = millis();
  }
}

void increment_pump_how_long() {
  if (not edit_time) {
    pump_how_long += 500;
    if (pump_how_long > 5000) {
      pump_how_long = 500; // wrap around to lowest
    }
    print_pump_how_long();  
  } else {
    delayTime += ONE_HOUR;
    print_pump_delay_length();
  }
}

void decrement_pump_how_long() {
  if (not edit_time) {
    pump_how_long -= 500;
    if (pump_how_long <= 0) {
      pump_how_long = 5000; // wrap around to max
    }
    print_pump_how_long();    
  } else {
    if (delayTime <= ONE_HOUR) {
      delayTime = 2; // hardcode to be set to 2s if under an hour purely for debugging
      print_pump_delay_length();
    }
  }
}

void print_pump_how_long() {
  Serial.print("SET TO: ");
  Serial.print(pump_how_long);
  Serial.print(" (");
  Serial.print(pump_how_long * .001);
  Serial.println(" seconds)");
  Serial.print("PUMP WILL DISPERSE ");
  Serial.print((pump_how_long * .0005));
  Serial.println("ml OF WATER");
}

void print_pump_delay_length() {
  Serial.print("PUMP WILL DELAY: ");
  Serial.print(delayTime / 3600);
  Serial.print("h (");
  Serial.print(delayTime);
  Serial.println("s)");
}

MOISTURE ReadMoisture() {
  int level = analogRead(A5);
  if (level > 300 && level < 400) {
    return WET;
  } else if (level >= 400 && level < 500) {
    return MOIST;
  } else if (level >= 500 && level < 550) {
    return DAMP;
  }
  return DRY;
}

void pump_on() {
  digitalWrite(PUMP, LOW);
  Serial.println("Pump has been enabled");
}

void pump_off() {
  digitalWrite(PUMP, HIGH);
  Serial.println("Pump has been disabled");
}
