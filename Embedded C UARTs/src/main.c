//*****************************************************************************
//
//*****************************************************************************
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "lm4f120h5qr.h"
#include "../team.h"
#include "inc/gpio.h"
#include "inc/uart.h"


/******************************************************************************
 * Defines
 *****************************************************************************/
#define PORTA                 0x40004000
#define PORTB                 0x40005000
#define PORTC                 0x40006000
#define PORTD                 0x40007000
#define PORTE                 0x40024000 
#define PORTF                 0x40025000


/******************************************************************************
 * Global Variables
 *****************************************************************************/
 GPIO_PORT *GpioPortA = (GPIO_PORT *)PORTA;
 GPIO_PORT *GpioPortB = (GPIO_PORT *)PORTB;
 GPIO_PORT *GpioPortC = (GPIO_PORT *)PORTC;
 GPIO_PORT *GpioPortD = (GPIO_PORT *)PORTD;
 GPIO_PORT *GpioPortE = (GPIO_PORT *)PORTE;
 GPIO_PORT *GpioPortF = (GPIO_PORT *)PORTF;
 
 
 
//*****************************************************************************
// External Functions
//*****************************************************************************
extern void PLL_Init(void);
extern void EnableInterrupts(void);
extern void DisableInterrupts(void);


//*****************************************************************************
//*****************************************************************************
void initializeBoard(void)
{
    // Initialize the PLLs so the the main CPU frequency is 80MHz
  PLL_Init();
  
  // Initialize UART0
  initUART0(115200);//115200
  
}

//*****************************************************************************
//*****************************************************************************
int 
main(void)
{
	static uint32_t user_count = 0;
	char userChar[79];
  initializeBoard();
  
  while(1)
  {	
		
		user_count++;
    printf("Enter String: ");
		
		scanf("%79s", userChar);
		
		printf("User Input (%u): %s \n", user_count , userChar);
		
  }
    
}

