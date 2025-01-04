=====
Usage
=====

SimpleDroidBLE is an Android-specific package that provides an API similar to
SimpleBLE, using modern Kotlin idiomatic code.

This code is currently under active development and some features are not yet
implemented or their API might change, but should be enough to help you get
started.

Consuming Locally
=================

If you want to use SimpleDroidBLE as part of your project from a local copy,
you can do so by adding the following to your `settings.gradle` or `settings.gradle.kts`file.
Make sure this include is before your `include(":app")` statement.

```groovy
includeBuild("path/to/simpledroidble") {
    dependencySubstitution {
        substitute module("org.simpleble.android:simpledroidble") with project(":simpledroidble")
    }
}

```kotlin
includeBuild("path/to/simpledroidble") {
    dependencySubstitution {
        substitute(module("org.simpleble.android:simpledroidble")).using(project(":simpledroidble"))
    }
}
```

Then, inside your `build.gradle` or `build.gradle.kts` file, you can add the
following dependency:

```groovy
dependencies {
    implementation "org.simpleble.android:simpledroidble"
}
```

```kotlin
dependencies {
    implementation("org.simpleble.android:simpledroidble")
}
```