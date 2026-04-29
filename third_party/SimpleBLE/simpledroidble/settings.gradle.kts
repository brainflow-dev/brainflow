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

includeBuild("../simpledroidbridge") {
    dependencySubstitution {
        substitute(module("org.simpleble.android.bridge:simpledroidbridge")).using(project(":"))
    }
}
include(":simpledroidble")
