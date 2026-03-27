//Pin Assignments
uint8_t OutputPin = PIN_PD6;
uint8_t InputPin = PIN_PD0;
uint8_t TimePin = PIN_PD2;
uint8_t MixPin = PIN_PD1;
uint8_t FbkPin = PIN_PD3;
//Audio Samples
int16_t inSample;
int16_t delaySample;
float drySample;
float wetSample;
float storeSample;
float outSample;
int16_t intSample;
uint16_t dacSample;
int16_t delayArray[4096];

//Step Counters
uint16_t sampleStep = 0;
uint16_t delayStep = 0;

//Controls
uint16_t mixRaw = 0;
float mixFactor = 0.0;
uint16_t fbkRaw = 0;
float fbkFactor = 0.0;
uint16_t delaySet = 0;
uint16_t delayTimeNew = 0;
uint16_t delayTime = 0;

ISR(TCA0_OVF_vect) {
  //experiment here - clearing the interrupt flag first
  TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;                 //clear the interrupt flags
  //get the input as a signed int
  inSample = analogRead(InputPin) - 2048;
  //get the signed int delayed sample
  delaySample = delayArray[delayStep];
  
  //convert the dry sample to a float between 1 and -1
  drySample = inSample / 2048.0;
  //convert the wet sample to a float between 1 and -1
  wetSample = delaySample / 2048.0;
  //prepare the sample to store in the array by starting with the feedback
  storeSample = wetSample;
  //attenuate the feedback
  storeSample *= fbkFactor;
  //add the dry sample
  storeSample += drySample;
  //convert to signed int
  delaySample = storeSample * 4095;
  //store the signed int sample in the delay
  delayArray[sampleStep] = delaySample;
  //attenuate the wet sample
  wetSample *= mixFactor;
  //combine the dry and wet samples together
  outSample = drySample + wetSample;
  //convert back to a signed int
  intSample = outSample * 2048;
  //constrain to the limits of the dac and make it unsigned
  if(intSample > 2047){intSample = 2047;}
  else if (intSample < -2048){intSample = -2048;}
  dacSample = intSample + 2048;
  dacSample = dacSample >> 2;
  //output to dac
  DAC0.DATAL = dacSample << 6;
  DAC0.DATAH = dacSample >> 2;
  //read the controls
  mixRaw = 4095 - (analogRead(MixPin) & 0b0000111111111100);
  mixFactor = mixRaw / 4095.0;
  fbkRaw = 4095 - (analogRead(FbkPin) & 0b0000111111111100);
  fbkFactor = fbkRaw / 4095.0;
  delayTimeNew = 4095 - (analogRead(TimePin) & 0b0000111111111100);
  if(delayTime < delayTimeNew){delayTime++;}
  else if (delayTime > delayTimeNew){delayTime--;}
  //increment the steps
  sampleStep++;
  sampleStep &= 0x0FFF;
  delayStep = sampleStep - delayTime;
  delayStep &= 0x0FFF;
  
}

void setup() {
  // put your setup code here, to run once:
for (int i = 0; i < 4096; i++){
  delayArray[i] = 0.0;
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

mixRaw = analogRead(MixPin);
delayTime = 4095 - (analogRead(TimePin) & 0b0000111111111100);
mixFactor = mixRaw / 4095.0;

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
