#include <msp430.h>
#include <stdio.h>

#define LED1 BIT0
#define LED2 BIT6
#define RX BIT1
#define TX BIT2
#define S2 BIT3
#define BUTTON BIT4

volatile char return_AT [10] = "";

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
    P1OUT = S2 | BUTTON;
    P1REN  = S2 | BUTTON;
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
    volatile unsigned int i;
    volatile char command[] = "AT+BAUD4";
    volatile char temp;

    WDTCTL = WDTPW | WDTHOLD;       // Stop watchdog timer
    _config_clock();
    _config_gpio();
    _config_uart();

    while(1)
    {
        if((P1IN & S2) == 0)
        {
            for(i = 0; i< strlen(command); i++)
            {
                while(!(IFG2 & UCA0TXIFG));
                UCA0TXBUF = command[i];
            }
            _delay_cycles(12000000);
        }
    }
}
#pragma vector = USCIAB0RX_VECTOR
__interrupt void USCI0RX_IRS (void)
{
    while(!(IFG2 & UCA0RXIFG));
    //strcat(return_AT, UCA0RXBUF);
    if(UCA0RXBUF == '9')
        P1OUT ^= 0x01;
}
