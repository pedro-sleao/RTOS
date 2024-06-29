/* Necessário para funções e macros básicas */
#include <avr/io.h>

/* Os próximos dois includes são necessários quando se usa interrupçoes */
#include <avr/cpufunc.h>
#include <avr/interrupt.h>

#include <stdint.h>

#define FOSC 16000000ul
#define BAUD 57600
#define MYUBRR (FOSC/(8ul*BAUD) - 1)

/* Caracteres especiais */
#define SINC 0x7e
#define XON 0x11
#define XOFF 0x13
#define ESC 0x7d

/* Protótipos das funções */
uint8_t write(uint8_t* buf, uint8_t n, int8_t close_packet);
uint8_t read(uint8_t* buf, uint8_t n);
uint8_t USART_Receive(void);
void USART_Transmit(uint8_t data);
void flow_off(void);
void flow_on(void);
void delay_ms(uint16_t ms);


/* Variáveis globais */
uint8_t rx_byte_cnt, tx_byte_cnt;
uint8_t rx_state, rx_sinc, tx_state=1;

ISR(USART_RX_vect) {
   uint8_t received_byte = UDR0;
    if (received_byte == XOFF) {
        tx_state = 0;
    } else if (received_byte == XON) {
        tx_state = 1;
    }
}

ISR(USART_UDRE_vect) {
}

int main(void) {
    /* Faça a configuração do que for necessário aqui */
    /* Seu código aqui */
    UCSR0A = (1 << U2X0); /* double speed */
    UCSR0B = (1<<RXEN0) | (1 << TXEN0) | (1 << RXCIE0); /* Enable rx and tx, rx complete interrupt, and data register empty interrupt */
    UCSR0C = (1<<UCSZ01) | (1<<UCSZ00); /* Set frame format: 8N1 */
    UBRR0H = (MYUBRR>>8); /* Set baud rate */
    UBRR0L = MYUBRR; /* Set baud rate */

    DDRB |= (1 << PB5); // Configura PB5 como saída (LED)
    PORTB &= ~(1 << PB5); // Garante que o LED está apagado no início   

    sei();

    /* Implemente os testes aqui. Veja o texto para os detalhes */
    uint8_t str_test[] = "Universidade Federal de Pernambuco\nDepartamento de Eletrônica e Sistemas";

    /* Loop infinito necessário em qualquer programa para
       embarcados */
    while (1) {
        write(str_test, sizeof(str_test)-1, 1);
        delay_ms(1000);
    }

    return 0;
}


uint8_t write(uint8_t* buf, uint8_t n, int8_t close_packet){
    tx_byte_cnt = 0;

    if (!tx_state) {
        return 0;
    }
    
    while ((tx_byte_cnt < n) && tx_state) {
        USART_Transmit(buf[tx_byte_cnt]);
        tx_byte_cnt++;
    }

    if (close_packet && tx_state) {
        USART_Transmit(SINC);
    }

    return tx_byte_cnt;
}

uint8_t read(uint8_t* buf, uint8_t n) {
}

uint8_t USART_Receive(void)
{
    /* Wait for data to be received */
    while (!(UCSR0A & (1<<RXC0)))
    ;
    /* Get and return received data from buffer */
    return UDR0;
}

void USART_Transmit(uint8_t data)
{
    /* Wait for empty transmit buffer */
    while (!(UCSR0A & (1<<UDRE0)));
    /* Put data into buffer, sends the data */
    UDR0 = data;
}

void flow_off(void) {
    USART_Transmit(XOFF);
}

void flow_on(void) {
    USART_Transmit(XON);
}

void delay_ms(uint16_t ms)
{
    uint16_t i, j;
    for(i=0; i<ms; i++)
        for(j=0; j<4000; j++)
            _NOP();
}