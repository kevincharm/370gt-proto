/**
 * MAX M8 Driver for Arduino
 * @author Kevin Tjiam <kevin@tjiam.com>
 */

#include <Arduino.h>
#include <Wire.h>
#include <debug_trace.h>
#include <app_error.h>
#include "max_m8.h"

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

uint32_t ubx_packet_send(max_m8_gps_t *p_gps, ubx_packet_t *p_packet) {
  uint32_t err_code;

  trace_print("\n=> proc ubx_packet_send()\n");

  ubx_packet_calculate_checksum(p_packet);

  trace_print("==> Calculated checksum: ");
  trace_print(p_packet->checksum_a);
  trace_print(p_packet->checksum_b);
  trace_print("\n");

  Wire.beginTransmission(p_gps->address);
  Wire.write(UBX_HEADER_1);
  Wire.write(UBX_HEADER_2);
  Wire.write(p_packet->message_class);
  Wire.write(p_packet->message_id);
  Wire.write(p_packet->payload_length & 0xFF);
  Wire.write((p_packet->payload_length >> 8) & 0xFF);
  trace_print("==> Begin writing payload...\n");
  for (uint16_t i = 0; i < p_packet->payload_length; i++) {
      Wire.write(p_packet->p_payload[i]);
  }
  trace_print("==> Finished writing payload...\n");
  Wire.write(p_packet->checksum_a);
  Wire.write(p_packet->checksum_b);
  Wire.endTransmission();

  trace_print("==> Sent UBX packet.");

  uint8_t rx_buffer[256];
  uint8_t rx_buffer_length = 0;
  Wire.requestFrom(p_gps->address, 40);
  while (Wire.available()) {
    rx_buffer[rx_buffer_length++] = Wire.read();
  }

  trace_print("UBX Response: ");
  for (uint8_t i = 0; i < rx_buffer_length; i++) {
    trace_print(rx_buffer[i]);
  }
  trace_print('\n');

  err_code = RESPONSE_OK;
  return err_code;
}

uint32_t max_m8_configure(max_m8_gps_t *p_gps, const max_m8_gps_conf_t gps_conf) {
  uint32_t err_code;

  p_gps->address = gps_conf.address;

  err_code = RESPONSE_OK;
  return err_code;
}


uint32_t max_m8_init(max_m8_gps_t *p_gps) {
  uint32_t err_code;

  // Join I2C
  pinMode(71, OUTPUT);
  for (int i = 0; i < 8; i++) {
    digitalWrite(71, HIGH);
    delayMicroseconds(3);
    digitalWrite(71, LOW);
    delayMicroseconds(3);
  }
  pinMode(71, INPUT);
  Wire.begin();

  static ubx_packet_t ubx_packet_test = {
    .message_class = UBX_CLASS_MON,
    .message_id = UBX_MON_VER,
    .payload_length = 0,
    .p_payload = 0
  };
  ubx_packet_send(p_gps, &ubx_packet_test);

  err_code = RESPONSE_OK;
  return err_code;
}
