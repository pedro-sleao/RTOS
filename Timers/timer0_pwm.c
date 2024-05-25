#include <avr/io.h>
#include <avr/cpufunc.h>
#include <avr/interrupt.h>

#define F_CPU 16000000

void setup_timer0(void)
{
    DDRD |= (1 << PD5);  /* OC0B para a placa Nano */

    OCR0A  = 200;
    OCR0B  = 40;
    TCCR0A = (1 << COM0B1) | (1 << WGM01) | (1 << WGM00);
    TCCR0B = (1 << WGM02) | (1 << CS00);
}


int main(void){

    setup_timer0();

    sei();

    while(1);

    return 0;
}