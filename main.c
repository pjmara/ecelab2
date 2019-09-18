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
#include <string.h>

// Function Prototypes
void swDelay(char numLoops);
void runtimerA2(void);
void stoptimerA2(int reset);
void configUserLEDs(char inbits);
void timerDelay(unsigned long int);
void configButtons();
char returnState();
char* numberToNote(char note);
int noteToPitch(char* note);
void playNote(char* note);
char noteToLED(char* note);
bool correctPress(unsigned char inbits);
// Declare globals here

enum state{INITIAL_SCREEN, COUNTDOWN, PLAY_SONG, PLAY_NOTE};
unsigned long int timer_cnt;
unsigned int timer_reset = 60000;
unsigned long int startTime = 0, endTime = 0;


// Main
void main(void)

 {
    srand (time(NULL));
    //enable interrupts:
    _BIS_SR(GIE);






    WDTCTL = WDTPW | WDTHOLD;    // Stop watchdog timer. Always need to stop this!!
                                 // You can then configure it properly, if desired

    // Initializing things
    initLeds();
    configDisplay();
    configKeypad();
    configUserLEDs(0);
    initLeds();
    runtimerA2();
    configButtons();
    int x = noteToPitch("A");
    Graphics_clearDisplay(&g_sContext); // Clear the display

    enum state currentState = INITIAL_SCREEN;
    unsigned char currKey=0, currLED = 0x00, dispSz = 3;
    int numCorrect = 0;
    int numWrong = 0;
    timer_cnt = 0;

    bool over = false;
    bool alreadyHitCorrect = false;
    int currentNoteIndex = 0;
    int songLength = 43;

    char twinkle[][3] = {"A", "A", "E", "E", "F", "F", "E", "D", "D", "C", "C", "B", "B", "A", "E", "E","D", "D", "C", "C", "B", "E", "E","D", "D", "C", "C", "B", "A", "A", "E", "E", "F", "F", "E", "D", "D", "C", "C", "B", "B", "A"};
    int duration[] = {500, 500, 500, 500, 500, 500, 1000, 500, 500, 500, 500, 500, 500, 1000, 500, 500, 500, 500, 500, 500, 1000, 500, 500, 500, 500, 500, 500, 1000, 500, 500, 500, 500, 500, 500, 1000, 500, 500, 500, 500, 500, 500, 1000};
    while (1)    // Forever loop
    {

        switch(currentState){
        case INITIAL_SCREEN:
            Graphics_clearDisplay(&g_sContext); // Clear the display
            numCorrect = 0;
            numWrong = 0;
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

            currKey=0, currLED = 0x00, dispSz = 3;
            numCorrect = 0;
            numWrong = 0;
            timer_cnt = 0;
            over = false;
            alreadyHitCorrect = false;
            currentNoteIndex = 0;

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
            currentState = PLAY_SONG;

        case PLAY_SONG:

            playNote(twinkle[currentNoteIndex]);
            currLED = noteToLED(twinkle[currentNoteIndex]);
            setLeds(currLED);
            startTime = timer_cnt;
            endTime = timer_cnt + duration[currentNoteIndex]/5;
            endTime = endTime % timer_reset;
            over = false;
            alreadyHitCorrect = false;
            currentState = PLAY_NOTE;

        case PLAY_NOTE:
            if(endTime <= timer_cnt){
                over = true;
            }
            if (correctPress(currLED)  && !alreadyHitCorrect){
                numCorrect++;
                alreadyHitCorrect = true;
                Graphics_clearDisplay(&g_sContext); // Clear the display

                char cor[15];
                sprintf(cor, "%d",numCorrect);
                Graphics_drawStringCentered(&g_sContext, cor, AUTO_STRING_LENGTH, 48, 45, TRANSPARENT_TEXT);

                Graphics_flushBuffer(&g_sContext);
                //BuzzerOff();
            }
            if(over){
                if (!alreadyHitCorrect) {
                    numWrong ++;
                }
                BuzzerOff();
                currentNoteIndex++;
                if (currentNoteIndex < songLength)
                    currentState = PLAY_SONG;
                else
                    currentState = INITIAL_SCREEN;
            }


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
char* numberToNote(char note) {
    if (note == '1') {
        return "A";
    }
    else if (note == '2') {
        return "Bb";
    }
    else if (note == '3') {
        return "B";
    }
    else if (note == '4') {
        return "C";
    }
    else if (note == '5') {
        return "C#";
    }
    else if (note == '6') {
        return "D";
    }
    else if (note == '7') {
        return "Eb";
    }
    else if (note == '8') {
        return "E";
    }
    else if (note == '9') {
        return "F";
    }
    else if (note == '*') {
        return "F#";
    }
    else if (note == '0') {
        return "G";
    }
    else if (note == '#') {
        return "Ab";
    }
    return "\0";
}

int noteToPitch(char* note) {
    if (strcmp(note, "A") == 0) {
        return (int) 32767/440;
    }
    else if (strcmp(note, "Bb") == 0) {
        return (int) 32767/466;
    }
    else if (strcmp(note, "B") == 0) {
        return (int) 32767/494;
    }
    else if (strcmp(note, "C") == 0) {
        return (int) 32767/523;
    }
    else if (strcmp(note, "C#") == 0) {
        return (int) 32767/554;
    }
    else if (strcmp(note, "D") == 0) {
        return (int) 32767/587;
    }
    else if (strcmp(note, "Eb") == 0) {
        return (int) 32767/622;
    }
    else if (strcmp(note, "E") == 0) {
        return (int) 32767/659;
    }
    else if (strcmp(note, "F") == 0) {
        return (int) 32767/698;
    }
    else if (strcmp(note, "F#") == 0) {
        return (int) 32767/740;
    }
    else if (strcmp(note, "G") == 0) {
        return (int) 32767/784;
    }
    else if (strcmp(note, "Ab") == 0) {
        return (int) 32767/831;
    }
    return 0;
}

char noteToLED(char* note) {
    if (strcmp(note, "A") == 0) {
        return 0x08;
    }
    else if (strcmp(note, "Bb") == 0) {
        return 0x08;
    }
    else if (strcmp(note, "B") == 0) {
        return 0x08;
    }
    else if (strcmp(note, "C") == 0) {
        return 0x04;
    }
    else if (strcmp(note, "C#") == 0) {
        return 0x04;
    }
    else if (strcmp(note, "D") == 0) {
        return 0x04;
    }
    else if (strcmp(note, "Eb") == 0) {
        return 0x02;
    }
    else if (strcmp(note, "E") == 0) {
        return 0x02;
    }
    else if (strcmp(note, "F") == 0) {
        return 0x02;
    }
    else if (strcmp(note, "F#") == 0) {
        return 0x01;
    }
    else if (strcmp(note, "G") == 0) {
        return 0x01;
    }
    else if (strcmp(note, "Ab") == 0) {
        return 0x01;
    }
    return 0;
}

bool correctPress(unsigned char inbits) {
    if (returnState() == inbits) {
        return true;
    }
    else
        return false;

}



void playNote(char* note) {
    BuzzerOnCustom(noteToPitch(note));
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

char returnState(){
    unsigned char mask = 0;
    if(~P2IN & BIT2){
        mask |= BIT1;
    }
    if(~P3IN & BIT6){
        mask |= BIT2;
    }
    if(~P7IN & BIT0){
        mask |= BIT3;
    }
    if(~P7IN & BIT4){
        mask |= BIT0;
    }
    return mask;
}


void configButtons() {
    P7SEL = P7SEL & (BIT1|BIT2|BIT3|BIT5|BIT6|BIT7);
    P3SEL = P3SEL & (BIT1|BIT2|BIT3|BIT5|BIT0|BIT7|BIT4);
    P2SEL = P2SEL & (BIT1|BIT6|BIT3|BIT5|BIT0|BIT7|BIT4);
    P7DIR = P7DIR & (BIT1|BIT2|BIT3|BIT5|BIT6|BIT7);
    P3DIR = P3DIR & (BIT1|BIT2|BIT3|BIT5|BIT0|BIT7|BIT4);
    P2DIR = P2DIR & (BIT1|BIT6|BIT3|BIT5|BIT0|BIT7|BIT4);
    P7REN = P7REN | (BIT0|BIT4);
    P7OUT = P7OUT | (BIT0|BIT4);
    P3REN = P3REN | (BIT6);
    P3OUT = P3OUT | (BIT6);
    P2REN = P2REN | (BIT2);
    P2OUT = P2OUT | (BIT2);
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
