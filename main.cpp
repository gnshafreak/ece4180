#include "mbed.h"
#include "rtos.h"
#include "SDFileSystem.h"
#include "wave_player.h"
#include "uLCD_4DGL.h"
#include "PinDetect.h"

//SDFileSystem sd(p5, p6, p7, p8, "sd"); //SD card
PwmOut audioOut(p26);     // Speaker PWM
AnalogIn mic(p16);
uLCD_4DGL LCD(p13,p14,p15);
PinDetect pb1(p20);

DigitalIn dip1(p5);
DigitalIn dip2(p6);
DigitalIn dip3(p7);
DigitalIn dip4(p8);
DigitalIn dip5(p9);
DigitalIn dip6(p10);
DigitalIn dip7(p11);
DigitalIn dip8(p12);

Mutex lcd_mutex;

void pb1_hit_callback(void)     // run program button interrupt
{

}

void LCD_thread1(void const *args)      // draw a circle in middle of screen
{
    
}

void LCD_thread2(void const *args)      // draw sine wave in corner of screen
{
    
}

void speaker()
{
    
}

int main()
{
    dip1.mode(PullUp);
    dip2.mode(PullUp);
    dip3.mode(PullUp);
    dip4.mode(PullUp);
    dip5.mode(PullUp);
    dip6.mode(PullUp);
    dip7.mode(PullUp);
    dip8.mode(PullUp);

    pb1.attach_deasserted(&pb1_hit_callback);

    Thread thread1(); //Start LED effect thread
    Thread thread2(LCD_thread1);
    Thread thread3(LCD_thread2);
    Thread thread4(speaker);
    
    char bnum=0;
    while(1) 
    {
        lcd_mutex.lock();
        
    }
}
