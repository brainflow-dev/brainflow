/*
 * c-periphery
 * https://github.com/vsergeev/c-periphery
 * License: MIT
 */

#include "test.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/wait.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#include "../src/gpio.h"

const char *device;
unsigned int pin_input, pin_output;

void test_arguments(void) {
    gpio_t *gpio;

    ptest();

    /* Allocate GPIO */
    gpio = gpio_new();
    passert(gpio != NULL);

    /* Invalid direction */
    passert(gpio_open(gpio, device, pin_input, 5) == GPIO_ERROR_ARG);

    /* Free GPIO */
    gpio_free(gpio);
}

void test_open_config_close(void) {
    gpio_t *gpio;
    bool value;
    gpio_direction_t direction;
    gpio_edge_t edge;
    char label[32];
    gpio_bias_t bias;
    gpio_drive_t drive;
    bool inverted;

    ptest();

    /* Allocate GPIO */
    gpio = gpio_new();
    passert(gpio != NULL);

    /* Open non-existent GPIO */
    passert(gpio_open(gpio, device, -1, GPIO_DIR_IN) == GPIO_ERROR_OPEN);
    passert(gpio_errno(gpio) == EINVAL);

    /* Open legitimate GPIO */
    passert(gpio_open(gpio, device, pin_output, GPIO_DIR_IN) == 0);

    /* Check properties */
    passert(gpio_line(gpio) == pin_output);
    passert(gpio_fd(gpio) >= 0);
    passert(gpio_chip_fd(gpio) >= 0);

    /* Check default label */
    passert(gpio_label(gpio, label, sizeof(label)) == 0);
    passert(strncmp(label, "periphery", sizeof(label)) == 0);

    /* Invalid direction */
    passert(gpio_set_direction(gpio, 5) == GPIO_ERROR_ARG);
    /* Invalid interrupt edge */
    passert(gpio_set_edge(gpio, 5) == GPIO_ERROR_ARG);
    /* Invalid bias */
    passert(gpio_set_bias(gpio, 5) == GPIO_ERROR_ARG);
    /* Invalid drive */
    passert(gpio_set_drive(gpio, 5) == GPIO_ERROR_ARG);

    /* Set direction out, check direction out, check value low */
    passert(gpio_set_direction(gpio, GPIO_DIR_OUT) == 0);
    passert(gpio_get_direction(gpio, &direction) == 0);
    passert(direction == GPIO_DIR_OUT);
    passert(gpio_read(gpio, &value) == 0);
    passert(value == false);
    /* Set direction out low, check direction out, check value low */
    passert(gpio_set_direction(gpio, GPIO_DIR_OUT_LOW) == 0);
    passert(gpio_get_direction(gpio, &direction) == 0);
    passert(direction == GPIO_DIR_OUT);
    passert(gpio_read(gpio, &value) == 0);
    passert(value == false);
    /* Set direction out high, check direction out, check value high */
    passert(gpio_set_direction(gpio, GPIO_DIR_OUT_HIGH) == 0);
    passert(gpio_get_direction(gpio, &direction) == 0);
    passert(direction == GPIO_DIR_OUT);
    passert(gpio_read(gpio, &value) == 0);
    passert(value == true);

    /* Set drive open drain, check drive open drain */
    passert(gpio_set_drive(gpio, GPIO_DRIVE_OPEN_DRAIN) == 0);
    passert(gpio_get_drive(gpio, &drive) == 0);
    passert(drive == GPIO_DRIVE_OPEN_DRAIN);
    /* Set drive open source, check drive open source */
    passert(gpio_set_drive(gpio, GPIO_DRIVE_OPEN_SOURCE) == 0);
    passert(gpio_get_drive(gpio, &drive) == 0);
    passert(drive == GPIO_DRIVE_OPEN_SOURCE);
    /* Set drive default, check drive default */
    passert(gpio_set_drive(gpio, GPIO_DRIVE_DEFAULT) == 0);
    passert(gpio_get_drive(gpio, &drive) == 0);
    passert(drive == GPIO_DRIVE_DEFAULT);

    /* Set inverted true, check inverted true */
    passert(gpio_set_inverted(gpio, true) == 0);
    passert(gpio_get_inverted(gpio, &inverted) == 0);
    passert(inverted == true);
    /* Set inverted false, check inverted false */
    passert(gpio_set_inverted(gpio, false) == 0);
    passert(gpio_get_inverted(gpio, &inverted) == 0);
    passert(inverted == false);

    /* Attempt to set interrupt edge on output GPIO */
    passert(gpio_set_edge(gpio, GPIO_EDGE_RISING) == GPIO_ERROR_INVALID_OPERATION);
    /* Attempt to read event on output GPIO */
    passert(gpio_read_event(gpio, &edge, NULL) == GPIO_ERROR_INVALID_OPERATION);

    /* Set direction in, check direction in */
    passert(gpio_set_direction(gpio, GPIO_DIR_IN) == 0);
    passert(gpio_get_direction(gpio, &direction) == 0);
    passert(direction == GPIO_DIR_IN);
    passert(gpio_read(gpio, &value) == 0);

    /* Set edge none, check edge none */
    passert(gpio_set_edge(gpio, GPIO_EDGE_NONE) == 0);
    passert(gpio_get_edge(gpio, &edge) == 0);
    passert(edge == GPIO_EDGE_NONE);
    /* Set edge rising, check edge rising */
    passert(gpio_set_edge(gpio, GPIO_EDGE_RISING) == 0);
    passert(gpio_get_edge(gpio, &edge) == 0);
    passert(edge == GPIO_EDGE_RISING);
    /* Set edge falling, check edge falling */
    passert(gpio_set_edge(gpio, GPIO_EDGE_FALLING) == 0);
    passert(gpio_get_edge(gpio, &edge) == 0);
    passert(edge == GPIO_EDGE_FALLING);
    /* Set edge both, check edge both */
    passert(gpio_set_edge(gpio, GPIO_EDGE_BOTH) == 0);
    passert(gpio_get_edge(gpio, &edge) == 0);
    passert(edge == GPIO_EDGE_BOTH);
    /* Set edge none, check edge none */
    passert(gpio_set_edge(gpio, GPIO_EDGE_NONE) == 0);
    passert(gpio_get_edge(gpio, &edge) == 0);
    passert(edge == GPIO_EDGE_NONE);

    /* Set bias pull up, check bias pull up */
    passert(gpio_set_bias(gpio, GPIO_BIAS_PULL_UP) == 0);
    passert(gpio_get_bias(gpio, &bias) == 0);
    passert(bias == GPIO_BIAS_PULL_UP);
    /* Set bias pull down, check bias pull down */
    passert(gpio_set_bias(gpio, GPIO_BIAS_PULL_DOWN) == 0);
    passert(gpio_get_bias(gpio, &bias) == 0);
    passert(bias == GPIO_BIAS_PULL_DOWN);
    /* Set bias disable, check bias disable */
    passert(gpio_set_bias(gpio, GPIO_BIAS_DISABLE) == 0);
    passert(gpio_get_bias(gpio, &bias) == 0);
    passert(bias == GPIO_BIAS_DISABLE);
    /* Set bias default, check bias default */
    passert(gpio_set_bias(gpio, GPIO_BIAS_DEFAULT) == 0);
    passert(gpio_get_bias(gpio, &bias) == 0);
    passert(bias == GPIO_BIAS_DEFAULT);

    /* Attempt to set drive on input GPIO */
    passert(gpio_set_drive(gpio, GPIO_DRIVE_OPEN_DRAIN) == GPIO_ERROR_INVALID_OPERATION);

    /* Close GPIO */
    passert(gpio_close(gpio) == 0);

    /* Open GPIO with advanced open */
    gpio_config_t config = {
        .direction = GPIO_DIR_IN,
        .edge = GPIO_EDGE_RISING,
        .bias = GPIO_BIAS_DEFAULT,
        .drive = GPIO_DRIVE_DEFAULT,
        .inverted = false,
        .label = "test123",
    };
    passert(gpio_open_advanced(gpio, device, pin_input, &config) == 0);

    /* Check properties */
    passert(gpio_line(gpio) == pin_input);
    passert(gpio_fd(gpio) >= 0);
    passert(gpio_chip_fd(gpio) >= 0);
    /* Check direction */
    passert(gpio_get_direction(gpio, &direction) == 0);
    passert(direction == GPIO_DIR_IN);
    /* Check edge */
    passert(gpio_get_edge(gpio, &edge) == 0);
    passert(edge == GPIO_EDGE_RISING);
    /* Check bias */
    passert(gpio_get_bias(gpio, &bias) == 0);
    passert(bias == GPIO_BIAS_DEFAULT);
    /* Check drive */
    passert(gpio_get_drive(gpio, &drive) == 0);
    passert(drive == GPIO_DRIVE_DEFAULT);
    /* Check inverted */
    passert(gpio_get_inverted(gpio, &inverted) == 0);
    passert(inverted == false);
    /* Check label */
    passert(gpio_label(gpio, label, sizeof(label)) == 0);
    passert(strncmp(label, "test123", sizeof(label)) == 0);

    /* Close GPIO */
    passert(gpio_close(gpio) == 0);

    /* Free GPIO */
    gpio_free(gpio);
}

