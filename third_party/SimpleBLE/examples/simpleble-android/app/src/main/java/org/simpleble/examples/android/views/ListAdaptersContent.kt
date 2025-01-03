package org.simpleble.examples.android.views

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
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
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import org.simpleble.android.Adapter
import org.simpleble.android.SimpleDroidBle


@Composable
fun ListAdaptersContent() {
    var simpleBleVersion by remember { mutableStateOf("") }
    var bluetoothEnabled by remember { mutableStateOf(false) }
    var adapterList by remember { mutableStateOf(emptyList<Adapter>()) }

    LaunchedEffect(Unit) {
        simpleBleVersion = SimpleDroidBle.getVersion()
        bluetoothEnabled = Adapter.isBluetoothEnabled()
        adapterList = Adapter.getAdapters()
    }

    Column(
        modifier = Modifier.fillMaxSize(),
        horizontalAlignment = Alignment.CenterHorizontally,
        verticalArrangement = Arrangement.Center
    ) {
        Text(
            text = "Using SimpleBLE version: $simpleBleVersion",
            style = MaterialTheme.typography.h6,
            textAlign = TextAlign.Center,
            modifier = Modifier.padding(16.dp)
        )

        Text(
            text = "Bluetooth enabled: $bluetoothEnabled",
            style = MaterialTheme.typography.h6,
            textAlign = TextAlign.Center,
            modifier = Modifier.padding(16.dp)
        )

        if (adapterList.isEmpty()) {
            Text(
                text = "No adapter found",
                style = MaterialTheme.typography.h6,
                textAlign = TextAlign.Center,
                modifier = Modifier.padding(16.dp)
            )
        } else {
            Text(
                text = "Adapters:",
                style = MaterialTheme.typography.h6,
                textAlign = TextAlign.Center,
                modifier = Modifier.padding(16.dp)
            )

            LazyColumn(
                modifier = Modifier.fillMaxWidth(),
                contentPadding = PaddingValues(16.dp)
            ) {
                items(adapterList) { adapter ->
                    Text(
                        text = "Adapter: ${adapter.identifier} [${adapter.address}]",
                        style = MaterialTheme.typography.body1,
                        modifier = Modifier.padding(8.dp)
                    )
                }
            }
        }
    }
}