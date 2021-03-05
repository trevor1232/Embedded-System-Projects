// Name: Trevor Scott
// Documentation
// Description: This project’s purpose is to demonstrate DC motors activity when using a pulse-width modulation(PWM) as a motor control, 
// and how to adjust it based on the desired frequency and different duty cycle values.
#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "PLL.h"
#include "PWM.h"
// in respect to INx 
#define BACKWARD  0x0A  //00001010
#define FORWARD   0x05  //00000101
#define BREAK  		0x00  //00000000

//predefine functions
void DisableInterrupts(); // Disable interrupts
void EnableInterrupts();  // Enable interrupts
void WaitForInterrupt(void);  // low power mode
void Switch_Init();
void MoveForward(unsigned long delay);
void MoveBackward(unsigned long delay);
void Switch_Init();
void MoveForward(unsigned long delay);
void MoveBackward(unsigned long delay);
void GPIOPortF_Handler(void);

unsigned long trackSpeed  = 0;	// control PWM
unsigned long trackMovement = 0; // control Direction
unsigned long Duty   = 0; // control Duty
// estimated time/1Khz, units = 7500/25000 = 0.3, for verifying 30%
uint16_t dutyPercent[] = {0,7500,15000,20000,24500}; // In percent: 0,30,60,80,98


int main(void){
	DisableInterrupts(); // disable interrupts while initializing
	PLL_Init();          // bus clock at 50 MHz
	PWM0B_Init(25000, 0); // 1khz (50Mhz/2)/1Khz = 25000
  PWM0A_Init(25000, 0); // 1khz
	Switch_Init();
	EnableInterrupts(); // enable after all initialization are done
  while(1){
    WaitForInterrupt();
  }
}


void Switch_Init(){
	SYSCTL_RCGC2_R |= 0x22; 
	// Port B_Motor
	GPIO_PORTB_AMSEL_R 		&= ~0x0F; 			// 3) disable analog function on PB3-0
	GPIO_PORTB_PCTL_R 		&= ~0x0000FFFF; // 4) enable regular GPIO
	GPIO_PORTB_DIR_R		  |= 0x0F; 				// 5) outputs on PB3-0
	GPIO_PORTB_AFSEL_R 		&= ~0x0F;			  // 6) regular function on PB3-0
	GPIO_PORTB_DEN_R 			|= 0x0F; 				// 7) enable digital on PB3-0	
	// Port F_LED
	GPIO_PORTF_AMSEL_R 		&= ~0x0E; 
	GPIO_PORTF_PCTL_R 	  &= ~0x0000FFF0;
	GPIO_PORTF_DIR_R 		  |= 0x0E;
	GPIO_PORTF_AFSEL_R 		&= ~0x0E;
	GPIO_PORTF_DEN_R 			|= 0x0E;
	GPIO_PORTF_DATA_R      = 0x02;
	// Port F_Buttons
	GPIO_PORTF_LOCK_R 		 = 0x4C4F434B;  // 2) unlock PortF PF0  	
	GPIO_PORTF_AMSEL_R 		&= ~0x11;  			// disable analog functionality on PF4
	GPIO_PORTF_PCTL_R 		&= ~0x000F000F; // configure PF4,0 as GPIO
	GPIO_PORTF_DIR_R 			&= ~0x11;    		// make PF4,0 in (built-in button)
	GPIO_PORTF_AFSEL_R 		&= ~0x11;  			// disable alt funct on PF4,0
	GPIO_PORTF_CR_R 			 = 0x11; 			  // allow changes to PF4,0
	GPIO_PORTF_PUR_R 			|= 0x11;     		// enable weak pull-up on PF4
	GPIO_PORTF_DEN_R 		  |= 0x11;     		// enable digital I/O on PF4
	GPIO_PORTF_DIR_R 			&= ~0x11; 			// make PF4,0 in (built-in button)
	// Interrupt 
	GPIO_PORTF_IS_R 			&= ~0x11;       // PF4,0 is edge-sensitive
	GPIO_PORTF_IBE_R 			&= ~0x11;       // PF4,0 is not both edges
	GPIO_PORTF_IEV_R 			&= ~0x11;    		// PF4,0 falling edge event
	GPIO_PORTF_ICR_R 			 = 0x11;      	// clear flag4,0
	GPIO_PORTF_IM_R 			|= 0x11;      	// arm interrupt on PF4,0
	NVIC_PRI7_R 					 = (NVIC_PRI7_R&0xFF00FFFF)|0x00A00000; // priority 5
	NVIC_EN0_R 						 = 0x40000000;      // enable interrupt 30 in NVIC
}

void MoveForward(unsigned long delay){
	if (delay >0){
	GPIO_PORTF_DATA_R = 0x08;
	GPIO_PORTB_DATA_R = FORWARD;
	}
}

void MoveBackward(unsigned long delay){
	if (delay >0){
	GPIO_PORTF_DATA_R = 0x04;
	GPIO_PORTB_DATA_R = BACKWARD;		
	}
}

// sw1 or sw2 is pressed
void GPIOPortF_Handler(void){  
	int i=0;
	for(i=0; i<2000000; i++); //debounce approx. 
	// sw 1 is pressed
	if (GPIO_PORTF_RIS_R&0x10) { 
		GPIO_PORTF_ICR_R = 0x10; // acknowledge flag 0
		trackSpeed = trackSpeed + 1;
		// cases for trackSpeed would work too
		if (trackSpeed%5 == 1) {
			Duty = dutyPercent[1];
			MoveForward(Duty);
		}
		else if (trackSpeed%5 == 2){
			Duty = dutyPercent[2];
		}
		else if (trackSpeed%5 == 3) {
			Duty = dutyPercent[3];
		}
		else if(trackSpeed%5 == 4) {
			Duty = dutyPercent[4];
		}
		else{
			Duty = dutyPercent[0];
			GPIO_PORTF_DATA_R = 0x02;
			GPIO_PORTB_DATA_R = BREAK;
			trackSpeed=0;
		}
		PWM0A_Duty(Duty); 	
		PWM0B_Duty(Duty); 
	} 
	// sw 2 is pressed
	if (GPIO_PORTF_RIS_R&0x01){   
		GPIO_PORTF_ICR_R = 0x01; // acknowledge flag 4
		trackMovement = trackMovement + 1;
		if(trackMovement%2 == 1) {
			MoveBackward(Duty);
		}
		else {
			MoveForward(Duty);
		}
	}
}

