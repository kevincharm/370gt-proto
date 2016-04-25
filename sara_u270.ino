/**
 * SARA U270
 * @author Kevin Tjiam <kevin@tjiam.com>
 */

#include <stdint.h>
#include <string.h>

#define TRACE

/* Server addresses and shit */
#define APN_TELSTRA                 "telstra.internet"
#define BACKEND_SERVER_ADDRESS      "jsonplaceholder.typicode.com"
#define BACKEND_SERVER_PORT         "80"

/* App return values */
#define RESPONSE_OK                 0
#define RESPONSE_ERROR              1
#define RESPONSE_TIMEOUT            2

/* AT parser FSM */
#define AT_PARSER_TIMEOUT           5000
#define AT_PARSER_OK                0
#define AT_PARSER_ERROR             1
#define AT_PARSER_BUSY              2
#define AT_PARSER_READY             5
static volatile uint32_t g_modem_parser_state = AT_PARSER_READY;
static volatile uint32_t g_modem_has_network = false;
static volatile uint32_t g_modem_has_internet = false;

/* Prototypes */
uint32_t send_modem_command(const char *command);

void set_modem_state(uint32_t state) {
    g_modem_parser_state = state;
}

bool is_modem_state(uint32_t state) {
    return g_modem_parser_state == state;
}

void network_ready() {
    if (g_modem_has_network && g_modem_has_internet) {
        // gets network info
        //send_modem_command("AT+COPS?");

        // gprs
        send_modem_command("AT+CGATT=1");                       /* GSM & GPRS registration */
        delay(1000);
        
        send_modem_command("AT+UPSD=0,1,\"" APN_TELSTRA "\"");
        //send_modem_command("AT+UPSD=0,7,\"0.0.0.0\"");          /* set dynamic IP */
        //send_modem_command("AT+UPSDA=0,1");                     /* save profile to NVM */
        send_modem_command("AT+UPSDA=0,3");                     /* activate connection */
        send_modem_command("AT+UPSND=0,8");                     /* check status of connection */
        send_modem_command("AT+UPSND=0,0");                     /* check assigned IP */

        // test HTTP GET
        send_modem_command("AT+UHTTP=0");
        send_modem_command("AT+UHTTP=0,1,\"" BACKEND_SERVER_ADDRESS "\"");
        send_modem_command("AT+UHTTP=0,5," BACKEND_SERVER_PORT);
        send_modem_command("AT+UHTTPC=0,1,\"/posts/1\",\"buffer_get\"");    /* must wait for +UUHTTPCR: 0,1,1 (HTTP GET success URC) */
        send_modem_command("AT+URDFILE=\"buffer_get\"");
    }
}

void handle_modem_response(char *response, uint16_t response_length) {
#ifdef TRACE
    if (strncmp(response, "[Res]", 5)) {
        SerialUSB.print("[Res]: ");
        for (int i=0; i<response_length; i++) {
            SerialUSB.print((char)response[i]);
        }
        SerialUSB.print("\r\n");
    }
#endif

    // Final response
    if (!strncmp(response, "OK", 2)) {
        set_modem_state(AT_PARSER_READY);
    } else if (!strncmp(response, "ERROR", 5) ||
               !strncmp(response, "+CME ERROR", 10) ||
               !strncmp(response, "+CMS ERROR", 10) ||
               !strncmp(response, "ABORTED", 7)) {
        set_modem_state(AT_PARSER_READY);
    }

    // URCs
    if (!strncmp(response, "+CREG: 1", 8)) {
        /* Connected to home network */
        g_modem_has_network = true;
        network_ready();
    } else if (!strncmp(response, "+UREG: 3", 8) ||
               !strncmp(response, "+UREG: 6", 8) ||
               !strncmp(response, "+UREG: 9", 8)) {
        /**
         * UREG URCs for Telstra
         * +UREG: 3 => WCDMA/UMTS
         * +UREG: 6 => HSDPA
         * +UREG: 9 => GPRS/EDGE
         */
        if (!g_modem_has_internet) {
            g_modem_has_internet = true;
            network_ready();
        }
    }
}

void flush_modem_responses() {
    while (Serial.available()) {
        uint8_t read_buffer[256];
        uint8_t read_buffer_length;

        // clear buffer
        memset(read_buffer, '\0', 256);
        read_buffer_length = Serial.readBytesUntil('\n', read_buffer, 255);

        handle_modem_response((char *)read_buffer, read_buffer_length);
    }
}

uint32_t send_modem_command(const char *command) {
    uint32_t err_code;

    delay(100);
    /* Write command to UART */
    for (int i=0; i<strlen(command); i++) {
        Serial.print(command[i]);
    }
    Serial.print("\r\n");

    /* Block while parsing response from modem */
    set_modem_state(AT_PARSER_BUSY);
    uint32_t begin_systick = millis();
    do {
        delay(20);
        flush_modem_responses();
    } while (is_modem_state(AT_PARSER_BUSY) && (begin_systick - millis() < AT_PARSER_TIMEOUT));

    /* Set return values */
    if (is_modem_state(AT_PARSER_BUSY)) {
        err_code = RESPONSE_TIMEOUT;
    } else {
        err_code = RESPONSE_OK;
    }

    /* Make sure AT parser state is reset */
    set_modem_state(AT_PARSER_READY);

    return err_code;
}

uint32_t configure_modem() {
    uint32_t err_code;

    while (!Serial);
    Serial.begin(115200);
    Serial.setTimeout(5000);
    flush_modem_responses();

    // wakeup
    do {
        delay(1000);
        err_code = send_modem_command("AT");
    } while (err_code != RESPONSE_OK);

    // reset factory settings
    send_modem_command("AT&F");

    // echo off
    send_modem_command("ATE1");

    // Check PIN ready
    send_modem_command("AT+CPIN?");

    // Disable DTE flow control
    send_modem_command("AT&K0");

    // Verbose errors
    send_modem_command("AT+CMEE=2");

    // Set modem baud rate
    send_modem_command("AT+IPR=115200");

    // Network registration
    //send_modem_command("AT+URAT?");     /* +URAT: 1,2 is UMTS dual-mode */
    //send_modem_command("AT+UBANDSEL?"); /* returns available bands */
    
    // Enable URCs.
    send_modem_command("AT+UREG=1");
    send_modem_command("AT+CREG=1");
}

void serialEvent() {
    flush_modem_responses();
}

void setup() {
    while (!SerialUSB);
    SerialUSB.begin(115200);
    configure_modem();
}

void loop() {
    //
}
