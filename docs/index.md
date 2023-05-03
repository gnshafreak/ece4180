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

The microconftrller will run the program that interfaces with the various I/O components in this device. 

### Wiring Diagram

![](https://github.com/gnshafreak/ece4180/blob/main/docs/pinout.png?raw=true)

The Class D amplifier is used to amplify the PWM output signal going to the speaker. This singal is playing various frequencies that will be harmonizing with the user input. The volume can be controlled by adding a potentionmeter to the volume output. 

![](https://github.com/gnshafreak/ece4180/blob/main/docs/classdamp.png?raw=true)

![](https://github.com/gnshafreak/ece4180/blob/main/docs/speaker.png?raw=true)

The MEMS microphone is used to take input frequency data. For our device we do not need a high level of precision since we are just interpretting an overall frequency, and it is convenient for the processor to not have to filter out subtle noises that a more precise microphone circuit could pick up. For increased precision if the device is improved upon, we could incorperate an op-amp circuit or decoupling capacitor. 

![](https://github.com/gnshafreak/ece4180/blob/main/docs/memsmic.png?raw=true)

The uLCD screen is used for the interface. It will display the current siwtches held on, and the freuqency and note being read by the microphone. It will also display a status box indicating weather or not a note is in tune. 

![](https://github.com/gnshafreak/ece4180/blob/main/docs/lcd.png?raw=true)

The dip switches are used to control what interval the speaker will be harmonizing at based on the input frequency from the microphone. 

### Schematic

![](https://github.com/gnshafreak/ece4180/blob/main/docs/circuit.png?raw=true)



### Source Code

The following code implements the harmonizing tuner using the parts listed above. Since there are many componenets incorperated in out design, we use the RTOS library in order to run and manage different threads for each sub-process. 

https://github.com/gnshafreak/ece4180/blob/44f9a57a1d9cdbf2a631d21dd6e3d84dc5162c20/harmonece%20(chordless)/main.cpp#L1-L284

The main thread using an FFT library to read analog microphone input and calculate freqency. The LCD thread updates the LCD screen based on the current frequency reading, performes note calculations, and displays the current closest note to the current frequency. This thread also changes the color of the notes in the scale depending on the state of the DIP switches. Finally, the third thread reads the values from the DIP switches and outputs calculated frequencies to the speaker depending on switch state.


### Photos/Videos


