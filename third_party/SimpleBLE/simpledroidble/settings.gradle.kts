pluginManagement {
    repositories {
        google {
            content {
                includeGroupByRegex("com\\.android.*")
                includeGroupByRegex("com\\.google.*")
                includeGroupByRegex("androidx.*")
            }
        }
        mavenCentral()
        gradlePluginPortal()
    }
}
dependencyResolutionManagement {
    repositoriesMode.set(RepositoriesMode.FAIL_ON_PROJECT_REPOS)
    repositories {
        google()
        mavenCentral()
    }
}

rootProject.name = "SimpleDroidBle"

includeBuild("../simpleble/src/backends/android/simpleble-bridge") {
    dependencySubstitution {
        substitute(module("org.simpleble.android.bridge:simpleble-bridge")).using(project(":"))
    }
}
include(":simpledroidble")
