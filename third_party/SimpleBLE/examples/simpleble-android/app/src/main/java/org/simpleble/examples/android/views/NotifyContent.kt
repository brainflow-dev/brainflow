package org.simpleble.examples.android.views

import android.util.Log
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material.Button
import androidx.compose.material.MaterialTheme
import androidx.compose.material.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.TextStyle
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.TextUnit
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.collectLatest
import kotlinx.coroutines.launch
import org.simpleble.android.BluetoothUUID
import org.simpleble.android.Peripheral
import org.simpleble.examples.android.viewmodels.BluetoothViewModel

@Composable
fun NotifyContent(bluetoothViewModel: BluetoothViewModel) {
    var scanResults by remember { mutableStateOf(emptyList<Peripheral>()) }
    var isScanning by remember { mutableStateOf(false) }

    var selectedDevice by remember { mutableStateOf<Peripheral?>(null) }
    var isConnected by remember { mutableStateOf(false) }

    var servicescharacteristics by remember { mutableStateOf(emptyList<Pair<BluetoothUUID, BluetoothUUID>>()) }
    var selectedServiceCharacteristic by remember { mutableStateOf<Pair<BluetoothUUID, BluetoothUUID>?>(null) }
    var receivedData by remember { mutableStateOf<ByteArray?>(null) }

    LaunchedEffect(Unit, selectedDevice) {
        CoroutineScope(Dispatchers.Main).launch {
            bluetoothViewModel.adapter.onScanActive.collect {
                Log.d("SimpleBLE", "Scan active: $it")
                isScanning = it
            }
        }

        CoroutineScope(Dispatchers.Main).launch {
            bluetoothViewModel.adapter.onScanFound.collect {
                Log.d("SimpleBLE", "Found device: ${it.identifier} [${it.address}] ${it.rssi} dBm")
                scanResults = scanResults + it
            }
        }

        selectedDevice?.let { peripheral ->
            CoroutineScope(Dispatchers.Main).launch {
                peripheral.onConnectionActive.collectLatest { active ->
                    isConnected = active
                }
            }

            CoroutineScope(Dispatchers.Main).launch {
                peripheral.onConnected.collectLatest {
                    Log.d("SimpleBLE", "Connected to ${peripheral.identifier} [${peripheral.address}]")

                    servicescharacteristics = peripheral.services().flatMap { service ->
                        service.characteristics.map { characteristic ->
                            Log.d("SimpleBLE", "Service: ${service.uuid} Characteristic: ${characteristic.uuid} [Notify: ${characteristic.canNotify} Indicate: ${characteristic.canIndicate} Read: ${characteristic.canRead} WriteCommand: ${characteristic.canWriteCommand} WriteRequest: ${characteristic.canWriteRequest}]")
                            Pair(BluetoothUUID(service.uuid), BluetoothUUID(characteristic.uuid))
                        }
                    }
                }
            }

            CoroutineScope(Dispatchers.Main).launch {
                peripheral.onDisconnected.collectLatest {
                    Log.d("SimpleBLE", "Disconnected from ${peripheral.identifier} [${peripheral.address}]")

                    servicescharacteristics = emptyList()
                    selectedServiceCharacteristic = null
                    receivedData = null
                }
            }
        }
    }

    Column(
        modifier = Modifier.fillMaxSize(),
        horizontalAlignment = Alignment.CenterHorizontally,
        verticalArrangement = Arrangement.Center
    ) {
        if (isScanning) {
            Text(
                text = "Scanning...",
                style = MaterialTheme.typography.h6,
                modifier = Modifier.padding(16.dp)
            )
        }

        Button(
            onClick = {
                if (!isScanning) {
                    CoroutineScope(Dispatchers.Main).launch {
                        scanResults = emptyList()
                        bluetoothViewModel.adapter.scanStart()
                    }
                } else {
                    CoroutineScope(Dispatchers.Main).launch {
                        bluetoothViewModel.adapter.scanStop()
                    }
                }
            },
            modifier = Modifier.padding(16.dp)
        ) {
            Text(text = if (isScanning) "Stop Scan" else "Start Scan")
        }


        selectedDevice?.let { peripheral ->
            Text(
                text = "Connecting to ${peripheral.identifier} [${peripheral.address}]",
                style = MaterialTheme.typography.body1,
                modifier = Modifier.padding(16.dp)
            )

            Button(
                onClick = {
                    if (!isConnected) {
                        CoroutineScope(Dispatchers.Main).launch {
                            peripheral.connect()
                        }
                    } else {
                        CoroutineScope(Dispatchers.Main).launch {
                            peripheral.disconnect()
                        }
                    }
                },
                modifier = Modifier.padding(16.dp)
            ) {
                Text(text = if (isConnected) "Disconnect" else "Connect")
            }

            if (isConnected) {
                Text(
                    text = "Successfully connected, printing services and characteristics..",
                    style = MaterialTheme.typography.body1,
                    modifier = Modifier.padding(16.dp)
                )

                selectedServiceCharacteristic?.let { servicecharacteristic ->
                    Button(
                        onClick = {
                            CoroutineScope(Dispatchers.Main).launch {
                                peripheral.notify(servicecharacteristic.first, servicecharacteristic.second).collect { it ->
                                    val hexString = it.joinToString(separator = " ") { "%02x".format(it) }
                                    Log.d("SimpleBLE", "Received notification: $hexString")
                                }
                            }
                            CoroutineScope(Dispatchers.Main).launch {
                                delay(10000)
                                peripheral.unsubscribe(servicecharacteristic.first, servicecharacteristic.second)
                            }
                        },
                        modifier = Modifier.padding(16.dp)
                    ) {
                        Text(text = "Subscribe to Notifications")
                    }

                    Button(
                        onClick = {
                            CoroutineScope(Dispatchers.Main).launch {
                                peripheral.read(servicecharacteristic.first, servicecharacteristic.second)
                            }
                        },
                        modifier = Modifier.padding(16.dp)
                    ) {
                        Text(text = "Read")
                    }

                    receivedData?.let { data ->
                        Text(
                            text = "Received: ${data.joinToString(separator = " ") { "%02x".format(it) }}",
                            style = MaterialTheme.typography.body1,
                            modifier = Modifier.padding(16.dp)
                        )
                    }
                }

                LazyColumn(
                    modifier = Modifier.fillMaxWidth(),
                    contentPadding = PaddingValues(16.dp)
                ) {
                    items(servicescharacteristics.withIndex().toList()) { (index, characteristic) ->
                        Text(
                            text = "[$index] ${characteristic.first} ${characteristic.second}",
                            style = TextStyle(
                                fontWeight = FontWeight.Normal,
                                fontSize = 8.sp,
                                lineHeight = 12.sp,
                                letterSpacing = 0.5.sp
                            ),
                            modifier = Modifier
                                .padding(8.dp)
                                .clickable {
                                    selectedServiceCharacteristic = characteristic
                                }
                        )
                    }
                }
            }
        }

        if (scanResults.isNotEmpty()) {
            Text(
                text = "The following devices were found:",
                style = MaterialTheme.typography.h6,
                modifier = Modifier.padding(16.dp)
            )

            LazyColumn(
                modifier = Modifier.fillMaxWidth(),
                contentPadding = PaddingValues(16.dp)
            ) {
                items(scanResults.withIndex().toList()) { (index, peripheral) ->
                    Text(
                        text = "[$index] ${peripheral.identifier} [${peripheral.address}]",
                        style = MaterialTheme.typography.body1,
                        modifier = Modifier
                            .padding(8.dp)
                            .clickable {
                                selectedDevice = peripheral
                            }
                    )
                }
            }
        } else {
            Text(
                text = "No devices found.",
                style = MaterialTheme.typography.body1,
                modifier = Modifier.padding(16.dp)
            )
        }
    }
}