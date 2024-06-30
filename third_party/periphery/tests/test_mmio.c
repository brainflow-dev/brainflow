/*
 * c-periphery
 * https://github.com/vsergeev/c-periphery
 * License: MIT
 */

#include "test.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "../src/mmio.h"

#define PAGE_SIZE               4096

#define CONTROL_MODULE_BASE     0x44e10000
#define USB_VID_PID_OFFSET      0x7f4
#define USB_VID_PID             0x04516141

#define RTCSS_BASE              0x44e3e000
#define RTC_SCRATCH2_REG_OFFSET 0x68

void test_arguments(void) {
    ptest();

    /* Check offset out of bounds in test_open_config_close() */
}

void test_open_config_close(void) {
    mmio_t *mmio;
    uintptr_t address;
    uint32_t value32;

    ptest();

    /* Allocate MMIO */
    mmio = mmio_new();
    passert(mmio != NULL);

    /* Open aligned base */
    passert(mmio_open(mmio, CONTROL_MODULE_BASE, PAGE_SIZE) == 0);
    passert(mmio_base(mmio) == CONTROL_MODULE_BASE);
    passert(mmio_size(mmio) == PAGE_SIZE);
    passert(mmio_ptr(mmio) != NULL);

    struct mmio_handle {
        uintptr_t base, aligned_base;
        size_t size, aligned_size;
        void *ptr;

        struct {
            int c_errno;
            char errmsg[96];
        } error;
    };

    /* Check alignment math */
    passert(((struct mmio_handle *)mmio)->base == CONTROL_MODULE_BASE);
    passert(((struct mmio_handle *)mmio)->size == PAGE_SIZE);
    passert(((struct mmio_handle *)mmio)->aligned_base == CONTROL_MODULE_BASE);
    passert(((struct mmio_handle *)mmio)->aligned_size == PAGE_SIZE);
    passert(mmio_ptr(mmio) == ((struct mmio_handle *)mmio)->ptr);

    passert(mmio_read32(mmio, PAGE_SIZE-3, &value32) == MMIO_ERROR_ARG);
    passert(mmio_read32(mmio, PAGE_SIZE-2, &value32) == MMIO_ERROR_ARG);
    passert(mmio_read32(mmio, PAGE_SIZE-1, &value32) == MMIO_ERROR_ARG);
    passert(mmio_read32(mmio, PAGE_SIZE, &value32) == MMIO_ERROR_ARG);
    passert(mmio_close(mmio) == 0);

    /* Open unaligned base */
    address = CONTROL_MODULE_BASE + 123;
    passert(mmio_open(mmio, address, PAGE_SIZE) == 0);
    passert(mmio_base(mmio) == address);
    passert(mmio_size(mmio) == PAGE_SIZE);
    passert(mmio_ptr(mmio) != NULL);

    /* Check alignment math */
    passert(((struct mmio_handle *)mmio)->base == address);
    passert(((struct mmio_handle *)mmio)->size == PAGE_SIZE);
    passert(((struct mmio_handle *)mmio)->aligned_base == (address - (address % sysconf(_SC_PAGESIZE))));
    passert(((struct mmio_handle *)mmio)->aligned_size == (PAGE_SIZE + (address % sysconf(_SC_PAGESIZE))));
    passert((size_t)((uint8_t *)mmio_ptr(mmio) - (uint8_t *)((struct mmio_handle *)mmio)->ptr) ==
                (size_t)(((struct mmio_handle *)mmio)->base - ((struct mmio_handle *)mmio)->aligned_base));

    passert(mmio_read32(mmio, PAGE_SIZE-3, &value32) == MMIO_ERROR_ARG);
    passert(mmio_read32(mmio, PAGE_SIZE-2, &value32) == MMIO_ERROR_ARG);
    passert(mmio_read32(mmio, PAGE_SIZE-1, &value32) == MMIO_ERROR_ARG);
    passert(mmio_read32(mmio, PAGE_SIZE, &value32) == MMIO_ERROR_ARG);
    passert(mmio_close(mmio) == 0);

    /* Free MMIO */
    mmio_free(mmio);
}

