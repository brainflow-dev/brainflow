import org.gradle.api.tasks.bundling.Jar

plugins {
    java
    application
}

group = "org.simplejavable.examples"
version = rootProject.version

repositories {
    mavenCentral()
}

dependencies {
    implementation(project(":java"))
}

java {
    toolchain {
        languageVersion = JavaLanguageVersion.of(17)
    }
}

val exampleClasses = listOf(
    "connect.ConnectExample",
    //"list_adapters.ListAdaptersExample",
    //"list_adapters_safe.ListAdaptersSafeExample",
    //"multiconnect.MultiConnectExample",
    "notify.NotifyExample",
    //"notify_multi.NotifyMultiExample",
    //"read.ReadExample",
    "scan.ScanExample",
    //"write.WriteExample"
)

// Create a JAR for each example, including the contents of simplejavable-v0.9.1.jar
exampleClasses.forEach { mainClass ->
    tasks.register<Jar>("jar${mainClass.split('.').last()}") {
        archiveClassifier.set(mainClass.split('.').last().lowercase())
        manifest {
            attributes["Main-Class"] = "org.simplejavable.examples.$mainClass"
        }
        from(sourceSets.main.get().output) // Include example's compiled classes
        // Include the contents of simplejavable-v0.9.1.jar with debug logging
        doFirst {
            println("Runtime classpath JARs for task ':${name}':")
            configurations.runtimeClasspath.get().files.forEach { file ->
                println("Found JAR: $file")
            }
        }
        val simplejavableJars = configurations.runtimeClasspath.get()
            .filter { it.name.contains("simplejavable") || it.name.contains("java-") }
        simplejavableJars.forEach { jar ->
            println("Unzipping JAR: $jar")
        }
        from(simplejavableJars.map { zipTree(it) }) // Include only once
        // Handle duplicate classes and resources
        duplicatesStrategy = DuplicatesStrategy.INCLUDE
        dependsOn(tasks.named("classes"))
    }
}

tasks.register("buildAllJars") {
    dependsOn(exampleClasses.map { "jar${it.split('.').last()}" })
}