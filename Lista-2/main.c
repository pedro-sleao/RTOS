#include <stdio.h>
#include <stdint.h>
#include <string.h>

enum {
    ST_READING = 0, ST_OPEN, ST_COMMENT, ST_CLOSE
};

enum {
    EV_BAR, EV_STAR
};

uint8_t g_state = ST_READING;

uint8_t process_event(uint8_t ev);

int main(void) {

    uint8_t input[100];
    uint8_t output[100];

    while (1) {
        printf("Insira o texto (ou 'exit' para sair): ");
        if (scanf("%100s", input) != 1) {
            printf("Falha ao ler a entrada\n");
            return 1;
        }

        if (strcmp((char *)input, "exit") == 0) {
            break;
        }

        uint8_t *pInput = input;
        uint8_t *pOutput = output;
        
        uint8_t ev;
        uint8_t c = 0;

        g_state = ST_READING;

        while (*pInput) {
            if (*pInput == '/') {
                ev = EV_BAR;
            } else if (*pInput == '*') {
                ev = EV_STAR;
            } else {
                ev = *pInput;
            }
            c = process_event(ev);
            if (c != 0) {
                *pOutput = c;
                pOutput++;
            }
            pInput++;
        }

        *pOutput = '\0';
        printf("Saida: %s\n", output);
    }

    return 0;
}

uint8_t process_event(uint8_t ev) {
    uint8_t output = 0;

    switch (g_state) {
        case ST_READING:
            if (ev == EV_BAR) {
                g_state = ST_OPEN;
                output = 0;
                break;
            } else if (ev == EV_STAR) {
                output = '*';
                break;
            }
            output = ev;
            break;
        case ST_OPEN:
            if (ev == EV_STAR) {
                g_state = ST_COMMENT;
                output = 0;
                break;
            } else if (ev == EV_BAR) {
                g_state = ST_OPEN;
                output = '/';
                break;
            }
            g_state = ST_READING;
            output = ev;
            break;
        case ST_COMMENT:
            if (ev == EV_STAR) {
                g_state = ST_CLOSE;
                output = 0;
                break;
            } 
            output = 0; 
            break;
        case ST_CLOSE:
            if (ev == EV_BAR) {
                g_state = ST_READING;
                output = 0;
                break;
            }
            g_state = ST_COMMENT;
            output = 0;
            break;
        default:
            output = ev;
    }
    return output;
}
