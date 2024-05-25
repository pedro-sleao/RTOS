#include <avr/io.h>
#include <avr/cpufunc.h>
#include <avr/interrupt.h>

#define F_CPU 16000000

ISR(TIMER1_COMPA_vect){
    /*
    * Desabilita a interrupção de comparação A do timer 1
    * Habilita a interrupção do INT0
    */
    TIMSK1 &= ~(1 << OCIE1A);
    EIMSK |= (1 << INT0);
}

ISR(INT0_vect){
    /*
    * Desabilita a interrupção do INT0
    * Toggle no PB5
    * Reseta o valor da contagem
    * Habilita a interrrupção de comparação A do timer 1
    */
    EIMSK &= ~(1 << INT0);
    PINB = (1 << PB5);
    TCNT1 = 0;
    TIMSK1 |= (1 << OCIE1A);
}


int main(void) {
    /* Define o LED como saída */
    DDRB |= (1 << PB5);

    /* Define INT0 como pull-up */
    DDRD &= ~(1 << PD2);
    PORTD |= (1 << PD2);

    /* Define a interrupção como falling-edge
     * Limpa a flag da interrupção
     * Habilita a interrupção para o INT0 */
    EICRA |= (1 << ISC01);
    EIFR |= (1 << INTF0);
    EIMSK |= (1 << INT0);

    /* 
    * Define o valor de comparação
    * Ativa o modo CTC
    * Divide o clock
    */
    OCR1A = 3125;
    TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10);
    TIFR1 = (1 << OCF1A);

    sei();

    while (1);

    return 0;
}