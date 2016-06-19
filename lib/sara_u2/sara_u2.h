/**
 * SARA U2 Driver for Arduino
 * @author Kevin Tjiam <kevin@tjiam.com>
 */

#ifndef SARA_U2_H__
#define SARA_U2_H__

#include <stdint.h>

/* AT parser FSM */
#define SARA_U2_AT_PARSER_TIMEOUT             5000
#define SARA_U2_AT_PARSER_OK                  0
#define SARA_U2_AT_PARSER_ERROR               1
#define SARA_U2_AT_PARSER_BUSY                2
#define SARA_U2_AT_PARSER_READY               5

struct sara_u2_modem_t;

typedef enum sara_u2_event_type_t {
  SARA_U2_EVENT_NETWORK_READY
} sara_u2_event_type_t;

typedef struct sara_u2_event_t {
  sara_u2_modem_t *modem;
  sara_u2_event_type_t event_type;
  void *event_data;
} sara_u2_event_t;

typedef struct sara_u2_modem_t {
  volatile uint32_t parser_state;
  volatile bool has_network;
  volatile bool has_internet;
  void (*event_handler)(sara_u2_event_t *event);
  const char *apn;
} sara_u2_modem_t;

typedef struct sara_u2_modem_conf_t {
  void (*event_handler)(sara_u2_event_t *event);
  const char *apn;
} sara_u2_modem_conf_t;

extern void sara_u2_accept_serial_event(sara_u2_modem_t *p_modem);
extern uint32_t sara_u2_configure(sara_u2_modem_t *p_modem, const sara_u2_modem_conf_t modem_conf);
extern uint32_t sara_u2_init(sara_u2_modem_t *p_modem);

#endif
