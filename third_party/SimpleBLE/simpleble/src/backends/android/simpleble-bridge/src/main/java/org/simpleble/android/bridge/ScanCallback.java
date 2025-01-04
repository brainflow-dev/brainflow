package org.simpleble.android.bridge;

import android.bluetooth.le.ScanResult;

import java.util.List;
import android.util.Log;

public class ScanCallback extends android.bluetooth.le.ScanCallback {

    public ScanCallback() {}

    @Override
    public void onScanResult(int callbackType, ScanResult result) {
        super.onScanResult(callbackType, result);
        onScanResultCallback(callbackType, result);
    }

    @Override
    public void onBatchScanResults(List<ScanResult> results) {
        super.onBatchScanResults(results);
        onBatchScanResultsCallback(results);
    }

    @Override
    public void onScanFailed(int errorCode) {
        super.onScanFailed(errorCode);
        onScanFailedCallback(errorCode);
    }

    private native void onScanResultCallback(int callbackType, android.bluetooth.le.ScanResult result);

    private native void onScanFailedCallback(int errorCode);

    private native void onBatchScanResultsCallback(List<android.bluetooth.le.ScanResult> results);

}
