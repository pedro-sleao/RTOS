#include <avr/io.h>
#include <avr/cpufunc.h>
#include <avr/interrupt.h>

#define F_CPU 16000000


void setup_timer1(void){
    // valor máximo = 2^16 - 1 = 65535
    TCNT1 = 65535 - F_CPU/1024;

    OCR1A = 57722;

    TCCR1A = 0x00;

    // 16M/1024 = 15625 Hz
    TCCR1B = (1 << CS12) | (1 << CS10);

    // Habilita a interrupção de comparação e de overflow
    TIMSK1 =  (1 << OCIE1A) | (1 << TOIE1); 
}

ISR(TIMER1_OVF_vect){
    PORTB &= ~(1 << PB5);
    TCNT1 = 65535 - F_CPU/1024;
}   

ISR(TIMER1_COMPA_vect){
    PORTB |= (1 << PB5);
}

int main(void){
    DDRB |= (1 << PB5);

    setup_timer1();

    sei();

    while(1);

    return 0;
}