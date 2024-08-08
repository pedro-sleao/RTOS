/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#define PEER_ADDRESS 0x90
#define LED_PIN PORTB_LED1
#define LED_PORT IOPORT2


volatile uint8_t flag;
void gpt_cb(GPTDriver* gptd) {
  (void) gptd;
  flag = 1;
  palTogglePad(LED_PORT, LED_PIN);
}

/*
 * Application entry point.
 */
int main(void) {
  uint8_t tx_buffer = 0;
  uint8_t rx_buffer[2];
  I2CConfig i2c_config = {.clock_speed = 100000};

  GPTConfig driver_config = {.frequency = 15625,
                             .callback = gpt_cb
  };

  SerialConfig serial_config = {.sc_brr = UBRR2x(9600),
                                .sc_bits_per_char = USART_CHAR_SIZE_8};

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  palSetPadMode(LED_PORT, LED_PIN, PAL_MODE_OUTPUT_PUSHPULL);
  palClearPad(LED_PORT, LED_PIN);

  palSetPadMode(IOPORT2, PB0, PAL_MODE_INPUT_PULLUP);

  gptStart(&GPTD1, &driver_config);
  gptStartContinuous(&GPTD1, 15625);

  i2cStart(&I2CD1, &i2c_config);

  sdStart(&SD1, &serial_config);

  while (true) {
    if (flag) {
      i2cMasterTransmitTimeout(&I2CD1, PEER_ADDRESS, &tx_buffer, 1, rx_buffer, 2, TIME_INFINITE);
      chprintf((BaseSequentialStream *) &SD1, "Val = %d\n", rx_buffer[0]);
      flag = 0;
    }
  }
}
