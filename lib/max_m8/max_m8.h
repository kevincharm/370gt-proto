/**
 * MAX M8 Driver for Arduino
 * @author Kevin Tjiam <kevin@tjiam.com>
 */

#ifndef MAX_M8_H__
#define MAX_M8_H__

#include <stdint.h>

/* UBX Protocol */
/* from: u-blox M8 Receiver Description Spec (Section 30, Page 125+) */
/* --- Sync headers --- */
#define UBX_HEADER_1        0xB5
#define UBX_HEADER_2        0x62
/* --- Message class --- */
#define UBX_CLASS_NAV       0x01
#define UBX_CLASS_RXM       0x02
#define UBX_CLASS_INF       0x04
#define UBX_CLASS_ACK       0x05
#define UBX_CLASS_CFG       0x06
#define UBX_CLASS_UPD       0x09
#define UBX_CLASS_MON       0x0A
#define UBX_CLASS_AID       0x0B
#define UBX_CLASS_TIM       0x0D
#define UBX_CLASS_ESF       0x10
#define UBX_CLASS_MGA       0x13
#define UBX_CLASS_LOG       0x21
#define UBX_CLASS_SEC       0x27
#define UBX_CLASS_HNR       0x28
/* --- Message ID --- */
/* UBX-NAV */
#define UBX_NAV_ATT         0x05
#define UBX_NAV_CLOCK       0x22
#define UBX_NAV_DGPS        0x31
#define UBX_NAV_DOP         0x04
#define UBX_NAV_EOE         0x61
#define UBX_NAV_GEOFENCE    0x39
#define UBX_NAV_ODO         0x09
#define UBX_NAV_ORB         0x34
#define UBX_NAV_POSECEF     0x01
#define UBX_NAV_POSLLH      0x02
#define UBX_NAV_PVT         0x07
#define UBX_NAV_RESETODO    0x10
#define UBX_NAV_SAT         0x35
#define UBX_NAV_SBAS        0x32
#define UBX_NAV_SOL         0x06
#define UBX_NAV_STATUS      0x03
#define UBX_NAV_SVINFO      0x30
#define UBX_NAV_TIMEBDS     0x24
#define UBX_NAV_TIMEGAL     0x25
#define UBX_NAV_TIMEGLO     0x23
#define UBX_NAV_TIMEGPS     0x20
#define UBX_NAV_TIMELS      0x26
#define UBX_NAV_TIMEUTC     0x21
#define UBX_NAV_VELECEF     0x11
#define UBX_NAV_VELNED      0x12
/* UBX-MON */
#define UBX_MON_GNSS        0x28
#define UBX_MON_HW2         0x0B
#define UBX_MON_HW          0x09
#define UBX_MON_VER         0x04

struct max_m8_gps_t;

typedef struct max_m8_gps_t {
  uint8_t address;
} max_m8_gps_t;

typedef struct max_m8_gps_conf_t {
  uint8_t address;
} max_m8_gps_conf_t;

extern uint32_t max_m8_configure(max_m8_gps_t *p_gps, const max_m8_gps_conf_t gps_conf);
extern uint32_t max_m8_init(max_m8_gps_t *p_gps);

#endif