/* Threaded poll helper functions */

typedef struct {
    sem_t *sem;
    gpio_t *gpio;
    int timeout_ms;
} gpio_poll_args_t;

void *gpio_poll_thread(void *arg) {
    gpio_poll_args_t *args = (gpio_poll_args_t *)arg;
    gpio_t *gpio = args->gpio;
    int timeout_ms = args->timeout_ms;

    assert(sem_post(args->sem) == 0);

    intptr_t ret = gpio_poll(gpio, timeout_ms);

    return (void *)ret;
}

void gpio_poll_start(pthread_t *thread, gpio_t *gpio, int timeout_ms) {
    sem_t sem;
    gpio_poll_args_t args = {.sem = &sem, .gpio = gpio, .timeout_ms = timeout_ms};

    assert(sem_init(&sem, 0, 0) == 0);
    assert(pthread_create(thread, NULL, &gpio_poll_thread, &args) == 0);
    assert(sem_wait(&sem) == 0);
}

int gpio_poll_join(pthread_t thread) {
    void *ret;

    assert(pthread_join(thread, &ret) == 0);

    return (intptr_t)ret;
}

void test_loopback(void) {
    gpio_t *gpio_in, *gpio_out;
    pthread_t poll_thread;
    bool value;
    gpio_edge_t edge;

    ptest();

    /* Allocate GPIO */
    gpio_in = gpio_new();
    passert(gpio_in != NULL);
    gpio_out = gpio_new();
    passert(gpio_out != NULL);

    /* Open in and out pins */
    passert(gpio_open(gpio_in, device, pin_input, GPIO_DIR_IN) == 0);
    passert(gpio_open(gpio_out, device, pin_output, GPIO_DIR_OUT) == 0);

    /* Drive out low, check in low */
    passert(gpio_write(gpio_out, false) == 0);
    passert(gpio_read(gpio_in, &value) == 0);
    passert(value == false);

    /* Drive out high, check in high */
    passert(gpio_write(gpio_out, true) == 0);
    passert(gpio_read(gpio_in, &value) == 0);
    passert(value == true);

    /* Check poll falling 1 -> 0 interrupt */
    passert(gpio_set_edge(gpio_in, GPIO_EDGE_FALLING) == 0);
    gpio_poll_start(&poll_thread, gpio_in, 1000);
    passert(gpio_write(gpio_out, false) == 0);
    passert(gpio_poll_join(poll_thread) == 1);
    passert(gpio_read(gpio_in, &value) == 0);
    passert(value == false);
    passert(gpio_read_event(gpio_in, &edge, NULL) == 0);
    passert(edge == GPIO_EDGE_FALLING);

    /* Check poll rising 0 -> 1 interrupt */
    passert(gpio_set_edge(gpio_in, GPIO_EDGE_RISING) == 0);
    gpio_poll_start(&poll_thread, gpio_in, 1000);
    passert(gpio_write(gpio_out, true) == 0);
    passert(gpio_poll_join(poll_thread) == 1);
    passert(gpio_read(gpio_in, &value) == 0);
    passert(value == true);
    passert(gpio_read_event(gpio_in, &edge, NULL) == 0);
    passert(edge == GPIO_EDGE_RISING);

    /* Set both edge */
    passert(gpio_set_edge(gpio_in, GPIO_EDGE_BOTH) == 0);

    /* Check poll falling 1 -> 0 interrupt */
    gpio_poll_start(&poll_thread, gpio_in, 1000);
    passert(gpio_write(gpio_out, false) == 0);
    passert(gpio_poll_join(poll_thread) == 1);
    passert(gpio_read(gpio_in, &value) == 0);
    passert(value == false);
    passert(gpio_read_event(gpio_in, &edge, NULL) == 0);
    passert(edge == GPIO_EDGE_FALLING);

    /* Check poll rising 0 -> 1 interrupt */
    gpio_poll_start(&poll_thread, gpio_in, 1000);
    passert(gpio_write(gpio_out, true) == 0);
    passert(gpio_poll_join(poll_thread) == 1);
    passert(gpio_read(gpio_in, &value) == 0);
    passert(value == true);
    passert(gpio_read_event(gpio_in, &edge, NULL) == 0);
    passert(edge == GPIO_EDGE_RISING);

    /* Check poll timeout */
    passert(gpio_poll(gpio_in, 1000) == 0);

    /* Test gpio_poll_multiple() API with one GPIO */
    gpio_t *gpios[1] = {gpio_in};
    bool gpios_ready[1] = {false};

    /* Check poll falling 1 -> 0 interrupt */
    passert(gpio_write(gpio_out, false) == 0);
    passert(gpio_poll_multiple(gpios, 1, 1000, gpios_ready) == 1);
    passert(gpios_ready[0] == true);
    passert(gpio_read(gpio_in, &value) == 0);
    passert(value == false);
    passert(gpio_read_event(gpio_in, &edge, NULL) == 0);
    passert(edge == GPIO_EDGE_FALLING);

    /* Check poll rising 0 -> 1 interrupt */
    passert(gpio_write(gpio_out, true) == 0);
    passert(gpio_poll_multiple(gpios, 1, 1000, gpios_ready) == 1);
    passert(gpios_ready[0] == true);
    passert(gpio_read(gpio_in, &value) == 0);
    passert(value == true);
    passert(gpio_read_event(gpio_in, &edge, NULL) == 0);
    passert(edge == GPIO_EDGE_RISING);

    /* Check poll timeout */
    passert(gpio_poll_multiple(gpios, 1, 1000, gpios_ready) == 0);
    passert(gpios_ready[0] == false);

    passert(gpio_close(gpio_in) == 0);
    passert(gpio_close(gpio_out) == 0);

    /* Open both GPIOs as inputs */
    passert(gpio_open(gpio_in, device, pin_input, GPIO_DIR_IN) == 0);
    passert(gpio_open(gpio_out, device, pin_output, GPIO_DIR_IN) == 0);

    /* Set bias pull-up, check value is high */
    passert(gpio_set_bias(gpio_in, GPIO_BIAS_PULL_UP) == 0);
    usleep(1000);
    passert(gpio_read(gpio_in, &value) == 0);
    passert(value == true);

    /* Set bias pull-down, check value is low */
    passert(gpio_set_bias(gpio_in, GPIO_BIAS_PULL_DOWN) == 0);
    usleep(1000);
    passert(gpio_read(gpio_in, &value) == 0);
    passert(value == false);

    passert(gpio_close(gpio_in) == 0);
    passert(gpio_close(gpio_out) == 0);

    /* Free GPIO */
    gpio_free(gpio_in);
    gpio_free(gpio_out);
}

