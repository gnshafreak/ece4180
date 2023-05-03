# Spring '23 Final Project - HarmonECE

## Madeline Belew, Noah Horton, Gabriel Shafiq

### Project Summary

Our project is both a harmonizer and a tuner that uses different components we became familiar with over the course of the semester. It listens for an input frequency on the microphone, and then depending on which switches are high, plays multiple notes to complete a chord. Status, current note, and chord are all displayed on the LCD.

### Parts List

- MBED LP1768
- uLCD
- MEMS Mic
- Class D Amp
- Speaker
- Potentiometer
- 8x DIP Switches
- 5V Power Supply

### Schematic

![](https://github.com/gnshafreak/ece4180/blob/main/docs/circuit.png?raw=true)

### Wiring Diagram

### Source Code

The following code implements the harmonizing tuner using the parts listed above. 

https://github.com/gnshafreak/ece4180/blob/44f9a57a1d9cdbf2a631d21dd6e3d84dc5162c20/harmonece%20(chordless)/main.cpp#L1-L284

The main thread using an FFT library to read analog microphone input and calculate freqency. The LCD thread updates the LCD screen based on the current frequency reading, performes note calculations, and displays the current closest note to the current frequency. This thread also changes the color of the notes in the scale depending on the state of the DIP switches. Finally, the third thread reads the values from the DIP switches and outputs calculated frequencies to the speaker depending on switch state.


### Photos/Videos


