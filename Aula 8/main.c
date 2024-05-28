/* Necessário para funções e macros básicas */
#include <avr/io.h>

/* Os próximos dois includes são necessários quando se usa interrupçoes */
#include <avr/cpufunc.h>
#include <avr/interrupt.h>

#include <stdint.h>

#define FOSC 16000000ul
#define BAUD 250000
#define MYUBRR (FOSC/(8ul*BAUD) - 1)


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
    if ((UCSR0A & (1 << UPE0) | UCSR0A & (1 << FE0))){
        
    } else{
        USART_Transmit(data);
    }
}

volatile uint16_t compa_count = 0;
ISR(TIMER0_COMPA_vect) {
    compa_count++;
    /* Pisque o LED a cada 320 ms */
    if (compa_count == 2000){
        PINB = (1 << PB5);
        compa_count = 0;
    }
    /* Introduza um delay de 20 instruções NOP */
    delay(20);
}

/*
 * Função para criar um pequeno delay variável de aproximadamente n
 * us. Máximo de 255 us
 */
void delay(uint8_t n) {
    uint8_t i;

    while (n--)
        for (i=0; i<4; i++)
            asm("nop");
}

void send_bits(uint8_t n){
    uint8_t j;

    for(j=0; j<8; j++){
        if(n & 1){
            PORTD |= (1 << PD2);
        } else{
            PORTD &= ~(1 << PD2);
        }
        n >>= 1;
    }
}

int main(void) {
    /*
     * Configure aqui a porta que controla o LED, iniciando-o apagado
     */
    DDRB |= (1 << PB5);

    DDRD |= (1 << PD2);

    /* Configura o timer 0 para dar timeout a cada 160 us (dica: use
       modo CTC) */
    TCCR0A |= (1 << WGM01);
    TCCR0B |= (1 << CS02);
    TIMSK0 |= (1 << OCIE0A);
    TCNT0 = 0;
    OCR0A = 10;

    /* Configura a interface serial para receber dados no formato 8N1
       a 480 kHz */
    UBRR0H = (MYUBRR>>8); /*Set baud rate */
    UBRR0L = MYUBRR; /*Set baud rate */
    UCSR0A = (1 << U2X0); /* double speed */
    UCSR0B = (1 << TXEN0) | (1 << RXEN0) | (1 << RXCIE0); /* Enable transmitter and receiver, and enable receive complete interrupt */
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); /* Set frame format: 8N1 */

    /* Habilita as interrupções */
    sei();

    /* Gera o sinal digital correspondente a 0xAA55. Se usar loop,
       coloque um delay de 2 instruções NOP antes de mudar o sinal */
    uint16_t data = 0xAA55;
    while (1) {
        /* O seu código aqui */
        send_bits((data >> 0) & 0XFF);
        send_bits((data >> 8) & 0XFF);

        /* Pequeno delay para separar os sinais individuais */
        delay(40);
    }

    return 0;
}
