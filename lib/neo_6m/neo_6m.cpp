/**
 * u-blox NEO-6M Driver for Arduino
 * @author Kevin Tjiam <kevin@tjiam.com>
 */

#include <Arduino.h>
#include <Wire.h>
#include <debug_trace.h>
#include <app_error.h>
#include "neo_6m.h"

/**
 *  UBX Checksum
 *  from: u-blox M8 Receiver Description Spec (Section 30.4, Page 127)
 */
static inline void ubx_packet_calculate_checksum(ubx_packet_t *p_packet) {
  p_packet->checksum_a = 0;
  p_packet->checksum_b = 0;

  p_packet->checksum_a += p_packet->message_class;
  p_packet->checksum_b += p_packet->checksum_a;
  p_packet->checksum_a += p_packet->message_id;
  p_packet->checksum_b += p_packet->checksum_a;
  p_packet->checksum_a += (p_packet->payload_length & 0xFF); /* little endian */
  p_packet->checksum_b += p_packet->checksum_a;
  p_packet->checksum_a += ((p_packet->payload_length >> 8) & 0xFF);
  p_packet->checksum_b += p_packet->checksum_a;

  for (uint16_t i = 0; i < p_packet->payload_length; i++) {
    p_packet->checksum_a += p_packet->p_payload[i];
    p_packet->checksum_b += p_packet->checksum_a;
  }
}

uint32_t ubx_packet_send(neo_6m_gps_t *p_gps, ubx_packet_t *p_packet) {
  uint32_t err_code;

  trace_print("\n=> proc ubx_packet_send()\n");

  ubx_packet_calculate_checksum(p_packet);

  trace_print("==> Calculated checksum: ");
  trace_print(p_packet->checksum_a);
  trace_print(p_packet->checksum_b);
  trace_print("\n");

  Serial2.print(UBX_HEADER_1);
  Serial2.print(UBX_HEADER_2);
  Serial2.print(p_packet->message_class);
  Serial2.print(p_packet->message_id);
  Serial2.print(p_packet->payload_length & 0xFF);
  Serial2.print((p_packet->payload_length >> 8) & 0xFF);
  trace_print("==> Begin writing payload...\n");
  for (uint16_t i = 0; i < p_packet->payload_length; i++) {
      Serial2.print(p_packet->p_payload[i]);
  }
  trace_print("==> Finished writing payload...\n");
  Serial2.print(p_packet->checksum_a);
  Serial2.print(p_packet->checksum_b);

  trace_print("==> Sent UBX packet.\n");

  err_code = RESPONSE_OK;
  return err_code;
}

void handle_gps_response(neo_6m_gps_t *p_modem, char *response, uint16_t response_length) {
  trace_print("[NEO-6M] ");
  for (int i=0; i<response_length; i++) {
    trace_print((char)response[i]);
  }
  trace_print("\r\n");
}

void flush_gps_responses(neo_6m_gps_t *p_gps) {
  while (Serial2.available()) {
    uint8_t read_buffer[256];
    uint8_t read_buffer_length;

    // clear buffer
    memset(read_buffer, '\0', 256);
    read_buffer_length = Serial2.readBytesUntil('\n', read_buffer, 255);

    handle_gps_response(p_gps, (char *)read_buffer, read_buffer_length);
  }
}

void neo_6m_accept_serial_event(neo_6m_gps_t *p_gps) {
  flush_gps_responses(p_gps);
}

uint32_t start_gps(neo_6m_gps_t *p_gps) {
  uint32_t err_code;

  while (!Serial2);
  Serial2.begin(9600);
  Serial2.setTimeout(5000);
  flush_gps_responses(p_gps);

  err_code = RESPONSE_OK;
  return err_code;
}

uint32_t neo_6m_configure(neo_6m_gps_t *p_gps, const neo_6m_gps_conf_t gps_conf) {
  uint32_t err_code;

  p_gps->event_handler = gps_conf.event_handler;

  err_code = RESPONSE_OK;
  return err_code;
}

uint32_t neo_6m_init(neo_6m_gps_t *p_gps) {
  uint32_t err_code;

  start_gps(p_gps);

  static ubx_packet_t ubx_cfg_port_packet = {
    .message_class = UBX_CLASS_CFG,
    .message_id = UBX_CFG_PRT,
    .payload_length = 20
  };
  ubx_cfg_port_packet.p_payload[0] = 1;    // portID - 1 or 2 for UART
  ubx_cfg_port_packet.p_payload[1] = 0;    // reserved0
  ubx_cfg_port_packet.p_payload[2] = 0;    // txReady
  ubx_cfg_port_packet.p_payload[3] = 0;    // txReady
  /* port config */
  uint32_t cfg_port_mode = 0x03 << 6;      // charLen - 8 bits
  cfg_port_mode |= 0x04 << 9;              // no parity
  ubx_cfg_port_packet.p_payload[4] = cfg_port_mode & 0xFF;
  ubx_cfg_port_packet.p_payload[5] = (cfg_port_mode >> 8) & 0xFF;
  ubx_cfg_port_packet.p_payload[6] = (cfg_port_mode >> 16) & 0xFF;
  ubx_cfg_port_packet.p_payload[7] = (cfg_port_mode >> 24) & 0xFF;
  /* baudrate */
  uint32_t baudrate = 9600;
  ubx_cfg_port_packet.p_payload[8] = baudrate & 0xFF;
  ubx_cfg_port_packet.p_payload[9] = (baudrate >> 8) & 0xFF;
  ubx_cfg_port_packet.p_payload[10] = (baudrate >> 16) & 0xFF;
  ubx_cfg_port_packet.p_payload[11] = (baudrate >> 24) & 0xFF;
  ubx_cfg_port_packet.p_payload[12] = 0x02; // inProtoMask LSB
  ubx_cfg_port_packet.p_payload[13] = 0x00;
  ubx_cfg_port_packet.p_payload[14] = 0x02; // outProtoMask LSB
  ubx_cfg_port_packet.p_payload[15] = 0x00;
  ubx_cfg_port_packet.p_payload[16] = 0x00; // always set to zero
  ubx_cfg_port_packet.p_payload[17] = 0x00; // always set to zero
  ubx_cfg_port_packet.p_payload[18] = 0x00; // always set to zero
  ubx_cfg_port_packet.p_payload[19] = 0x00; // always set to zero
  ubx_packet_send(p_gps, &ubx_cfg_port_packet);

  delay(200);

  static ubx_packet_t ubx_packet_test = {
    .message_class = UBX_CLASS_CFG,
    .message_id = UBX_CFG_PRT,
    .payload_length = 0,
    .p_payload = 0
  };
  ubx_packet_send(p_gps, &ubx_packet_test);

  err_code = RESPONSE_OK;
  return err_code;
}

void neo_6m_poll_cfg_prt(neo_6m_gps_t *p_gps) {
  static ubx_packet_t ubx_packet_test = {
    .message_class = UBX_CLASS_CFG,
    .message_id = UBX_CFG_PRT,
    .payload_length = 0,
    .p_payload = 0
  };
  ubx_packet_send(p_gps, &ubx_packet_test);
}
