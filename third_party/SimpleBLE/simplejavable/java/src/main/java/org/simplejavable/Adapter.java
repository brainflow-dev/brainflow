package org.simplejavable;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

public class Adapter {
    private EventListener eventListener;
    private final long instanceId;

    private final Callback callbacks = new Callback() {
        @Override
        public void onScanStart() {
            if (eventListener != null) {
                eventListener.onScanStart();
            }
        }

        @Override
        public void onScanStop() {
            if (eventListener != null) {
                eventListener.onScanStop();
            }
        }

        @Override
        public void onScanUpdated(long peripheralId) {
            if (eventListener != null) {
                eventListener.onScanUpdated(new Peripheral(instanceId, peripheralId));
            }
        }

        @Override
        public void onScanFound(long peripheralId) {
            if (eventListener != null) {
                eventListener.onScanFound(new Peripheral(instanceId, peripheralId));
            }
        }
    };

    private Adapter(long newInstanceId) {
        this.instanceId = newInstanceId;
        nativeAdapterRegister(instanceId, callbacks);
    }

    public String getIdentifier() {
        String identifier = nativeAdapterIdentifier(instanceId);
        return identifier != null ? identifier : "";
    }

    public BluetoothAddress getAddress() {
        String address = nativeAdapterAddress(instanceId);
        return new BluetoothAddress(address != null ? address : "");
    }

    public void scanStart() {
        nativeAdapterScanStart(instanceId);
    }

    public void scanStop() {
        nativeAdapterScanStop(instanceId);
    }

    public void scanFor(int timeoutMs) throws Exception {
        nativeAdapterScanFor(instanceId, timeoutMs);
    }

    public boolean getScanIsActive() {
        return nativeAdapterScanIsActive(instanceId);
    }

    public List<Peripheral> scanGetResults() {
        long[] results = nativeAdapterScanGetResults(instanceId);
        List<Peripheral> peripherals = new ArrayList<>();
        for (long id : results) {
            peripherals.add(new Peripheral(instanceId, id));
        }
        return peripherals;
    }

    public void setEventListener(EventListener listener) {
        this.eventListener = listener;
    }

    public List<Peripheral> getPairedPeripherals() {
        long[] results = nativeAdapterGetPairedPeripherals(instanceId);
        List<Peripheral> peripherals = new ArrayList<>();
        for (long id : results) {
            peripherals.add(new Peripheral(instanceId, id));
        }
        return peripherals;
    }

    public static boolean isBluetoothEnabled() {
        return nativeIsBluetoothEnabled();
    }

    public static List<Adapter> getAdapters() {
        long[] nativeAdapterIds = nativeGetAdapters();
        List<Adapter> adapters = new ArrayList<>();

        for (long nativeAdapterId : nativeAdapterIds) {
            adapters.add(new Adapter(nativeAdapterId));
        }

        return adapters;
    }

    // Native method declarations
    private static native long[] nativeGetAdapters();
    private static native boolean nativeIsBluetoothEnabled();
    private native void nativeAdapterRegister(long adapterId, Callback callback);
    private native String nativeAdapterIdentifier(long adapterId);
    private native String nativeAdapterAddress(long adapterId);
    private native void nativeAdapterScanStart(long adapterId);
    private native void nativeAdapterScanStop(long adapterId);
    private native void nativeAdapterScanFor(long adapterId, int timeout);
    private native boolean nativeAdapterScanIsActive(long adapterId);
    private native long[] nativeAdapterScanGetResults(long adapterId);
    private native long[] nativeAdapterGetPairedPeripherals(long adapterId);

    private interface Callback {
        void onScanStart();
        void onScanStop();
        void onScanUpdated(long peripheralId);
        void onScanFound(long peripheralId);
    }

    public interface EventListener {
        void onScanStart();
        void onScanStop();
        void onScanUpdated(Peripheral peripheral);
        void onScanFound(Peripheral peripheral);
    }

    static {
        try {
            NativeLibraryLoader.loadLibrary("simplejavable");
        } catch (IOException e) {
            throw new RuntimeException("Failed to load native library", e);
        }

        Runtime.getRuntime().addShutdownHook(new Thread(() -> {
            // TODO: We might need this to clean up the native library.
            // System.out.println("JVM shutdown initiated at " + System.currentTimeMillis());
        }));
    }
}