void test_loopback(void) {
    mmio_t *mmio;
    uint32_t value32;
    uint8_t data[4];
    uint8_t vector[] = { 0xaa, 0xbb, 0xcc, 0xdd };

    ptest();

    /* Allocate MMIO */
    mmio = mmio_new();
    passert(mmio != NULL);

    /* Read USB VID/PID */
    passert(mmio_open(mmio, CONTROL_MODULE_BASE, PAGE_SIZE) == 0);
    passert(mmio_read32(mmio, USB_VID_PID_OFFSET, &value32) == 0);
    passert(value32 == USB_VID_PID);
    passert(mmio_close(mmio) == 0);

    /* Read USB VID/PID via byte read */
    passert(mmio_open(mmio, CONTROL_MODULE_BASE, PAGE_SIZE) == 0);
    passert(mmio_read(mmio, USB_VID_PID_OFFSET, data, 4) == 0);
    passert(data[0] == (USB_VID_PID & 0xff));
    passert(data[1] == ((USB_VID_PID >> 8) & 0xff));
    passert(data[2] == ((USB_VID_PID >> 16) & 0xff));
    passert(data[3] == ((USB_VID_PID >> 24) & 0xff));
    passert(mmio_close(mmio) == 0);

    /* Write/Read RTC Scratch2 Register */
    passert(mmio_open(mmio, RTCSS_BASE, PAGE_SIZE) == 0);
    passert(mmio_write32(mmio, RTC_SCRATCH2_REG_OFFSET, 0xdeadbeef) == 0);
    passert(mmio_read32(mmio, RTC_SCRATCH2_REG_OFFSET, &value32) == 0);
    passert(value32 == 0xdeadbeef);
    passert(mmio_close(mmio) == 0);

    /* Write/Read RTC Scratch2 Register via byte write */
    passert(mmio_open(mmio, RTCSS_BASE, PAGE_SIZE) == 0);
    passert(mmio_write(mmio, RTC_SCRATCH2_REG_OFFSET, vector, 4) == 0);
    passert(mmio_read32(mmio, RTC_SCRATCH2_REG_OFFSET, &value32) == 0);
    passert(value32 == 0xddccbbaa);
    passert(mmio_read(mmio, RTC_SCRATCH2_REG_OFFSET, data, 4) == 0);
    passert(memcmp(data, vector, 4) == 0);
    passert(mmio_close(mmio) == 0);

    /* Free MMIO */
    mmio_free(mmio);
}

struct rtc_ss {
    volatile uint32_t seconds;       /* 0x00 */
    volatile uint32_t minutes;       /* 0x04 */
    volatile uint32_t hours;         /* 0x08 */
    volatile uint32_t days;          /* 0x0C */
    volatile uint32_t months;        /* 0x10 */
    volatile uint32_t years;         /* 0x14 */
    volatile uint32_t weeks;         /* 0x18 */

    volatile uint32_t reserved1;     /* 0x1C */

    volatile uint32_t alarm_seconds; /* 0x20 */
    volatile uint32_t alarm_minutes; /* 0x24 */
    volatile uint32_t alarm_hours;   /* 0x28 */
    volatile uint32_t alarm_days;    /* 0x2C */
    volatile uint32_t alarm_months;  /* 0x30 */
    volatile uint32_t alarm_years;   /* 0x34 */

    volatile uint32_t reserved2;     /* 0x38 */
    volatile uint32_t reserved3;     /* 0x3C */

    volatile uint32_t rtc_ctrl;      /* 0x40 */
    volatile uint32_t rtc_status;    /* 0x44 */
    volatile uint32_t rtc_interrupts;/* 0x48 */
};

#define BCD_HI(x)   (((x) >> 4) & 0xf)
#define BCD_LO(x)   ((x) & 0xf)
#define BCD2DEC(x)  (10*BCD_HI(x) + BCD_LO(x))

void test_interactive(void) {
    mmio_t *mmio;
    uint32_t rtc_start, rtc_stop;
    time_t start, stop;
    struct rtc_ss *rtc;

    ptest();

    /* Allocate MMIO */
    mmio = mmio_new();
    passert(mmio != NULL);

    passert(mmio_open(mmio, RTCSS_BASE, PAGE_SIZE) == 0);
    rtc = (struct rtc_ss *)mmio_ptr(mmio);

    printf("Waiting for seconds ones digit to reset to 0...\n");

    start = time(NULL);
    /* Wait until seconds low go to 0, so we don't have to deal with overflows
     * in comparing times */
    while (BCD_LO(rtc->seconds) != 0) {
        usleep(500000);
        passert((time(NULL) - start) < 12);
    }

    start = time(NULL);
    rtc_start = rtc->seconds;

    printf("Date: %04d-%02d-%02d\n", 2000 + BCD2DEC(rtc->years), BCD2DEC(rtc->months), BCD2DEC(rtc->days));
    printf("Time: %02d:%02d:%02d %s\n", BCD2DEC(rtc->hours & 0x7f), BCD2DEC(rtc->minutes), BCD2DEC(rtc->seconds), (rtc->hours & 0x80) ? "PM" : "AM");

    sleep(3);

    printf("Date: %02d-%02d-%02d\n", 2000 + BCD2DEC(rtc->years), BCD2DEC(rtc->months), BCD2DEC(rtc->days));
    printf("Time: %02d:%02d:%02d %s\n", BCD2DEC(rtc->hours & 0x7f), BCD2DEC(rtc->minutes), BCD2DEC(rtc->seconds), (rtc->hours & 0x80) ? "PM" : "AM");

    rtc_stop = rtc->seconds;
    stop = time(NULL);

    /* Check that time has passed */
    passert((stop - start) > 2);
    passert((rtc_stop - rtc_start) > 2);

    passert(mmio_close(mmio) == 0);

    /* Free MMIO */
    mmio_free(mmio);
}

int main(void) {
    printf("WARNING: This test suite assumes a BeagleBone Black (AM335x) host!\n");
    printf("Other systems may experience unintended and dire consequences!\n");
    printf("Press enter to continue!\n");
    getc(stdin);

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

