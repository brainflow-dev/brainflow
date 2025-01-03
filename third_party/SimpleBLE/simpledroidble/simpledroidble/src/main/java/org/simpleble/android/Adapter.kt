package org.simpleble.android

import android.annotation.SuppressLint
import android.util.Log
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.channels.BufferOverflow
import kotlinx.coroutines.flow.MutableSharedFlow
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext

class Adapter private constructor(newInstanceId: Long) {
    private val _onScanStart = MutableSharedFlow<Unit>()
    private val _onScanStop = MutableSharedFlow<Unit>()
    private val _onScanActive = MutableSharedFlow<Boolean>()
    private val _onScanUpdated = MutableSharedFlow<Peripheral>()
    private val _onScanFound = MutableSharedFlow<Peripheral>()

    private var instanceId: Long = newInstanceId

    private val callbacks = object : Callback {
        override fun onScanStart() {
            CoroutineScope(Dispatchers.Main).launch {
                _onScanStart.emit(Unit)
            }
            CoroutineScope(Dispatchers.Main).launch {
                _onScanActive.emit(true)
            }
        }

        override fun onScanStop() {
            CoroutineScope(Dispatchers.Main).launch {
                _onScanStop.emit(Unit)
            }
            CoroutineScope(Dispatchers.Main).launch {
                _onScanActive.emit(false)
            }
        }

        override fun onScanUpdated(peripheralId: Long) {
            CoroutineScope(Dispatchers.Main).launch {
                _onScanUpdated.emit(Peripheral(instanceId, peripheralId))
            }
        }

        override fun onScanFound(peripheralId: Long) {
            CoroutineScope(Dispatchers.Main).launch {
                _onScanFound.emit(Peripheral(instanceId, peripheralId))
            }
        }
    }

    init {
        Log.d("SimpleBLE", "Adapter ${this.hashCode()}.init")
        nativeAdapterRegister(instanceId, callbacks)
    }

    val identifier: String get() {
        return nativeAdapterIdentifier(instanceId) ?: ""
    }

    val address: BluetoothAddress get() {
        return BluetoothAddress(nativeAdapterAddress(instanceId) ?: "")
    }

    fun scanStart() {
        nativeAdapterScanStart(instanceId)
    }

    fun scanStop() {
        nativeAdapterScanStop(instanceId)
    }

    suspend fun scanFor(timeoutMs: Int) {
        withContext(Dispatchers.IO) {
            nativeAdapterScanFor(instanceId, timeoutMs)
        }
    }

    val scanIsActive: Boolean get() {
        return nativeAdapterScanIsActive(instanceId)
    }

    fun scanGetResults(): List<Peripheral> {
        return nativeAdapterScanGetResults(instanceId).map { Peripheral(instanceId, it) }
    }

    val onScanStart get() = _onScanStart

    val onScanStop get() = _onScanStop

    val onScanActive get() = _onScanActive

    val onScanUpdated get() = _onScanUpdated

    val onScanFound get() = _onScanFound

    fun getPairedPeripherals(): List<Peripheral> {
        // TODO: Implement
        return emptyList()
    }

    companion object {

        @JvmStatic
        fun isBluetoothEnabled(): Boolean {
            return nativeIsBluetoothEnabled()
        }

        @JvmStatic
        fun getAdapters(): List<Adapter> {
//            if (SimpleDroidBle.permissionsGranted.not()) {
//                return emptyList()
//            }

            val nativeAdapterIds = nativeGetAdapters()
            val adapters = ArrayList<Adapter>()

            for (nativeAdapterId in nativeAdapterIds) {
                adapters.add(Adapter(nativeAdapterId))
            }

            return adapters
        }

        @JvmStatic
        private external fun nativeGetAdapters(): LongArray

        private external fun nativeIsBluetoothEnabled(): Boolean
    }

    // ----------------------------------------------------------------------------

    private external fun nativeAdapterRegister(adapterId: Long, callback: Callback)

    private external fun nativeAdapterIdentifier(adapterId: Long): String?

    private external fun nativeAdapterAddress(adapterId: Long): String?

    private external fun nativeAdapterScanStart(adapterId: Long)

    private external fun nativeAdapterScanStop(adapterId: Long)

    private external fun nativeAdapterScanFor(adapterId: Long, timeout: Int)

    private external fun nativeAdapterScanIsActive(adapterId: Long): Boolean

    private external fun nativeAdapterScanGetResults(adapterId: Long) : LongArray

    // TODO: Implement
    private external fun nativeAdapterGetPairedPeripherals(adapterId: Long): LongArray

    // ----------------------------------------------------------------------------

    private interface Callback {
        fun onScanStart()
        fun onScanStop()
        fun onScanUpdated(peripheralId: Long)
        fun onScanFound(peripheralId: Long)
    }

}
