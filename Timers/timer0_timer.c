#include <avr/io.h>
#include <avr/cpufunc.h>
#include <avr/interrupt.h>

#define F_CPU 16000000

void setup_timer0(void){
    // valor máximo = 2^8 - 1 = 255
    TCNT0 = 0; 

    TCCR0A = 0x00; 

    // 16M/1024 = 15625 Hz
    TCCR0B = (1 << CS02) | (1 << CS00); 

    // Habilita a interrupção de overflow
    TIMSK0 = (1 << TOIE0); 
}

volatile uint8_t overflow_count = 0;

ISR(TIMER0_OVF_vect){
    overflow_count++;
    TCNT0 = 0;

    if(overflow_count == 61) // Numero de overflows necessários para contar 1 segundo com 15626 Hz
    {
        PINB = (1 << PB5);
        overflow_count = 0;
    }
}

int main(void) {
    DDRB |= (1 << PB5);

    setup_timer0();

    sei();

    while (1);

    return 0;
}