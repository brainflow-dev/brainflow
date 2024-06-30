/*
 * c-periphery
 * https://github.com/vsergeev/c-periphery
 * License: MIT
 */

#include "test.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include "../src/pwm.h"

unsigned int chip;
unsigned int channel;

void test_arguments(void) {
    ptest();

    /* No real argument validation needed in the PWM wrapper */
}

static double fabs(double x) {
    return (x < 0) ? -x : x;
}

void test_open_config_close(void) {
    pwm_t *pwm;
    uint64_t period_ns;
    uint64_t duty_cycle_ns;
    double period;
    double frequency;
    double duty_cycle;
    pwm_polarity_t polarity;
    bool enabled;

    ptest();

    /* Allocate PWM */
    pwm = pwm_new();
    passert(pwm != NULL);

    /* Open non-existent PWM chip */
    passert(pwm_open(pwm, 9999, channel) == PWM_ERROR_OPEN);

    /* Open non-existent PWM channel */
    passert(pwm_open(pwm, chip, 9999) == PWM_ERROR_OPEN);

    /* Open legitimate PWM chip/channel */
    passert(pwm_open(pwm, chip, channel) == 0);

    /* Check properties */
    passert(pwm_chip(pwm) == chip);
    passert(pwm_channel(pwm) == channel);

    /* Initialize period and duty cycle */
    passert(pwm_set_period(pwm, 5e-3) == 0);
    passert(pwm_set_duty_cycle(pwm, 0) == 0);

    /* Set period, check period, check period_ns, check frequency */
    passert(pwm_set_period(pwm, 1e-3) == 0);
    passert(pwm_get_period(pwm, &period) == 0);
    passert(fabs(period - 1e-3) < 1e-4);
    passert(pwm_get_period_ns(pwm, &period_ns) == 0);
    passert(fabs(period_ns - 1000000) < 1e5);
    passert(pwm_get_frequency(pwm, &frequency) == 0);
    passert(fabs(frequency - 1000) < 100);

    passert(pwm_set_period(pwm, 5e-4) == 0);
    passert(pwm_get_period(pwm, &period) == 0);
    passert(fabs(period - 5e-4) < 1e-5);
    passert(pwm_get_period_ns(pwm, &period_ns) == 0);
    passert(fabs(period_ns - 500000) < 1e4);
    passert(pwm_get_frequency(pwm, &frequency) == 0);
    passert(fabs(frequency - 2000) < 100);

    /* Set frequency, check frequency, check period, check period_ns */
    passert(pwm_set_frequency(pwm, 1000) == 0);
    passert(pwm_get_frequency(pwm, &frequency) == 0);
    passert(fabs(frequency - 1000) < 100);
    passert(pwm_get_period(pwm, &period) == 0);
    passert(fabs(period - 1e-3) < 1e-4);
    passert(pwm_get_period_ns(pwm, &period_ns) == 0);
    passert(fabs(period_ns - 1000000) < 1e5);

    passert(pwm_set_frequency(pwm, 2000) == 0);
    passert(pwm_get_frequency(pwm, &frequency) == 0);
    passert(fabs(frequency - 2000) < 100);
    passert(pwm_get_period(pwm, &period) == 0);
    passert(fabs(period - 5e-4) < 1e-5);
    passert(pwm_get_period_ns(pwm, &period_ns) == 0);
    passert(fabs(period_ns - 500000) < 1e4);

    /* Set period_ns, check period_ns, check period, check frequency */
    passert(pwm_set_period_ns(pwm, 1000000) == 0);
    passert(pwm_get_period_ns(pwm, &period_ns) == 0);
    passert(fabs(period_ns - 1000000) < 1e5);
    passert(pwm_get_period(pwm, &period) == 0);
    passert(fabs(period - 1e-3) < 1e-4);
    passert(pwm_get_frequency(pwm, &frequency) == 0);
    passert(fabs(frequency - 1000) < 100);

    passert(pwm_set_period_ns(pwm, 500000) == 0);
    passert(pwm_get_period_ns(pwm, &period_ns) == 0);
    passert(fabs(period_ns - 500000) < 1e4);
    passert(pwm_get_period(pwm, &period) == 0);
    passert(fabs(period - 5e-4) < 1e-5);
    passert(pwm_get_frequency(pwm, &frequency) == 0);
    passert(fabs(frequency - 2000) < 100);

    passert(pwm_set_period_ns(pwm, 1000000) == 0);

    /* Set duty cycle, check duty cycle, check duty_cycle_ns */
    passert(pwm_set_duty_cycle(pwm, 0.25) == 0);
    passert(pwm_get_duty_cycle(pwm, &duty_cycle) == 0);
    passert(fabs(duty_cycle - 0.25) < 1e-3);
    passert(pwm_get_duty_cycle_ns(pwm, &duty_cycle_ns) == 0);
    passert(fabs(duty_cycle_ns - 250000) < 1e4);

    passert(pwm_set_duty_cycle(pwm, 0.50) == 0);
    passert(pwm_get_duty_cycle(pwm, &duty_cycle) == 0);
    passert(fabs(duty_cycle - 0.50) < 1e-3);
    passert(pwm_get_duty_cycle_ns(pwm, &duty_cycle_ns) == 0);
    passert(fabs(duty_cycle_ns - 500000) < 1e4);

    passert(pwm_set_duty_cycle(pwm, 0.75) == 0);
    passert(pwm_get_duty_cycle(pwm, &duty_cycle) == 0);
    passert(fabs(duty_cycle - 0.75) < 1e-3);
    passert(pwm_get_duty_cycle_ns(pwm, &duty_cycle_ns) == 0);
    passert(fabs(duty_cycle_ns - 750000) < 1e4);

    /* Set duty_cycle_ns, check duty_cycle_ns, check duty_cycle */
    passert(pwm_set_duty_cycle_ns(pwm, 250000) == 0);
    passert(pwm_get_duty_cycle_ns(pwm, &duty_cycle_ns) == 0);
    passert(fabs(duty_cycle_ns - 250000) < 1e4);
    passert(pwm_get_duty_cycle(pwm, &duty_cycle) == 0);
    passert(fabs(duty_cycle - 0.25) < 1e-3);

    passert(pwm_set_duty_cycle_ns(pwm, 500000) == 0);
    passert(pwm_get_duty_cycle_ns(pwm, &duty_cycle_ns) == 0);
    passert(fabs(duty_cycle_ns - 500000) < 1e4);
    passert(pwm_get_duty_cycle(pwm, &duty_cycle) == 0);
    passert(fabs(duty_cycle - 0.50) < 1e-3);

    passert(pwm_set_duty_cycle_ns(pwm, 750000) == 0);
    passert(pwm_get_duty_cycle_ns(pwm, &duty_cycle_ns) == 0);
    passert(fabs(duty_cycle_ns - 750000) < 1e4);
    passert(pwm_get_duty_cycle(pwm, &duty_cycle) == 0);
    passert(fabs(duty_cycle - 0.75) < 1e-3);

    /* Set polarity, check polarity */
    passert(pwm_set_polarity(pwm, PWM_POLARITY_NORMAL) == 0);
    passert(pwm_get_polarity(pwm, &polarity) == 0);
    passert(polarity == PWM_POLARITY_NORMAL);

    passert(pwm_set_polarity(pwm, PWM_POLARITY_INVERSED) == 0);
    passert(pwm_get_polarity(pwm, &polarity) == 0);
    passert(polarity == PWM_POLARITY_INVERSED);

    /* Set enabled, check enabled */
    passert(pwm_set_enabled(pwm, true) == 0);
    passert(pwm_get_enabled(pwm, &enabled) == 0);
    passert(enabled == true);

    passert(pwm_set_enabled(pwm, false) == 0);
    passert(pwm_get_enabled(pwm, &enabled) == 0);
    passert(enabled == false);

    /* Use pwm_enable()/pwm_disable(), check enabled */
    passert(pwm_enable(pwm) == 0);
    passert(pwm_get_enabled(pwm, &enabled) == 0);
    passert(enabled == true);

    passert(pwm_disable(pwm) == 0);
    passert(pwm_get_enabled(pwm, &enabled) == 0);
    passert(enabled == false);

    /* Set invalid polarity */
    passert(pwm_set_polarity(pwm, 123) == PWM_ERROR_ARG);

    passert(pwm_close(pwm) == 0);

    /* Free PWM */
    pwm_free(pwm);
}

