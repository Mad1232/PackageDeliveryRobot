

#include "button.h"
#include "lcd.h"
// Global varibles
//volatile int button_event;
volatile int button_num;

/**
 * Initialize PORTE and configure bits 0-3 to be used as inputs for the buttons.
 */
void button_init() {
    static uint8_t initialized = 0;
    lcd_init();
        //Check if already initialized
        if(initialized){
            return;
        }

        SYSCTL_RCGCGPIO_R |= 0b010000;
        GPIO_PORTE_DIR_R &= 0b00000000;
        GPIO_PORTE_DEN_R |= 0b11111111;


        initialized = 1;
}

int button_getButton() {

    if((GPIO_PORTE_DATA_R & 0b00001000) == 0b00000000){
        return 4; // rightmost button
    }
    else if((GPIO_PORTE_DATA_R & 0b00000100) == 0b00000000){
        return 3;
    }
    else if((GPIO_PORTE_DATA_R & 0b00000010) == 0b00000000){
        return 2;
    }
    else if ((GPIO_PORTE_DATA_R & 0b00000001) == 0b00000000){
        return 1; //leftmost button
    }
    return 0;
}





