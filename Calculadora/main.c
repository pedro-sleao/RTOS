#include <stdio.h>
#include <stdint.h>

enum {
    EV0, EV1, EV2, EV3, EV4, EV5, EV6, EV7, EV8, EV9,
    EVMAIS, EVMENOS, EVMULT, EVDIV,
    EVIGUAL, EVCLEAR
};

enum {
    ST_WAITSIGN1, ST_INT1, ST_INTX1, ST_FRAC1, /* Primeiro superestado */
    ST_WAITSIGN2, ST_INT2, ST_INTX2, ST_FRAC2, ST_ANSWER, /* Segundo superestado */
    ST_ERROR
};

uint8_t g_state;
void process_event(uint8_t ev) {
    switch (g_state) {
        case ST_WAITSIGN1:
            break;
        case ST_INT1:
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
            break;
        default:
            break;
    }

        
}

int main(void) {

    uint8_t input[21];
    uint8_t *pInput = input;

    uint64_t rint1, rint2 = 0;
    uint64_t rfrac1, rfrac2 = 0;

    while(1) {
        scanf("%21s", pInput);
        printf("%s", pInput);
        
    }


    return 0;
}