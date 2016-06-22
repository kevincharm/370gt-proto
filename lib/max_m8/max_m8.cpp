/**
 * MAX M8 Driver for Arduino
 * @author Kevin Tjiam <kevin@tjiam.com>
 */

#include <Arduino.h>
#include <Wire.h>
#include <debug_trace.h>
#include <app_error.h>
#include "max_m8.h"

uint32_t max_m8_configure(max_m8_gps_t *p_gps, const max_m8_gps_conf_t gps_conf) {
  uint32_t err_code;

  p_gps->address = gps_conf.address;

  err_code = RESPONSE_OK;
  return err_code;
}

uint32_t max_m8_init(max_m8_gps_t *p_gps) {
  uint32_t err_code;

  Wire1.begin();
  Wire1.beginTransmission(p_gps->address);
  //Wire1.write();
  Wire1.endTransmission();

  err_code = RESPONSE_OK;
  return err_code;
}
