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
    /*Enable transmitter */
    UCSR0B = (1<<TXEN0);
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

void delay_ms(uint16_t ms)
{
    uint16_t i, j;
    for(i=0; i<ms; i++)
        for(j=0; j<4000; j++)
            _NOP();
}

int main(void){
    uint8_t tx_data = 0;

    setup_usart();
    
    sei();

    while(1){
        USART_Transmit(tx_data);

        if(tx_data >= 9){
            tx_data = 0;
        } else{
            tx_data++;
        }

        delay_ms(250);
    };

    return 0;
}