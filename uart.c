volatile char uart_data;
volatile char flag;

#include <inc/tm4c123gh6pm.h>
#include <stdint.h>
#include "timer.h"
#include "uart.h"

void uart_init(void){

  //enable clock to GPIO port B
  SYSCTL_RCGCGPIO_R |= 0x02;

  //enable clock to UART1
  SYSCTL_RCGCUART_R |= 0b00000010;

  //wait for GPIOB and UART1 peripherals to be ready
  while ((SYSCTL_PRGPIO_R & 0x02) == 0) {};
  while ((SYSCTL_PRUART_R & 0b00000010) == 0) {};

  //enable alternate functions on port B pins
  GPIO_PORTB_AFSEL_R |= 0b00000011;

  //enable digital functionality on port B pins
  GPIO_PORTB_DEN_R |= 0x03;

  //enable UART1 Rx and Tx on port B pins
  GPIO_PORTB_PCTL_R = 0x11;

  //calculate baud rate
  uint16_t iBRD = 0x08; //use equations
  uint16_t fBRD = 44; //use equations

  //turn off UART1 while setting it up
  UART1_CTL_R &= ~0x1; // 0b1111 1111 1111 1110 as oth bit is uart

  //set baud rate
  
  UART1_IBRD_R = iBRD;
  UART1_FBRD_R = fBRD;

  //set frame, 8 data bits, 1 stop bit, no parity, no FIFO
  
  UART1_LCRH_R = 0x00000060;

  //use system clock as source
 
  UART1_CC_R = 0x0;

  //re-enable UART1 and also enable RX, TX (three bits)
  UART1_CTL_R = 0x0301;


}

void uart_sendChar(char data){
    while((UART1_FR_R & 0x20) != 0){}

        UART1_DR_R = data;
}

char uart_receive(void){
    char rdata;

    while ((UART1_FR_R & 0x10) != 0) {
    }

    rdata = (char) UART1_DR_R & 0xFF;
    return rdata;
}

void uart_sendStr(const char *data){

    while(*data != '\0')
    {
        uart_sendChar(*data);
        data++;
    }
}

void uart_interrupt_init() {
    // Enable interrupts for receiving bytes through UART1
    UART1_IM_R |= 0b10000;
    // Find the NVIC enable register and bit responsible for UART1 
    NVIC_EN0_R |= 0b1000000; 

    
    IntRegister(INT_UART1, uart_interrupt_handler);
}

void uart_interrupt_handler() {
    //Check the Masked Interrupt Status
    if (UART1_MIS_R & 0b10000) {
        flag = 1;
    }

    //Copy the data
    uart_data = uart_receive();

    //Clear the interrupt
    UART1_ICR_R |= 0b10000;
}
