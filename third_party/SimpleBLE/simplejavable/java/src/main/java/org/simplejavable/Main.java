package org.simplejavable;

public class Main {
    public static void main(String[] args) throws Exception {
        // Check if Bluetooth is enabled
        // System.out.println("Bluetooth enabled: " + Adapter.isBluetoothEnabled());

        // Get list of adapters
        var adapterList = Adapter.getAdapters();

        if (adapterList.isEmpty()) {
            System.out.println("No adapter found");
            System.exit(1);
        }

        // Print information for each adapter
        for (var adapter : adapterList) {
            System.out.println("Adapter: " + adapter.getIdentifier() + " [" + adapter.getAddress() + "]");
        }

        var adapter = adapterList.get(0);

        class ScanCallback implements Adapter.EventListener {
            @Override
            public void onScanStart() {
                System.out.println("Scan started");
            }

            @Override
            public void onScanStop() {
                System.out.println("Scan stopped");
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

        System.out.println("Scanning for 5 seconds");
        adapter.scanFor(5000);

        System.out.println("Scan is done.");

        // TODO: There is a bug in MacOS that causes the app to hang once it finished executing.
        // This is a workaround to prevent the app from hanging.
        System.exit(0);

        // TODO: These are helper functions to try to debug the issue.

        // class CrashUtil {
        //     private static final Unsafe unsafe;

        //     static {
        //         try {
        //             Field f = Unsafe.class.getDeclaredField("theUnsafe");
        //             f.setAccessible(true);
        //             unsafe = (Unsafe) f.get(null);
        //         } catch (Exception e) {
        //             throw new RuntimeException(e);
        //         }
        //     }

        //     public static void crash() {
        //         // Write to an invalid memory address
        //         unsafe.putInt(0L, 42);
        //     }
        // }


        // Map<Thread, StackTraceElement[]> allThreads = Thread.getAllStackTraces();
        // for (Map.Entry<Thread, StackTraceElement[]> entry : allThreads.entrySet()) {
        //     Thread thread = entry.getKey();
        //     StackTraceElement[] stack = entry.getValue();
        //     System.out.println("Thread: " + thread.getName() + " (daemon: " + thread.isDaemon() + ")");
        //     for (StackTraceElement element : stack) {
        //         System.out.println("\t" + element);
        //     }
        // }

        // CrashUtil.crash();
    }
}