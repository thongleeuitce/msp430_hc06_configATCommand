#include <msp430.h>
#include <stdio.h>

#define LED1 BIT0
#define LED2 BIT6
#define RX BIT1
#define TX BIT2
#define S2 BIT3

#define pinnumber "AT+PIN5555"
#define name "AT+NAMEthonglee"
#define baudrate "AT+BAUD4"

volatile unsigned int count = 0;
volatile unsigned int i;

void _config_clock(void)
{
    if(CALBC1_12MHZ == 0xFF)
        while(1);
    DCOCTL = 0;
    BCSCTL1 = CALBC1_12MHZ;
    DCOCTL = CALDCO_12MHZ;

    BCSCTL2 = SELM_0 | DIVM_0;
}

void _config_gpio(void)
{
    P1DIR = LED1 | LED2; // Set P1.0 to output direction
    P1OUT = S2;
    P1REN  = S2;
}

void _config_uart(void)
{
    P1SEL = RX | TX;            // P1.1 is RX, P1.2 is TX
    P1SEL2 = RX | TX;

    UCA0CTL1 = UCSWRST | UCSSEL_2;

    UCA0CTL0 = 0x00;
    UCA0MCTL = UCBRF_2 | UCBRS_0 | UCOS16;
    UCA0BR0 = 78;
    UCA0BR1 = 00;

    UCA0CTL1 &= ~UCSWRST;
    IE2 = UCA0RXIE;
    _BIS_SR(GIE);
}
void main(void)
{
    volatile char command_name[] = name;
    volatile char command_pin[] = pinnumber;
    volatile char command_baud[] = baudrate;

    WDTCTL = WDTPW | WDTHOLD;       // Stop watchdog timer
    _config_clock();
    _config_gpio();
    _config_uart();

    while(1)
    {
        if((P1IN & S2) == 0)
        {
            switch(count)
            {
            case 0:
                transmit_AT(command_name);
                break;
            case 1:
                transmit_AT(command_pin);
                break;
            case 2:
                transmit_AT(command_baud);
                break;
            }
        }
    }
}

void transmit_AT(char* temp)
{
    for (i = 0; i< strlen(temp); i++)
    {
        while(!(IFG2 & UCA0TXIFG));
        UCA0TXBUF = temp[i];
    }
    count ++;
    _delay_cycles(12000000);
}

#pragma vector = USCIAB0RX_VECTOR
__interrupt void USCI0RX_IRS (void)
{
    while(!(IFG2 & UCA0RXIFG));
    if (UCA0RXBUF == 'K')
        P1OUT ^= 0x01;
}
