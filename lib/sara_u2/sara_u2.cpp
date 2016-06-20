/**
 * SARA U2 Driver for Arduino
 * @author Kevin Tjiam <kevin@tjiam.com>
 */

#include <Arduino.h>
#include <string.h>
#include <debug_trace.h>
#include <app_error.h>
#include "sara_u2.h"

uint32_t send_modem_command(sara_u2_modem_t *p_modem, const char *command);

static inline void set_modem_state(sara_u2_modem_t *p_modem, uint32_t state) {
    p_modem->parser_state = state;
}

bool is_modem_state(sara_u2_modem_t *p_modem, uint32_t state) {
    return p_modem->parser_state == state;
}

void network_ready(sara_u2_modem_t *p_modem) {
    if (p_modem->has_network && p_modem->has_internet) {
        // gets network info
        //send_modem_command(p_modem, "AT+COPS?");

        // gprs
        send_modem_command(p_modem, "AT+CGATT=1");                       /* GSM & GPRS registration */
        delay(1000);

        /* build apn string */
        static const char set_apn_at_verb[] = "AT+UPSD=0,1,";
        char set_apn_at_command[strlen(set_apn_at_verb) + strlen(p_modem->apn)];
        strcpy(set_apn_at_command, set_apn_at_verb);
        strcat(set_apn_at_command, p_modem->apn);
        send_modem_command(p_modem, (const char *)set_apn_at_command);
        //send_modem_command(p_modem, "AT+UPSD=0,7,\"0.0.0.0\"");          /* set dynamic IP */
        //send_modem_command(p_modem, "AT+UPSDA=0,1");                     /* save profile to NVM */
        send_modem_command(p_modem, "AT+UPSDA=0,3");                     /* activate connection */
        send_modem_command(p_modem, "AT+UPSND=0,8");                     /* check status of connection */
        send_modem_command(p_modem, "AT+UPSND=0,0");                     /* check assigned IP */

        // test HTTP GET (self-test)
        send_modem_command(p_modem, "AT+UHTTP=0");
        //send_modem_command(p_modem, "AT+UHTTP=0,1,\"" BACKEND_SERVER_ADDRESS "\"");
        send_modem_command(p_modem, "AT+UHTTP=0,1,\"jsonplaceholder.typicode.com\"");
        //send_modem_command(p_modem, "AT+UHTTP=0,5," BACKEND_SERVER_PORT);
        send_modem_command(p_modem, "AT+UHTTP=0,5,80");
        send_modem_command(p_modem, "AT+UHTTPC=0,1,\"/posts/1\",\"buffer_get\"");    /* must wait for +UUHTTPCR: 0,1,1 (HTTP GET success URC) */

        sara_u2_event_t event = {
          .modem = p_modem,
          .event_type = SARA_U2_EVENT_NETWORK_READY,
          .event_data = NULL
        };
        p_modem->event_handler(&event);
    }
}

void handle_modem_response(sara_u2_modem_t *p_modem, char *response, uint16_t response_length) {
    trace_print("[SARA-U270] ");
    for (int i=0; i<response_length; i++) {
        trace_print((char)response[i]);
    }
    trace_print("\r\n");

    // Final response
    if (!strncmp(response, "OK", 2)) {
        set_modem_state(p_modem, SARA_U2_AT_PARSER_READY);
    } else if (!strncmp(response, "ERROR", 5) ||
               !strncmp(response, "+CME ERROR", 10) ||
               !strncmp(response, "+CMS ERROR", 10) ||
               !strncmp(response, "ABORTED", 7)) {
        set_modem_state(p_modem, SARA_U2_AT_PARSER_READY);
    }

    // URCs
    if (!strncmp(response, "+CREG: 1", 8)) {
        /* Connected to home network */
        p_modem->has_network = true;
        network_ready(p_modem);
    } else if (!strncmp(response, "+UREG: 3", 8) ||
               !strncmp(response, "+UREG: 6", 8) ||
               !strncmp(response, "+UREG: 9", 8)) {
        /**
         * UREG URCs for Telstra
         * +UREG: 3 => WCDMA/UMTS
         * +UREG: 6 => HSDPA
         * +UREG: 9 => GPRS/EDGE
         */
        if (!p_modem->has_internet) {
            p_modem->has_internet = true;
            network_ready(p_modem);
        }
    } else if (!strncmp(response, "+UUHTTPCR: 0,1,1", 16)) {
        /* HTTP GET success URC */
        send_modem_command(p_modem, "AT+URDFILE=\"buffer_get\"");
        /* TODO: sscanf the result codes */
    }
}

