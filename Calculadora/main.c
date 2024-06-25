#include <stdio.h>
#include <stdint.h>
#include <ctype.h>

enum {
    EV0 = 0, EV1, EV2, EV3, EV4, EV5, EV6, EV7, EV8, EV9,
    EVMAIS, EVMENOS, EVMULT, EVDIV,
    EVPONTO, EVIGUAL, EVCLEAR,
    EVINVALIDCHAR
};

enum {
    ST_WAITSIGN1 = 0, ST_INT1, ST_INTX1, ST_FRAC1, /* Primeiro superestado */
    ST_WAITSIGN2, ST_INT2, ST_INTX2, ST_FRAC2, ST_ANSWER, /* Segundo superestado */
    ST_ERROR
};

int8_t signal = 1;
uint64_t rint, rfrac = 0;
float result;

uint8_t g_state;
void process_event(uint8_t ev);

int main(void) {

    uint8_t input[21];
    uint8_t *pInput = input;

    uint8_t ev;

    while(1) {
        scanf("%20s", pInput);
        while(*pInput) {
            if (isdigit(*pInput)) {
                ev = (uint8_t) *pInput - 48; /* Subtrair 48 para pegar o valor real do digito pela tabela ASCII */
            } else if (*pInput == '+') {
                ev = EVMAIS;
            } else if (*pInput == '-') {
                ev = EVMENOS;
            } else if (*pInput == '*') {
                ev = EVMULT;
            } else if (*pInput == '/') {
                ev = EVDIV;
            } else if (*pInput == '.') {
                ev = EVPONTO;
            } else if (*pInput == '=') {
                ev = EVIGUAL;
            } else if (*pInput == 'c') {
                ev = EVCLEAR;
            } else {
                ev = EVINVALIDCHAR;
            }
            process_event(ev);
            pInput++;
        }
        
    }


    return 0;
}

void process_event(uint8_t ev) {
    switch (g_state) {
        case ST_WAITSIGN1:
            if (ev == EVMAIS) {
                signal = 1;
            } else if (ev == EVMENOS) {
                signal = -1;
            } else if (ev <= EV9) {
                rint = ev;
                g_state = ST_INT2;
            } else {
                g_state = ST_ERROR;
            }
            break;
        case ST_INT1:
            if (rint == 0) {
                if (ev <= EV9) {
                    rint = ev;
                } else {
                    g_state = ST_ERROR;
                }
            } else if (ev == EVPONTO) {
                g_state = ST_FRAC1;
            }
            
            break;
        case ST_INTX1:
            break;
        case ST_FRAC1:
            break;
        case ST_WAITSIGN2:
            break;
        case ST_INT2:
            break;
        case ST_INTX2:
            break;
        case ST_FRAC2:
            break;
        case ST_ANSWER:
            break;
        case ST_ERROR:
            rint, rfrac = 0;
            break;
        default:
            break;
    }

        
}