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
//Print frequencies of notes, detected freq, and circle display graphic to LCD
// void ulcd(void const *arguments) {
//     int freq[] = {WHITE, WHITE, WHITE, WHITE, WHITE,WHITE, WHITE, WHITE};
    
//     int prev_note = 0;

//     while(1) {

//         float curr_freq = rootFreq;
//         float difference = 0;

//         while(curr_freq > 31) {
//             curr_freq = curr_freq / 2;
//         }
//         for (int i = 0; i < 12; i++) {
//             float lower = 0;
//             float upper = 100;
//             if (i != 0) {
//                 lower = notefreqs[i-1] + (notefreqs[i] - notefreqs[i - 1])/2;
//             } 
//             if (i != 11) { 
//                 upper = notefreqs[i] + (notefreqs[i+1] - notefreqs[i])/2;
//             }
//             if (curr_freq >= lower && curr_freq <= upper) {
//                 note = i;
//                 difference = curr_freq/notefreqs[i];
//                 break;
//             }
//         }

//         uLCD.text_width(3); //1X size text
//         uLCD.text_height(3);    
//         if(prev_note != note) {
//             uLCD.locate(2,1);
//             uLCD.printf("      ");
//             uLCD.locate(2,1);
//             uLCD.printf("%s", notes[note]);
//         }
//         uLCD.locate(1, 2);
//         uLCD.text_width(1); //1X size text
//         uLCD.text_height(1);   
//         uLCD.printf(" %.2f Hz ", rootFreq);
       
//         pc.printf("difference %f", difference);

//         if(difference < 0.99 || difference > 1.01 ) {
//             uLCD.filled_rectangle(30, 60, 100, 80, 0xFF0000);   
//         } else {
//             uLCD.filled_rectangle(30, 60, 100, 80, 0x00FF00);
//         }
        

//         //uLCD.printf("        %f", octave);

//         if (dip1) {freq[0] = RED;} else {freq[0] = WHITE;}
//         if (dip2) {freq[1] = RED;} else {freq[1] = WHITE;}
//         if (dip3) {freq[2] = RED;} else {freq[2] = WHITE;}
//         if (dip4) {freq[3] = RED;} else {freq[3] = WHITE;}
//         if (dip5) {freq[4] = RED;} else {freq[4] = WHITE;}
//         if (dip6) {freq[5] = RED;} else {freq[5] = WHITE;}
//         if (dip7) {freq[6] = RED;} else {freq[6] = WHITE;}
//         if (dip8) {freq[7] = RED;} else {freq[7] = WHITE;}
       
//         uLCD.filled_circle(5, 122, 3, freq[0]);
//         uLCD.filled_circle(20, 117, 3, freq[1]);
//         uLCD.filled_circle(35, 112, 3, freq[2]);
//         uLCD.filled_circle(50, 107, 3, freq[3]);
//         uLCD.filled_circle(65, 102, 3, freq[4]);
//         uLCD.filled_circle(80, 97, 3, freq[5]);
//         uLCD.filled_circle(95, 92, 3, freq[6]);
//         uLCD.filled_circle(110, 87, 3, freq[7]);


//         prev_note = note;
//         Thread::wait(4);
//     }
// }

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

    
    //set intial lcd values
    // uLCD.background_color(BLACK);
    // uLCD.baudrate(3000000);
    // uLCD.color(WHITE);
    // uLCD.locate(1,1);
    // uLCD.printf("Harmonizing Tuner");

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
