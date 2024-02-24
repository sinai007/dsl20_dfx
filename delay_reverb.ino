#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include "CabIR.h"
#include "WavLoader.h"
#include "ToggleButton.h"

// #include "ir.h"

CabIR  cab;
WavLoader loader;
AudioFilterStateVariable cabFilter;        //xy=937.1429100036621,304.2856788635254

// GUItool: begin automatically generated code
AudioInputI2S            input;           //xy=70.28570938110352,561.4285545349121
AudioEffectDelay         delayEffect;         //xy=245.9999771118164,329.2856979370117
AudioAnalyzePeak         input_peak;          //xy=357.14283752441406,632.8571586608887
AudioMixer4              delayMix; //xy=474.00003814697266,436.1428813934326
AudioAnalyzePeak         delay_peak;          //xy=664.2857142857142,508.5714285714285
AudioEffectFreeverb      reverb;      //xy=737.2857627868652,284.8571586608887
AudioFilterStateVariable filter;        //xy=937.1429100036621,304.2856788635254
AudioMixer4              reverbMix; //xy=1043.2857513427734,439.0000457763672
AudioAnalyzePeak         reverb_peak;          //xy=1262.8571428571427,388.57142857142856
AudioMixer4              effectsMix;         //xy=1403.7142639160156,522.8570976257324
AudioOutputI2S           output;           //xy=1605.7143440246582,522.8571166992188
AudioAnalyzePeak         output_peak; //xy=1611.4286346435547,628.5714435577393
AudioAnalyzePeak         cab_peak;          //xy=1262.8571428571427,388.57142857142856

AudioConnection          patchCord0(input, 0, cab, 0);
AudioConnection          patchCord01(input, 0, cab, 1);
AudioConnection          patchCord02(cab, 0, cabFilter, 0);
AudioConnection          patchCord1(cabFilter, 0, effectsMix, 3);
AudioConnection          patchCord2(cabFilter, 0, delayMix, 3);
AudioConnection          patchCord3(cabFilter, 0, delayEffect, 0);
// AudioConnection          patchCord1(input, 0, effectsMix, 3);
// AudioConnection          patchCord2(input, 0, delayMix, 3);
// AudioConnection          patchCord3(input, 0, delayEffect, 3);
AudioConnection          patchCord4(input, 0, input_peak, 0);

AudioConnection          patchCord5(delayEffect, 0, delayMix, 0);
AudioConnection          patchCord6(delayEffect, 1, delayMix, 1);
AudioConnection          patchCord7(delayEffect, 2, delayMix, 2);
AudioConnection          patchCord8(delayMix, reverb);
AudioConnection          patchCord9(delayMix, 0, reverbMix, 1);
AudioConnection          patchCord10(delayMix, delay_peak);
AudioConnection          patchCord11(reverb, 0, filter, 0);
AudioConnection          patchCord12(filter, 0, reverbMix, 0);
AudioConnection          patchCord13(reverbMix, 0, effectsMix, 0);
AudioConnection          patchCord14(reverbMix, reverb_peak);
AudioConnection          patchCord15(effectsMix, 0, output, 0);
AudioConnection          patchCord16(effectsMix, 0, output, 1);
AudioConnection          patchCord17(effectsMix, output_peak);
// GUItool: end automatically generated code
AudioConnection pacthCord18(cab, 0, cab_peak, 0);


AudioControlSGTL5000     sgtl5000_1;


float delay_time = 450;
float delay_damping = 0.25;
float delay_feedback = .60;
float reverb_damping = 0.5;
float reverb_size = 0.5; 
float reverb_mix = 0.5;
float effects_mix = 0.25;

float lpf = 2500;

float dial = 0.8;

#define CHANNEL_SWITCH 32
#define CAB_SWITCH 31
#define REVERB_LEVEL A1
#define CLEAN_CHANNEL 33
#define ULTRA_CHANNEL 34


ToggleButton channelSwitch = ToggleButton( "Channel", CHANNEL_SWITCH); 
ToggleButton cabSwitch = ToggleButton( "Cab", CAB_SWITCH); 

