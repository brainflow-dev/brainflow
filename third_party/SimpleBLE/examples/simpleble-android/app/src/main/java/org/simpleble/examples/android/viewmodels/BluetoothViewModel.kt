package org.simpleble.examples.android.viewmodels

import androidx.compose.runtime.mutableStateOf
import androidx.lifecycle.ViewModel
import org.simpleble.android.Adapter

class BluetoothViewModel : ViewModel() {
    private lateinit var _adapter: Adapter
    val adapter: Adapter
        get() {
            if (!::_adapter.isInitialized) {
                _adapter = Adapter.getAdapters()[0]
            }
            return _adapter
        }
}