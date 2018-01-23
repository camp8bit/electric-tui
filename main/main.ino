#include "FastLED.h"

// 9x9 grid
#define NUM_LEDS 9 * 9
#define DATA_PIN 9

#define RUNTIME (long) 8 * 3600 * 1000

#include <Stepper.h>

 
// change this to the number of steps on your motor
#define STEPS 200
 
// create an instance of the stepper class, specifying
// the number of steps of the motor and the pins it's
// attached to
Stepper stepper(STEPS, 6,7,8, 12);
 
// Define the array of leds
CRGB leds[NUM_LEDS];

// twelve servo objects can be created on most boards

int pos = 0;    // variable to store the servo position

long cablePosition = 0; // cable position in steps
long destination = 0; // destination position in steps
long motorSleep;
long lightSleep;

int encoder0Click = 3;
int encoder0PinA = 5;
int encoder0PinB = 4;
int encoder0Pos = 0;
int encoder0PinALast = LOW;

void setup() {
  stepper.setSpeed(200);

  // ductedFan.attach(7);
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(192); // save some amps
  
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }
  FastLED.show();
  
  // ductedFan.write(0);

  delay(500);

  destination = -2000;
  motorSleep = millis() + random(0, 2000);
  // lightSleep = millis() + random(0, 2000);

  // Configure encoder
  pinMode (encoder0Click, INPUT);
  pinMode (encoder0PinA, INPUT);
  pinMode (encoder0PinB, INPUT);
  digitalWrite(encoder0Click, HIGH);  

  config();
}

int travelDistance = 1000;

void config () {
  bool n;

  while (!!digitalRead(encoder0Click)) {
    n = digitalRead(encoder0PinA);
    
    if ((encoder0PinALast == LOW) && (n == HIGH)) {
      if (digitalRead(encoder0PinB) == LOW) {
        encoder0Pos--;
      } else {
        encoder0Pos++;
      }
      // Serial.print (encoder0Pos);
      // Serial.print ("/");
    }
    encoder0PinALast = n;  

    encoder0Pos = max(0, encoder0Pos);

    // Each LED is 3 rotations
    travelDistance = encoder0Pos * 600;
    destination = travelDistance;
    
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    fill_solid(leds, encoder0Pos, CRGB::Green);
    FastLED.show();

  }
}
int maxPos = 180;
long offset = 0;

// Divider governs the speed of pixel to pixel movement
int divider = 3;

void rainbow () {
  for (int x= 0; x < 9 ; x++) {
    byte hue = x * 20 + offset;
    CRGB color = CHSV(hue, 255, 255);

    int y = (offset / divider) % 9;
    leds[x + y * 9] = color;
  }  
}

// front to the back
void accelerate () {
  CRGB color = CHSV(floor(offset / divider) * 32, 255, 255);
  
  for (int y = 0; y < 9 ; y++) {
    int x = (offset / divider) % 9;

    leds[x + y * 9] = color;
  }  
}

// side to side
void sweep () {
  CRGB color = CHSV(floor(offset / 100) * 32, 255, 255);
  
  for (int x = 0; x < 9 ; x++) {
    int y = (offset / divider) % 9;

    leds[x + y * 9] = color;
  }  
}

// side to side
void leftRightStrobe () {
  CRGB color = CHSV(floor(offset / 100) * 32, 255, 255);

  int left = (offset / divider / 64) % 2 == 0;
  
  for (int x = 0; x < 9 ; x++) {
    for (int y = 0; y < 9 ; y++) {
      bool active = left ? y < 4 : y > 4;

      active = active && offset % 8 == 0;

      if (active) {
        leds[x + y * 9] = color;
      }
    }
  }  
}
void diagonal () {
  CRGB color = CHSV(floor(offset / 100) * 32, 255, 255);

  int o = (offset / divider) % 9;
  
  for (int x = 0; x < 9 ; x++) {
    for (int y = 0; y < 9 ; y++) {
      bool active = (x + y) % 9 == o;

      if (active) {
        leds[x + y * 9] = color;
      }
    }
  }  
}

void snow () {
  CRGB color = CRGB(255, 255, 255);

  for (int x = 0; x < 9 ; x++) {
    for (int y = 0; y < 9 ; y++) {
      bool active = random(100) < 5;

      if (active) {
        leds[x + y * 9] = color;
      }
    }
  }  
}

void checkerboard () {
  CRGB color = CHSV(floor(offset / 100) * 32, 255, 255);

  for (int x = 0; x < 9 ; x++) {
    for (int y = 0; y < 9 ; y++) {
      int x2 = x + offset / (divider / 2);
      
      bool active = (x2 / 4 + y / 3) % 2 == 0;

      if (active) {
        leds[x + y * 9] = color;
      }
    }
  }  
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { accelerate, sweep, diagonal, snow, rainbow, leftRightStrobe, checkerboard };
uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}

int stepperSpeed = 200;

void loop() {
  EVERY_N_SECONDS( 4 ) { nextPattern(); } // change patterns periodically

  EVERY_N_MILLISECONDS( 40 ) {
    offset+=1;
    fill_solid(leds, NUM_LEDS, CRGB::Black);

    if ((motorSleep > millis()) || (millis() > RUNTIME)) {
      // lights are black if motor is asleep
    } else {
      gPatterns[gCurrentPatternNumber]();
    }
    
    FastLED.show();
  }

  // ductedFan.write(100 * e->value());
  // ductedFan.write(180);
  // ductedFan.write(0);

  // run a 100fps
  // delay(2);                       // waits 15ms for the servo to reach the position

  // One rotation

  int steps = 0;;
  int maxSteps = 20;

  if ((motorSleep > millis()) || (millis() > RUNTIME)) {
    // do nothing
    delay(50);

    // Disable h bridge (dont have standby pin hooked up)
    digitalWrite(6, LOW);
    digitalWrite(7, LOW);
    digitalWrite(8, LOW);
    digitalWrite(12, LOW);
  } else {
    // Try accelerating
    stepperSpeed += 1;

    stepperSpeed  = min(200, stepperSpeed);
    
    stepper.setSpeed(stepperSpeed);

    if (destination > cablePosition) {
      steps = min(maxSteps, destination - cablePosition);
    } else if (destination < cablePosition) {
      steps = max(-maxSteps, destination - cablePosition);
    }
  
    if (destination == cablePosition) {
      stepperSpeed = 100;

      // Disable h bridge (dont have standby pin hooked up)
      digitalWrite(6, LOW);
      digitalWrite(7, LOW);
      digitalWrite(8, LOW);
      digitalWrite(12, LOW);

      // Sleep the motor 3 to 10 minutes
      motorSleep = millis() + random(3, 10) * 60 * 1000;
      
      // Set a new destination for when we wake up
      if (cablePosition < 0) {
        destination = travelDistance;
      } else {
        destination = -travelDistance;
      }
    }
  }
    
  stepper.step(steps);
  cablePosition += steps;
}

