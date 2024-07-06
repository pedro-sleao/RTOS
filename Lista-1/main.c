/* Necessário para funções e macros básicas */
#include <avr/io.h>

/* Os próximos dois includes são necessários quando se usa interrupçoes */
#include <avr/cpufunc.h>
#include <avr/interrupt.h>

#include <stdint.h>
#include <string.h>

#define FOSC 16000000ul
#define BAUD 57600
#define MYUBRR (FOSC/(8ul*BAUD) - 1)
#define BUFFER_SIZE 255

/* Caracteres especiais */
#define SINC 0x7e
#define XON 0x11
#define XOFF 0x13
#define ESC 0x7d

/* Protótipos das funções */
uint8_t write(uint8_t* buf, uint8_t n, int8_t close_packet);
uint8_t read(uint8_t* buf, uint8_t n);
uint16_t read_rx_buffer(void);
uint8_t write_tx_buffer(uint8_t c);
void enqueue_rx_buffer(uint8_t received_data);
void flow_off(void);
void flow_on(void);
uint8_t is_flow_on(void);
void delay_ms(uint16_t ms);
void blink_led(void);
void turn_led_on(void);

/* Variáveis globais */
uint8_t rx_byte_cnt, tx_byte_cnt;
uint8_t rx_esc, rx_state=1, rx_sinc, tx_state=1;
uint8_t rx_buffer[BUFFER_SIZE]; /* buffer para recepção */
uint8_t rx_head, rx_tail; /* ponteiros para o buffer circular */
uint8_t tx_buffer[BUFFER_SIZE];     /* buffer para transmissão */
uint8_t tx_head, tx_tail;   /* ponteiros para o buffer circular */
uint8_t usart_transmitting;

volatile uint8_t rcvd_byte=0;
ISR(USART_RX_vect) {
    uint8_t received_data = UDR0;
    
    if (!rx_esc) {
        if (received_data == ESC) {
            rx_esc = 1;
        } else if (received_data == XOFF) {
            tx_state = 0;
        } else if (received_data == XON) {
            tx_state = 1;
        } else if (received_data == SINC){
            rx_sinc = 1;
            rcvd_byte = 1;
        } else {
            enqueue_rx_buffer(received_data);
        }
    } else {
        enqueue_rx_buffer(received_data);
        rx_esc = 0;
    }
}

/* Flag UDREn indica se o buffer de transmissão UDRn está vazio */
ISR(USART_UDRE_vect)
{
    /*
     * Caso tx_head == tx_tail é porque todos os dados foram transmitidos e desabilita a interrupção para UDREn
     * Caso contrário envia o dado que estava no tx_tail
     */
    if (tx_head == tx_tail) {
        UCSR0B &= ~(1 << UDRIE0);
        usart_transmitting = 0;
    } else {
        UDR0 = tx_buffer[tx_tail];
        if (++tx_tail >= BUFFER_SIZE)
            tx_tail = 0;
    }
}

int main(void) {
    /* Faça a configuração do que for necessário aqui */
    /* Seu código aqui */
    UCSR0A = (1 << U2X0); /* double speed */
    UCSR0B = (1<<RXEN0) | (1 << TXEN0) | (1 << RXCIE0); /* Enable rx and tx, rx complete interrupt, and data register empty interrupt */
    UCSR0C = (1<<UCSZ01) | (1<<UCSZ00); /* Set frame format: 8N1 */
    UBRR0H = (MYUBRR>>8); /* Set baud rate */
    UBRR0L = MYUBRR; /* Set baud rate */

    DDRB |= (1 << PB5); // Configura PB5 como saída (LED)
    PORTB &= ~(1 << PB5); // Garante que o LED está apagado no início   

    sei();

    /* Implemente os testes aqui. Veja o texto para os detalhes */
    
    /* Teste 2 */
    uint8_t str_test[] = "Universidade Federal de Pernambuco\nDepartamento de Eletrônica e Sistemas";
    write(str_test, sizeof(str_test)-1, 1);

    /* Teste 3*/
    uint8_t i = sizeof(str_test);
    uint8_t j = 0;
    while (i) {
        if (i >= 10) {
            i -= write(str_test+j, 10, 0);
            j += 10;
        } else {
            write(str_test+j, i, 1);
            i = 0;
        }
    }

    /* Teste 4 */
    uint16_t nbytes = 0;
    uint8_t buffer[255];
    while (nbytes < 300) {
        nbytes += read(buffer, 300-nbytes > 254 ? 254 : 300-nbytes) - 1; /* Conferir se precisa desse -1 */
    }
    flow_off();
    if (is_flow_on()) {
        turn_led_on();
    } else {
        blink_led();
    }

    /* Teste 5 */
    flow_on(); /* Precisa do flow_on()? */
    PORTB &= ~(1 << PB5); /* Apaga o LED */
    nbytes = 0;
    while (nbytes < 300) {
        nbytes += read(buffer, 300-nbytes > 254 ? 254 : 300-nbytes) - 1; /* Conferir se precisa desse -1 */
    }

    flow_off();
    if (is_flow_on()) {
        turn_led_on();
    } else {
        blink_led();
    }

    /* Teste 6 */
    flow_on(); /* Precisa do flow_on()? */
    PORTB &= ~(1 << PB5); /* Apaga o LED */
    
    uint8_t binary_string[] = {0x00, 0x01, 0x11, 0x02, 0x13, 0x04, 0x7e, 0x05, 0x7d, 0x06};
    i = 0;
    
    /* É pra ficar reenviando a string ou envia ela apenas uma vez? */
    while (i < sizeof(binary_string)) { 
        if (!write(binary_string+i, 1, 0)) {
            PORTB |= (1 << PB5); /* Acende o LED */
        } else {
            PORTB &= ~(1 << PB5); /* Apaga o LED */
            i++;
        }
    }
    
    /* Teste 7 */
    uint8_t comp_str[] = "Desenvolvimento de sistemas embarcados";
    uint8_t comp_buffer[sizeof(comp_str)];
    uint8_t k = 0;
    /* Loop infinito necessário em qualquer programa para
       embarcados */
    while (1) {
        /* Letra a */
        memset(comp_buffer, 0, sizeof(comp_buffer));
        read(comp_buffer, 254);

        for (int z = 0; z < sizeof(comp_buffer); z++){
            write_tx_buffer(comp_buffer[z]);
        }
        write_tx_buffer('\n');
        
        if (memcmp(comp_buffer, comp_str, strlen((const char *)comp_str)+1) == 0) {
            PORTB |= (1 << PB5);
        } else {
            PORTB &= ~(1 << PB5);
        }

        memset(comp_buffer, 0, sizeof(comp_buffer));
        while (read(comp_buffer+k, 10) == 11) {
            k += 10;
        };
        k = 0;

        for (int z = 0; z < sizeof(comp_buffer); z++){
            write_tx_buffer(comp_buffer[z]);
        }
        write_tx_buffer('\n');

        if (memcmp(comp_buffer, comp_str, strlen((const char *)comp_str)+1) == 0) {
            PORTB |= (1 << PB5);
        } else {
            PORTB &= ~(1 << PB5);
        }
    }

    return 0;
}

