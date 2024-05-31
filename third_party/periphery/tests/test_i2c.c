/*
 * c-periphery
 * https://github.com/vsergeev/c-periphery
 * License: MIT
 */

#include "test.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "../src/i2c.h"

const char *i2c_bus_path;

void test_arguments(void) {
    ptest();

    /* No real argument validation needed in the i2c wrapper */
}

void test_open_config_close(void) {
    i2c_t *i2c;

    ptest();

    /* Allocate I2C */
    i2c = i2c_new();
    passert(i2c != NULL);

    /* Open invalid i2c bus */
    passert(i2c_open(i2c, "/foo/bar") == I2C_ERROR_OPEN);

    /* Open legitimate i2c bus */
    passert(i2c_open(i2c, i2c_bus_path) == 0);
    passert(i2c_close(i2c) == 0);

    /* Free I2C */
    i2c_free(i2c);
}

void test_loopback(void) {
    ptest();

    printf("No general way to do a loopback test for I2C without a real component, skipping...\n");
}

bool getc_yes(void) {
    char buf[4];
    fgets(buf, sizeof(buf), stdin);
    return (buf[0] == 'y' || buf[0] == 'Y');
}

void test_interactive(void) {
    char str[256];
    i2c_t *i2c;
    uint8_t msg1[] = { 0xaa, 0xbb, 0xcc, 0xdd };
    struct i2c_msg msgs[1];

    ptest();

    /* Allocate I2C */
    i2c = i2c_new();
    passert(i2c != NULL);

    passert(i2c_open(i2c, i2c_bus_path) == 0);

    printf("Starting interactive test. Get out your logic analyzer, buddy!\n");
    printf("Press enter to continue...\n");
    getc(stdin);

    /* Check tostring */
    passert(i2c_tostring(i2c, str, sizeof(str)) > 0);
    printf("I2C description: %s\n", str);
    printf("I2C description looks OK? y/n\n");
    passert(getc_yes());

    /* There isn't much we can do without assuming a device on the other end,
     * because I2C needs an acknowledgement bit on each transferred byte.
     *
     * But we can send a transaction and expect it to time out. */

    /* S [ 0x7a W ] [0xaa] [0xbb] [0xcc] [0xdd] */
    msgs[0].addr = 0x7a;
    msgs[0].flags = 0; /* Write */
    msgs[0].len = sizeof(msg1);
    msgs[0].buf = msg1;

    printf("Press enter to start transfer...");
    getc(stdin);
    passert(i2c_transfer(i2c, msgs, 1) < 0);
    passert(i2c_errno(i2c) == EREMOTEIO);
    passert(i2c_close(i2c) == 0);
    printf("I2C transfer occurred? y/n\n");
    passert(getc_yes());

    /* Free I2C */
    i2c_free(i2c);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <I2C device>\n\n", argv[0]);
        fprintf(stderr, "[1/4] Arguments test: No requirements.\n");
        fprintf(stderr, "[2/4] Open/close test: I2C device should be real.\n");
        fprintf(stderr, "[3/4] Loopback test: No test.\n");
        fprintf(stderr, "[4/4] Interactive test: I2C bus should be observed with an oscilloscope or logic analyzer.\n\n");
        fprintf(stderr, "Hint: for Raspberry Pi 3, enable I2C1 with:\n");
        fprintf(stderr, "   $ echo \"dtparam=i2c_arm=on\" | sudo tee -a /boot/config.txt\n");
        fprintf(stderr, "   $ sudo reboot\n");
        fprintf(stderr, "Use pins I2C1 SDA (header pin 2) and I2C1 SCL (header pin 3),\n");
        fprintf(stderr, "and run this test with:\n");
        fprintf(stderr, "    %s /dev/i2c-1\n\n", argv[0]);
        exit(1);
    }

    i2c_bus_path = argv[1];

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

