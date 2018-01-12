#include "FastLED.h"

// 9x9 grid
#define NUM_LEDS 9 * 9
#define DATA_PIN 9

// Define the array of leds
CRGB leds[NUM_LEDS];

#include <Servo.h>

Servo ductedFan;  // create servo object to control a servo
// twelve servo objects can be created on most boards

int pos = 0;    // variable to store the servo position

float  lerp(float a, float b, float f)
{
    return a + f * (b - a);
}

class Envelope {
  public:
    float attack, decay, sustain, release, start;
    int period;

  // attack, decay and release are in millis
  // sustain is an amplitude (float of 0-1)
  
  Envelope (float a, float d, float s, float r) {
    start = millis();

    attack = a;
    decay = d; 
    sustain = s;
    release = r; 
  }

  // You have to specify how long the period is (the time from
  // noteOn to noteOff) in millis
  void trigger (int p) {
    start = millis();
    period = p;
  }

  float value () {
    int t = millis() - start;

    if (t < attack) {
      return lerp (0, 1, 1.0 / attack * t);
    } else if (t < attack + decay) {
      return lerp (1, sustain, 1.0 / decay * (t - attack));
    } else if (t < period - release) {
      return sustain;
    } else if (t < period) {
      return lerp (sustain, 0, 1.0 / release * (t - period + release ));
    } else {
      return 0;
    }
  }
};

Envelope *e;

void setup() {
  ductedFan.attach(7);  // attaches the servo on pin 9 to the servo object
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);

  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }
  FastLED.show();

  delay(10000);

  e = new Envelope(5000, 5000, 0.2, 5000);
  e->trigger(60000);
}

int maxPos = 180;

void loop() {
  int v = e->value() * 255;
  
  for (int i = 0; i < NUM_LEDS; i++) {
    // leds[i] = CHSV(92, 255, v);
    leds[i] = CRGB::Black;
  }

  FastLED.show();

  // ductedFan.write((float) e->value() * maxPos);
  ductedFan.write(180);
  // ductedFan.write(0);

  delay(15);                       // waits 15ms for the servo to reach the position
}

