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
import androidx.compose.ui.unit.dp
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.collectLatest
import kotlinx.coroutines.launch
import org.simpleble.android.Adapter
import org.simpleble.android.BluetoothUUID
import org.simpleble.android.Peripheral
import org.simpleble.examples.android.viewmodels.BluetoothViewModel


@Composable
fun ConnectContent(bluetoothViewModel: BluetoothViewModel) {
    var scanResults by remember { mutableStateOf(emptyList<Peripheral>()) }
    var isScanning by remember { mutableStateOf(false) }

    var selectedDevice by remember { mutableStateOf<Peripheral?>(null) }
    var isConnected by remember { mutableStateOf(false) }
    var mtu by remember { mutableStateOf(0) }

    LaunchedEffect(Unit, selectedDevice) {
        CoroutineScope(Dispatchers.Main).launch {
            bluetoothViewModel.adapter.onScanActive.collect {
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

                    mtu = if (active) {
                        peripheral.mtu
                    } else {
                        0
                    }
                }
            }

            CoroutineScope(Dispatchers.Main).launch {
                peripheral.onConnected.collectLatest {
                    Log.d("SimpleBLE", "Connected to ${peripheral.identifier} [${peripheral.address}]")
                }
            }

            CoroutineScope(Dispatchers.Main).launch {
                peripheral.onDisconnected.collectLatest {
                    Log.d("SimpleBLE", "Disconnected from ${peripheral.identifier} [${peripheral.address}]")
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
                    text = "Successfully connected.",
                    style = MaterialTheme.typography.body1,
                    modifier = Modifier.padding(16.dp)
                )

                Text(
                    text = "MTU: ${peripheral.mtu}",
                    style = MaterialTheme.typography.body1,
                    modifier = Modifier.padding(16.dp)
                )

                LazyColumn(
                    modifier = Modifier.fillMaxWidth(),
                    contentPadding = PaddingValues(8.dp)
                ) {
                    items(peripheral.services().withIndex().toList()) { (index, service) ->
                        Text(
                            text = "Service: ${service.uuid}",
                            style = MaterialTheme.typography.body1,
                            modifier = Modifier.padding(16.dp)
                        )

                        service.characteristics.forEach { characteristic ->
                            Text(
                                text = "Characteristic: ${characteristic.uuid}",
                                style = MaterialTheme.typography.body2,
                                modifier = Modifier.padding(start = 32.dp)
                            )

//                        Text(
//                            text = "Capabilities: ${characteristic.capabilities.joinToString(", ")}",
//                            style = MaterialTheme.typography.body2,
//                            modifier = Modifier.padding(start = 32.dp)
//                        )

                            characteristic.descriptors.forEach { descriptor ->
                                Text(
                                    text = "Descriptor: ${descriptor.uuid}",
                                    style = MaterialTheme.typography.body2,
                                    modifier = Modifier.padding(start = 48.dp)
                                )
                            }
                        }
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
                contentPadding = PaddingValues(8.dp)
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