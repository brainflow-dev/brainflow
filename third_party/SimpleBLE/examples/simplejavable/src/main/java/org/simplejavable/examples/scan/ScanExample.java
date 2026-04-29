package org.simplejavable.examples.scan;

import org.simplejavable.Adapter;
import org.simplejavable.Peripheral;
import org.simplejavable.examples.common.Utils;

import java.util.List;
import java.util.Map;
import java.util.Optional;

public class ScanExample {
    public static void main(String[] args) {
        Optional<Adapter> adapterOptional = Utils.getAdapter();

        if (adapterOptional.isEmpty()) {
            System.exit(1);
            return;
        }

        Adapter adapter = adapterOptional.get();

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
                System.out.println("Updated device: " + peripheral.getIdentifier() + " [" + peripheral.getAddress() + "] "
                        + peripheral.getRssi() + " dBm");
            }

            @Override
            public void onScanFound(Peripheral peripheral) {
                System.out.println("Found device: " + peripheral.getIdentifier() + " [" + peripheral.getAddress() + "] "
                        + peripheral.getRssi() + " dBm");
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

        System.out.println("Scan complete.");

        List<Peripheral> peripherals = adapter.scanGetResults();
        System.out.println("The following devices were found:");
        for (int i = 0; i < peripherals.size(); i++) {
            Peripheral peripheral = peripherals.get(i);
            String connectableString = peripheral.isConnectable() ? "Connectable" : "Non-Connectable";
            String peripheralString = peripheral.getIdentifier() + " [" + peripheral.getAddress() + "] "
                    + peripheral.getRssi() + " dBm";

            System.out.println("[" + i + "] " + peripheralString + " " + connectableString);

            System.out.println("    Tx Power: " + peripheral.getTxPower() + " dBm");
            System.out.println("    Address Type: " + peripheral.getAddressType());

            // Note: The Java API doesn't expose service data directly in the Peripheral class
            // If needed, you would need to connect to the peripheral to get service information

            Map<Integer, byte[]> manufacturerData = peripheral.manufacturerData();
            for (Map.Entry<Integer, byte[]> entry : manufacturerData.entrySet()) {
                System.out.println("    Manufacturer ID: " + entry.getKey());
                System.out.println("    Manufacturer data: " + bytesToHex(entry.getValue()));
            }
        }

        System.exit(0);
    }

    private static String bytesToHex(byte[] bytes) {
        StringBuilder sb = new StringBuilder();
        for (byte b : bytes) {
            sb.append(String.format("%02X ", b));
        }
        return sb.toString().trim();
    }
}