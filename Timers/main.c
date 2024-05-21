#include <avr/io.h>
#include <avr/cpufunc.h>
#include <avr/interrupt.h>

#define F_CPU 16000000

void setup_timer1(void){
    TCNT1 = 65535 - 3125;

    TCCR1A = 0x00;

    // 16M/1024 = 15625 Hz
    TCCR1B = (1 << CS12) | (1 << CS10);
    
}

ISR(TIMER1_OVF_vect){
    TIMSK1 &= ~(1 << TOIE1);
    EIFR |= (1 << INTF0);
    EIMSK |= (1 << INT0);
}

ISR(INT0_vect){
    EIMSK &= ~(1 << INT0);
    PINB = (1 << PB5);
    TIMSK1 = (1 << TOIE1);
    TCNT1 = 65535 - 3125;
}


int main(void) {
    DDRB |= (1 << PB5);

    DDRD &= ~(1 << PD2);
    PORTD |= (1 << PD2);

    EICRA |= (1 << ISC01) | (1 << ISC00);
    EIFR |= (1 << INTF0);
    EIMSK |= (1 << INT0);

    setup_timer1();

    sei();

    while (1);

    return 0;
}