#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

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
AudioConnection          patchCord1(input, 0, effectsMix, 3);
AudioConnection          patchCord2(input, 0, delayMix, 3);
AudioConnection          patchCord3(input, 0, delayEffect, 0);
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

AudioControlSGTL5000     sgtl5000_1;

// Use these with the Teensy Audio Shield
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7   // Teensy 4 ignores this, uses pin 11
#define SDCARD_SCK_PIN   14  // Teensy 4 ignores this, uses pin 13


float delay_time = 450;
float delay_damping = 0.25;
float delay_feedback = .60;
float reverb_damping = 0.5;
float reverb_size = 0.5; 
float reverb_mix = 0.5;
float effects_mix = 0.25;

float lpf = 2000;

float dial = 0.3;

void setup() {
  Serial.begin(9600);

  // Audio connections require memory to work.  For more
  // detailed information, see the MemoryAndCpuUsage example
  AudioMemory(700);

  sgtl5000_1.enable();
  sgtl5000_1.volume(0.5);

  if(dial < 0.5) {
    delay_feedback = 0;
    effects_mix = dial / 2;
  } else {
    delay_feedback = dial - 0.25;
    reverb_mix = 0.5;
    effects_mix = (dial - 0.5);
  }

  set_parameters();
}

void set_parameters() {

  delayEffect.delay(0, delay_time);
  delayEffect.delay(1, delay_time*2);
  delayEffect.delay(2, delay_time*3);

  // delayEffect.delay(2, 600);

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
  effectsMix.gain(1, 1 - effects_mix);

  filter.frequency(lpf);
}

void process()
{
   delay(5000);
   set_parameters();
   
    Serial.print("Max CPU Usage=");
    Serial.print(AudioProcessorUsageMax());
    Serial.print("%; Max Mem Usage=");
    Serial.print(AudioMemoryUsageMax());        
    Serial.print(" blks; input_peak=");
    Serial.print(input_peak.read());
    Serial.print("; output_peak=");
    Serial.print(output_peak.read());
    Serial.print("; delay_peak=");
    Serial.print(delay_peak.read());    
    Serial.print("; reverb_peak=");
    Serial.println(reverb_peak.read());    
    // Serial.print(AudioMemoryUsageMax());    
    // Serial.println(" blks");

    // Serial.print("Reverb=");
    // Serial.print(reverb_mix);
    // Serial.print("; delay_feedback");
    // Serial.print(delay_feedback);
    // Serial.print("; effects_mix");    
    // Serial.println(effects_mix);
}


void loop() {
  process();
  // playFile("GUITAR.WAV");  // filenames are always uppercase 8.3 format
}
