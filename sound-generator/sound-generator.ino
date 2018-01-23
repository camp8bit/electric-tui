#include <MozziGuts.h>
#include <Oscil.h>
#include <EventDelay.h>
#include <ADSR.h>
#include <tables/sin8192_int8.h> 
#include <tables/saw8192_int8.h> 
#include <mozzi_rand.h>
#include <mozzi_midi.h>

#define CONTROL_RATE 64

Oscil <8192, AUDIO_RATE> aOscil(SAW8192_DATA);; 
Oscil <8192, AUDIO_RATE> bOscil(SIN8192_DATA);; 

// for triggering the envelope
EventDelay noteDelay;

ADSR <CONTROL_RATE, AUDIO_RATE> envelope;

boolean note_is_on = true;

void setup(){
  //Serial.begin(9600); // for Teensy 3.1, beware printout can cause glitches
  Serial.begin(115200);
  randSeed(); // fresh random
  noteDelay.set(2000); // 2 second countdown
  startMozzi(CONTROL_RATE);
}


unsigned int duration, attack, decay, sustain, release_ms;
int baseFreq;

void updateControl(){
  if(noteDelay.ready()){
      bool longNote = rand(5) == 0;
      
      // choose envelope levels
//      unsigned int attack_level = rand(128)+32;
//      unsigned int decay_level = rand(255);


      envelope.setADLevels(255,0);

    // generate a random new adsr time parameter value in milliseconds
     unsigned int new_value = rand(300) + 5;
     // Serial.println(new_value);
     // randomly choose one of the adsr parameters and set the new value
     switch (rand(4)){
       case 0:
       attack = new_value;
       break;
       
       case 1:
       decay = new_value;
       break;
       
       case 2:
       sustain = new_value;
       break;
       
       case 3:
       release_ms = new_value;
       break;
     }

     if (longNote) {
       envelope.setTimes(attack * 4,decay * 4,sustain * 4,release_ms * 4);    
       noteDelay.start(4 * (attack+decay+sustain+release_ms)+rand(5000));
     } else {
       envelope.setTimes(attack,decay,sustain,release_ms);    
       noteDelay.start(attack+decay+sustain+release_ms+rand(5000));
     }
     envelope.noteOn();

     byte midi_note = rand(40)+50;
     baseFreq = (int)mtof(midi_note);
/*
     // print to screen
     Serial.print("midi_note\t"); Serial.println(midi_note);
     Serial.print("attack_level\t"); Serial.println(attack_level);
     Serial.print("decay_level\t"); Serial.println(decay_level);
     Serial.print("attack\t\t"); Serial.println(attack);
     Serial.print("decay\t\t"); Serial.println(decay);
     Serial.print("sustain\t\t"); Serial.println(sustain);
     Serial.print("release\t\t"); Serial.println(release_ms);
     Serial.println();
*/
     // wait up to 5 seconds between twirts
     
   }

  envelope.update();
} 


int updateAudio(){
  int env = envelope.next();
  
  aOscil.setFreq(baseFreq + env);
  bOscil.setFreq(baseFreq + env * 3 / 2);

  return (int) (env * ((bOscil.next() + aOscil.next()) >> 1))>>8;
}


void loop(){
  audioHook(); // required here
}

