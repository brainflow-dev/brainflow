/*
 * c-periphery
 * https://github.com/vsergeev/c-periphery
 * License: MIT
 */

#include "test.h"

#include <stdlib.h>
#include <stdbool.h>

#include "../src/spi.h"

const char *device;

void test_arguments(void) {
    spi_t *spi;

    ptest();

    /* Allocate SPI */
    spi = spi_new();
    passert(spi != NULL);

    /* Invalid mode */
    passert(spi_open(spi, device, 4, 1e6) == SPI_ERROR_ARG);
    /* Invalid bit order */
    passert(spi_open_advanced(spi, device, 0, 1e6, LSB_FIRST+1, 8, 0) == SPI_ERROR_ARG);

    /* Free SPI */
    spi_free(spi);
}

void test_open_config_close(void) {
    spi_t *spi;
    unsigned int mode;
    spi_bit_order_t bit_order;
    uint8_t bits_per_word;
    uint32_t max_speed;

    ptest();

    /* Allocate SPI */
    spi = spi_new();
    passert(spi != NULL);

    passert(spi_open(spi, device, 0, 100000) == 0);

    /* Confirm bit_order = MSB first, bits_per_word = 8 */
    passert(spi_get_bit_order(spi, &bit_order) == 0);
    passert(bit_order == MSB_FIRST);
    passert(spi_get_bits_per_word(spi, &bits_per_word) == 0);
    passert(bits_per_word == 8);

    /* Not going to try different bit order or bits per word, because not all
     * SPI controllers support them */

    /* Try modes 1,2,3,0 */
    passert(spi_set_mode(spi, 1) == 0);
    passert(spi_get_mode(spi, &mode) == 0);
    passert(mode == 1);
    passert(spi_set_mode(spi, 2) == 0);
    passert(spi_get_mode(spi, &mode) == 0);
    passert(mode == 2);
    passert(spi_set_mode(spi, 3) == 0);
    passert(spi_get_mode(spi, &mode) == 0);
    passert(mode == 3);
    passert(spi_set_mode(spi, 0) == 0);
    passert(spi_get_mode(spi, &mode) == 0);
    passert(mode == 0);

    /* Try 100KHz, 500KHz, 1MHz */
    passert(spi_set_max_speed(spi, 100000) == 0);
    passert(spi_get_max_speed(spi, &max_speed) == 0);
    passert(max_speed == 100000);
    passert(spi_set_max_speed(spi, 500000) == 0);
    passert(spi_get_max_speed(spi, &max_speed) == 0);
    passert(max_speed == 500000);
    passert(spi_set_max_speed(spi, 1000000) == 0);
    passert(spi_get_max_speed(spi, &max_speed) == 0);
    passert(max_speed == 1000000);

    passert(spi_close(spi) == 0);

    /* Free SPI */
    spi_free(spi);
}

void test_loopback(void) {
    spi_t *spi;
    uint8_t buf[32];
    unsigned int i;

    ptest();

    /* Allocate SPI */
    spi = spi_new();
    passert(spi != NULL);

    passert(spi_open(spi, device, 0, 100000) == 0);

    for (i = 0; i < sizeof(buf); i++)
        buf[i] = i;

    passert(spi_transfer(spi, buf, buf, sizeof(buf)) == 0);

    for (i = 0; i < sizeof(buf); i++)
        passert(buf[i] == i);

    passert(spi_close(spi) == 0);

    /* Free SPI */
    spi_free(spi);
}

bool getc_yes(void) {
    char buf[4];
    fgets(buf, sizeof(buf), stdin);
    return (buf[0] == 'y' || buf[0] == 'Y');
}

