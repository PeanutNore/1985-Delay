/* 1985 Delay
 * Digital Delay Pedal for Guitar
 * Reverse Delay with Octave Up
 * 
 * For AVR128DA28 microcontrollers using DxCore library
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
//Audio Samples
unsigned int sampleIn = 512;
int mathSample;
unsigned int USdelaySample = 256;
int delaySample = 0;
uint8_t sampleOutLow = 0;
uint8_t sampleOutHigh = 0;
byte delayArray[14336];
unsigned int sampleStep = 0;                    //the position of the "record head". Moves "clockwise" (counts up)
int delayStep = 14335;                           //the position of the "playback head". Moves "counter clockwise" (counts down)
unsigned int delayTime = 0;


ISR(TCA0_OVF_vect) {
  delayStep--;                                  //moves the playback head counterclockwise
  delayStep--;                                  //moves the playback head counterclockwise again, so it goes twice as fast as the record head
  sampleStep++;                                 //moves the record head clockwise
  delayTime = analogRead(TimePin) * 2;            //determine the delay time by reading the pot on the Time pim
  delayTime = 14335 - delayTime;
  if (delayStep < 0) {delayStep = delayTime;}     //resets the playback head if it's out of bounds
  if (sampleStep > delayTime) {sampleStep = 0;}   //resets the record head if it's out of bounds
  sampleIn = analogRead(InputPin) >> 2;           //ADC is running in 12 bit but the DAC is 10 bit so toss 2 bits
  delaySample = delayArray[delayStep] - 128;     //Retrieve the delayed sample from the array. let's try just leaving it at 8 bits. Center it around 0.
  delayArray[sampleStep] = sampleIn >> 2;       //store what we're about to output in the delay array
  mathSample = sampleIn - 512;                  //center the 10 bit input sample around 0
  mathSample += delaySample;                    //add the delayed sample to the input sample
  mathSample += 512;                            //re-center around 512
  if (mathSample > 1023) {mathSample = 1023;}   //constrain high values to 10 bits
  if (mathSample < 0) {mathSample = 0;}         //prevent negative values
  sampleIn = mathSample;                        //switch back to the unsigned int for bitwise operations
  sampleOutLow = sampleIn << 6;                   //variable for to sends the 2 lowest bits of the audio sample to the DAC
  sampleOutHigh = sampleIn >> 2;                  //variable for to sends the 8 highest bits of the sample to the DAC 
  DAC0.DATAL = sampleOutLow;                      //make sending of low bits
  DAC0.DATAH = sampleOutHigh;                     //make sending of high bits and be triggering DAC output
  TCA0.SINGLE.INTFLAGS = 1;                       //clear the interrupt flags
}

void setup()
{
for (int i = 0; i < 14336; i++){
  delayArray[i] = 128;
}
pinMode(OutputPin, OUTPUT);
pinMode(InputPin, INPUT);
pinMode(TimePin, INPUT);
analogReference(EXTERNAL);
analogReadResolution(12);
analogSampleDuration(0);
delayTime = analogRead(TimePin) * 2;
delayTime = 14335 - delayTime;
DAC0.DATAL = 0b00000000;
DAC0.DATAH = 0b10000000;
DAC0.CTRLA = 0b11000001;
takeOverTCA0();
TCA0.SINGLE.INTFLAGS = 1;
TCA0.SINGLE.INTCTRL  = 0b00000001; /* OVF interrupt every PER cycles */
TCA0.SINGLE.CTRLA    = 0b00000110; /* Clock prescaler / 8 (3 MHz)    */
TCA0.SINGLE.PER      = 300;        /* Effective frequency 10 kHz     */
TCA0.SINGLE.CTRLA   |= 1;          /* Timer enable                   */
}


void loop()
{
  //do nothing
}
