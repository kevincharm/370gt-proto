/**
 * 370GT
 * @author Kevin Tjiam <kevin@tjiam.com>
 */

#include <Arduino.h>
#include "app_error.h"

void app_err_check(uint32_t err_code) {
  SerialUSB.print("err_code: ");
  SerialUSB.print(err_code);
  SerialUSB.print("\n");
}
