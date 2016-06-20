/**
 * 370GT
 * @author Kevin Tjiam <kevin@tjiam.com>
 */

#ifndef DEBUG_TRACE_H__
#define DEBUG_TRACE_H__

#include <Arduino.h>

//#define trace_print(...)
#define trace_print(...) SerialUSB.print(__VA_ARGS__)

#endif
