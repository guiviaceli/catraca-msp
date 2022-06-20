#include <msp430.h>
#include <stdio.h>

#define redLED BIT0
#define greenLED BIT6
#define tx BIT2
#define rx BIT1

unsigned int counter;
const char string[] = {"Hello World!\r\n"};

//configure UART module
void configureUART(void)
{
    UCA0CTL1 |= UCSSEL_2; // SMCLK
    UCA0BR0 = 104; // 1MHz 9600
    UCA0BR1 = 0x00; // 1MHz 9600
    UCA0MCTL = UCBRS0; // Modulation UCBRSx = 1
    UCA0CTL1 &= ~UCSWRST; // Initialize USCI state machine
    IE2 |= UCA0RXIE; // Enable USCI_A0 RX interrupt
}

//configure ports
void configurePorts(void)
{
   P2DIR = 0xFF;
   P2OUT = 0x00;
   P1DIR |= redLED + greenLED;
   P1OUT = 0x00;
   P1SEL = rx + tx;
   P1SEL2 = rx + tx;
}

void iniTimer(void){
    WDTCTL = WDTPW + WDTHOLD; // Stop watchdog timer
    DCOCTL = CALDCO_1MHZ; // Set DCO to 1MHz
    BCSCTL1 = CALBC1_1MHZ; // Set DCO to 1MHz
}

/*
void configureTimer(void)
{
    TA0CTL = TASSEL_2 + MC0; // SMCLK, upmode
    TA0CCR0 = 1000; // 1ms
    TA0CCTL0 = CCIE; // CCR0 interrupt enabled
}
*/

pragma vector = USIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
    P1OUT |= redLED;
    if (UCA0RXBUF == 'H')
    {
        counter = 0;
        UC0IE |= UCA0TXIE;
        UCA0TXBUF = string[counter++];
    }
   P1OUT &= ~redLED;
}

#pragma vector = USIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void)
{
    P1OUT |= greenLED;
    UCA0TXBUF = string[counter++];
    if (counter == sizeof(string) -1)
        UC0IE &= ~UCA0TXIE;
    P1OUT &= ~greenLED;
}

void main(void){
    configureUART();
    configurePorts();
    iniTimer();
    configureTimer();

    while(1);
}