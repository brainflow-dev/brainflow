package org.simpleble.android

import android.util.Log
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.MutableSharedFlow
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext

class Peripheral internal constructor(newAdapterId: Long, newInstanceId: Long) {

    private var instanceId: Long = newInstanceId
    private var adapterId: Long = newAdapterId

    private val _onConnected = MutableSharedFlow<Unit>()
    private val _onConnectionActive = MutableSharedFlow<Boolean>()
    private val _onDisconnected = MutableSharedFlow<Unit>()

    private val callbacks = object : Callback {
        override fun onConnected() {
            CoroutineScope(Dispatchers.Main).launch {
                _onConnected.emit(Unit)
            }
            CoroutineScope(Dispatchers.Main).launch {
                _onConnectionActive.emit(true)
            }
        }

        override fun onDisconnected() {
            CoroutineScope(Dispatchers.Main).launch {
                _onDisconnected.emit(Unit)
            }
            CoroutineScope(Dispatchers.Main).launch {
                _onConnectionActive.emit(false)
            }
        }
    }

    init {
        Log.d("SimpleBLE", "Peripheral ${this.hashCode()}.init")
        nativePeripheralRegister(adapterId, instanceId, callbacks)
    }

    val identifier: String get() {
        return nativePeripheralIdentifier(adapterId, instanceId) ?: ""
    }

    val address: BluetoothAddress get() {
        return BluetoothAddress(nativePeripheralAddress(adapterId, instanceId) ?: "")
    }

    val addressType: BluetoothAddressType get() {
        return BluetoothAddressType.fromInt(nativePeripheralAddressType(adapterId, instanceId))
    }

    val rssi: Int get() {
        return nativePeripheralRssi(adapterId, instanceId)
    }

    val txPower: Int get() {
        return nativePeripheralTxPower(adapterId, instanceId)
    }

    val mtu: Int get() {
        return nativePeripheralMtu(adapterId, instanceId)
    }

    suspend fun connect() {
        withContext(Dispatchers.IO) {
            nativePeripheralConnect(adapterId, instanceId)
        }
    }

    suspend fun disconnect() {
        withContext(Dispatchers.IO) {
            nativePeripheralDisconnect(adapterId, instanceId)
        }
    }

    val isConnected: Boolean get() {
        return nativePeripheralIsConnected(adapterId, instanceId)
    }

    val isConnectable: Boolean get() {
        return nativePeripheralIsConnectable(adapterId, instanceId)
    }

    val isPaired: Boolean get() {
        return nativePeripheralIsPaired(adapterId, instanceId)
    }

    fun unpair() {
        nativePeripheralUnpair(adapterId, instanceId)
    }

    fun services(): List<Service> {
        return nativePeripheralServices(adapterId, instanceId)
    }

    fun manufacturerData(): Map<Int, ByteArray> {
        return nativePeripheralManufacturerData(adapterId, instanceId)
    }

    fun read(service: BluetoothUUID, characteristic: BluetoothUUID): ByteArray {
        return nativePeripheralRead(adapterId, instanceId, service.toString(), characteristic.toString())
    }

    fun writeRequest(service: BluetoothUUID, characteristic: BluetoothUUID, data: ByteArray) {
        // TODO: Implement
    }

    fun writeCommand(service: BluetoothUUID, characteristic: BluetoothUUID, data: ByteArray) {
        // TODO: Implement
    }

    fun notify(
        service: BluetoothUUID,
        characteristic: BluetoothUUID
    ): MutableSharedFlow<ByteArray> {
        val payloadFlow = MutableSharedFlow<ByteArray>()
        val dataCallback = object : DataCallback {
            override fun onDataReceived(data: ByteArray) {
                CoroutineScope(Dispatchers.Main).launch {
                    payloadFlow.emit(data)
                }
            }
        }

        nativePeripheralNotify(adapterId, instanceId, service.toString(), characteristic.toString(), dataCallback)
        return payloadFlow
    }

