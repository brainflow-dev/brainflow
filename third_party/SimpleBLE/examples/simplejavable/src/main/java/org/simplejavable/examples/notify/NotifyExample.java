package org.simplejavable.examples.notify;

import org.simplejavable.Adapter;
import org.simplejavable.BluetoothUUID;
import org.simplejavable.Peripheral;
import org.simplejavable.Service;
import org.simplejavable.examples.common.Utils;

import java.util.ArrayList;
import java.util.List;
import java.util.Optional;
import java.util.UUID;

public class NotifyExample {
    public static void main(String[] args) {
        Optional<Adapter> adapterOptional = Utils.getAdapter();

        if (adapterOptional.isEmpty()) {
            System.exit(1);
            return;
        }

        Adapter adapter = adapterOptional.get();
        List<Peripheral> peripherals = new ArrayList<>();

        class ScanCallback implements Adapter.EventListener {
            @Override
            public void onScanStart() {
                System.out.println("Scan started.");
            }

            @Override
            public void onScanStop() {
                System.out.println("Scan stopped.");
            }

            @Override
            public void onScanUpdated(Peripheral peripheral) {
                // Not used in this example
            }

            @Override
            public void onScanFound(Peripheral peripheral) {
                System.out.println("Found device: " + peripheral.getIdentifier() + " [" + peripheral.getAddress() + "]");
                if (peripheral.isConnectable()) {
                    peripherals.add(peripheral);
                }
            }
        }

        adapter.setEventListener(new ScanCallback());

        try {
            adapter.scanFor(5000);
        } catch (Exception e) {
            System.err.println("Scan failed: " + e.getMessage());
            System.exit(1);
            return;
        }

        System.out.println("The following devices were found:");
        for (int i = 0; i < peripherals.size(); i++) {
            Peripheral peripheral = peripherals.get(i);
            System.out.println("[" + i + "] " + peripheral.getIdentifier() + " [" + peripheral.getAddress() + "]");
        }

        Optional<Integer> selection = Utils.getUserInputInt("Please select a device to connect to", peripherals.size() - 1);
        if (selection.isEmpty()) {
            System.exit(1);
            return;
        }

        Peripheral peripheral = peripherals.get(selection.get());

        class PeripheralCallback implements Peripheral.EventListener {
            @Override
            public void onConnected() {
                System.out.println("Successfully connected, printing services and characteristics..");

                // Store all service and characteristic uuids in a list
                List<ServiceCharacteristic> uuids = new ArrayList<>();
                for (Service service : peripheral.services()) {
                    for (org.simplejavable.Characteristic characteristic : service.characteristics()) {
                        uuids.add(new ServiceCharacteristic(service.uuid(), characteristic.uuid()));
                    }
                }

                System.out.println("The following services and characteristics were found:");
                for (int i = 0; i < uuids.size(); i++) {
                    System.out.println("[" + i + "] " + uuids.get(i).serviceUuid + " " + uuids.get(i).characteristicUuid);
                }

                Optional<Integer> charSelection = Utils.getUserInputInt("Please select a characteristic to read", uuids.size() - 1);
                if (charSelection.isEmpty()) {
                    peripheral.disconnect();
                    System.exit(1);
                    return;
                }

                ServiceCharacteristic selected = uuids.get(charSelection.get());
                BluetoothUUID serviceUuid = new BluetoothUUID(selected.serviceUuid);
                BluetoothUUID characteristicUuid = new BluetoothUUID(selected.characteristicUuid);

                // Subscribe to the characteristic
                peripheral.notify(serviceUuid, characteristicUuid, data -> {
                    System.out.println("Received: " + bytesToHex(data));
                });

                // Wait for 5 seconds
                try {
                    Thread.sleep(5000);
                } catch (InterruptedException e) {
                    // Handle interruption
                }

                // Unsubscribe and disconnect
                peripheral.unsubscribe(serviceUuid, characteristicUuid);
                peripheral.disconnect();
                System.out.println("Disconnected.");
                System.exit(0);
            }

            @Override
            public void onDisconnected() {
                System.out.println("Disconnected.");
                System.exit(0);
            }
        }

        peripheral.setEventListener(new PeripheralCallback());

        System.out.println("Connecting to " + peripheral.getIdentifier() + " [" + peripheral.getAddress() + "]");
        peripheral.connect();

        // Keep the application running to receive callbacks
        try {
            Thread.sleep(Long.MAX_VALUE);
        } catch (InterruptedException e) {
            peripheral.disconnect();
        }

        System.exit(0);
    }

    private static class ServiceCharacteristic {
        final String serviceUuid;
        final String characteristicUuid;

        ServiceCharacteristic(String serviceUuid, String characteristicUuid) {
            this.serviceUuid = serviceUuid;
            this.characteristicUuid = characteristicUuid;
        }
    }

    private static String bytesToHex(byte[] bytes) {
        StringBuilder sb = new StringBuilder();
        for (byte b : bytes) {
            sb.append(String.format("%02X ", b));
        }
        return sb.toString().trim();
    }
}