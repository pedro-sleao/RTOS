#include <stdio.h>
#include <stdint.h>
#include <string.h>

enum {
    ST_READING = 0, ST_OPEN, ST_COMMENT, ST_CLOSE
};

uint8_t g_state = ST_READING;

int main(void) {

    uint8_t input[101];
    uint8_t output[101];

    while (1) {
        printf("Insira o texto (100 caracteres): ");
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("Falha ao ler a entrada\n");
            return 1;
        }

        uint8_t *pInput = input;
        uint8_t *pOutput = output;

        g_state = ST_READING;

        while (*pInput) {
            switch (g_state) {
                case ST_READING:
                    if (*pInput == '/') {
                        g_state = ST_OPEN;
                    } else {
                        *pOutput = *pInput;
                        pOutput++;
                    }
                    break;
                case ST_OPEN:
                    if (*pInput == '*') {
                        g_state = ST_COMMENT;
                    } else {
                        *pOutput = '/';
                        pOutput++;
                        *pOutput = *pInput;
                        pOutput++;
                        g_state = ST_READING;
                    }
                    break;
                case ST_COMMENT:
                    if (*pInput == '*') {
                        g_state = ST_CLOSE;
                    }
                    break;
                case ST_CLOSE:
                    if (*pInput == '/') {
                        g_state = ST_READING;
                    } else {
                        g_state = ST_COMMENT;
                    }
                    break;
                default:
                    break;
            }
            pInput++;
        }

        *pOutput = '\0';
        printf("Saida: %s\n", output);
    }

    return 0;
}
