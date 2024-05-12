/*
Members
1) Prakarsha Poudel
2) Spencer Opitz
3) Marina 
4) Preston Gaskill
*/


#include "lcd.h"
#include <math.h>
#include "servo.h"
#include "ping.h"
#include "adc.h"
#include "open_interface.h"
#include "Timer.h"
#include "uart.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <inc/tm4c123gh6pm.h>
#include "music.h"

extern volatile enum {LOW, HIGH, DONE} state;  //Set by ISR // in lab9 ping.c or ping_templace.c ours is ping_template.c
extern volatile unsigned int rising_time;  //Pulse start time  // in lab9 ping.c or ping_templace.c
extern volatile unsigned int falling_time; //Pulse end time  // in lab9 ping.c or ping_templace.c
extern volatile char uart_data;
extern volatile char flag;
extern volatile int degrees;
extern volatile int direction;
extern volatile float pcv;
extern volatile int right_calibration_val = 313200; // set
extern volatile int left_calibration_val = 287172;  // set

void move_backwards(oi_t *sensor_data);
void move_forward(oi_t *sensor_data, float distance_mm);
void turn_left(oi_t *sensor_data, double degrees);
void turn_right(oi_t *sensor_data, double degrees);

int scan(); // to scan objects

//Stores all of the information about each object scanned
typedef struct {
    int object;
    int start_angle;
    int end_angle;
    int mid_angle;
    double ping_distance;
    int adc_distance;
    double linear_width;
    char type;
} objects;

int main(void){
    //Initializations
    timer_init();
    lcd_init();
    adc_init();
    uart_init();
    uart_interrupt_init();
    servo_init();
    ping_init();

    oi_t *sensor_data = oi_alloc();
    oi_init(sensor_data); // sensor
//
//    //pYTHON GUI
//    char my_data;       // Variable to get bytes from Client
//    char command[100];  // Buffer to store command from Client
//    int index = 0;      // Index position within the command buffer

    while(1){

        //char command = cyBot_getByte();
        oi_update(sensor_data);

        if (sensor_data->cliffFrontLeftSignal < 20 ||  sensor_data->cliffFrontRightSignal < 20 ||  sensor_data->cliffLeftSignal < 20 || sensor_data->cliffRightSignal < 20) {
            uart_sendStr("\n\r: Hole in the way!");
            oi_setWheels(0, 0);

            //If left hole, turn right
            if (sensor_data->cliffLeftSignal < sensor_data->cliffRightSignal) {
                move_backwards(sensor_data);
                turn_left(sensor_data, 90);
                uart_sendStr("\n\rHit the boundary: turned LEFT!");
            }

            //Else, turn left
            else {
                move_backwards(sensor_data);
                turn_right(sensor_data, 90);
            }
        }
        //If out of bounds, move backwards and stay in the course
        if (sensor_data->cliffFrontLeftSignal > 2700 || sensor_data->cliffFrontRightSignal > 2600) {
            uart_sendStr("\n\r: Hit the boundary!");

            if (sensor_data->cliffLeftSignal < sensor_data->cliffRightSignal) {
                move_backwards(sensor_data);
                turn_left(sensor_data, 90);
                uart_sendStr("\n\rHit the boundary: turned LEFT!");
            }
            else {
                move_backwards(sensor_data);
                turn_right(sensor_data, 90);
                uart_sendStr("\n\rHit the boundary: turned RIGHT!");
            }
        }

        //If left bump, move around it
        if (sensor_data->bumpLeft) {
            uart_sendStr("\n\r: Bumped into object on the left side!");
            move_backwards(sensor_data);
            oi_update(sensor_data);
            turn_right(sensor_data, 80);
            uart_sendStr("\n\r, so I turned RIGHT!");
        }

        //If right bump, move around it
        else if (sensor_data->bumpRight) {
            uart_sendStr("\n\r: Bumped into object on the right side!");
            move_backwards(sensor_data);
            oi_update(sensor_data);
            turn_left(sensor_data, 80);
            uart_sendStr("\n\r, so I turned LEFT!");
        }


        if(flag){
            //If input is 'w', move forward
            if (uart_data == 'w') {
                //oi_setWheels(150, 150);
                move_forward(sensor_data, 150);
            }

            //If input is 's', move backwards
            else if (uart_data == 's') {
                //oi_setWheels(-150, -150);
                move_backwards(sensor_data);
            }


            //If input is 'a', turn left
            else if (uart_data == 'a') {
                oi_setWheels(150, -150);
            }

            //If input is 'd', turn right
            else if (uart_data == 'd') {
                oi_setWheels(-150, 150);
            }


            //If input is 'f', turn left
            else if (uart_data == 'f') {
                oi_setWheels(400, -400);
                timer_waitMillis(500);
                oi_setWheels(0, 0);
            }

            //If input is 'g', turn right 
            else if (uart_data == 'g') {
                oi_setWheels(-400, 400);
                timer_waitMillis(500);
                oi_setWheels(0, 0);
            }
            // If input is 'p', play music
            else if(uart_data == 'p'){
                load_songs();
                oi_play_song(1);
            }

            else if(uart_data == 'm'){
                oi_setWheels(0,0);
                int obj = scan();
            }

            else{
                oi_setWheels(0,0);
            }
            flag = 0;
        }
    }



    oi_free(sensor_data);


    return 0;

}
void turn_right(oi_t *sensor_data, double degrees){
    double sum = 0.0;
    oi_setWheels(-50,50); // move forward
    while(abs(sum) < degrees){
        oi_update(sensor_data);
        sum += sensor_data->angle;
    }
    oi_setWheels(0,0); //stop
}

