import org.gradle.internal.os.OperatingSystem

plugins {
    id("java-library")
}

group = "org.simplejavable"
version = "v${file("../../VERSION").readText().trim()}"

repositories {
    mavenCentral()
}

java {
    toolchain {
        languageVersion.set(JavaLanguageVersion.of(17))
    }
}

// Native library acquisition options
val nativeLibPath: String? by project // -PnativeLibPath=...
val buildFromCMake: String? by project // -PbuildFromCMake (presence is what matters)

// Build native libraries using CMake
tasks.register<Exec>("generateCMake") {
    val cmakePath = "../cpp" // Default CMake location
    val cmakeBuildPath = layout.buildDirectory.dir("build_cpp").get().asFile
    workingDir(cmakePath)
    commandLine(
        "cmake",
        "-B", cmakeBuildPath.absolutePath,
        "-DCMAKE_BUILD_TYPE=Release"
    )
}

tasks.register<Exec>("buildNativeCMake") {
    dependsOn("generateCMake")
    val cmakePath = "../cpp" // Default CMake location
    val cmakeBuildPath = layout.buildDirectory.dir("build_cpp").get().asFile
    workingDir(cmakePath)
    commandLine(
        "cmake",
        "--build", cmakeBuildPath.absolutePath,
        "--config", "Release"
    )
}

// Add native libraries to jar based on the selected mode
tasks.jar {
    // TODO: Remove this once main class is not needed.
    manifest {
        attributes["Main-Class"] = "org.simplejavable.Main"
    }

    buildFromCMake?.let {
        // Build from CMake when explicitly requested
        dependsOn("buildNativeCMake")
        val cmakeBuildOutputPath = layout.buildDirectory.dir("build_cpp/lib").get().asFile
        val currentArch = System.getProperty("os.arch").let { arch ->
            when {
                arch.contains("amd64") || arch.contains("x86_64") -> "x64"
                arch.contains("aarch64") -> "aarch64"
                arch.contains("x86") || arch.contains("i386") || arch.contains("i686") -> "x86"
                else -> error("Unsupported architecture: $arch")
            }
        }
        from(cmakeBuildOutputPath) {
            include("**/*.so", "**/*.dll", "**/*.dylib")
            into("native/$currentArch")
        }
    } ?: nativeLibPath?.let { path ->
        // Use local path approach when CMake build not requested
        // Assumes the directory structure within 'path' matches the desired 'native/<arch>' structure
        val nativeLibDir = file(path)
        if (nativeLibDir.isDirectory) {
            from(nativeLibDir) {
                include("**/*.so", "**/*.dll", "**/*.dylib")
                into("native")
            }
        } else {
             logger.warn("nativeLibPath '$path' provided is not a directory. Cannot include native libraries.")
        }
    } ?: error("Please provide -PnativeLibPath or use -PbuildFromCMake to build from CMake")
}