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

rootProject.name = "SimpleBLE Example"

// NOTE: This is somewhat of a hack to consume simpledroidble directly from the source code
includeBuild("../../simpledroidble") {
    dependencySubstitution {
        substitute(module("org.simpleble.android:simpledroidble")).using(project(":simpledroidble"))
    }
}
include(":app")