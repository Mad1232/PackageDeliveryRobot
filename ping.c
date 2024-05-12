
#include "ping.h"
#include "Timer.h"

volatile unsigned long START_TIME = 0;
volatile int edge = 1;
volatile unsigned long END_TIME = 0;
volatile unsigned long delta = 0; 
volatile enum{LOW, HIGH, DONE} STATE = LOW; // State of ping echo pulse
int counter = 0;

void ping_init (void){


    //Initialize GPIO port B
    SYSCTL_RCGCGPIO_R |= 0x02;
    while((SYSCTL_PRGPIO_R & 0x02) == 0) {}

    GPIO_PORTB_AFSEL_R |= 0b1000;//
    GPIO_PORTB_PCTL_R |= 0x7000;//
    GPIO_PORTB_DEN_R |= 0b1000;//


    // turn timer on
    SYSCTL_RCGCTIMER_R |= 0b1000;
    while((SYSCTL_PRTIMER_R) & 0b1000 == 0){}



    TIMER3_CTL_R &= ~0x100;
    TIMER3_CFG_R |= 0x4;

    // count-down
    TIMER3_TBMR_R &= ~0x08; 

    TIMER3_TBMR_R |= 0x7;

    TIMER3_TBPR_R |= 0xFF;

    TIMER3_TBILR_R |= 0xFFFF;
    // enable interrupt mask
    TIMER3_IMR_R |= 0x400;
    // count both edges
    TIMER3_CTL_R |= 0xC00;
    TIMER3_ICR_R |= 0x400;


    //Enable interrupts for timer 3
    NVIC_EN1_R |= 0x10;
    NVIC_PRI9_R |= 0x20;

    IntRegister(INT_TIMER3B, TIMER3B_Handler);

    IntMasterEnable();

    // Configure and enable the timer
}

void ping_trigger (void){
    STATE = LOW;
    // Disable timer and disable timer interrupt
    TIMER3_CTL_R &= ~0x100;
    TIMER3_IMR_R &= ~0x400; //0b 0000 1111 0000 0000
    // Disable alternate function (disconnect timer from port pin)
    GPIO_PORTB_AFSEL_R &= ~0b1000;
    GPIO_PORTB_DIR_R |= 0x8;
    GPIO_PORTB_DATA_R &= ~0x8; //reset

    // YOUR CODE HERE FOR PING TRIGGER/START PULSE
    GPIO_PORTB_DATA_R |= 0x8;


    timer_waitMicros(5);//wait

    GPIO_PORTB_DATA_R &= ~0x8; //reset
    GPIO_PORTB_DIR_R &= ~0x8;  // reset

    // Clear an interrupt that may have been erroneously triggered
    TIMER3_ICR_R |= 0x400;
    // Re-enable alternate function, timer interrupt, and timer
    GPIO_PORTB_AFSEL_R |= 0b1000;
    TIMER3_IMR_R |= 0x400;
    TIMER3_CTL_R |= 0x100;
}

void TIMER3B_Handler(void){

    if(TIMER3_MIS_R & 0x400){ //check if handler
        TIMER3_ICR_R |= 0x400 ;// this should clear interrupt.

        if(STATE == LOW){
            // start time
            START_TIME = TIMER3_TBR_R;
            STATE = HIGH;
        }
        else if(STATE == HIGH){
            // end time
            END_TIME = TIMER3_TBR_R;
            STATE = DONE;
        }
    }





}

float ping_getDistance (void){


    ping_trigger(); // Trigger the ultrasonic sensor

    // Wait until both edges of the signal haven't been detected
    while (STATE != DONE) {};

    // Calculate the time difference between the rising and falling edges
    float difference = 0;
    int overflow = (END_TIME > START_TIME);

    if (START_TIME > END_TIME) {
        difference = START_TIME - END_TIME; // No overflow

    }

    else {
        difference = ((unsigned long) overflow << 24) + START_TIME - END_TIME; // Overflow occurred
        counter++;

    }

    // Convert time difference to distance in centimeters
    difference = difference / 16000000; // Convert to seconds
    difference = (difference / 2) * 343 * 100;  // Distance = speed * time



    return difference; // Return the calculated distance

}
