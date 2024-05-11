/* Necessário para funções e macros básicas */
#include <avr/io.h>

/* Os próximos dois includes são necessários quando se usa interrupçoes */
#include <avr/cpufunc.h>
#include <avr/interrupt.h>

#include <stdint.h>

ISR(INT0_vect) {
    PINB = (1 << PB5); // Toggle
}

ISR(INT1_vect) {
    if ((PORTB >> PB5) & 1){
        EICRA |= (1 << ISC01) | (0 << ISC00);
    }
    else {
        EICRA &= ~(1 << ISC01);
        EICRA |= (1 << ISC00);
    }
}

int main(void) {
    /* Configure aqui o pino INT0 (mapeado no pino PD2 da porta D)
     * como entrada com pull-up (ver seção 14.2.1 da folha de dados do
     * ATMega328p) */
    DDRD &= ~(1 << PD2);
    PORTD |= (1 << PD2);

    DDRD &= ~(1 << PD3);
    PORTD |= (1 << PD3);

    DDRB |= (1 << PB5);
    
    /*
     * Configure aqui os registradores EICRA, EIFR e EIMSK aqui para
     * habilitar (ou não) a interrupção associada à mudança no pino
     * INT0 (veja Seção 13.2 da Folha de Dados do ATMega328p).
     */
    EICRA |= (1 << ISC01) | (0 << ISC00);

    EICRA |= (1 << ISC11) | (0 << ISC10);

    EIFR |= (1 << INTF0) | (1 << INTF1);
    EIMSK |= (1 << INT0) | (1 << INT1);


    /* Habilita globalmente as instruções */
    sei();

    /* Aguarda mudar algo nos estados do botão. */
    while (1) {
    }

    return 0;
}