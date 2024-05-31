/*
 * c-periphery
 * https://github.com/vsergeev/c-periphery
 * License: MIT
 */

#ifndef _PERIPHERY_LED_H
#define _PERIPHERY_LED_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

enum led_error_code {
    LED_ERROR_ARG       = -1, /* Invalid arguments */
    LED_ERROR_OPEN      = -2, /* Opening LED */
    LED_ERROR_QUERY     = -3, /* Querying LED attributes */
    LED_ERROR_IO        = -4, /* Reading/writing LED brightness */
    LED_ERROR_CLOSE     = -5, /* Closing LED */
};

typedef struct led_handle led_t;

/* Primary Functions */
led_t *led_new(void);
int led_open(led_t *led, const char *name);
int led_read(led_t *led, bool *value);
int led_write(led_t *led, bool value);
int led_close(led_t *led);
void led_free(led_t *led);

/* Getters */
int led_get_brightness(led_t *led, unsigned int *brightness);
int led_get_max_brightness(led_t *led, unsigned int *max_brightness);

/* Setters */
int led_set_brightness(led_t *led, unsigned int brightness);

/* Miscellaneous */
int led_name(led_t *led, char *str, size_t len);
int led_tostring(led_t *led, char *str, size_t len);

/* Error Handling */
int led_errno(led_t *led);
const char *led_errmsg(led_t *led);

#ifdef __cplusplus
}
#endif

#endif