void test_interactive(void) {
    char str[256];
    spi_t *spi;
    uint8_t buf[] = { 0x55, 0xaa, 0x0f, 0xf0 };

    ptest();

    /* Allocate SPI */
    spi = spi_new();
    passert(spi != NULL);

    passert(spi_open(spi, device, 0, 100000) == 0);

    printf("Starting interactive test. Get out your logic analyzer, buddy!\n");
    printf("Press enter to continue...\n");
    getc(stdin);

    /* Check tostring */
    passert(spi_tostring(spi, str, sizeof(str)) > 0);
    printf("SPI description: %s\n", str);
    printf("SPI description looks OK? y/n\n");
    passert(getc_yes());

    /* Mode 0 transfer */
    printf("Press enter to start transfer...");
    getc(stdin);
    passert(spi_transfer(spi, buf, NULL, sizeof(buf)) == 0);
    printf("SPI data 0x55, 0xaa, 0x0f, 0xf0\n");
    printf("SPI transfer speed <= 100KHz, mode 0 occurred? y/n\n");
    passert(getc_yes());

    /* Mode 1 transfer */
    passert(spi_set_mode(spi, 1) == 0);
    printf("Press enter to start transfer...");
    getc(stdin);
    passert(spi_transfer(spi, buf, NULL, sizeof(buf)) == 0);
    printf("SPI data 0x55, 0xaa, 0x0f, 0xf0\n");
    printf("SPI transfer speed <= 100KHz, mode 1 occurred? y/n\n");
    passert(getc_yes());

    /* Mode 2 transfer */
    passert(spi_set_mode(spi, 2) == 0);
    printf("Press enter to start transfer...");
    getc(stdin);
    passert(spi_transfer(spi, buf, NULL, sizeof(buf)) == 0);
    printf("SPI data 0x55, 0xaa, 0x0f, 0xf0\n");
    printf("SPI transfer speed <= 100KHz, mode 2 occurred? y/n\n");
    passert(getc_yes());

    /* Mode 3 transfer */
    passert(spi_set_mode(spi, 3) == 0);
    printf("Press enter to start transfer...");
    getc(stdin);
    passert(spi_transfer(spi, buf, NULL, sizeof(buf)) == 0);
    printf("SPI data 0x55, 0xaa, 0x0f, 0xf0\n");
    printf("SPI transfer speed <= 100KHz, mode 3 occurred? y/n\n");
    passert(getc_yes());

    passert(spi_set_mode(spi, 0) == 0);

    /* 500KHz transfer */
    passert(spi_set_max_speed(spi, 500000) == 0);
    printf("Press enter to start transfer...");
    getc(stdin);
    passert(spi_transfer(spi, buf, NULL, sizeof(buf)) == 0);
    printf("SPI data 0x55, 0xaa, 0x0f, 0xf0\n");
    printf("SPI transfer speed <= 500KHz, mode 0 occurred? y/n\n");
    passert(getc_yes());

    /* 1MHz transfer */
    passert(spi_set_max_speed(spi, 1000000) == 0);
    printf("Press enter to start transfer...");
    getc(stdin);
    passert(spi_transfer(spi, buf, NULL, sizeof(buf)) == 0);
    printf("SPI data 0x55, 0xaa, 0x0f, 0xf0\n");
    printf("SPI transfer speed <= 1MHz, mode 0 occurred? y/n\n");
    passert(getc_yes());

    passert(spi_close(spi) == 0);

    /* Free SPI */
    spi_free(spi);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <SPI device>\n\n", argv[0]);
        fprintf(stderr, "[1/4] Arguments test: No requirements.\n");
        fprintf(stderr, "[2/4] Open/close test: SPI device should be real.\n");
        fprintf(stderr, "[3/4] Loopback test: SPI MISO and MOSI should be connected with a wire.\n");
        fprintf(stderr, "[4/4] Interactive test: SPI MOSI, CLK, CS should be observed with an oscilloscope or logic analyzer.\n\n");
        fprintf(stderr, "Hint: for Raspberry Pi 3, enable SPI0 with:\n");
        fprintf(stderr, "   $ echo \"dtparam=spi=on\" | sudo tee -a /boot/config.txt\n");
        fprintf(stderr, "   $ sudo reboot\n");
        fprintf(stderr, "Use pins SPI0 MOSI (header pin 19), SPI0 MISO (header pin 21), SPI0 SCLK (header pin 23),\n");
        fprintf(stderr, "connect a loopback between MOSI and MISO, and run this test with:\n");
        fprintf(stderr, "    %s /dev/spidev0.0\n\n", argv[0]);
        exit(1);
    }

    device = argv[1];

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

