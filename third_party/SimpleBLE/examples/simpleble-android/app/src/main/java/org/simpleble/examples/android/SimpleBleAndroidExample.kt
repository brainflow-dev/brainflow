package org.simpleble.examples.android

import android.app.Application
import android.util.Log

class SimpleBleAndroidExample : Application() {

    override fun onCreate() {
        Log.d("SimpleBleAndroidExample", "onCreate()")
        super.onCreate()
    }

    override fun onTerminate() {
        Log.d("SimpleBleAndroidExample", "onTerminate()")
        super.onTerminate()
    }
}