void turn_left(oi_t *sensor_data, double degrees){
    double sum = 0.0;
    oi_setWheels(50,-50);
    while(abs(sum) < degrees){
        oi_update(sensor_data);
        sum += sensor_data->angle;
    }
    oi_setWheels(0,0); //stop
}

void move_forward(oi_t *sensor_data, float distance_mm){
    double sum = 0.0;
    oi_setWheels(150,150); //Move forward


    while(sum < distance_mm){
        oi_update(sensor_data);
        sum += sensor_data->distance;
    }
    oi_setWheels(0,0); //stop
}

void move_backwards(oi_t *sensor_data){
    double sum = 0;
    //Move (15 cms = 150mm) back
    oi_setWheels(-100,-100) ;//  move back
    while(sum < 150){
        oi_update(sensor_data);
        sum -= sensor_data->distance; // use -> notation since pointer
    }
    oi_setWheels(0, 0); //stop
}


int scan(){
    int i;
    char buffer[100];
    //Values for displaying data
    char start[] = "Degrees\t\tDistance (cm)\t\tIRValue\t\tDirection\tLinearWidth\n\r";
   
    char left = 'l';
    char right = 'r';
    char direction;

    uart_sendStr("\n\r");
    uart_sendStr(start);

    //Values to store data collected from the scan
    char layout[100];

    
    int previousVal = 0;
    int previousAngle = 0;
    int curVal;
    int distVal = 70;
    int Ir_Val ;
    int objectstart = 0;
    int startangle = 0;
    int endangle = 0;
    int linearwidth = 0;

    //Scan 180 degrees
    for (i = 0; i <= 180; i += 4 ) {  
        servo_move(i);

        //Give bot time to turn
        if (i == 0) {
            timer_waitMillis(500);
        }

        if(i < 90){
            direction = 'R' ;

        }
        else{
            direction = 'L' ;
        }
        //Store current IR distance in cm
        curVal = 654371 * pow(adc_read(), -1.420); // can be changed later
        Ir_Val = adc_read();

        if(Ir_Val >= 1000 && !objectstart){
            objectstart = 1;
            
            startangle = i;
            linearwidth = 0;
        }

        else if(objectstart && Ir_Val <= 950){
            endangle = i;
            linearwidth  = (curVal * sin(((endangle - startangle) * (M_PI)/180)));
            objectstart = 0;
        }

        sprintf(layout, "%d\t\t%d\t\t%d\t\t\t%c\t\t\%d\n\r", i, curVal,Ir_Val,direction,linearwidth);

        uart_sendStr(layout);

        previousVal = curVal;
        previousAngle = i;


    }

    //Formatting to print out the objects and their values to Putty
    uart_sendStr("\n\r");

    servo_move(0);

    return 1;
}