void flush_modem_responses(sara_u2_modem_t *p_modem) {
    while (Serial.available()) {
        uint8_t read_buffer[256];
        uint8_t read_buffer_length;

        // clear buffer
        memset(read_buffer, '\0', 256);
        read_buffer_length = Serial.readBytesUntil('\n', read_buffer, 255);

        handle_modem_response(p_modem, (char *)read_buffer, read_buffer_length);
    }
}

uint32_t send_modem_command(sara_u2_modem_t *p_modem, const char *command) {
    uint32_t err_code;

    delay(100);
    /* Write command to UART */
    for (uint32_t i = 0; i < strlen(command); i++) {
        Serial.print(command[i]);
    }
    Serial.print("\r\n");

    /* Block while parsing response from modem */
    set_modem_state(p_modem, SARA_U2_AT_PARSER_BUSY);
    uint32_t begin_systick = millis();
    do {
        delay(20);
        flush_modem_responses(p_modem);
    } while (is_modem_state(p_modem, SARA_U2_AT_PARSER_BUSY) && (begin_systick - millis() < SARA_U2_AT_PARSER_TIMEOUT));

    /* Set return values */
    if (is_modem_state(p_modem, SARA_U2_AT_PARSER_BUSY)) {
        err_code = RESPONSE_TIMEOUT;
    } else {
        err_code = RESPONSE_OK;
    }

    /* Make sure AT parser state is reset */
    set_modem_state(p_modem, SARA_U2_AT_PARSER_READY);

    return err_code;
}

uint32_t start_modem(sara_u2_modem_t *p_modem) {
    uint32_t err_code;

    while (!Serial);
    Serial.begin(115200);
    Serial.setTimeout(5000);
    flush_modem_responses(p_modem);

    // wakeup
    do {
        delay(1000);
        err_code = send_modem_command(p_modem, "AT");
    } while (err_code != RESPONSE_OK);

    // reset factory settings
    send_modem_command(p_modem, "AT&F");

    // echo off
    send_modem_command(p_modem, "ATE1");

    // Check PIN ready
    send_modem_command(p_modem, "AT+CPIN?");

    // Disable DTE flow control
    send_modem_command(p_modem, "AT&K0");

    // Verbose errors
    send_modem_command(p_modem, "AT+CMEE=2");

    // Set modem baud rate
    send_modem_command(p_modem, "AT+IPR=115200");

    // Network registration
    //send_modem_command(p_modem, "AT+URAT?");     /* +URAT: 1,2 is UMTS dual-mode */
    //send_modem_command(p_modem, "AT+UBANDSEL?"); /* returns available bands */

    // Enable URCs.
    send_modem_command(p_modem, "AT+UREG=1");
    send_modem_command(p_modem, "AT+CREG=1");

    err_code = RESPONSE_OK;
    return err_code;
}

void sara_u2_accept_serial_event(sara_u2_modem_t *p_modem) {
  flush_modem_responses(p_modem);
}

uint32_t sara_u2_configure(sara_u2_modem_t *p_modem, const sara_u2_modem_conf_t modem_conf) {
  uint32_t err_code;

  p_modem->apn = modem_conf.apn;
  p_modem->event_handler = modem_conf.event_handler;

  err_code = RESPONSE_OK;
  return err_code;
}

uint32_t sara_u2_init(sara_u2_modem_t *p_modem) {
  uint32_t err_code;

  // init defaults
  p_modem->parser_state = SARA_U2_AT_PARSER_READY;
  p_modem->has_network = false;
  p_modem->has_internet = false;

  start_modem(p_modem);

  err_code = RESPONSE_OK;
  return err_code;
}
