#include <avr/io.h>
#include <avr/cpufunc.h>
#include <avr/interrupt.h>
#include <string.h>

#define FOSC 16000000ul
#define BAUD 9600
#define MYUBRR (FOSC/(8ul*BAUD) - 1)
#define BUFFER_SIZE 20

void setup_usart(void){
    /*Set baud rate */
    UBRR0H = (MYUBRR>>8);
    UBRR0L = MYUBRR;
    /* double speed */
    UCSR0A = (1 << U2X0); 
    /* Enable transmitter */
    UCSR0B = (1<<TXEN0);
    /* Set frame format: 8N1 */
    UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);
}

void delay_ms(uint16_t ms)
{
    uint16_t i, j;
    for(i=0; i<ms; i++)
        for(j=0; j<4000; j++)
            _NOP();
}

uint8_t tx_buffer[BUFFER_SIZE];     /* buffer para transmissão */
uint8_t tx_head, tx_tail;   /* ponteiros para o buffer circular */
uint8_t usart_transmitting;

uint8_t write(uint8_t c)
{
    uint8_t rc = 0;

    cli();

    if(!usart_transmitting){
        UDR0 = c;
        UCSR0B |= (1 << UDRIE0);
        usart_transmitting = 1;
    } else{
        uint8_t next = tx_head + 1;
        if (next >= BUFFER_SIZE)
            next = 0;
        if (next != tx_tail) {
            tx_buffer[tx_head] = c;
            tx_head = next;
        } else  /* buffer cheio */
            rc = 1;
    }
    sei();
    return rc;
}

/* Flag UDREn indica se o buffer de transmissão UDRn está vazio */
ISR(USART_UDRE_vect)
{
    /*
     * Caso tx_head == tx_tail é porque todos os dados foram transmitidos e desabilita a interrupção para UDREn
     * Caso contrário envia o dado que estava no tx_tail
     */
    if (tx_head == tx_tail) {
        UCSR0B &= ~(1 << UDRIE0);
        usart_transmitting = 0;
    } else {
        UDR0 = tx_buffer[tx_tail];
        if (++tx_tail >= BUFFER_SIZE)
            tx_tail = 0;
    }
}

int main(void){
    char tx_data[] = "Hello, world!\n";
    uint8_t i;

    setup_usart();
    
    sei();

    while(1){
        /* Verifica se o dado foi escrito ou se o buffer estava cheio (Precisa desse if?) */
        if(!write(tx_data[i])){
            if (i < strlen(tx_data)){
                i++;
            } else{
                i = 0;
            }
        } 

        delay_ms(250);
    }

    return 0;
}