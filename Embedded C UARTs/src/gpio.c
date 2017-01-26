#include "inc/gpio.h"
#include "inc/lm4f120h5qr.h"

#ifndef	PORTA
#define PORTA                 0x40004000
#endif

int32_t initPortAUart0(void)
{
    GPIO_PORT *myPort = (GPIO_PORT *)PORTA;
	
		uint32_t delay;
	
	// *******************************************
  // Configure GPIO PA0 and PA1 to be UART Pins
  // *******************************************

  // Turn on the clock gating register for GPIO port A
  // Make sure not to turn of any of the other ports
  SYSCTL_RCGCGPIO_R |= 0x3;                    

  delay = SYSCTL_RCGCGPIO_R;

  // Set the Digital Enable
	myPort->DigitalEnable |= 0x3;              

  // Set the Alternate Function
  myPort->AlternateFunctionSelect |=  0x3;    

  // Set the Port Control Register
  myPort->PortControl |= (GPIO_PCTL_PA0_U0RX | GPIO_PCTL_PA1_U0TX);

	return 1;
	
}
