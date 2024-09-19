#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#define SLEEP_SEC(x) _sleep(x * 1000)
#else
#include <unistd.h>
#define SLEEP_SEC(x) sleep(x)
#endif

#include "simpleble_c/simpleble.h"

static void print_buffer_hex(uint8_t* buf, size_t len, bool newline) {
    for (size_t i = 0; i < len; i++) {
        printf("%02X", buf[i]);

        if (i < (len - 1)) {
            printf(" ");
        }
    }

    if (newline) {
        printf("\n");
    }
}

static void adapter_on_scan_start(simpleble_adapter_t adapter, void* userdata);
static void adapter_on_scan_stop(simpleble_adapter_t adapter, void* userdata);
static void adapter_on_scan_found(simpleble_adapter_t adapter, simpleble_peripheral_t peripheral, void* userdata);
static void adapter_on_scan_updated(simpleble_adapter_t adapter, simpleble_peripheral_t peripheral, void* userdata);

int main() {
    // NOTE: It's necessary to call this function before any other to allow the
    // underlying driver to run its initialization routine.
    size_t adapter_count = simpleble_adapter_get_count();
    if (adapter_count == 0) {
        printf("No adapter was found.\n");
        return 1;
    }

    // TODO: Allow the user to pick an adapter.
    simpleble_adapter_t adapter = simpleble_adapter_get_handle(0);
    if (adapter == NULL) {
        printf("No adapter was found.\n");
        return 1;
    }

    simpleble_adapter_set_callback_on_scan_start(adapter, adapter_on_scan_start, NULL);
    simpleble_adapter_set_callback_on_scan_stop(adapter, adapter_on_scan_stop, NULL);
    simpleble_adapter_set_callback_on_scan_found(adapter, adapter_on_scan_found, NULL);
    simpleble_adapter_set_callback_on_scan_updated(adapter, adapter_on_scan_updated, NULL);

    simpleble_adapter_scan_for(adapter, 5000);

    // Sleep for an additional second before returning.
    // If there are any detections during this period, it means that the
    // internal peripheral took longer to stop than anticipated.
    SLEEP_SEC(1);

    size_t peripheral_count = simpleble_adapter_scan_get_results_count(adapter);
    for (size_t peripheral_index = 0; peripheral_index < peripheral_count; peripheral_index++) {
        simpleble_peripheral_t peripheral = simpleble_adapter_scan_get_results_handle(adapter, peripheral_index);

        char* peripheral_identifier = simpleble_peripheral_identifier(peripheral);
        char* peripheral_address = simpleble_peripheral_address(peripheral);

        bool peripheral_connectable = false;
        simpleble_peripheral_is_connectable(peripheral, &peripheral_connectable);

        int16_t peripheral_rssi = simpleble_peripheral_rssi(peripheral);

        printf("[%zu] %s [%s] %d dBm %s\n", peripheral_index, peripheral_identifier, peripheral_address,
               peripheral_rssi, peripheral_connectable ? "Connectable" : "Non-Connectable");

        size_t services_count = simpleble_peripheral_services_count(peripheral);
        for (size_t service_index = 0; service_index < services_count; service_index++) {
            simpleble_service_t service;
            simpleble_peripheral_services_get(peripheral, service_index, &service);

            printf("    Service UUID: %s\n", service.uuid.value);
            printf("    Service data: ");
            print_buffer_hex(service.data, service.data_length, true);
        }

        size_t manufacturer_data_count = simpleble_peripheral_manufacturer_data_count(peripheral);
        for (size_t manuf_data_index = 0; manuf_data_index < manufacturer_data_count; manuf_data_index++) {
            simpleble_manufacturer_data_t manuf_data;
            simpleble_peripheral_manufacturer_data_get(peripheral, manuf_data_index, &manuf_data);
            printf("    Manufacturer ID: %04X\n", manuf_data.manufacturer_id);
            printf("    Manufacturer data: ");
            print_buffer_hex(manuf_data.data, manuf_data.data_length, true);
        }

        // Let's not forget to release the associated handles and memory
        simpleble_peripheral_release_handle(peripheral);
        simpleble_free(peripheral_address);
        simpleble_free(peripheral_identifier);
    }

    // Let's not forget to release the associated handle.
    simpleble_adapter_release_handle(adapter);

    return 0;
}

static void adapter_on_scan_start(simpleble_adapter_t adapter, void* userdata) {
    char* identifier = simpleble_adapter_identifier(adapter);

    if (identifier == NULL) {
        return;
    }

    printf("Adapter %s started scanning.\n", identifier);

    // Let's not forget to clear the allocated memory.
    simpleble_free(identifier);
}

static void adapter_on_scan_stop(simpleble_adapter_t adapter, void* userdata) {
    char* identifier = simpleble_adapter_identifier(adapter);

    if (identifier == NULL) {
        return;
    }

    printf("Adapter %s stopped scanning.\n", identifier);

    // Let's not forget to clear the allocated memory.
    simpleble_free(identifier);
}

static void adapter_on_scan_found(simpleble_adapter_t adapter, simpleble_peripheral_t peripheral, void* userdata) {
    char* adapter_identifier = simpleble_adapter_identifier(adapter);
    char* peripheral_identifier = simpleble_peripheral_identifier(peripheral);
    char* peripheral_address = simpleble_peripheral_address(peripheral);

    if (adapter_identifier == NULL || peripheral_identifier == NULL || peripheral_address == NULL) {
        return;
    }

    printf("Adapter %s found device: %s [%s]\n", adapter_identifier, peripheral_identifier, peripheral_address);

    // Let's not forget to release the associated handles and memory
    simpleble_peripheral_release_handle(peripheral);
    simpleble_free(peripheral_address);
    simpleble_free(peripheral_identifier);
}

static void adapter_on_scan_updated(simpleble_adapter_t adapter, simpleble_peripheral_t peripheral, void* userdata) {
    char* adapter_identifier = simpleble_adapter_identifier(adapter);
    char* peripheral_identifier = simpleble_peripheral_identifier(peripheral);
    char* peripheral_address = simpleble_peripheral_address(peripheral);

    if (adapter_identifier == NULL || peripheral_identifier == NULL || peripheral_address == NULL) {
        return;
    }

    printf("Adapter %s updated device: %s [%s]\n", adapter_identifier, peripheral_identifier, peripheral_address);

    // Let's not forget to release the associated handles and memory
    simpleble_peripheral_release_handle(peripheral);
    simpleble_free(peripheral_address);
    simpleble_free(peripheral_identifier);
}
