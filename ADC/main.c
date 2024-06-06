/* Necessário para funções e macros básicas */
#include <avr/io.h>

/* Os próximos dois includes são necessários quando se usa interrupçoes */
#include <avr/cpufunc.h>
#include <avr/interrupt.h>

#define FOSC 16000000ul
#define BAUD 9600
#define MYUBRR (FOSC/(8ul*BAUD) - 1)
#define BUFFER_SIZE 20

void setup_adc(void)
{
    PORTC &= ~_BV(PC0);
    DDRC &= ~_BV(PC0);
    ADMUX = _BV(REFS0); /* V_REF = Vcc, usando ADC0 */
    ADCSRB = _BV(ADTS2) | _BV(ADTS1); /* Trigger no timer1 overflow */
    DIDR0 = _BV(ADC0D); /* disabilita buffer de entrada no pin AC0 */
    /* Habilita módulo e inicia conversão; habilita conversão automática
     * quando timer1 overflow; habilita interrupção */
    ADCSRA = _BV(ADEN) | _BV(ADSC) | _BV(ADATE) | _BV(ADIE) | \
             _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);
}

void setup_usart(void){
    /*Set baud rate */
    UBRR0H = (MYUBRR>>8);
    UBRR0L = MYUBRR;
    /* double speed */
    UCSR0A = (1 << U2X0); 
    /* Enable transmitter */
    UCSR0B = (1<<TXEN0);
    /* Set frame format: 8N1 */
    UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);
}

void setup_timer1(void)
{
    /*
     * Agora configuramos setup_timer1 no modo Fast PWM com overflow
     * a cada 2 centésimos de segundo
     */
    OCR1AH = 1250 >> 8;
    OCR1AL = 1250 & 0xFF;
    TCCR1A = _BV(WGM11) | _BV(WGM10);
    TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS12);
}


void delay_ms(uint16_t ms)
{
    uint16_t i, j;
    for(i=0; i<ms; i++)
        for(j=0; j<4000; j++)
            _NOP();
}

uint8_t tx_buffer[BUFFER_SIZE];     /* buffer para transmissão */
uint8_t tx_head, tx_tail;   /* ponteiros para o buffer circular */
uint8_t usart_transmitting;

uint8_t write(uint8_t c)
{
    uint8_t rc = 0;

    cli();

    if(!usart_transmitting){
        UDR0 = c;
        UCSR0B |= (1 << UDRIE0);
        usart_transmitting = 1;
    } else{
        uint8_t next = tx_head + 1;
        if (next >= BUFFER_SIZE)
            next = 0;
        if (next != tx_tail) {
            tx_buffer[tx_head] = c;
            tx_head = next;
        } else  /* buffer cheio */
            rc = 1;
    }
    sei();
    return rc;
}

ISR(USART_UDRE_vect)
{
    if (tx_head == tx_tail) {
        UCSR0B &= ~(1 << UDRIE0);
        usart_transmitting = 0;
    } else {
        UDR0 = tx_buffer[tx_tail];
        if (++tx_tail >= BUFFER_SIZE)
            tx_tail = 0;
    }
}

volatile uint8_t conversion_complete;
volatile uint16_t data;

ISR(ADC_vect)
{
    conversion_complete = 1;
    data = ADC;
    TIFR1 = _BV(TOV1);
}

int main(void) {

    setup_timer1();
    setup_usart();
    setup_adc();

    sei();

    while(1){
        while(!conversion_complete);

        conversion_complete = 0;
        write(data & 0xFF);
        write((data >> 8) & 0xFF);
        delay_ms(250);  
    }

    return 0;
}