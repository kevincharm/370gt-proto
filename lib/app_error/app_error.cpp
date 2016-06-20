/**
 * 370GT
 * @author Kevin Tjiam <kevin@tjiam.com>
 */

#include <Arduino.h>
#include <debug_trace.h>
#include "app_error.h"

void app_err_check(uint32_t err_code) {
  trace_print("err_code: ");
  trace_print(err_code);
  trace_print("\n");
}
