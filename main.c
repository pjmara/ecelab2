/************** ECE2049 DEMO CODE ******************/
/**************  13 March 2019   ******************/
/***************************************************/

#include <msp430.h>
#include <stdlib.h>
#include <stdio.h>

/* Peripherals.c and .h are where the functions that implement
 * the LEDs and keypad, etc are. It is often useful to organize
 * your code by putting like functions together in files.
 * You include the header associated with that file(s)
 * into the main file of your project. */
#include "peripherals.h"

// Function Prototypes
void swDelay(char numLoops);
void runtimerA2(void);
void stoptimerA2(int reset);
void configUserLEDs(char inbits);
void timerDelay(unsigned long int);
// Declare globals here

enum state{INITIAL_SCREEN, GENERATE_LEVEL, DRAW_SCREEN, CHECK_KEYPAD, GAME_CONDITIONS, COUNTDOWN, GAME_CONDITION};
unsigned long int timer_cnt;
unsigned int timer_reset = 60000;
unsigned long int startTime = 0, endTime = 0;

// Main
void main(void)

 {
    srand (time(NULL));
    //enable interrupts:
    _BIS_SR(GIE);

    timer_cnt = 0;
    enum state currentState = INITIAL_SCREEN;
    unsigned char currKey=0, dispSz = 3;




    WDTCTL = WDTPW | WDTHOLD;    // Stop watchdog timer. Always need to stop this!!
                                 // You can then configure it properly, if desired

    // Initializing things
    initLeds();
    configDisplay();
    configKeypad();
    configUserLEDs(0);
    initLeds();
    runtimerA2();
    Graphics_clearDisplay(&g_sContext); // Clear the display
    while (1)    // Forever loop
    {

        switch(currentState){
        case INITIAL_SCREEN:


                // Write some text to the display
                Graphics_drawStringCentered(&g_sContext, "Welcome", AUTO_STRING_LENGTH, 48, 35, TRANSPARENT_TEXT);
                Graphics_drawStringCentered(&g_sContext, "to", AUTO_STRING_LENGTH, 48, 45, TRANSPARENT_TEXT);
                Graphics_drawStringCentered(&g_sContext, "MSP430 Hero", AUTO_STRING_LENGTH, 48, 55, TRANSPARENT_TEXT);
                Graphics_drawStringCentered(&g_sContext, "* to Start", AUTO_STRING_LENGTH, 48, 65, TRANSPARENT_TEXT);


                // Draw a box around everything because it looks nice
                Graphics_Rectangle box = {.xMin = 5, .xMax = 91, .yMin = 5, .yMax = 91 };
                Graphics_drawRectangle(&g_sContext, &box);

                // We are now done writing to the display.  However, if we stopped here, we would not
                // see any changes on the actual LCD.  This is because we need to send our changes
                // to the LCD, which then refreshes the display.
                // Since this is a slow operation, it is best to refresh (or "flush") only after
                // we are done drawing everything we need.
                Graphics_flushBuffer(&g_sContext);
                currKey = getKey();
                if (currKey == '*'){
                    currentState = COUNTDOWN;
                }
                break;
        case COUNTDOWN:
            BuzzerOnCustom(74-1);
            Graphics_clearDisplay(&g_sContext); // Clear the display
            Graphics_drawStringCentered(&g_sContext, "3...", AUTO_STRING_LENGTH, 48, 45, TRANSPARENT_TEXT);
            Graphics_flushBuffer(&g_sContext);
            setLeds(0x04);
            timerDelay(1000);
            setLeds(0x00);
            BuzzerOff();

            BuzzerOnCustom(66-1);
            Graphics_clearDisplay(&g_sContext); // Clear the display
            Graphics_drawStringCentered(&g_sContext, "2...", AUTO_STRING_LENGTH, 48, 45, TRANSPARENT_TEXT);
            Graphics_flushBuffer(&g_sContext);
            setLeds(0x02);
            timerDelay(1000);
            setLeds(0x00);
            BuzzerOff();

            BuzzerOnCustom(63-1);
            Graphics_clearDisplay(&g_sContext); // Clear the display
            Graphics_drawStringCentered(&g_sContext, "1...", AUTO_STRING_LENGTH, 48, 45, TRANSPARENT_TEXT);
            Graphics_flushBuffer(&g_sContext);
            setLeds(0x01);
            timerDelay(1000);
            setLeds(0x00);
            BuzzerOff();

            BuzzerOnCustom(59-1);
            Graphics_clearDisplay(&g_sContext); // Clear the display
            Graphics_drawStringCentered(&g_sContext, "GO...", AUTO_STRING_LENGTH, 48, 45, TRANSPARENT_TEXT);
            Graphics_flushBuffer(&g_sContext);
            setLeds(0x07);
            timerDelay(1000);
            setLeds(0x00);
            BuzzerOff();

            Graphics_clearDisplay(&g_sContext); // Clear the display
            currentState = INITIAL_SCREEN;
        }



     /*   // Check if any keys have been pressed on the 3x4 keypad
        currKey = getKey();
        if (currKey == '*')
            BuzzerOn();
        if (currKey == '#')
            BuzzerOff();
        if ((currKey >= '0') && (currKey <= '9'))
            setLeds(currKey - 0x30);

        if (currKey)
        {
            dispThree[1] = currKey;
            // Draw the new character to the display
            Graphics_drawStringCentered(&g_sContext, dispThree, dispSz, 48, 55, OPAQUE_TEXT);

            // Refresh the display so it shows the new data
            Graphics_flushBuffer(&g_sContext);

            // wait awhile before clearing LEDs
            swDelay(1);
            setLeds(0);
        }*/

    }  // end while (1)
}


