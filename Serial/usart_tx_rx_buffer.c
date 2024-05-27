#include <avr/io.h>
#include <avr/cpufunc.h>
#include <avr/interrupt.h>

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
    /* Enable transmitter and receiver, and enable receive complete interrupt */
    UCSR0B = (1 << TXEN0) | (1 << RXEN0) | (1 << RXCIE0);
    /* Set frame format: 8N1 */
    UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);
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


uint8_t rx_buffer[BUFFER_SIZE];     /* buffer para transmissão */
uint8_t rx_head, rx_tail;

uint16_t read(void)
{
    uint8_t rc = 0, c = 0;

    cli();
    if (rx_head != rx_tail) {
        c = rx_buffer[rx_tail];
        if (++rx_tail >= BUFFER_SIZE)
            rx_tail = 0;
    } else 
        rc = 1;
    sei();
    return ((uint16_t) rc << 8) | c;
}

volatile uint8_t rcvd_byte;

ISR(USART_RX_vect)
{
    uint8_t next = rx_head + 1;
    if (next >= BUFFER_SIZE)
        next = 0;
    
    /* Se o buffer estiver cheio desprezamos o byte recebido mais antigo e
     * colocamos o byte que acabamos de receber em seu lugar */
    rx_buffer[rx_head] = UDR0;
    rx_head = next;
    if (next == rx_tail) { /* buffer cheio */
        rx_tail++;
        if (rx_tail >= BUFFER_SIZE)
            rx_tail = 0;
    }
    rcvd_byte = 1;
}

int main(void){
    setup_usart();

    uint16_t rc = 0;
    uint8_t c = 0;
    
    ++c; /* para calar o warning do compilador sobre c não estar sendo usado */

    sei();

    while(1) {
        if (rcvd_byte) {
            rc = read();
            if (rc & 0xFF00)
                continue;
            
            write(rc);
            
            c = rc;
            rcvd_byte = 0;
        }
    }

    return 0;
}