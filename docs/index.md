Spring '23 Final Project - HarmonECE

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

The microcontroller will run the program that interfaces with the various I/O components in this device. 

### Wiring Diagram

![](https://github.com/gnshafreak/ece4180/blob/main/docs/pinout.png?raw=true)

###### *pushbuttons wired to pins 5-12

The Class D amplifier is used to amplify the PWM output signal going to the speaker. This signal is playing various frequencies that will be harmonizing with the user input. The volume can be controlled by adding a potentionmeter to the volume output. 

![](https://github.com/gnshafreak/ece4180/blob/main/docs/classdamp.png?raw=true)

![](https://github.com/gnshafreak/ece4180/blob/main/docs/speaker.png?raw=true)

The MEMS microphone is used to take input frequency data. For our device we do not need a high level of precision since we are just interpretting an overall frequency, and it is convenient for the processor to not have to filter out subtle noises that a more precise microphone circuit could pick up. For increased precision if the device is improved upon, we could incorporate an op-amp circuit or decoupling capacitor. 

![](https://github.com/gnshafreak/ece4180/blob/main/docs/memsmic.png?raw=true)

The uLCD screen is used for the interface. It will display the current siwtches held on, and the freuqency and note being read by the microphone. It will also display a status box indicating weather or not a note is in tune. 

![](https://github.com/gnshafreak/ece4180/blob/main/docs/lcd.png?raw=true)

The dip switches are used to control what interval the speaker will be harmonizing at based on the input frequency from the microphone. They are connected to the MBED pins 5-12.

![](https://github.com/gnshafreak/ece4180/blob/main/docs/dip.png?raw=true)

### Schematic

![](https://github.com/gnshafreak/ece4180/blob/main/docs/circuit.png?raw=true)

### Source Code

The following code implements the harmonizing tuner using the parts listed above. Since there are many components incorporated in out design, we use the RTOS library in order to run and manage different threads for each sub-process. 

    #include "mbed.h"
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
    
        while(1) {
            float t = timer.read();
            if(pb2) {
                speaker.period(1/freq2);
                speaker = 0.5; 
            } else if(pb3) {
                speaker.period(1/freq3);
                speaker = 0.5;  
    
            } else if(pb4) {
                speaker.period(1/freq4);
                speaker = 0.5;  
               
            } else if(pb5) {
                speaker.period(1/freq5);
                speaker = 0.5;  
               
            } else if(pb6) {
                speaker.period(1/freq6);
                speaker = 0.5;     
            } else if(pb7) {
                speaker.period(1/freq7);
                speaker = 0.5;   
            } 
            else {
                speaker = 0;
            }
    
            if (pb2) freq2 = readfreq * 9/8;
            if (pb3) freq3 = readfreq * 5/4;
            if (pb4) freq4 = readfreq * 4 /3;
            if (pb5) freq5 = readfreq * 3/2;
            if (pb6) freq6 = readfreq * 8/5;
            if (pb7) freq7 = readfreq * 9/5;
    
        }
        Thread::wait(100);
    }

    //Print frequencies of notes, detected freq, and circle display graphic to LCD
    void ulcd(void const *arguments) {
        int freq[] = {WHITE, WHITE, WHITE, WHITE, WHITE,WHITE, WHITE, WHITE};
        
        int n;
        int prev_note = 0;
        int prev_octave;
    
        int cx;
        int old_i = 0;
        int old_oct= 0;
        int oct;
    
        int closest_freq = 100;
        while(1) {
    
            float curr_freq = readfreq;
            float difference = 0;
    
            while(curr_freq > 31) {
                curr_freq = curr_freq / 2;
            }
            for (int i = 0; i < 12; i++) {
                float lower = 0;
                float upper = 100;
                if (i != 0) {
                    lower = notefreqs[i-1] + (notefreqs[i] - notefreqs[i - 1])/2;
                } 
                if (i != 11) { 
                    upper = notefreqs[i] + (notefreqs[i+1] - notefreqs[i])/2;
                }
                if (curr_freq >= lower && curr_freq <= upper) {
                    note = i;
                    difference = curr_freq/notefreqs[i];
                    break;
                }
            }
    
            uLCD.text_width(3); //1X size text
            uLCD.text_height(3);    
            if(prev_note != note) {
                uLCD.locate(2,1);
                uLCD.printf("      ");
                uLCD.locate(2,1);
                uLCD.printf("%s", notes[note]);
            }
            uLCD.locate(1, 2);
            uLCD.text_width(1); //1X size text
            uLCD.text_height(1);   
            uLCD.printf(" %.2f Hz ", readfreq);
           
            //pc.printf("difference %f", difference);
    
            if(difference < 0.99 || difference > 1.01 ) {
                uLCD.filled_rectangle(30, 60, 100, 80, 0xFF0000);   
            } else {
                uLCD.filled_rectangle(30, 60, 100, 80, 0x00FF00);
            }

            //uLCD.printf("        %f", octave);
    
            if (pb1) {freq[0] = RED;} else {freq[0] = WHITE;}
            if (pb2) {freq[1] = RED;} else {freq[1] = WHITE;}
            if (pb3) {freq[2] = RED;} else {freq[2] = WHITE;}
            if (pb4) {freq[3] = RED;} else {freq[3] = WHITE;}
            if (pb5) {freq[4] = RED;} else {freq[4] = WHITE;}
            if (pb6) {freq[5] = RED;} else {freq[5] = WHITE;}
            if (pb7) {freq[6] = RED;} else {freq[6] = WHITE;}
            if (pb8) {freq[7] = RED;} else {freq[7] = WHITE;}
           
            uLCD.filled_circle(5, 122, 3, freq[0]);
            uLCD.filled_circle(20, 117, 3, freq[1]);
            uLCD.filled_circle(35, 112, 3, freq[2]);
            uLCD.filled_circle(50, 107, 3, freq[3]);
            uLCD.filled_circle(65, 102, 3, freq[4]);
            uLCD.filled_circle(80, 97, 3, freq[5]);
            uLCD.filled_circle(95, 92, 3, freq[6]);
            uLCD.filled_circle(110, 87, 3, freq[7]);

            prev_note = note;
            Thread::wait(1);
        }
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
    
        wait(0.01);
        
        //set intial lcd values
        uLCD.background_color(BLACK);
        uLCD.baudrate(3000000);
        uLCD.color(WHITE);
        uLCD.locate(1,1);
        uLCD.printf("Harmonizing Tuner");
        
        //launch threads
        Thread ulcd_thread(&ulcd);
        // Thread speaker_thread(&playspeaker);
        Thread pb1_thread(&dopb);
        //Thread calc(&calculate_note);
        //set up main thread
        int samples = 1024;
        float arr [samples+1];
        arr[0] = 0;
        float max = 0;
        float loc = 0;
        float freq;
        osThreadSetPriority(osThreadGetId(), osPriorityHigh);
        //Loop and constantly be finding frequency of signal

        while (1) { 
            my_time.reset();
            my_time.start();
           
            for (int i = 1; i <=samples; i++){
                arr[i] = mic.instantlevel(); //Sample microphone
                wait(.00025); // ~4 Khz
            }
    
            my_time.stop();
            freq = samples/my_time.read(); 
    
            //Turn sampled array into its fourier transform
            vRealFFT(arr,samples); 
            
            //Loop through and find most apparent frequency of sampled signal 
            for(int i = 2; i <= samples; i++){ 
                arr[i] *= arr[i];
                if(max<arr[i]){
                    loc = freq/samples * i/2; //determine frequency by using sample rate,index in array
                    max = arr[i];
                    }
                }
            
            readfreq = loc; 
           // pc.printf("Read frequency %f", readfreq);
            max=0;
            wait(0.25);
            Thread::wait(100);
        }  
    }

The main thread uses an FFT library to read analog microphone input and calculate freqency. The LCD thread updates the LCD screen based on the current frequency reading, performes note calculations, and displays the current closest note to the current frequency. This thread also changes the color of the notes in the scale depending on the state of the DIP switches. Finally, the third thread reads the values from the DIP switches and outputs calculated frequencies to the speaker depending on switch state.

### Picture/Videos

![](https://github.com/gnshafreak/ece4180/blob/main/docs/pic.png?raw=true)

Tuner/Harmonizer

<iframe src="https://drive.google.com/file/d/1GZkmW75rmcu3rBb8dnT_PoWiY3D1GndY/preview" width="640" height="480" allow="autoplay"></iframe>

Chord Harmonizer (No LCD)

<iframe src="https://drive.google.com/file/d/1mY5u1WiaA05HA8MtmO97CyndCMfCPHXk/preview" width="640" height="480" allow="autoplay"></iframe>

### Conclusion

We were able to successfully meet all the initial goals of our project, being creating a tuner that identified frequencies coming through a microphone peripheral and a harmonizer that would emit harmonic frequencies, creating chords. However, we ran into the significant bottleneck of the mbed's limited processing power. To achieve the full chord harmonizing functionality while also updating the LCD, we had to use two mbeds to run our various threads at sufficient frequencies to prevent aliasing. Additionally, in order to prevent feedback between the speaker and mic, we increased the physical distance between the two and control the speaker volume. Perhaps Kanye West will enlist our services for his next album.

**Future work** would include using a microcontroller with more processing power and integrating the different functionalities. We could also implement a faster frequency extraction method to decrease the computational load on the processor. We could also create mechanical casing to create dsitance between the speaker and the microphone to help avoid feedback and increase ease of use. There is also room for improvmenet on the uLCD interface as we could give more information on how out of tune a note is. We could also add more switches or digital sensors to give control over all 12 tones. 

### Appendix

Below is an alternate working build we had that allowed multiple switches being flipped at a time to create complete chords. This was far more processor intensive, so we had to remove the LCD functionality and therefore the tuner as well. This implementation is far more conceptually complex, employing advanced digital signal processing (DSP).

```
#include "mbed.h"
#include "MAX4466.h"
#include "FFT.h"
#include "rtos.h"
#include "uLCD_4DGL.h"
#include "PinDetect.h"
#include <string>
#include <math.h>
#include "I2S.h"


#define MAX_VOICES 7 
#define MAX_FREQ 2000
#define PI 3.1415926
#define m2 256.0/243.0
#define M2 9.0/8.0
#define m3 32.0/27.0
#define M3 81.0/64.0
#define P4 4.0/3.0
#define P5 3.0/2.0
#define m6 128.0/81.0
#define M6 27.0/16.0
#define m7 16.0/9.0
#define M7 243.0/128.0
#define sample_freq 4000.0


DigitalIn dip1(p5);     //dip 1-7 control intervals
DigitalIn dip2(p6);
DigitalIn dip3(p7);
DigitalIn dip4(p8);
DigitalIn dip5(p9);
DigitalIn dip6(p10);
DigitalIn dip7(p11);
DigitalIn dip8(p12);    //controls major or minor scale
AnalogOut spkr(p18); 
Serial pc(USBTX,USBRX);
MAX4466 mic(p16);
Timer tim;
BusOut mylevel(LED4,LED3,LED2,LED1);


const int fs = MAX_FREQ * 2;
volatile float rootFreq = 0.0;
volatile int activeVoices = 0.0;
volatile int note = 0;
std::string notes[] = {"C","Db","D","Eb","E","F","Gb","G","Ab","A","Bb","B"};
float notefreqs[] = {16.35,17.32,18.35,19.45,20.60,21.93,23.12,24.50,25.96,27.50,29.14,30.87};

volatile int counter = 0;
volatile int samples = 1024;
float arr[1024 + 1];




// Define a structure to represent a single voice
struct Voice 
{
    float freq; // The frequency of the note being played
    float waveform_value; // The current value in the waveform data
    int waveform_index; // The index of the current value in the waveform data
};
Voice voices[MAX_VOICES];       // array of notes that could possibly be played

// Thread controlling which intervals (voices) are activated with dip switches
void intervalControl(void const *arguments)
{
    pc.printf("\r\nentered intervalControl");
    // dip8 high = major scale, dip8 low = minor scale
    while(1)
    {
        if(dip1) voices[0].freq = rootFreq;                     // root
        else voices[0].freq = 0;

        if(dip2 && dip8) voices[1].freq = rootFreq * M2;        // major 2nd
        else if(dip2 && !dip8) voices[1].freq = rootFreq * m2;  // minor 2nd
        else voices[1].freq = 0;

        if(dip3 && dip8) voices[2].freq = rootFreq * M3;        // major 3rd
        else if(dip3 && !dip8) voices[2].freq = rootFreq * m3;  // minor 3rd
        else voices[2].freq = 0;

        if(dip4) voices[3].freq = rootFreq * P4;                // perfect 4th
        else voices[3].freq = 0;

        if(dip5) voices[4].freq = rootFreq * P5;                // perfect 5th
        else voices[4].freq = 0;

        if(dip6 && dip8) voices[5].freq = rootFreq * M6;        // major 6th
        else if(dip6 && !dip8) voices[5].freq = rootFreq * m6;  // minor 6th
        else voices[5].freq = 0;

        if(dip7 && dip8) voices[6].freq = rootFreq * M7;        // major 7th
        else if(dip7 && !dip8) voices[6].freq = rootFreq * m7;  // minor 7th
        else voices[6].freq = 0;

        Thread::wait(100);
    }
}



void micFreq(void const *arguments)
{
    int samples = 1024;
    float arr [samples+1];
    arr[0] = 0;
    float max = 0;
    float loc = 0;
    float freq;
    osThreadSetPriority(osThreadGetId(), osPriorityHigh);
    //Loop and constantly be finding frequency of signal
    while (1) 
    { 
        tim.reset();
        tim.start();

        for (int i = 1; i <= samples; i++)
        {
            arr[i] = mic.instantlevel(); //Sample microphone
            Thread::wait(1); // ~1 Khz
        }

        tim.stop();
        freq = samples/tim.read();
         //Turn sampled array into its fourier transform
        vRealFFT(arr, samples); 
        
        //Loop through and find most apparent frequency of sampled signal 
        for(int i = 2; i <= samples; i++)
        { 
            arr[i] *= arr[i];
            if(max<arr[i])
            {
                loc = freq/samples * i/2; //determine frequency by using sample rate,index in array
                max = arr[i];
            }
        }

        rootFreq = loc; 
        max=0;
        Thread::wait((float).00025);
        pc.printf("rootfreq = %f",rootFreq);
    
    }
}

int main() {
    dip1.mode(PullUp);
    dip2.mode(PullUp);
    dip3.mode(PullUp);
    dip4.mode(PullUp);
    dip5.mode(PullUp);
    dip6.mode(PullUp);
    dip7.mode(PullUp);
    dip8.mode(PullUp);
    wait(0.01);

    //pc.printf("\r\nentered main");
    float sample_value;       // value of the additive output to AnalogOut
    
    // Create the wave table
    float wavetable[fs];
    for (int i = 0; i < fs; i++)
    {
        wavetable[i] = sin(2*PI*i/fs) * 0.5 + 0.5;
    }

    // initialize voices
    for(int i = 0; i < MAX_VOICES; i++)
    {
        voices[i].waveform_index = 0;
        voices[i].waveform_value = 0.0;
    }
    //pc.printf("\r\nvoice 1 freq = %f",voices[0].freq);

    Thread intervals(&intervalControl);
    Thread mic(&micFreq);
    // Thread lcd(&ulcd);
    //pc.printf("\r\nthreads created");
    //osThreadSetPriority(osThreadGetId(), osPriorityHigh);
    
    // 
    
    while(1) 
    {

        activeVoices = 0;
        sample_value = 0;
        for(int i = 0; i < MAX_VOICES; i++) // iterate through each voice
        {
            if (voices[i].freq > 0.0)       // check if voice is activated
            {
                activeVoices++;             // tracks how many voices are activated
                sample_value += wavetable[voices[i].waveform_index];
                voices[i].waveform_index = (voices[i].waveform_index + (int)(voices[i].freq)) % fs;     // Find next sample in wavetable based on frequency
            }
        }

        sample_value /= activeVoices;
        spkr = sample_value;
        wait(1.0/fs);
        
       
    }
}

```

