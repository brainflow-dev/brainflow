package org.simpleble.examples.android.activities;

import android.Manifest
import android.annotation.SuppressLint
import android.content.pm.PackageManager
import android.os.Build
import android.os.Bundle
import android.util.Log
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.padding
import androidx.compose.material.BottomNavigation
import androidx.compose.material.BottomNavigationItem
import androidx.compose.material.Button
import androidx.compose.material.Icon
import androidx.compose.material.MaterialTheme
import androidx.compose.material.Scaffold
import androidx.compose.material.Surface
import androidx.compose.material.Text
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.AccountBox
import androidx.compose.material.icons.filled.Info
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.core.app.ActivityCompat
import org.simpleble.android.Adapter
import org.simpleble.android.SimpleDroidBle
import org.simpleble.examples.android.viewmodels.BluetoothViewModel
import org.simpleble.examples.android.views.ConnectContent
import org.simpleble.examples.android.views.ListAdaptersContent
import org.simpleble.examples.android.views.NotifyContent
import org.simpleble.examples.android.views.ReadContent
import org.simpleble.examples.android.views.ScanContent
import java.lang.ref.WeakReference

class MainActivity : ComponentActivity() {
    private val bluetoothHasPermissions = mutableStateOf(false)
    private val bluetoothViewModel = BluetoothViewModel()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        // Set the activity reference for SimpleDroidBle. This is required for requesting permissions.
        // NOTE: This is a bit of a hacky way to do this, but it's the only way to do it for now.
        SimpleDroidBle.contextReference = WeakReference(this)
        bluetoothHasPermissions.value = SimpleDroidBle.hasPermissions

        setContent {
            MaterialTheme {
                Surface {
                    if (bluetoothHasPermissions.value) {
                        ExampleView(bluetoothViewModel)
                    } else {
                        Column {
                            Button(onClick = { SimpleDroidBle.requestPermissions() }) {
                                Text("Request Bluetooth Permissions")
                            }
                            Text("Bluetooth permissions are required to use this app")
                            Text("Please grant the permissions and restart the app")
                        }
                    }
                }
            }
        }
    }

    override fun onRequestPermissionsResult(requestCode: Int, permissions: Array<out String>, grantResults: IntArray) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        SimpleDroidBle.handleOnRequestPermissionsResult(requestCode, permissions, grantResults)
        bluetoothHasPermissions.value = SimpleDroidBle.hasPermissions
    }
}



@Composable
fun ExampleView(bluetoothViewModel: BluetoothViewModel) {
    var selectedTab by remember { mutableIntStateOf(0) }

    Scaffold(
        bottomBar = {
            BottomNavigation {
                BottomNavigationItem(
                    label = { Text("Adapter") },
                    icon = { Icon(Icons.Default.Info, contentDescription = null) },
                    selected = selectedTab == 0,
                    onClick = { selectedTab = 0 }
                )
                BottomNavigationItem(
                    label = { Text("Scan") },
                    icon = { Icon(Icons.Default.AccountBox, contentDescription = null) },
                    selected = selectedTab == 1,
                    onClick = { selectedTab = 1 }
                )
                BottomNavigationItem(
                    label = { Text("Connect") },
                    icon = { Icon(Icons.Default.AccountBox, contentDescription = null) },
                    selected = selectedTab == 2,
                    onClick = { selectedTab = 2 }
                )
                BottomNavigationItem(
                    label = { Text("Read") },
                    icon = { Icon(Icons.Default.AccountBox, contentDescription = null) },
                    selected = selectedTab == 3,
                    onClick = { selectedTab = 3 }
                )
                BottomNavigationItem(
                    label = { Text("Notify") },
                    icon = { Icon(Icons.Default.AccountBox, contentDescription = null) },
                    selected = selectedTab == 4,
                    onClick = { selectedTab = 4 }
                )
            }
        }
    ) { innerPadding ->
        Column(
            modifier = Modifier.padding(innerPadding)
        ) {
            when (selectedTab) {
                0 -> ListAdaptersContent()
                1 -> ScanContent(bluetoothViewModel)
                2 -> ConnectContent(bluetoothViewModel)
                3 -> ReadContent(bluetoothViewModel)
                4 -> NotifyContent(bluetoothViewModel)
                else -> ListAdaptersContent() // Default
            }
        }
    }
}

