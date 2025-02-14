# 1985-Delay
A Digital Delay pedal for guitar built around the AVR128DA28 MCU
    
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

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

The maximum delay time available depends on the sample rate used and the bit depth of delay samples.
Main branch (1985_Delay) provides 1.4 seconds of delay at 10kHz and 8 bits and is tested and working with minor issues.
Highres branch (1985_Delay_Highres) provides 700ms of delay at 10kHz and 12 bits and is untested.

Adjust the sample rate as desired to balance available delay time and audio quality.

To use this software you will need:
* Arduino IDE version 1.8.19 or greater
* DxCore library for AVR128DA28 support (https://github.com/SpenceKonde/DxCore)
* Any USB to UPDI programmer. I recommend the Adafruit UPDI Friend because it is easy to use and under $20
* An AVR128DA28 MCU and appropriate supporting circuitry. Refer to the KiCad schematic and board files included in this repository.
