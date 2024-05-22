#include <avr/io.h>
#include <avr/cpufunc.h>
#include <avr/interrupt.h>

#define F_CPU 16000000

ISR(TIMER1_OVF_vect){
    /*
    * Desabilita a interrupção de overflow do timer 1
    * Habilita a interrupção do INT0
    */
    TIMSK1 &= ~(1 << TOIE1);
    EIMSK |= (1 << INT0);
}

ISR(INT0_vect){
    /*
    * Desabilita a interrupção do INT0
    * Toggle no PB5
    * Reseta o valor da contagem
    * Habilita a interrrupção de overflow do timer 1
    */
    EIMSK &= ~(1 << INT0);
    PINB = (1 << PB5);
    TCNT1 = 0;
    TIMSK1 |= (1 << TOIE1);
}


int main(void) {
    /* Define o LED como saída */
    DDRB |= (1 << PB5);

    /* Define INT0 como pull-up */
    DDRD &= ~(1 << PD2);
    PORTD |= (1 << PD2);

    /* Define a interrupção como rising-edge
     * Limpa a flag da interrupção
     * Habilita a interrupção para o INT0 */
    EICRA |= (1 << ISC01) | (1 << ISC00);
    EIFR |= (1 << INTF0);
    EIMSK |= (1 << INT0);

    /* 
    * Define o valor de comparação
    * Ativa o modo CTC
    * Divide o clock
    */
    OCR1A = (F_CPU/1024)*0.2;
    TCCR1A = (1 << WGM10);
    TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10);

    sei();

    while (1);

    return 0;
}