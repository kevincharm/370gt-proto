/**
 * 370GT
 * @author Kevin Tjiam <kevin@tjiam.com>
 */

#include <Arduino.h>
#include <stdint.h>
#include <string.h>
#include <debug_trace.h>
#include <app_error.h>
#include <sara_u2.h>
#include <max_m8.h>
#include "conf.h"

static sara_u2_modem_t g_modem;
static max_m8_gps_t g_gps;

void modem_event_handler(sara_u2_event_t *p_event) {
  trace_print("Got modem event!\n");
  if (p_event->event_type == SARA_U2_EVENT_NETWORK_READY) {
    trace_print("Network ready!\n");
  }
}

void serialEvent() {
  sara_u2_accept_serial_event(&g_modem);
}

void setup() {
  while (!SerialUSB);
  SerialUSB.begin(115200);

  trace_print("\n\n*** Kev's 370GT Vehicle Remote Compute Unit ***\n\n");

  static const sara_u2_modem_conf_t modem_conf = {
    .event_handler = modem_event_handler,
    .apn = APN_TELSTRA
  };
  trace_print("Configuring modem.\n");
  sara_u2_configure(&g_modem, modem_conf);
  trace_print("Initialising modem.\n");
  sara_u2_init(&g_modem);

  static const max_m8_gps_conf_t gps_conf = {
    .address = GPS_I2C_ADDRESS
  };
  trace_print("Configuring GPS.\n");
  max_m8_configure(&g_gps, gps_conf);
  trace_print("Initialising GPS.\n");
  max_m8_init(&g_gps);

  trace_print("Finished initialisation.\n");
}

void loop() {
  //
}
