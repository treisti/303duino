#include <MozziGuts.h>
#include <Sample.h>
#include <LowPassFilter.h>

#include <MIDI.h>
#include <mozzi_midi.h>

#include "tb_303.h"
#include "envelope.h"
#include "distorsion.h"

//DEBUG
#include <EventDelay.h>
EventDelay kTriggerDelay;
byte debugnote[] = {0x24, 0x27, 0x2b, 0x30, 0x24, 0x27, 0x33, 0x30, 0x24, 0x27, 0x2b, 0x30, 0x24, 0x33, 0x27, 0x30, 0x20, 0x27, 0x2b, 0x30, 0x20, 0x27, 0x2b, 0x30, 0x20, 0x27, 0x2b, 0x30, 0x20, 0x27, 0x2b, 0x30};
int debugcurrent = 0;
//END_DEBUG

float midiToPercent(int midi) {
  return (float)midi / 127.0f;
}

float analogToPercent(int analog) {
  return (float)analog / 1023.0f;
}

float midiNoteToFrequency(int midiNote, float tuning) {
  return tuning * pow(2, midiNote-57);
}


Sample <TB_303_NUM_CELLS, AUDIO_RATE> wave(TB_303_DATA);

Envelope env(50, 100, 100);
Distorsion d(20);
LowPassFilter lpf;

byte master_channel = 0x90;
byte current_note = 0x00;


MIDI_CREATE_DEFAULT_INSTANCE();
#define CONTROL_RATE 256


void HandleNoteOn(byte channel, byte note, byte velocity) {
  if(channel == master_channel) {
    current_note = note;
    env.start();
    wave.setFreq((float) TB_303_SAMPLERATE / (float) TB_303_NUM_CELLS * (float)pow(2, (note-0x18)/12.0f) * 1.5f);
  }
}

void HandleNoteOff(byte channel, byte note, byte velocity) {
  if(channel == master_channel && current_note == note) {
    current_note = 0x00;
    env.end();
  }
}


void setup(){
  startMozzi(CONTROL_RATE);

  MIDI.setHandleNoteOn(HandleNoteOn);
  MIDI.setHandleNoteOff(HandleNoteOff);
  MIDI.begin(MIDI_CHANNEL_OMNI);  
  
  wave.setLoopingOn();

  kTriggerDelay.set(110); // DEBUG
}


void updateControl(){
  MIDI.read();
  
  env.update();
  lpf.setCutoffFreq(env.get() * 50);

  // DEBUG
  if(kTriggerDelay.ready()){
    HandleNoteOn(master_channel, debugnote[debugcurrent], 0x00);
    debugcurrent++;
    debugcurrent%=32;
    kTriggerDelay.start();
  }
  //END_DEBUG

  d.setDrive(0.5f);
  lpf.setResonance((int)(1.0f * 255.0f));
  //analogToPercent(mozziAnalogRead(A0)
  
  env.setAttack(0.1);
  env.setDecay(0.2);
  env.setSustain(0.5);
  env.setRelease(0.0);
}


int updateAudio(){
  return d.next(lpf.next(env.next(wave.next())));
}


void loop(){
  audioHook();
}
