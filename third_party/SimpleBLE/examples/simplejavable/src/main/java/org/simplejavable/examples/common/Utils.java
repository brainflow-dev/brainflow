package org.simplejavable.examples.common;

import org.simplejavable.Adapter;
import org.simplejavable.Peripheral;

import java.util.List;
import java.util.Optional;
import java.util.Scanner;

public class Utils {
    public static Optional<Adapter> getAdapter() {
        // if (!Adapter.isBluetoothEnabled()) {
        //     System.out.println("Bluetooth is not enabled!");
        //     return Optional.empty();
        // }

        List<Adapter> adapterList = Adapter.getAdapters();

        if (adapterList.isEmpty()) {
            System.err.println("No adapter was found.");
            return Optional.empty();
        }

        if (adapterList.size() == 1) {
            Adapter adapter = adapterList.get(0);
            System.out.println("Using adapter: " + adapter.getIdentifier() + " [" + adapter.getAddress() + "]");
            return Optional.of(adapter);
        }

        System.out.println("Available adapters:");
        for (int i = 0; i < adapterList.size(); i++) {
            Adapter adapter = adapterList.get(i);
            System.out.println("[" + i + "] " + adapter.getIdentifier() + " [" + adapter.getAddress() + "]");
        }

        Optional<Integer> adapterSelection = getUserInputInt("Select an adapter", adapterList.size() - 1);

        return adapterSelection.map(adapterList::get);
    }

    public static Optional<Integer> getUserInputInt(String line, int max) {
        Scanner scanner = new Scanner(System.in);

        while (true) {
            System.out.print(line + " (0-" + max + "): ");
            if (!scanner.hasNextInt()) {
                scanner.nextLine(); // Clear invalid input
                return Optional.empty();
            }

            int ret = scanner.nextInt();
            scanner.nextLine(); // Consume newline

            if (ret <= max) {
                return Optional.of(ret);
            }
        }
    }
}