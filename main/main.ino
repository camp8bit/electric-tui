#include "FastLED.h"

// 9x9 grid
#define NUM_LEDS 9 * 9
#define DATA_PIN 9

// Define the array of leds
CRGB leds[NUM_LEDS];

// twelve servo objects can be created on most boards

int pos = 0;    // variable to store the servo position

float  lerp(float a, float b, float f)
{
    return a + f * (b - a);
}

class Envelope {
  public:
    float attack, decay, sustain, release, start;
    long period;

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
  void trigger (long p) {
    start = millis();
    period = p;
  }

  float value () {
    long t = millis() - start;

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

  bool finished () {
    return millis() - start > period;
  }
};

Envelope *e;

void setup() {
  // ductedFan.attach(7);
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);

  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }
  FastLED.show();
  
  // ductedFan.write(0);

  delay(2000);

  e = new Envelope(5000, 5000, 0.2, 5000);
  e->trigger(20000);
}

int maxPos = 180;
long offset = 0;

void rainbow () {
  int v = e->value() * 255;

  for (int rows = 0; rows < 9 ; rows++) {
    for (int i = 0; i < 9; i++) {
      byte hue = i * 20 + offset;
      leds[i + rows * 9] = CHSV(hue, 255, v);
    }
  }  
}

// Divider governs the speed of pixel to pixel movement
int divider = 3;

// front to the back
void accelerate () {
  int v = e->value() * 255;

  CRGB color = CHSV(floor(offset / divider) * 32, 255, 255);
  
  for (int y = 0; y < 9 ; y++) {
    int x = (offset / divider) % 9;

    leds[x + y * 9] = color;
  }  
}

// front to the back and back to front
void doorsClose () {
  int v = e->value() * 255;

  CRGB color = CHSV(floor(offset / divider) * 32, 255, 255);
  
  for (int y = 0; y < 9 ; y++) {
    int x = (offset / divider) % 9;
    leds[x + y * 9] = color;

    x = 9 - (offset / divider) % 9;
    leds[x + y * 9] = color;
  }  
}


// side to side
void sweep () {
  int v = e->value() * 255;

  CRGB color = CHSV(floor(offset / 100) * 32, 255, 255);
  
  for (int x = 0; x < 9 ; x++) {
    int y = (offset / divider) % 9;

    leds[x + y * 9] = color;
  }  
}

// side to side
void leftRightStrobe () {
  int v = e->value() * 255;

  CRGB color = CRGB::White;

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
  int v = e->value() * 255;

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
  CRGB color = CRGB(255, 255, 255);

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
SimplePatternList gPatterns = { accelerate, sweep, diagonal, snow, rainbow, doorsClose, leftRightStrobe, checkerboard };
uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}

void loop() {
  offset+=1;

  fill_solid(leds, NUM_LEDS, CRGB::Black);

  EVERY_N_SECONDS( 10 ) { nextPattern(); } // change patterns periodically

  // accelerate();
  // sweep();
  // diagonal();
  // snow();
  // rainbow();
  // doorsClose();
  // leftRightStrobe();
  // checkerboard();
  gPatterns[gCurrentPatternNumber]();

  FastLED.show();

  if (e->finished()) {
    e->trigger(20000);
  }
  
  // ductedFan.write(100 * e->value());
  // ductedFan.write(180);
  // ductedFan.write(0);

  // run a 100fps
  delay(2);                       // waits 15ms for the servo to reach the position
  delay(10);
}

