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

#define LED_PIN PORTB_LED1
#define LED_PORT IOPORT2

void gpt_cb(GPTDriver* gptd) {
  (void) gptd;
  palTogglePad(LED_PORT, LED_PIN);
}

/*
 * Application entry point.
 */
int main(void) {
  GPTConfig driver_config = {.frequency = 16000000,
                             .callback = gpt_cb
  };

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

  gptStart(&GPTD2, &driver_config);
  gptStartContinuous(&GPTD2, 255);

  
  while (true) {

  }
}
