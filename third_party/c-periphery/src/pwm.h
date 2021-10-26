/*
 * c-periphery
 * https://github.com/vsergeev/c-periphery
 * License: MIT
 */

#ifndef _PERIPHERY_PWM_H
#define _PERIPHERY_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

enum pwm_error_code {
    PWM_ERROR_ARG           = -1, /* Invalid arguments */
    PWM_ERROR_OPEN          = -2, /* Opening PWM */
    PWM_ERROR_QUERY         = -3, /* Querying PWM attributes */
    PWM_ERROR_CONFIGURE     = -4, /* Configuring PWM attributes */
    PWM_ERROR_CLOSE         = -5, /* Closing PWM */
};

typedef enum pwm_polarity {
    PWM_POLARITY_NORMAL,    /* Normal polarity */
    PWM_POLARITY_INVERSED,  /* Inversed polarity */
} pwm_polarity_t;

typedef struct pwm_handle pwm_t;

/* Primary Functions */
pwm_t *pwm_new(void);
int pwm_open(pwm_t *pwm, unsigned int chip, unsigned int channel);
int pwm_enable(pwm_t *pwm);
int pwm_disable(pwm_t *pwm);
int pwm_close(pwm_t *pwm);
void pwm_free(pwm_t *pwm);

/* Getters */
int pwm_get_enabled(pwm_t *pwm, bool *enabled);
int pwm_get_period_ns(pwm_t *pwm, uint64_t *period_ns);
int pwm_get_duty_cycle_ns(pwm_t *pwm, uint64_t *duty_cycle_ns);
int pwm_get_period(pwm_t *pwm, double *period);
int pwm_get_duty_cycle(pwm_t *pwm, double *duty_cycle);
int pwm_get_frequency(pwm_t *pwm, double *frequency);
int pwm_get_polarity(pwm_t *pwm, pwm_polarity_t *polarity);

/* Setters */
int pwm_set_enabled(pwm_t *pwm, bool enabled);
int pwm_set_period_ns(pwm_t *pwm, uint64_t period_ns);
int pwm_set_duty_cycle_ns(pwm_t *pwm, uint64_t duty_cycle_ns);
int pwm_set_period(pwm_t *pwm, double period);
int pwm_set_duty_cycle(pwm_t *pwm, double duty_cycle);
int pwm_set_frequency(pwm_t *pwm, double frequency);
int pwm_set_polarity(pwm_t *pwm, pwm_polarity_t polarity);

/* Miscellaneous */
unsigned int pwm_chip(pwm_t *pwm);
unsigned int pwm_channel(pwm_t *pwm);
int pwm_tostring(pwm_t *pwm, char *str, size_t len);

/* Error Handling */
int pwm_errno(pwm_t *pwm);
const char *pwm_errmsg(pwm_t *pwm);

#ifdef __cplusplus
}
#endif

#endif

