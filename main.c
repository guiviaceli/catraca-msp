#include <msp430.h>

unsigned long i = 0;
unsigned char rx_index = 0;

unsigned char RX_data [32];

/*
This section controls the NFC reader for the project.
Components: MSP430g2553, ID20-LA, Buzzer, yellow led, resistors and jumpers.
ID20-LA must be connected as following: pin 1 - gnd, pin 2, pin 3, pin 4 and pin 5 - disconnected, pin 6 - series with the yellow led, going to gnd, pin 7 - gnd, pin 8 - P1.1, pin 9 and pin 10 - disconnected, pin 11 - Vcc.
The following code will be connected to the keyboard reader and the 7 segments dysplay ones.
The buzzer is connected to P1.0, a resistor and gnd
*/

void configurePorts(void)
{
    // Configure P1 as output in low level 
    P1DIR = 0xFF;
    P1OUT = 0x00;

     // Configure P2 as output in low level 
    P2DIR = 0xFF;
    P2OUT = 0x00;
}

// The buzzer bips shortly twice 
void allowed(void)
{
    // Turns on the buzzer and wait for a while
    P1OUT |= BIT0;
    for(i=0;i<100000;i++);
    // Turns off the buzzer and wait for a while
    P1OUT &=~ BIT0;
    for(i=0;i<100000;i++);
    // Turns on the buzzer and wait for a while
    P1OUT |= BIT0;
    for(i=0;i<100000;i++);
    P1OUT &= ~BIT0;
    // Turns off the buzzer and wait for a while
    //! This loop is probably unnecessary
    for(i=0;i<1000000;i++);
}

// The buzzer bips once for longer
void denied(void)
{
    // Turns on the buzzer and wait for a while
    P1OUT |= BIT0;
    for(i=0;i<1000000;i++);
    // Turns off the buzzer
    P1OUT &=~ BIT0;
}

void ini_u_con(void){
    /*
    MCLK: 16 MHz
    SMCLK: 8 MHz
    ACLK: 32768 Hz
    LFXT1: 32k crystal
    DC0: 16 MHz
    */
    WDTCTL = WDTPW + WDTHOLD;
    DCOCTL = CALDCO_16MHZ;
    BCSCTL1 = CALBC1_16MHZ;
    BCSCTL2 = DIVS0;
    BCSCTL3 = XCAP0 + XCAP1;

    // waits for the capacitor to be charged
    while(BCSCTL3 & LFXT1OF);
    __enable_interrupt();
}

void configureUART(void)
{
    /*
    9600 bps
    8 bits of data
    SMCLK 8MHz
    */

    // Enable software reset
    UCA0CTL1 |= UCSWRST;
    UCA0CTL0 = 0;
    // Selects the SMCLK as clock source
    UCA0CTL1 |= UCSSEL1 + UCSWRST;
    // Frequency: 8,000,000, Baud rate: 9600 -> 833 -> 341
    UCA0BR0 = 0x41;
    UCA0BR1 = 0x03;
    // Second stage modulation
    UCA0MCTL = UCBRS1;
    //P1.1 and P1.2 in the rx and tx mode
    P1SEL = BIT1 + BIT2;
    P1SEL2 = BIT1 + BIT2;
    // Disable the software reset
    UCA0CTL1 &= ~UCSWRST;
    // Clears the RX flag
    IFG2 &= ~ UCA0RXIFG;
    // Enable the RX to generate flags
    IE2 |= UCA0RXIE;
}

// RTI de RX
//TODO make sure this rti works
#pragma vector=USCIAB0RX_VECTOR
__interrupt void RTI_da_USCI_A0_RX(void){
    // Will read the NFC tag, if it's the allowed one, the buzzer will beep shortlt twice, if it's not allowed, the rti will make one long beep
    //! Not working yet
    denied();
//    P1OUT ^= BIT0;   // P1OUT = P1OUT ^ BIT0;
    RX_data[rx_index] = UCA0RXBUF;

    if(rx_index >= 31){
        rx_index = 0;
    }else{
        rx_index++;
    }
}

/*void ini_timer(void){
    TA0CTL= TASSEL1 + MC0;
    TA0CCTL0 = CCIE;
    TA0CCR0 = 39999;
}*/


void main(void)
{
    configurePorts();
    ini_u_con();
    configureUART();
    //ini_timer();
    allowed();

    denied();

    // Loops forever to keep the MSP430G2553 working
    do{
        __bis_SR_register(LPM0_bits + GIE);
    }while(1);
}