#include <avr/io.h>
#include <avr/cpufunc.h>
#include <avr/interrupt.h>

#define FOSC 16000000ul
#define BAUD 9600
#define MYUBRR (FOSC/(8ul*BAUD) - 1)

void setup_usart(void){
    /*Set baud rate */
    UBRR0H = (MYUBRR>>8);
    UBRR0L = MYUBRR;
    /* double speed */
    UCSR0A = (1 << U2X0); 
    /* Enable receiver and transmitter, and enable receive complete interrupt */
    UCSR0B = (1<<RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
    /* Set frame format: 8N1 */
    UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);
}

void USART_Transmit(uint8_t data)
{
    /* Wait for empty transmit buffer */
    while (!(UCSR0A & (1<<UDRE0)));
    /* Put data into buffer, sends the data */
    UDR0 = data;
}

uint8_t USART_Receive(void)
{
    /* Wait for data to be received */
    while (!(UCSR0A & (1<<RXC0)))
    ;
    /* Get and return received data from buffer */
    return UDR0;
}

ISR(USART_RX_vect){
    uint8_t data;

    data = USART_Receive();
    USART_Transmit(data+1);
}

int main(void){
    setup_usart();

    sei();

    while(1);

    return 0;
}
