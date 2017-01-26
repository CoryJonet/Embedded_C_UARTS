#include "inc/uart.h"

extern void EnableInterrupts(void);
extern void DisableInterrupts(void);
extern uint32_t initPortAUart0(void);

#define BUFFER_SIZE 16

CircularBuffer rx_Buffer;
CircularBuffer tx_Buffer;

UART_PERIPH *myUart;

/****************************************************************************
 * Configure UART 0 for 8-n-1 with RX and TX interrupts enabled.
 * Enable the RX and TX FIFOs as well.
 ****************************************************************************/
bool initUART0(uint32_t baud)
{
	//Delay for clock gating
	uint32_t delay;
	
	// *******************************
  // Set up the UART registers
  // *******************************
  myUart = (UART_PERIPH *)UART0;
	
	//Initialize 
	initPortAUart0();

  // Eanble the clock gating register
  // ( Not found in the UART_PERIPH struct)
  SYSCTL_RCGCUART_R |= SYSCTL_RCGCUART_R0;     

  delay = SYSCTL_RCGCUART_R;

  // Set the baud rate
	if(baud == 9600)
	{
		myUart->IntegerBaudRateDiv = 520;
		myUart->FracBaudRateDiv = 53;
	}
	else
	{
		myUart->IntegerBaudRateDiv = 43;         
		myUart->FracBaudRateDiv = 26;
	}		

  // Configure the Line Control for 8-n-1
  myUart->LineControl |= (UART_LCRH_WLEN_8 | UART_LCRH_FEN);    

  // Enable the UART - Need to enabel both TX and RX
  myUart->UARTControl |= (UART_CTL_RXE | UART_CTL_TXE | UART_CTL_UARTEN);                 

  // Wait until the UART is avaiable
  while( !(SYSCTL_PRUART_R & SYSCTL_PRUART_R0 ))
  {

	}

  delay = 500;
  while( delay != 0)
  {
    delay--;
  }
	
	UART0_IFLS_R |= (UART_IFLS_RX1_8 | UART_IFLS_TX1_8);
	
	UART0_IM_R |= (UART_IM_RXIM | UART_IM_RTIM | UART_IM_TXIM); // enable TX and RX FIFO interrupts and RX time-out interrupt
	
  NVIC_EN0_R |= NVIC_EN0_INT5;          // enable interrupt 5 in NVIC
	
	cBufInit(&rx_Buffer, 16);							// init buffer w/ size 16
	
	cBufInit(&tx_Buffer, 16);							// init buffer w/ size 16
	
	EnableInterrupts();

  return true;
}

/****************************************************************************
 * This function returns a single character from the Rx circular buffer.  
 * It takes one argument which indicates if the function will wait until 
 * data is found.  
 ****************************************************************************/
int uartRx(bool block)
{
	char character;
	
	if(!block)
	{
		// If !block and the buffer is empty
		if(cBufGetFreeCount(&rx_Buffer) == 0)
		{
			return -1;
		}
		else // if the buffer isn't empty
		{
			DisableInterrupts();									// crit section start
			cBufGetChar(&rx_Buffer, &character); // grab the chars from buffer
			EnableInterrupts();									// crit section start
			UART0_IM_R |= UART_IM_RXIM | UART_IM_RTIM; // re-enable the interrupts
		}
		
	}

	else // block = true 
	{
		while(rx_Buffer.count == 0) 
		{
			// if the buffer is empty, wait until it isn't
		}
		DisableInterrupts();										// crit section start
		cBufGetChar(&rx_Buffer, &character);	 // grab the chars from buffer
		EnableInterrupts();										// crit section start
		UART0_IM_R |= UART_IM_RXIM | UART_IM_RTIM; // re-enable the interrupts
		
	}
	
	return (int) character;
}

/****************************************************************************
 * This function accepts a single character and places it into Tx circular 
 * buffer if there is not room in the Tx hardware FIFO.
 ****************************************************************************/
void uartTx(int data)
{	
	if ((UART0_FR_R & UART_FR_TXFF) == 0)   // write to hardware FIFO if not full
	{
		while(UART0_FR_R & UART_FR_BUSY) {}		// wait until the FIFO is not busy
		myUart->Data = (char) data; 					
	}
	
	else	// Else if the FIFO is full, add to Buffer instead
	{
		while(cBufGetFreeCount(&tx_Buffer) == 0)  
		{
			// wait until the buffer isn't full
		}
		
		cBufAddChar(&tx_Buffer, (char) data);

		// Enable TX interrupts 
		UART0_IM_R |= UART_IM_TXIM;
		
	}
	
}


/*****************************************************************************
*
* UART0 Handler
*
*****************************************************************************/
void UART0IntHandler()
{
	//DisableInterrupts();
	
	//Receive
  //Rx timeout OR rx full interrupts
  if(((UART0_MIS_R & UART_MIS_RXMIS) != 0) || ((UART0_MIS_R & UART_MIS_RTMIS) != 0))
  {

		//rx circular buffer full still
		while(((UART0_FR_R & UART_FR_RXFE) == 0))
		{	
			if(cBufGetFreeCount(&rx_Buffer) != 0)
			{
				cBufAddChar(&rx_Buffer, (char) myUart->Data);
			}
		}
		  
	    //Remove interrupt
			UART0_IM_R &= ~(UART_IM_RXIM | UART_IM_RTIM);
  }
	
	//Transmit
	if((UART0_MIS_R & UART_MIS_TXMIS) != 0)
	{
		//Check if Tx Buffer is empty
		if (cBufGetFreeCount(&tx_Buffer) == 16)
		{
				//Disable interrupt
				UART0_IM_R &= ~UART_IM_TXIM;
			
		}
		
		//Check if Tx isn't empty
		else
		{
			// Do this if the FIFO isn't full, and the buffer isn't empty
			while(cBufGetFreeCount(&tx_Buffer) != 16 && (UART0_FR_R & UART_FR_TXFF) == 0)
			{
				cBufGetChar(&tx_Buffer, (char*) myUart->Data);
			}
			
		}
		//Clear interrupt
		UART0_ICR_R |= UART_ICR_TXIC;
	}
	
	//EnableInterrupts();
}

//*****************************************************************************
// DO NOT MODIFY below
//*****************************************************************************

//****************************************************************************
//  This function is called from MicroLIB's stdio library.  By implementing
//  this function, MicroLIB's getchar(), scanf(), etc will now work.
// ****************************************************************************/
int fgetc(FILE* stream)
{
   int c;

   if (stream != stdin)
   {
      errno = EINVAL; // should probably be ENOSTR
      return EOF;
   }

   c = uartRx(true);

   if (c == '\r')
      c = '\n';
   
   fputc(c, stdout);

   return c;
}

//****************************************************************************
// This function is called from MicroLIB's stdio library.  By implementing
// this function, MicroLIB's putchar(), puts(), printf(), etc will now work.
// ****************************************************************************/
int fputc(int c, FILE* stream)
{
   if (stream != stdout) // bah! to stderr
   {
      errno = EINVAL; // should probably be ENOSTR
      return EOF;
   }

   uartTx(c);

   if (c == '\n')
      uartTx('\r');

   return c;
}





