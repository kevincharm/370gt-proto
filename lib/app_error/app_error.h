/**
 * 370GT
 * @author Kevin Tjiam <kevin@tjiam.com>
 */

#ifndef APP_ERROR_H__
#define APP_ERROR_H__

#include <stdint.h>

/* App return values */
#define RESPONSE_OK                 0
#define RESPONSE_ERROR              1
#define RESPONSE_TIMEOUT            2

void app_err_check(uint32_t err_code);

#endif
