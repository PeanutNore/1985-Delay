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
int mathIn;
int mathDelay;
int mathFbk;
int mathOut;
uint16_t delayArray[7168];
//Step Counters
uint16_t sampleStep = 0;
uint16_t delayStep = 0;
//Controls
uint16_t delayTime = 0;
uint8_t mix = 8;
uint8_t fbk = 4;
//Lookup Tables
uint8_t sampleMixF[16] = {0, 1, 3, 1, 3, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1}; //the factor to multiply the input sample by for each mix setting
uint8_t sampleMixS[16] = {0, 3, 4, 2, 3, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0}; //how many bits right to shift the input sample for each mix setting
uint16_t sampleMixO[16] = {0, 256, 384, 512, 768, 1024, 1536, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048};//the number to subtract to center the sample around 0 after applying mix factor and shift
uint8_t delayMixF[16] =  {1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 1, 3, 1, 3, 1, 0};
uint8_t delayMixS[16] =  {0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 3, 2, 4, 3, 0};
uint16_t delayMixO[16] = {2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 1536, 1024, 768, 512, 384, 256, 0};
uint8_t feedbackF[8] = {1, 3, 1, 3, 1, 3, 1, 0};
uint8_t feedbackS[8] = {0, 2, 1, 3, 2, 4, 3, 0};
uint16_t feedbackO[8] = {2048, 1536, 1024, 768, 512, 384, 256, 0};

ISR(TCA0_OVF_vect) {
  //experiment here - clearing the interrupt flag first
  TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;                 //clear the interrupt flags
  //this concludes the experiment. don't forget about the commented line below.
  delayTime = analogRead(TimePin) >> 2;
  delayTime *= 7;
  delayStep = sampleStep + 1;
  delayStep += delayTime;
  if (delayStep > 7167) {delayStep -= 7168;}
  sampleIn = analogRead(InputPin);
  mathIn = sampleIn - 2048;
  sampleDelay = delayArray[delayStep];
  //compute the feedback amount and store the new sample with feedback in the delay array
  fbk = analogRead(FbkPin) >> 9;
  sampleFbk = sampleDelay * feedbackF[fbk];
  sampleFbk = sampleFbk >> feedbackS[fbk];
  mathFbk = sampleFbk - feedbackO[fbk];
  //experiment follows to eliminate undesired infinite repeats
  if (mathFbk < 7 && mathFbk > -8) {mathFbk = 0;} //gets rid of very small feedback samples
  //experiment concludes
  mathFbk += mathIn;
  mathFbk += 2048;
  if (mathFbk > 4095){mathFbk = 4095;}
  if (mathFbk < 0){mathFbk = 0;}
  sampleFbk = mathFbk;
  delayArray[sampleStep] = sampleFbk;
  //compute the mix of sample in and delay sample and prepare sample out
  mix = analogRead(MixPin) >> 8;
  sampleIn = sampleIn * sampleMixF[mix];
  sampleIn = sampleIn >> sampleMixS[mix];
  mathIn = sampleIn - sampleMixO[mix];
  sampleDelay *= delayMixF[mix];
  sampleDelay = sampleDelay >> delayMixS[mix];
  mathDelay = sampleDelay - delayMixO[mix];
  mathOut = mathIn + mathDelay;
  mathOut += 2048;
  if (mathOut > 4095){mathOut = 4095;}
  if (mathOut < 0){mathOut = 0;}
  sampleOut = mathOut >> 2;
  sampleOutLow = sampleOut << 6;               //variable for to sends the 2 lowest bits of the audio sample to the DAC
  sampleOutHigh = sampleOut >> 2;              //variable for to sends the 8 highest bits of the sample to the DAC 
  sampleStep++;
  if (sampleStep > 7167){sampleStep = 0;}
  DAC0.DATAL = sampleOutLow;                  //make sending of low bits
  DAC0.DATAH = sampleOutHigh;                 //make sending of high bits and be triggering DAC output
  //TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;                 //clear the interrupt flags (commented out for experiment)
}

void setup() {
for (int i = 0; i < 7168; i++){
  delayArray[i] = 512;
}
pinMode(OutputPin, OUTPUT);
pinMode(InputPin, INPUT);
pinMode(TimePin, INPUT);
pinMode(MixPin, INPUT);
pinMode(FbkPin, INPUT);
analogReference(EXTERNAL);
analogReadResolution(12);
analogSampleDuration(0);
delayTime = analogRead(TimePin);
delayTime *= 7;
DAC0.DATAL = 0b00000000;
DAC0.DATAH = 0b10000000;
DAC0.CTRLA = 0b11000001;
takeOverTCA0();
TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;
TCA0.SINGLE.INTCTRL  = 0b00000001; // overflow interrupt every PER cycles
TCA0.SINGLE.CTRLA    = TCA_SINGLE_CLKSEL_DIV1_gc; // Clock prescaler / 1 (24 MHz)    
TCA0.SINGLE.PER      = 1600;        // 300 * 8 = 2400 clocks, for a 10kHz sample rate, 200 * 8 = 1600 clocks 15kHz sample rate, 150 * 8 = 1200 clocks 20kHz sample rate, 125 * 8 = 1000 clocks 24kHz sample rate
TCA0.SINGLE.CTRLA   |= 1;          // Enables the timer                 

}

void loop() {
  // put your main code here, to run repeatedly:

}
