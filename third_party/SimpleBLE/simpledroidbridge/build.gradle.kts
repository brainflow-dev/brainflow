plugins {
    id("com.android.library") version "8.7.1"
}

version = "v${file("../VERSION").readText().trim()}"

android {
    namespace = "org.simpleble.android.bridge"
    compileSdk = 31

    defaultConfig {
        minSdk = 31
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_1_9
        targetCompatibility = JavaVersion.VERSION_1_9
    }
    buildTypes {
        getByName("debug") {
            isJniDebuggable = true
        }
    }
}
