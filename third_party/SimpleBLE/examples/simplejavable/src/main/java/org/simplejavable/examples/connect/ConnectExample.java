package org.simplejavable.examples.connect;

import org.simplejavable.Adapter;
import org.simplejavable.Characteristic;
import org.simplejavable.Descriptor;
import org.simplejavable.Peripheral;
import org.simplejavable.Service;
import org.simplejavable.examples.common.Utils;

import java.util.ArrayList;
import java.util.List;
import java.util.Optional;

public class ConnectExample {
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

        System.out.println("The following connectable devices were found:");
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
                System.out.println("Successfully connected.");
                System.out.println("MTU: " + peripheral.getMtu());
                List<Service> services = peripheral.services();
                for (Service service : services) {
                    System.out.println("Service: " + service.uuid());
                    for (Characteristic characteristic : service.characteristics()) {
                        System.out.println("  Characteristic: " + characteristic.uuid());
                        System.out.print("    Capabilities: ");
                        if (characteristic.canRead()) System.out.print("read ");
                        if (characteristic.canWriteRequest()) System.out.print("write_request ");
                        if (characteristic.canWriteCommand()) System.out.print("write_command ");
                        if (characteristic.canNotify()) System.out.print("notify ");
                        if (characteristic.canIndicate()) System.out.print("indicate ");
                        System.out.println();

                        for (Descriptor descriptor : characteristic.descriptors()) {
                            System.out.println("    Descriptor: " + descriptor.uuid());
                        }
                    }
                }
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
}