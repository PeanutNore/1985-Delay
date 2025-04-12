/* 1985 Delay
 * Digital Delay Pedal for Guitar
 * High resolution version with half delay time - reads and stores full 12 bit samples in delay array
 * Version 0.2 - tested and working on breadboard
 * 
 *
 * 
 * For AVR512DA28 microcontrollers using DxCore library
 * 
 * Copyright 2025 Samuel Brown. All Rights Reservered.
 * 
 * Licensed under Creative Commons CC-BY-NC-SA 4.0
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * 
 * You are free to share and adapt this software for non-
 * commercial purposes provided that you give appropriate 
 * credit and attribution, and distribute your contributions 
 * under the same CC-BY-NC-SA license.
 * 
 * For commercial licensing contact sam.brown.rit08@gmail.com
 */

//Pin Assignments
uint8_t OutputPin = PIN_PD6;
uint8_t InputPin = PIN_PD0;
uint8_t TimePin = PIN_PD2;
uint8_t MixPin = PIN_PD1;
uint8_t FbkPin = PIN_PD3;
//Audio Samples
uint16_t sampleIn;
uint16_t sampleDelay;
uint16_t sampleFbk;
uint16_t sampleOut;
uint8_t sampleOutHigh;
uint8_t sampleOutLow;
int16_t mathIn;
int16_t mathDelay;
int16_t mathFbk;
int16_t mathOut;
uint16_t delayArray[8000];
//Step Counters
uint16_t sampleStep = 0;
uint16_t delayStep = 0;
//Controls
uint16_t delayTime = 0;
uint16_t mix = 8;
uint16_t fbk = 8;
//Lookup Tables
uint16_t mixOffsets[17] = {0, 1920, 1792, 1664, 1536, 1408, 1280, 1152, 1024, 896, 768, 640, 512, 384, 256, 128, 0};

ISR(TCA0_OVF_vect) {
  //experiment here - clearing the interrupt flag first
  TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;                 //clear the interrupt flags
  //this concludes the experiment. don't forget about the commented line below.
  //read the audio input
  sampleIn = analogRead(InputPin);                          

  //read the delay time
  delayTime = analogRead(TimePin) >> 4;
  ADC0.MUXPOS = ((InputPin & 0x7F) << ADC_MUXPOS_gp);
  delayTime *= 32;
  if (delayTime > 7998){delayTime = 7998;}

  //set the delay step
  delayStep = sampleStep + 1;
  delayStep += delayTime;
  if (delayStep > 7999) {delayStep -= 8000;}

  //get the delayed sample from the array
  sampleDelay = delayArray[delayStep];
  sampleFbk = sampleDelay;
  
  //compute the feedback amount and store the new sample with feedback in the delay array
  fbk = analogRead(FbkPin) >> 8;
  fbk ^= 0x000F;
  if (fbk > 0 ){fbk++;}
  sampleFbk *= fbk;
  sampleFbk = sampleFbk >> 4;
  sampleFbk += mixOffsets[fbk];
  mathFbk = sampleFbk - 2048;
  sampleIn += analogRead(InputPin);   //read the input a second time...
  sampleIn = sampleIn >> 1;           //and average them to reduce quantization noise
  mathIn = sampleIn - 2048;
  mathFbk += mathIn;
  if (mathFbk > 2047){mathFbk = 2047;}
  if (mathFbk < -2048){mathFbk = -2048;}
  delayArray[sampleStep] = mathFbk + 2048;
  
  //compute the mix of sample in and delay sample and prepare sample out
  mix = analogRead(MixPin) >> 8;
  mix ^= 0x000F;
  mix++;
  sampleDelay *= mix;
  sampleDelay = sampleDelay >> 4;
  sampleDelay += mixOffsets[mix];
  mathDelay = sampleDelay - 2048;  
  mathOut = mathIn + mathDelay;
  if (mathOut > 2047){mathOut = 2047;}
  if (mathOut < -2048){mathOut = -2048;}
  sampleOut = mathOut + 2048;
  sampleOut = sampleOut >> 2;
  sampleOutLow = sampleOut << 6;               //variable for to sends the 2 lowest bits of the audio sample to the DAC
  sampleOutHigh = sampleOut >> 2;              //variable for to sends the 8 highest bits of the sample to the DAC 
  sampleStep++;
  if (sampleStep > 7999){sampleStep = 0;}
  DAC0.DATAL = sampleOutLow;                  //make sending of low bits
  DAC0.DATAH = sampleOutHigh;                 //make sending of high bits and be triggering DAC output
  //TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;                 //clear the interrupt flags (commented out for experiment)
  ADC0.MUXPOS = ((InputPin & 0x7F) << ADC_MUXPOS_gp);
}

void setup() {
for (int i = 0; i < 8000; i++){
  delayArray[i] = 512;
}
pinMode(OutputPin, OUTPUT);
pinMode(InputPin, INPUT);
pinMode(TimePin, INPUT);
pinMode(MixPin, INPUT);
pinMode(FbkPin, INPUT);
analogReference(EXTERNAL);
DACReference(EXTERNAL); //found to be missing when testing on final hardware- not an issue on the breadboard running everything at 5V
analogReadResolution(12);
analogSampleDuration(0);
delayTime = analogRead(TimePin) >> 2;
delayTime *= 8;
if (delayTime > 7999){delayTime = 7999;}
DAC0.DATAL = 0b00000000;
DAC0.DATAH = 0b10000000;
DAC0.CTRLA = 0b11000001;
takeOverTCA0();
TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;
TCA0.SINGLE.INTCTRL  = 0b00000001; // overflow interrupt every PER cycles
TCA0.SINGLE.CTRLA    = TCA_SINGLE_CLKSEL_DIV1_gc; // Clock prescaler / 1 (24 MHz)    
TCA0.SINGLE.PER      = 1000;        // 300 * 8 = 2400 clocks, for a 10kHz sample rate, 200 * 8 = 1600 clocks 15kHz sample rate, 150 * 8 = 1200 clocks 20kHz sample rate, 125 * 8 = 1000 clocks 24kHz sample rate
TCA0.SINGLE.CTRLA   |= 1;          // Enables the timer                 

}

void loop() {
  // put your main code here, to run repeatedly:

}
