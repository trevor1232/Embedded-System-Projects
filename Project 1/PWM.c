// PWM.c
// Runs on TM4C123
// Use PWM0/PB6 and PWM1/PB7 to generate pulse-width modulated outputs.
// Daniel Valvano
// March 28, 2014

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2014
  Program 6.7, section 6.3.2

 Copyright 2014 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

#include <stdint.h>
#include "tm4c123gh6pm.h"

#define PWM_0_GENA_ACTCMPAD_ONE 0x000000C0  // Set the output signal to 1
#define PWM_0_GENA_ACTLOAD_ZERO 0x00000008  // Set the output signal to 0
#define PWM_0_GENB_ACTCMPBD_ONE 0x00000C00  // Set the output signal to 1
#define PWM_0_GENB_ACTLOAD_ZERO 0x00000008  // Set the output signal to 0

#define SYSCTL_RCC_USEPWMDIV    0x00100000  // Enable PWM Clock Divisor
#define SYSCTL_RCC_PWMDIV_M     0x000E0000  // PWM Unit Clock Divisor
#define SYSCTL_RCC_PWMDIV_2     0x00000000  // /2
static uint16_t period0;


// period is 16-bit number of PWM clock cycles in one period (3<=period)
// period for PB6 and PB7 must be the same
// duty is number of PWM clock cycles output is high  (2<=duty<=period-1)
// PWM clock rate = processor clock rate/SYSCTL_RCC_PWMDIV
//                = BusClock/2 
//                = 80 MHz/2 = 40 MHz (in this example)
// Output on PB6/M0PWM0


void PWM0B_Init(uint16_t period, uint16_t duty){
  volatile unsigned long delay;
  SYSCTL_RCGCPWM_R |= 0x01;             // step 1 activate PWM0, pg 354
  SYSCTL_RCGCGPIO_R |= 0x02;            // step 2 activate port B
  delay = SYSCTL_RCGCGPIO_R;            // allow time to finish activating
	period0 = period;
	GPIO_PORTB_LOCK_R = 0x4C4F434B;       // unlock GPIO Port
	GPIO_PORTB_CR_R |=0x80; // what will change which is PB7
	GPIO_PORTB_AFSEL_R |=0x80;  //since we use clk we enable alt function, pg 143 textbook
	GPIO_PORTB_PCTL_R &= ~0xF0000000;     // configure PB7 as PWM0, pg 163 textbook
  GPIO_PORTB_PCTL_R |= 0x40000000;		// Will be used for digital functions
	GPIO_PORTB_AMSEL_R &= ~0x80;          // disable analog functionality on PB6
	GPIO_PORTB_DEN_R |= 0x80;             // enable digital I/O on PB6
	GPIO_PORTB_DIR_R |=0x80;  // output pin
		
	SYSCTL_RCC_R |= SYSCTL_RCC_USEPWMDIV; // step 3 use PWM divider, pg 254
  SYSCTL_RCC_R &= ~SYSCTL_RCC_PWMDIV_M; //  clear PWM divider field
  SYSCTL_RCC_R += SYSCTL_RCC_PWMDIV_2;  //  configure for /2 divider
	PWM0_0_CTL_R = 0; // this means it will count from load to 0, pg 1270
	PWM0_0_GENB_R = 0x80c; // step 4; low on load, high on cmpa down, pg 1285
	PWM0_0_LOAD_R = period -1; // step 5 cycles needed to count down to 0
	PWM0_0_CMPB_R = duty -1; // step 6 count value when output rises
	PWM0_0_CTL_R=0x00000001; // step 7 enable/start generator, pg 1270
	PWM0_ENABLE_R |= 0x00000002;          // enable PB7/M0PWM1, pg 1247
}

void PWM0A_Init(uint16_t period, uint16_t duty){
  period0 = period;
	SYSCTL_RCGCGPIO_R |= 0x02; // step 1  to activate port B
	SYSCTL_RCGCPWM_R |=0x01; // step 2 to activate pwm0, pg354
  volatile unsigned long delay = SYSCTL_RCGCGPIO_R; // allow time to finish activating
	GPIO_PORTB_LOCK_R = 0x4C4F434B;       // unlock GPIO Port
	GPIO_PORTB_CR_R |=0x40; // what will change which is PB6
	GPIO_PORTB_AFSEL_R |=0x40;  //since we use clk we enable alt function, pg 143 textbook
	GPIO_PORTB_DIR_R |=0x40;  // output pin
	GPIO_PORTB_PCTL_R &= ~0x0F000000;     // configure PB6 as PWM0, pg 163 textbook
  GPIO_PORTB_PCTL_R |= 0x04000000;		// Will be used for digital functions
	GPIO_PORTB_AMSEL_R &= ~0x40;          // disable analog functionality on PB6
	GPIO_PORTB_DEN_R |= 0x40;             // enable digital I/O on PB6
	
	SYSCTL_RCC_R |= SYSCTL_RCC_USEPWMDIV; // step 3 use PWM divider, pg 254
  SYSCTL_RCC_R &= ~SYSCTL_RCC_PWMDIV_M; //  clear PWM divider field
  SYSCTL_RCC_R += SYSCTL_RCC_PWMDIV_2;  //  configure for /2 divider
	PWM0_0_CTL_R = 0; // this means it will count from load to 0, pg 1270
	PWM0_0_GENA_R = 0xc8; // step 4; low on load, high on cmpa down, pg 1282
	PWM0_0_LOAD_R = period -1; // step 5 cycles needed to count down to 0
	PWM0_0_CMPA_R = duty -1; // step 6 count value when output rises
	PWM0_0_CTL_R=0x00000001; // step 7 enable/start generator, pg 1270
	PWM0_ENABLE_R |= 0x00000001;          // enable PB6/M0PWM0, pg 1247
}

// change duty cycle of PB6
// @param duty compare value to load into the pwm generator
void PWM0A_Duty( uint16_t duty){
	// when speed is 0 we want to disable any motion
	if(duty == 0x00 ){
		// disable any signal going to the motor, stopping any speed
		GPIO_PORTB_DATA_R = 0x00; 
		GPIO_PORTB_AFSEL_R = 0x00;
	}
	// when we hit out period
	else if(duty == period0 ){
		//when we hit our goal
		GPIO_PORTB_DATA_R |=0x40;
		//turn off generator, may not need to though
		GPIO_PORTB_AFSEL_R =0x00;
	}
	else{
		// we want our generator to be working (pwm) so we want to make sure we enable it
		GPIO_PORTB_AFSEL_R |=0x40;
		PWM0_0_CMPA_R= duty -1;
	}
}

// change duty cycle of PB7
// @param duty compare value to load into the pwm generator
void PWM0B_Duty( uint16_t duty){
	// when speed is 0 we want to disable any motion
	if(duty == 0x00 ){
		// disable any signal going to the motor, stopping any speed
		GPIO_PORTB_DATA_R = 0x00; 
		GPIO_PORTB_AFSEL_R = 0x00;
	}
	// when we hit out period
	else if(duty == period0 ){
		//when we hit our goal
		GPIO_PORTB_DATA_R |=0x80;
		//turn off generator, may not need to though
		GPIO_PORTB_AFSEL_R =0x00;	
	}
	else{
		// we want our generator to be working (pwm) so we want to make sure we enable it
		GPIO_PORTB_AFSEL_R |=0x80;
		PWM0_0_CMPA_R= duty -1;
	}
}

