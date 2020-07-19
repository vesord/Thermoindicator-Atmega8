# Thermoindicator Atmega8
## Description
Model of thermoindicator on Atmega8 and DS1822

It was made due to my studying in KSTU, Russia. 

The main goals of the project are learn how to work with simple microcontroller such as Atmega8.

## Usage

To run simulation you need to install [Proteus](https://www.labcenter.com/downloads/ "Official Proteus trial version") 
and attach _.hex_ file which appears after compiling source code by [AtmelStudio](https://www.microchip.com/mplab/avr-support/atmel-studio-7 "Official Atmel Studio free version")

## More

MC gets temperature information and updates display approx. each 1s.

4DIG 7SEG CA was used by display. For real realization its better to connect pins 1-4 of LED with pulling-down RES or with transistors.
By connecting it into proteus indications turned off. IDK why.

PCB layout does not implemented.

You can see [pictures](./Pictures).

You can see [source code](./AtmelStudio/Thermoindicator_atmega8/Thermoindicator_atmega8).
