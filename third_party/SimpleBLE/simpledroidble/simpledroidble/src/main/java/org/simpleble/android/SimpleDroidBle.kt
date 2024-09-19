package org.simpleble.android

import android.Manifest
import android.app.Activity
import android.content.Context
import android.content.pm.PackageManager
import android.util.Log
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import java.lang.ref.WeakReference

class SimpleDroidBle {
    companion object {

        lateinit var contextReference: WeakReference<Context>
        private val context: Context
            get() {
                return contextReference.get()
                    ?: throw IllegalStateException("PermissionsManager: Permissions requested outside activity!")
            }
        private var hasPermissionBluetooth: Boolean = false
        private var hasPermissionBluetoothAdmin: Boolean = false
        private var hasPermissionBluetoothConnect: Boolean = false
        private var hasPermissionBluetoothScan: Boolean = false

        val hasPermissions: Boolean
            get() {
                hasPermissionBluetooth = isPermissionGranted(Manifest.permission.BLUETOOTH)
                hasPermissionBluetoothAdmin = isPermissionGranted(Manifest.permission.BLUETOOTH_ADMIN)
                hasPermissionBluetoothConnect = isPermissionGranted(Manifest.permission.BLUETOOTH_CONNECT)
                hasPermissionBluetoothScan = isPermissionGranted(Manifest.permission.BLUETOOTH_SCAN)

                return hasPermissionBluetooth &&
                        hasPermissionBluetoothAdmin &&
                        hasPermissionBluetoothConnect &&
                        hasPermissionBluetoothScan
            }

        init {
            System.loadLibrary("simpleble-jni")
        }

        fun requestPermissions() {
            if (!hasPermissions) {
                ActivityCompat.requestPermissions(
                    context as Activity,
                    arrayOf(
                        Manifest.permission.BLUETOOTH,
                        Manifest.permission.BLUETOOTH_ADMIN,
                        Manifest.permission.BLUETOOTH_CONNECT,
                        Manifest.permission.BLUETOOTH_SCAN
                    ),
                    1
                )
            }
        }

        fun handleOnRequestPermissionsResult(requestCode: Int, permissions: Array<out String>, grantResults: IntArray) {
            if (requestCode != 1) return

            val length = permissions.size
            if (length != grantResults.size) {
                Log.e("PermissionsManager", "Permissions and grant results are not the same size!")
                return
            }

            for (i in 0 until length) {
                val permission = permissions[i]
                val grantResult = grantResults[i]
                when (permission) {
                    Manifest.permission.BLUETOOTH -> {
                        hasPermissionBluetooth = grantResult == PackageManager.PERMISSION_GRANTED
                    }
                    Manifest.permission.BLUETOOTH_ADMIN -> {
                        hasPermissionBluetoothAdmin = grantResult == PackageManager.PERMISSION_GRANTED
                    }
                    Manifest.permission.BLUETOOTH_CONNECT -> {
                        hasPermissionBluetoothConnect = grantResult == PackageManager.PERMISSION_GRANTED
                    }
                    Manifest.permission.BLUETOOTH_SCAN -> {
                        hasPermissionBluetoothScan = grantResult == PackageManager.PERMISSION_GRANTED
                    }
                }
            }
        }


        @JvmStatic
        fun getVersion(): String {
            return "1.0.0"
        }

        private fun isPermissionGranted(permission: String): Boolean {
            return ContextCompat.checkSelfPermission(
                context,
                permission
            ) == PackageManager.PERMISSION_GRANTED
        }

    }
}