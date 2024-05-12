#include "adc.h"
#include "REF_tm4c123gh6pm.h"
#include <stdint.h>
void adc_init(void) {


    SYSCTL_RCGCGPIO_R |= 0b00000010;
        while ((SYSCTL_PRGPIO_R & 0b00000010) == 0) {};
        GPIO_PORTB_DIR_R &= ~0x10;
        GPIO_PORTB_AFSEL_R |= 0x10;
        GPIO_PORTB_DEN_R &= ~0x10;
        GPIO_PORTB_AMSEL_R |= 0x10;
        SYSCTL_RCGCADC_R |= 0x01;

        while ((SYSCTL_PRADC_R & 0b00000001) == 0) {};

        ADC0_SAC_R &= 0x6;
        ADC0_ACTSS_R &= ~0x0008;
        ADC0_EMUX_R &= ~0xF000;
        ADC0_SSMUX3_R &= ~0xF000;
        ADC0_SSMUX3_R += 10; // this
        ADC0_SSCTL3_R = 0x0006;
        ADC0_ACTSS_R |= 0x0008;


}

uint16_t adc_read(void){

    int result;

    ADC0_PSSI_R = 0x0008; //trigger
    while((ADC0_RIS_R & 0x08) == 0) {}
    result = ADC0_SSFIFO3_R & 0xFFFF; //
    ADC0_ISC_R = 0x0008; //Clears the flags
    return result;

}