// Cabinet Parameters ----------------------------------------------------------
#define partitionsize 128

const int nc = 17920; // number of taps for the FIR filter 
const int PROGMEM nfor = nc / partitionsize; // number of partition blocks --> nfor = nc / partitionsize       ** nfor should not be greater than 140
const uint32_t PROGMEM FFT_L = 2 * partitionsize; 

float32_t DMAMEM maskgen[FFT_L * 2];            
float32_t DMAMEM  fftout[nfor][512];  // nfor should not exceed 140

float32_t irBuffer[17920];
// END Cabinet Parameters ----------------------------------------------------------

void enable_headphones() {
  sgtl5000_1.muteLineout();
  sgtl5000_1.muteHeadphone();
  sgtl5000_1.audioPostProcessorEnable();
  sgtl5000_1.surroundSoundEnable();
  sgtl5000_1.surroundSound(2, 7);
  sgtl5000_1.unmuteHeadphone();
}

void toggleCab() {
  cab.toggleBypass();
  Serial.printf("Cab Switched: %d\n", cabSwitch.buttonState);
}

void toggleChannel() {
  Serial.printf("Channel Switched: %d\n", channelSwitch.buttonState);
  digitalToggle(CLEAN_CHANNEL);
  digitalToggle(ULTRA_CHANNEL);
}
  
void setup() {

  Serial.begin(9600);
  AudioMemory(700);

  sgtl5000_1.enable();
  sgtl5000_1.volume(0.5);
  enable_headphones();
  
  pinMode(CLEAN_CHANNEL, OUTPUT);
  pinMode(ULTRA_CHANNEL, OUTPUT);

  digitalWrite(ULTRA_CHANNEL, HIGH);
  digitalWrite(CLEAN_CHANNEL, LOW);

  channelSwitch.setup();
  cabSwitch.setup();

  // loader.as_samples("M25i.wav", 0, irBuffer, 17920);
  loader.as_samples("M25.wav", 0, irBuffer, 17920);
  // loader.raw("T75.raw", irBuffer, 17920);

  set_parameters();

  cab.begin(0,.15,*fftout,nfor); // turn off update routine until after filter mask is generated, set Audio_gain=1.00 , point to fftout array, specify number of partitions
  cab.impulse(irBuffer, maskgen,nc);
  cab.bypass(false);
}

void set_parameters() {

  if(dial < 0.1) {
    effects_mix = 0;
  } else if(dial < 0.5) {
    delay_feedback = 0;
    effects_mix = dial;
    reverb_mix = .5;
    reverb_size = .5 + dial;
    reverb_damping = 1 - dial;
  } else {
    delay_feedback = dial - 0.25;
    reverb_mix = 0.5;
    reverb_damping =  0.25;
    reverb_size = 0.75;
    effects_mix = 0.5;// (dial - 0.5);
  }

  // set delay effects
  delayEffect.delay(0, delay_time);
  delayEffect.delay(1, delay_time*2);
  delayEffect.delay(2, delay_time*3);

  // set reverb effects
  reverb.damping(reverb_damping);
  reverb.roomsize(reverb_size);
  
  // 100% Wet Reverb
  reverbMix.gain(0, reverb_mix);
  reverbMix.gain(1, 1-reverb_mix);

  // 100% Delay with tails attenuated
  delayMix.gain(0, delay_feedback);
  delayMix.gain(1, delay_feedback*delay_damping);
  delayMix.gain(2, delay_feedback*delay_damping*delay_damping);
  delayMix.gain(3, 1-delay_feedback);
  
  // 50/50 Wet/Dry
  effectsMix.gain(0, effects_mix);
  // effectsMix.gain(1, 1 - effects_mix);

  // Filter out the fizzies
  filter.frequency(lpf);
  cabFilter.frequency(5000);
}

void process()
{
  channelSwitch.update(&toggleChannel);
  cabSwitch.update(&toggleCab);
 
  int v = analogRead(REVERB_LEVEL);
  dial = v / 1023.0;

  set_parameters();
     
}


void loop() {
  process();
}