void test_loopback(void) {
    ptest();
}

bool getc_yes(void) {
    char buf[4];
    fgets(buf, sizeof(buf), stdin);
    return (buf[0] == 'y' || buf[0] == 'Y');
}

void test_interactive(void) {
    char str[256];
    pwm_t *pwm;

    ptest();

    /* Allocate PWM */
    pwm = pwm_new();
    passert(pwm != NULL);

    passert(pwm_open(pwm, chip, channel) == 0);

    printf("Starting interactive test. Get out your oscilloscope, buddy!\n");
    printf("Press enter to continue...\n");
    getc(stdin);

    /* Set initial parameters and enable PWM */
    passert(pwm_set_duty_cycle(pwm, 0.0) == 0);
    passert(pwm_set_frequency(pwm, 1e3) == 0);
    passert(pwm_set_polarity(pwm, PWM_POLARITY_NORMAL) == 0);
    passert(pwm_enable(pwm) == 0);

    /* Check tostring */
    passert(pwm_tostring(pwm, str, sizeof(str)) > 0);
    printf("PWM description: %s\n", str);
    printf("PWM description looks OK? y/n\n");
    passert(getc_yes());

    /* Set 1 kHz frequency, 0.25 duty cycle */
    passert(pwm_set_frequency(pwm, 1e3) == 0);
    passert(pwm_set_duty_cycle(pwm, 0.25) == 0);
    printf("Frequency is 1 kHz, duty cycle is 25%%? y/n\n");
    passert(getc_yes());

    /* Set 1 kHz frequency, 0.50 duty cycle */
    passert(pwm_set_frequency(pwm, 1e3) == 0);
    passert(pwm_set_duty_cycle(pwm, 0.50) == 0);
    printf("Frequency is 1 kHz, duty cycle is 50%%? y/n\n");
    passert(getc_yes());

    /* Set 2 kHz frequency, 0.25 duty cycle */
    passert(pwm_set_frequency(pwm, 2e3) == 0);
    passert(pwm_set_duty_cycle(pwm, 0.25) == 0);
    printf("Frequency is 2 kHz, duty cycle is 25%%? y/n\n");
    passert(getc_yes());

    /* Set 2 kHz frequency, 0.50 duty cycle */
    passert(pwm_set_frequency(pwm, 2e3) == 0);
    passert(pwm_set_duty_cycle(pwm, 0.50) == 0);
    printf("Frequency is 2 kHz, duty cycle is 50%%? y/n\n");
    passert(getc_yes());

    passert(pwm_set_duty_cycle(pwm, 0.0) == 0);
    passert(pwm_disable(pwm) == 0);

    passert(pwm_close(pwm) == 0);

    /* Free PWM */
    pwm_free(pwm);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <PWM chip> <PWM channel>\n\n", argv[0]);
        fprintf(stderr, "[1/4] Arguments test: No requirements.\n");
        fprintf(stderr, "[2/4] Open/close test: PWM channel should be real.\n");
        fprintf(stderr, "[3/4] Loopback test: No test.\n");
        fprintf(stderr, "[4/4] Interactive test: PWM channel should be observed with an oscilloscope or logic analyzer.\n\n");
        fprintf(stderr, "Hint: for Raspberry Pi 3, enable PWM0 and PWM1 with:\n");
        fprintf(stderr, "   $ echo \"dtoverlay=pwm-2chan,pin=18,func=2,pin2=13,func2=4\" | sudo tee -a /boot/config.txt\n");
        fprintf(stderr, "   $ sudo reboot\n");
        fprintf(stderr, "Monitor GPIO 18 (header pin 12), and run this test with:\n");
        fprintf(stderr, "    %s 0 0\n", argv[0]);
        fprintf(stderr, "or, monitor GPIO 13 (header pin 33), and run this test with:\n");
        fprintf(stderr, "    %s 0 1\n\n", argv[0]);
        exit(1);
    }

    chip = strtoul(argv[1], NULL, 10);
    channel = strtoul(argv[2], NULL, 10);

    test_arguments();
    printf(" " STR_OK "  Arguments test passed.\n\n");
    test_open_config_close();
    printf(" " STR_OK "  Open/close test passed.\n\n");
    test_loopback();
    printf(" " STR_OK "  Loopback test passed.\n\n");
    test_interactive();
    printf(" " STR_OK "  Interactive test passed.\n\n");

    printf("All tests passed!\n");
    return 0;
}