    fun indicate(
        service: BluetoothUUID,
        characteristic: BluetoothUUID
    ): MutableSharedFlow<ByteArray> {
        val payloadFlow = MutableSharedFlow<ByteArray>()
        val dataCallback = object : DataCallback {
            override fun onDataReceived(data: ByteArray) {
                CoroutineScope(Dispatchers.Main).launch {
                    payloadFlow.emit(data)
                }
            }
        }

        nativePeripheralIndicate(adapterId, instanceId, service.toString(), characteristic.toString(), dataCallback)
        return payloadFlow
    }

    fun unsubscribe(service: BluetoothUUID, characteristic: BluetoothUUID) {
        nativePeripheralUnsubscribe(adapterId, instanceId, service.toString(), characteristic.toString())
    }

    fun read(
        service: BluetoothUUID,
        characteristic: BluetoothUUID,
        descriptor: BluetoothUUID
    ): ByteArray {
        // TODO: Implement
        return ByteArray(0)
    }

    fun write(
        service: BluetoothUUID,
        characteristic: BluetoothUUID,
        descriptor: BluetoothUUID,
        data: ByteArray
    ) {
        // TODO: Implement
    }

    val onConnected get() = _onConnected

    val onDisconnected get() = _onDisconnected

    val onConnectionActive get() = _onConnectionActive

    /// ----------------------------------------------------------------------------

    private external fun nativePeripheralRegister(adapterId: Long, instanceId: Long, callback: Callback)

    private external fun nativePeripheralIdentifier(adapterId: Long, instanceId: Long): String?

    private external fun nativePeripheralAddress(adapterId: Long, instanceId: Long): String?

    private external fun nativePeripheralAddressType(adapterId: Long, instanceId: Long): Int

    private external fun nativePeripheralRssi(adapterId: Long, instanceId: Long): Int

    private external fun nativePeripheralTxPower(adapterId: Long, instanceId: Long): Int

    private external fun nativePeripheralMtu(adapterId: Long, instanceId: Long): Int

    private external fun nativePeripheralConnect(adapterId: Long, instanceId: Long)

    private external fun nativePeripheralDisconnect(adapterId: Long, instanceId: Long)

    private external fun nativePeripheralIsConnected(adapterId: Long, instanceId: Long): Boolean

    private external fun nativePeripheralIsConnectable(adapterId: Long, instanceId: Long): Boolean

    private external fun nativePeripheralIsPaired(adapterId: Long, instanceId: Long): Boolean

    private external fun nativePeripheralUnpair(adapterId: Long, instanceId: Long)

    private external fun nativePeripheralServices(adapterId: Long, instanceId: Long): List<Service>

    private external fun nativePeripheralManufacturerData(adapterId: Long, instanceId: Long): Map<Int, ByteArray>

    private external fun nativePeripheralRead(
        adapterId: Long,
        instanceId: Long,
        service: String,
        characteristic: String
    ): ByteArray

    private external fun nativePeripheralWriteRequest(
        adapterId: Long,
        instanceId: Long,
        service: String,
        characteristic: String,
        data: ByteArray
    )

    private external fun nativePeripheralWriteCommand(
        adapterId: Long,
        instanceId: Long,
        service: String,
        characteristic: String,
        data: ByteArray
    )

    private external fun nativePeripheralNotify(
        adapterId: Long,
        instanceId: Long,
        service: String,
        characteristic: String,
        dataCallback: DataCallback
    )

    private external fun nativePeripheralIndicate(
        adapterId: Long,
        instanceId: Long,
        service: String,
        characteristic: String,
        dataCallback: DataCallback
    )

    private external fun nativePeripheralUnsubscribe(
        adapterId: Long,
        instanceId: Long,
        service: String,
        characteristic: String)

    private external fun nativePeripheralDescriptorRead(
        adapterId: Long,
        instanceId: Long,
        service: String,
        characteristic: String,
        descriptor: String
    ): ByteArray

    private external fun nativePeripheralDescriptorWrite(
        adapterId: Long,
        instanceId: Long,
        service: String,
        characteristic: String,
        descriptor: String,
        data: ByteArray
    )

    // ----------------------------------------------------------------------------

    private interface DataCallback {
        fun onDataReceived(data: ByteArray)
    }


    private interface Callback {
        fun onConnected()
        fun onDisconnected()
    }

}