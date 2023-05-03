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

![](https://github.com/gnshafreak/ece4180/blob/main/docs/wiringdia.png?raw=true)

### Source Code

The following code implements the harmonizing tuner using the parts listed above. 

`#include "mbed.h"
#include "MAX4466.h"
#include "FFT.h"
#include "rtos.h"
#include "uLCD_4DGL.h"
#include "PinDetect.h"
#include <string>
#include <math.h>
#include "I2S.h"


DigitalIn pb1(p5);
DigitalIn pb2(p6);
DigitalIn pb3(p7);
DigitalIn pb4(p8);
DigitalIn pb5 (p9);
DigitalIn pb6 (p10);
DigitalIn pb7 (p11);
DigitalIn pb8 (p12);

MAX4466 mic(p16);
PwmOut speaker(p21);
AnalogOut speaker2(p18);
uLCD_4DGL uLCD(p13,p14,p15);
Timer my_time;
Timer timer;

//Uses I2S hardware for input

Serial pc(USBTX, USBRX);
#define MAX_NOTE 11
#define MAX_OCT 8
#define PI 3.14159
std::string notes[] = {"C","Db","D","Eb","E","F","Gb","G","Ab","A","Bb","B"};
float notefreqs[] = {16.35,17.32,18.35,19.45,20.60,21.93,23.12,24.50,25.96,27.50,29.14,30.87};


volatile float readfreq = 0.00;
volatile float octave = 0;
volatile int note = 0;
volatile int prev_note = 0;
volatile int i = 0;
volatile int m=0;
volatile int counter = 0;




void dopb(void const *arguments) {
    float freq1 = 0;
    float freq2 = 0;
    float freq3 = 0;
    float freq4 = 0;
    float freq5 = 0;
    float freq6 = 0;
    float freq7 = 0;
    speaker = 0;
    timer.start();

​    while(1) {
​        float t = timer.read();
​        if(pb2) {
​            speaker.period(1/freq2);
​            speaker = 0.5; 
​        } else if(pb3) {
​            speaker.period(1/freq3);
​            speaker = 0.5;  

​        } else if(pb4) {
​            speaker.period(1/freq4);
​            speaker = 0.5;  
​           
​        } else if(pb5) {
​            speaker.period(1/freq5);
​            speaker = 0.5;  
​           
​        } else if(pb6) {
​            speaker.period(1/freq6);
​            speaker = 0.5;     
​        } else if(pb7) {
​            speaker.period(1/freq7);
​            speaker = 0.5;   
​        } 
​        else {
​            speaker = 0;
​        }

​        if (pb2) freq2 = readfreq * 9/8;
​        if (pb3) freq3 = readfreq * 5/4;
​        if (pb4) freq4 = readfreq * 4 /3;
​        if (pb5) freq5 = readfreq * 3/2;
​        if (pb6) freq6 = readfreq * 8/5;
​        if (pb7) freq7 = readfreq * 9/5;

​    }
​    Thread::wait(100);
}

void ulcd_update(void const *argument) {
    int freq[] = {WHITE, WHITE, WHITE, WHITE, WHITE,WHITE, WHITE, WHITE};
    
    int n;
    int prev_note = 0;
    int prev_octave;
    while(1) {
        if (pb1) {freq[0] = RED;} else {freq[0] = WHITE;}
        if (pb2) {freq[1] = RED;} else {freq[1] = WHITE;}
        if (pb3) {freq[2] = RED;} else {freq[2] = WHITE;}
        if (pb4) {freq[3] = RED;} else {freq[3] = WHITE;}
        if (pb5) {freq[4] = RED;} else {freq[4] = WHITE;}
        if (pb6) {freq[5] = RED;} else {freq[5] = WHITE;}
        if (pb7) {freq[6] = RED;} else {freq[6] = WHITE;}
        if (pb8) {freq[7] = RED;} else {freq[7] = WHITE;}
        pc.printf("pb8: %f", pb8);
        

​        uLCD.filled_circle(5, 122, 3, freq[0]);
​        uLCD.filled_circle(20, 117, 3, freq[1]);
​        uLCD.filled_circle(35, 112, 3, freq[2]);
​        uLCD.filled_circle(50, 107, 3, freq[3]);
​        uLCD.filled_circle(65, 102, 3, freq[4]);
​        uLCD.filled_circle(80, 97, 3, freq[5]);
​        uLCD.filled_circle(95, 92, 3, freq[6]);
​        uLCD.filled_circle(110, 87, 3, freq[7]);

​    }
}


//Print frequencies of notes, detected freq, and circle display graphic to LCD
void ulcd(void const *arguments) {
    int freq[] = {WHITE, WHITE, WHITE, WHITE, WHITE,WHITE, WHITE, WHITE};
    
    int n;
    int prev_note = 0;
    int prev_octave;

​    int cx;
​    int old_i = 0;
​    int old_oct= 0;
​    int oct;

​    int closest_freq = 100;
​    while(1) {

​        float curr_freq = readfreq;
​        float difference = 0;

​        while(curr_freq > 31) {
​            curr_freq = curr_freq / 2;
​        }
​        for (int i = 0; i < 12; i++) {
​            float lower = 0;
​            float upper = 100;
​            if (i != 0) {
​                lower = notefreqs[i-1] + (notefreqs[i] - notefreqs[i - 1])/2;
​            } 
​            if (i != 11) { 
​                upper = notefreqs[i] + (notefreqs[i+1] - notefreqs[i])/2;
​            }
​            if (curr_freq >= lower && curr_freq <= upper) {
​                note = i;
​                difference = curr_freq/notefreqs[i];
​                break;
​            }
​        }

​        uLCD.text_width(3); //1X size text
​        uLCD.text_height(3);    
​        if(prev_note != note) {
​            uLCD.locate(2,1);
​            uLCD.printf("      ");
​            uLCD.locate(2,1);
​            uLCD.printf("%s", notes[note]);
​        }
​        uLCD.locate(1, 2);
​        uLCD.text_width(1); //1X size text
​        uLCD.text_height(1);   
​        uLCD.printf(" %.2f Hz ", readfreq);
​       
​        //pc.printf("difference %f", difference);

​        if(difference < 0.99 || difference > 1.01 ) {
​            uLCD.filled_rectangle(30, 60, 100, 80, 0xFF0000);   
​        } else {
​            uLCD.filled_rectangle(30, 60, 100, 80, 0x00FF00);
​        }
​        

​        //uLCD.printf("        %f", octave);

​        if (pb1) {freq[0] = RED;} else {freq[0] = WHITE;}
​        if (pb2) {freq[1] = RED;} else {freq[1] = WHITE;}
​        if (pb3) {freq[2] = RED;} else {freq[2] = WHITE;}
​        if (pb4) {freq[3] = RED;} else {freq[3] = WHITE;}
​        if (pb5) {freq[4] = RED;} else {freq[4] = WHITE;}
​        if (pb6) {freq[5] = RED;} else {freq[5] = WHITE;}
​        if (pb7) {freq[6] = RED;} else {freq[6] = WHITE;}
​        if (pb8) {freq[7] = RED;} else {freq[7] = WHITE;}
​       
​        uLCD.filled_circle(5, 122, 3, freq[0]);
​        uLCD.filled_circle(20, 117, 3, freq[1]);
​        uLCD.filled_circle(35, 112, 3, freq[2]);
​        uLCD.filled_circle(50, 107, 3, freq[3]);
​        uLCD.filled_circle(65, 102, 3, freq[4]);
​        uLCD.filled_circle(80, 97, 3, freq[5]);
​        uLCD.filled_circle(95, 92, 3, freq[6]);
​        uLCD.filled_circle(110, 87, 3, freq[7]);


​        prev_note = note;
​        Thread::wait(1);
​    }
}



int main() {
    //set up pindetect
    pb1.mode(PullUp);
    pb2.mode(PullUp);
    pb3.mode(PullUp);
    pb4.mode(PullUp);
    pb5.mode(PullUp);
    pb6.mode(PullUp);
    pb7.mode(PullUp);
    pb8.mode(PullUp);

​    wait(0.01);
​    
​    //set intial lcd values
​    uLCD.background_color(BLACK);
​    uLCD.baudrate(3000000);
​    uLCD.color(WHITE);
​    uLCD.locate(1,1);
​    uLCD.printf("Harmonizing Tuner");
​    
​    //launch threads
​    Thread ulcd_thread(&ulcd);
​    // Thread speaker_thread(&playspeaker);
​    Thread pb1_thread(&dopb);
​    //Thread calc(&calculate_note);
​    //set up main thread
​    int samples = 1024;
​    float arr [samples+1];
​    arr[0] = 0;
​    float max = 0;
​    float loc = 0;
​    float freq;
​    osThreadSetPriority(osThreadGetId(), osPriorityHigh);
​    //Loop and constantly be finding frequency of signal


​    while (1) { 
​        my_time.reset();
​        my_time.start();
​       
​        for (int i = 1; i <=samples; i++){
​            arr[i] = mic.instantlevel(); //Sample microphone
​            wait(.00025); // ~4 Khz
​        }

​        my_time.stop();
​        freq = samples/my_time.read(); 

​        //Turn sampled array into its fourier transform
​        vRealFFT(arr,samples); 
​        
​        //Loop through and find most apparent frequency of sampled signal 
​        for(int i = 2; i <= samples; i++){ 
​            arr[i] *= arr[i];
​            if(max<arr[i]){
​                loc = freq/samples * i/2; //determine frequency by using sample rate,index in array
​                max = arr[i];
​                }
​            }
​        
​        readfreq = loc; 
​       // pc.printf("Read frequency %f", readfreq);
​        max=0;
​        wait(0.25);
​        Thread::wait(100);
​    }  
}`

The main thread using an FFT library to read analog microphone input and calculate freqency. The LCD thread updates the LCD screen based on the current frequency reading, performes note calculations, and displays the current closest note to the current frequency. This thread also changes the color of the notes in the scale depending on the state of the DIP switches. Finally, the third thread reads the values from the DIP switches and outputs calculated frequencies to the speaker depending on switch state.


### Photos/Videos

