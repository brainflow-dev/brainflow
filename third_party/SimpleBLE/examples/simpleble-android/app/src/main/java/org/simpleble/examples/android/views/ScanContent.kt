package org.simpleble.examples.android.views

import android.util.Log
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
import kotlinx.coroutines.launch
import org.simpleble.android.Adapter
import org.simpleble.android.Peripheral
import org.simpleble.examples.android.viewmodels.BluetoothViewModel


@Composable
fun ScanContent(bluetoothViewModel: BluetoothViewModel) {
    var scanActive by remember { mutableStateOf(false) }
    var scanResults by remember { mutableStateOf(emptyList<Peripheral>()) }

    LaunchedEffect(Unit) {
        CoroutineScope(Dispatchers.Main).launch {
            bluetoothViewModel.adapter.onScanStart.collect {
                Log.d("SimpleBLE", "Scan started.")
            }
        }

        CoroutineScope(Dispatchers.Main).launch {
            bluetoothViewModel.adapter.onScanStop.collect {
                Log.d("SimpleBLE", "Scan stopped.")
            }
        }

        CoroutineScope(Dispatchers.Main).launch {
            bluetoothViewModel.adapter.onScanActive.collect {
                Log.d("SimpleBLE", "Scan active: $it")
                scanActive = it
            }
        }

        CoroutineScope(Dispatchers.Main).launch {
            bluetoothViewModel.adapter.onScanFound.collect {
                Log.d("SimpleBLE", "Found device: ${it.identifier} [${it.address}] ${it.rssi} dBm ${it.hashCode()}")
                scanResults = scanResults + it
            }
        }

        CoroutineScope(Dispatchers.Main).launch {
            bluetoothViewModel.adapter.onScanUpdated.collect {
                Log.d("SimpleBLE", "Updated device: ${it.identifier} [${it.address}] ${it.rssi} dBm ${it.hashCode()}")
            }
        }
    }

    Column(
        modifier = Modifier.fillMaxSize(),
        horizontalAlignment = Alignment.CenterHorizontally,
        verticalArrangement = Arrangement.Center
    ) {
        Button(
            onClick = {
                if (!bluetoothViewModel.adapter.scanIsActive) {
                    CoroutineScope(Dispatchers.Main).launch {
                        scanResults = emptyList()
                        bluetoothViewModel.adapter.scanFor(5000)
                    }
                }
            },
            modifier = Modifier.padding(16.dp)
        ) {
            Text(text = if (scanActive) "Scanning..." else "Start Scan")
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
                    val connectableString = if (peripheral.isConnectable) "Connectable" else "Non-Connectable"
                    Text(
                        text = "[$index] ${peripheral.identifier} [${peripheral.address}] ${peripheral.rssi} dBm $connectableString",
                        style = MaterialTheme.typography.body1,
                        modifier = Modifier.padding(8.dp)
                    )
                    Text(
                        text = "Tx Power: ${peripheral.txPower} dBm",
                        style = MaterialTheme.typography.body2,
                        modifier = Modifier.padding(start = 16.dp)
                    )
                    Text(
                        text = "Address Type: ${peripheral.addressType}",
                        style = MaterialTheme.typography.body2,
                        modifier = Modifier.padding(start = 16.dp)
                    )

//                    peripheral.services.forEach { service ->
//                        Text(
//                            text = "Service UUID: ${service.uuid}",
//                            style = MaterialTheme.typography.body2,
//                            modifier = Modifier.padding(start = 16.dp)
//                        )
//                        Text(
//                            text = "Service data: ${service.data}",
//                            style = MaterialTheme.typography.body2,
//                            modifier = Modifier.padding(start = 16.dp)
//                        )
//                    }
//
//                    peripheral.manufacturerData.forEach { (manufacturerId, data) ->
//                        Text(
//                            text = "Manufacturer ID: $manufacturerId",
//                            style = MaterialTheme.typography.body2,
//                            modifier = Modifier.padding(start = 16.dp)
//                        )
//                        Text(
//                            text = "Manufacturer data: $data",
//                            style = MaterialTheme.typography.body2,
//                            modifier = Modifier.padding(start = 16.dp)
//                        )
//                    }
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