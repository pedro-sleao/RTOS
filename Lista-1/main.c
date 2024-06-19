/* Necessário para funções e macros básicas */
#include <avr/io.h>

/* Os próximos dois includes são necessários quando se usa interrupçoes */
#include <avr/cpufunc.h>
#include <avr/interrupt.h>

#include <stdint.h>

uint8_t write(uint8_t* buf, uint8_t n, int8_t close_packet);
uint8_t read(uint8_t* buf, uint8_t n);
void flow_off();
void flow_on();

ISR(USART_RX_vect) {
}

ISR(USART_UDRE_vect) {
}

int main(void) {
    /* Faça a configuração do que for necessário aqui */
    /* Seu código aqui */
    UCSR0A = 0;
    UCSR0B = 0;
    UCSR0C = 0;
    UBRR0 = 1;

    sei();

    /* Implemente os testes aqui. Veja o texto para os detalhes */

    /* Loop infinito necessário em qualquer programa para
       embarcados */
    while (1) {
    }

    return 0;
}


uint8_t write(uint8_t* buf, uint8_t n, int8_t close_packet){
    return 0;
}

uint8_t read(uint8_t* buf, uint8_t n){
    return 0;
}

void flow_off(){
}

void flow_on(){
}