/* Funções */

uint8_t write(uint8_t* buf, uint8_t n, int8_t close_packet){
    tx_byte_cnt = 0;

    if (!tx_state) {
        return 0;
    }
    
    while ((tx_byte_cnt < n) && tx_state) {
        if (buf[tx_byte_cnt] == SINC || buf[tx_byte_cnt] == XON || buf[tx_byte_cnt] == XOFF || buf[tx_byte_cnt] == ESC) {
            write_tx_buffer(ESC);
        }
        write_tx_buffer(buf[tx_byte_cnt]);
        tx_byte_cnt++;
    }

    if (close_packet && tx_state) {
        write_tx_buffer(SINC);
    }

    return tx_byte_cnt;
}

/* Falta limitar o byte_cnt caso ultrapasse o tamanho do buffer */
uint8_t read(uint8_t* buf, uint8_t n) {
    uint8_t received_byte, byte_cnt = 0;

    while (byte_cnt < n) { 
        if (!rx_state) {
            return 0;
        }
        while (!rcvd_byte);

        received_byte = read_rx_buffer();

        if (rx_sinc) {
            buf[byte_cnt] = '\0';
            rx_sinc = 0;
            return byte_cnt;
        }

        buf[byte_cnt] = received_byte;
        byte_cnt++;

        rcvd_byte = 0;
    }
    return n+1;
}

uint16_t read_rx_buffer(void)
{
    uint8_t rc = 0, c = 0;

    cli();
    if (rx_head != rx_tail) {
        c = rx_buffer[rx_tail];
        if (++rx_tail >= BUFFER_SIZE)
            rx_tail = 0;
    } else 
        rc = 1;
    sei();
    return ((uint16_t) rc << 8) | c;
}

uint8_t write_tx_buffer(uint8_t c) {
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

void enqueue_rx_buffer(uint8_t received_data) {
    uint8_t next = rx_head + 1;
        if (next >= BUFFER_SIZE)
            next = 0;
        
        /* Se o buffer estiver cheio desprezamos o byte recebido mais antigo e
        * colocamos o byte que acabamos de receber em seu lugar */
        rx_buffer[rx_head] = received_data;
        rx_head = next;
        if (next == rx_tail) { /* buffer cheio */
            rx_tail++;
            if (rx_tail >= BUFFER_SIZE)
                rx_tail = 0;
        }
        rcvd_byte = 1;
}

void flow_off(void) {
    write_tx_buffer(XOFF);
    rx_state = 0;
}

void flow_on(void) {
    write_tx_buffer(XON);
    rx_state = 1;
}

uint8_t is_flow_on(void) {
    return tx_state;
}

void delay_ms(uint16_t ms)
{
    uint16_t i, j;
    for(i=0; i<ms; i++)
        for(j=0; j<4000; j++)
            _NOP();
}

void blink_led(void) {
    uint8_t k = 5;
    while (k--) {
        PINB = (1 << PB5);
        delay_ms(1000);
    }
}

void turn_led_on(void) {
    PORTB |= (1 << PB5);
    delay_ms(5000);
    PORTB &= ~(1 << PB5);
}