void runtimerA2(void) {
    // This function configures and starts Timer A2
    // Timer is counting ~0.01 seconds //
    // Input: none, Output: none //
    // smj, ECE2049, 17 Sep 2013 //
    // Use ACLK, 16 Bit, up mode, 1 divider
    TA2CTL = TASSEL_1 + MC_1 + ID_0;
    TA2CCR0 = 163;       // 163+1 = 164 ACLK tics = ~0.005 seconds
    TA2CCTL0 = CCIE;     // TA2CCR0 interrupt enabled
}


void stoptimerA2(int reset) {
    // This function stops Timer A2 and resets the global time variable
    // if input reset = 1 //
    // Input: reset, Output: none //
    // smj, ECE2049, 17 Sep 2013 //
    TA2CTL = MC_0;        // stop timer
    TA2CCTL0 &= ~CCIE;    // TA2CCR0 interrupt disabled
    if(reset)
        timer_cnt=0;
}

// Timer A2 interrupt service routine
#pragma vector=TIMER2_A0_VECTOR
__interrupt void TimerA2_ISR (void) {
        timer_cnt++;
        if (timer_cnt >= timer_reset)
            timer_cnt = 0;
        if (timer_cnt%200==0) { // blink LEDs once a second
            P1OUT = P1OUT ^ BIT0;
            P4OUT ^= BIT7;
        }
}

void timerDelay(unsigned long int ms){
    startTime = timer_cnt;
    endTime = timer_cnt + ms/5;
    endTime = endTime % timer_reset;
    bool loop = true;
    while(loop){
        if(endTime <= timer_cnt){
            loop = false;
        }
    }
}

void configUserLEDs(char inbits) {
    P1SEL = P1SEL & ~(BIT0);
    P4SEL = P4SEL & ~(BIT7);

    P4DIR = P4DIR | (BIT7);
    P1DIR = P1DIR | (BIT0);

}

void swDelay(char numLoops)
{
    // This function is a software delay. It performs
    // useless loops to waste a bit of time
    //
    // Input: numLoops = number of delay loops to execute
    // Output: none
    //
    // smj, ECE2049, 25 Aug 2013

    volatile unsigned int i,j;  // volatile to prevent removal in optimization
                                // by compiler. Functionally this is useless code

    for (j=0; j<numLoops; j++)
    {
        i = 50000 ;                 // SW Delay
        while (i > 0)               // could also have used while (i)
           i--;
    }
}
