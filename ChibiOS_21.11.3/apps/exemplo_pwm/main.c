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

/*
 * Application entry point.
 */
int main(void) {

  static PWMConfig pwmcfg = { .frequency = 15625,
                              .period = 255,
                              .callback = 0,
                              .channels = {{PWM_OUTPUT_ACTIVE_HIGH, 0},
                                {PWM_OUTPUT_DISABLED, 0}},
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

  pwmStart(&PWMD2, &pwmcfg);
  pwmEnableChannel(&PWMD2, 0, 0);

  while (true) {
  }
}