bool getc_yes(void) {
    char buf[4];
    fgets(buf, sizeof(buf), stdin);
    return (buf[0] == 'y' || buf[0] == 'Y');
}

void test_interactive(void) {
    char str[256];
    gpio_t *gpio;

    ptest();

    /* Allocate GPIO */
    gpio = gpio_new();
    passert(gpio != NULL);

    passert(gpio_open(gpio, device, pin_output, GPIO_DIR_OUT) == 0);

    printf("Starting interactive test. Get out your multimeter, buddy!\n");
    printf("Press enter to continue...\n");
    getc(stdin);

    /* Check tostring */
    passert(gpio_tostring(gpio, str, sizeof(str)) > 0);
    printf("GPIO description: %s\n", str);
    printf("GPIO description looks OK? y/n\n");
    passert(getc_yes());

    /* Drive GPIO out low */
    passert(gpio_write(gpio, false) == 0);
    printf("GPIO out is low? y/n\n");
    passert(getc_yes());

    /* Drive GPIO out high */
    passert(gpio_write(gpio, true) == 0);
    printf("GPIO out is high? y/n\n");
    passert(getc_yes());

    /* Drive GPIO out low */
    passert(gpio_write(gpio, false) == 0);
    printf("GPIO out is low? y/n\n");
    passert(getc_yes());

    passert(gpio_close(gpio) == 0);

    /* Free GPIO */
    gpio_free(gpio);
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <GPIO chip device> <GPIO #1> <GPIO #2>\n\n", argv[0]);
        fprintf(stderr, "[1/4] Argument test: No requirements.\n");
        fprintf(stderr, "[2/4] Open/close test: GPIO #2 should be real.\n");
        fprintf(stderr, "[3/4] Loopback test: GPIOs #1 and #2 should be connected with a wire.\n");
        fprintf(stderr, "[4/4] Interactive test: GPIO #2 should be observed with a multimeter.\n\n");
        fprintf(stderr, "Hint: for Raspberry Pi 3,\n");
        fprintf(stderr, "Use GPIO 17 (header pin 11) and GPIO 27 (header pin 13),\n");
        fprintf(stderr, "connect a loopback between them, and run this test with:\n");
        fprintf(stderr, "    %s /dev/gpiochip0 17 27\n\n", argv[0]);
        exit(1);
    }

    device = argv[1];
    pin_input = strtoul(argv[2], NULL, 10);
    pin_output = strtoul(argv[3], NULL, 10);

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

