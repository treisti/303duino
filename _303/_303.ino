#include <MozziGuts.h>
#include <Sample.h>
#include <LowPassFilter.h>

#include <MIDI.h>
#include <mozzi_midi.h>

#include <tables/saw8192_int8.h>
#include <tables/square_no_alias512_int8.h>

#include "envelope.h"
#include "distorsion.h"

float midiToPercent(int midi) {
  return (float)midi / 127.0f;
}

float analogToPercent(int analog) {
  return (float)analog / 1023.0f;
}

float midiNoteToFrequency(int midiNote, float tuning) {
  return tuning * pow(2, midiNote-57);
}

Sample <SAW8192_NUM_CELLS, AUDIO_RATE> wave_saw(SAW8192_DATA);
Sample <SQUARE_NO_ALIAS512_NUM_CELLS, AUDIO_RATE> wave_square(SQUARE_NO_ALIAS512_DATA);

Envelope env(50, 100, 100);
Distorsion d(20);
LowPassFilter lpf;

bool set_button = false;
bool switch_param = false;
byte wave_shape;
byte legato_mode;

byte master_channel = 1;
byte current_note = 0;
byte previous_note = 0;
int glide_step = 0;


MIDI_CREATE_DEFAULT_INSTANCE();
#define CONTROL_RATE 128


void HandleNoteOn(byte channel, byte note, byte velocity) {
  if(channel == master_channel) {
    if(!current_note) {
      env.start();
    } else if(legato_mode == 0 || legato_mode == 2) {
      env.start();
    }
    
    previous_note = current_note;
    current_note = note;

    if(previous_note) {
      glide_step = 0;
    }
  }
}

void HandleNoteOff(byte channel, byte note, byte velocity) {
  if(channel == master_channel && current_note == note) {
    previous_note = 0;
    current_note = 0;
    env.end();
  }
}


void setup(){
  startMozzi(CONTROL_RATE);

  MIDI.setHandleNoteOn(HandleNoteOn);
  MIDI.setHandleNoteOff(HandleNoteOff);
  MIDI.begin(MIDI_CHANNEL_OMNI);

  pinMode(2, INPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(11, INPUT);

  wave_shape = 0;
  legato_mode = 0;
  if(digitalRead(11) == HIGH) {
    displayLegatoMode();
  } else {
    displayWaveShape();
  }
  
  wave_saw.setLoopingOn();
  wave_square.setLoopingOn();
}


void updateControl(){
  MIDI.read();

  /*float att = 0.0;
  float dec = 0.2;
  float sust = 0.5;
  float rel = 0.1;
  float drive = 0.0;
  float cutoff = 0.2;
  float reso = 1.0;
  float amt = 1.0;*/

  float att = analogToPercent(mozziAnalogRead(A0));
  float dec = analogToPercent(mozziAnalogRead(A1));
  float sust = analogToPercent(mozziAnalogRead(A2));
  float rel = analogToPercent(mozziAnalogRead(A3));
  float drive = 0.0;
  float cutoff = analogToPercent(mozziAnalogRead(A4));
  float reso = analogToPercent(mozziAnalogRead(A5));
  float amt = analogToPercent(mozziAnalogRead(A6));

  int fine_tuning = mozziAnalogRead(A7);

  if(digitalRead(11) == HIGH) {
    if(!switch_param) {
      displayLegatoMode();
      switch_param = true;
    }
    updateLegatoMode();
  } else {
    if(switch_param) {
      displayWaveShape();
      switch_param = false;
    }
    updateWaveShape();
  }
  
  env.update();

  lpf.setCutoffFreq((amt * env.get() + (1.0 - amt)) * cutoff * 150);
  lpf.setResonance(reso * 255);

  d.setDrive(drive);
  
  env.setAttack(att);
  env.setDecay(dec);
  env.setSustain(sust);
  env.setRelease(rel);

  updateFrequency(fine_tuning, 10);
}

void updateWaveShape() {
  if(digitalRead(2) == HIGH) {
    if(!set_button) {
      set_button = true;
      
      wave_shape ++;
      if(wave_shape > 1) {
        wave_shape = 0;
      }

      displayWaveShape();
    }
  } else if(set_button) {
    set_button = false;
  }
}

void displayWaveShape() {
  digitalWrite(3, LOW);
  digitalWrite(4, LOW);
  digitalWrite(3 + wave_shape, HIGH);
}

void updateLegatoMode() {
  if(digitalRead(2) == HIGH) {
    if(!set_button) {
      set_button = true;
      
      legato_mode ++;
      if(legato_mode > 3) {
        legato_mode = 0;
      }

      displayLegatoMode();
    }
  } else if(set_button) {
    set_button = false;
  }
}

void displayLegatoMode() {
  digitalWrite(3, LOW);
  digitalWrite(4, LOW);
  switch(legato_mode) {
    case 1: digitalWrite(3, HIGH); break;
    case 2: digitalWrite(4, HIGH); break;
    case 3: digitalWrite(3, HIGH); digitalWrite(4, HIGH); break;
  }
}

void updateFrequency(int fine_tuning, int glide_range) {
    float freq = (float)pow(2, ((current_note-12) + analogToPercent(fine_tuning))/12.0f) * 8.0f;

    if(previous_note && legato_mode >= 2 && glide_step < glide_range) {
      float previous_freq = (float)pow(2, ((previous_note-12) + analogToPercent(fine_tuning))/12.0f) * 8.0f;
      freq = previous_freq + glide_step * (freq - previous_freq) / glide_range;
      glide_step ++;
    }

    wave_saw.setFreq(freq * (float) SAW8192_SAMPLERATE / (float) SAW8192_NUM_CELLS);
    wave_square.setFreq(freq * (float) SQUARE_NO_ALIAS512_SAMPLERATE / (float) SQUARE_NO_ALIAS512_NUM_CELLS);
}

int updateAudio(){
  //d.next(lpf.next(env.next(wave.next()))) << 6

  switch(wave_shape) {
    case 0: return d.next(lpf.next(env.next(wave_saw.next()))) << 6;
    case 1: return d.next(lpf.next(env.next(wave_square.next()))) << 6;
  }
  return 0;
}


void loop(){
  audioHook();
}
