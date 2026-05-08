package org.simplejavable;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.StandardCopyOption;
import java.util.Locale;

public class NativeLibraryLoader {
    // Directory structure inside the JAR:
    // /native
    //   /windows
    //     /x86/library.dll
    //     /x64/library.dll
    //     /aarch64/library.dll
    //   /linux
    //     /x86/liblibrary.so
    //     /x64/liblibrary.so
    //     /aarch64/liblibrary.so
    //   /macos
    //     /x86/liblibrary.dylib
    //     /x64/liblibrary.dylib
    //     /aarch64/liblibrary.dylib

    private static final String NATIVE_FOLDER = "/native";

    public static void loadLibrary(String libraryName) throws IOException {
        String osName = getOperatingSystem();
        String archName = getArchitecture();
        String libExtension = getLibraryExtension(osName);

        // Construct the path to the native library inside the JAR
        String resourcePath = String.format("%s/%s/%s%s",
            NATIVE_FOLDER, archName,
            getLibraryPrefix(osName), libraryName + libExtension);

        // Create a temporary directory for extracting the native library
        Path tempDir = createTempDirectory();

        // Extract and load the library
        Path libraryPath = extractAndGetLibraryPath(resourcePath, tempDir, libraryName + libExtension);
        System.load(libraryPath.toAbsolutePath().toString());
    }

    private static String getOperatingSystem() {
        String os = System.getProperty("os.name").toLowerCase(Locale.ROOT);
        if (os.contains("windows")) {
            return "windows";
        } else if (os.contains("linux")) {
            return "linux";
        } else if (os.contains("mac")) {
            return "macos";
        }
        throw new UnsupportedOperationException("Unsupported operating system: " + os);
    }

    private static String getArchitecture() {
        String arch = System.getProperty("os.arch").toLowerCase(Locale.ROOT);
        if (arch.contains("amd64") || arch.contains("x86_64")) {
            return "x64";
        } else if (arch.contains("aarch64")) {
            return "aarch64";
        } else if (arch.contains("x86") || arch.contains("i386") || arch.contains("i686")) {
            return "x86";
        }
        throw new UnsupportedOperationException("Unsupported architecture: " + arch);
    }

    private static String getLibraryExtension(String os) {
        switch (os) {
            case "windows": return ".dll";
            case "linux": return ".so";
            case "macos": return ".dylib";
            default: throw new UnsupportedOperationException("Unsupported OS: " + os);
        }
    }

    private static String getLibraryPrefix(String os) {
        return (os.equals("linux") || os.equals("macos")) ? "lib" : "";
    }

    private static Path createTempDirectory() throws IOException {
        String tempDirName = "native-libs-" + System.nanoTime();
        Path tempDir = Files.createTempDirectory(tempDirName);
        tempDir.toFile().deleteOnExit();
        return tempDir;
    }

    private static Path extractAndGetLibraryPath(String resourcePath, Path tempDir, String fileName)
            throws IOException {
        Path targetPath = tempDir.resolve(fileName);

        // Extract the library file from the JAR
        try (InputStream in = NativeLibraryLoader.class.getResourceAsStream(resourcePath)) {
            if (in == null) {
                // List contents of the native folder. TODO: Remove this once we don't need it anymore.
                try (InputStream nativeFolder = NativeLibraryLoader.class.getResourceAsStream(NATIVE_FOLDER)) {
                    if (nativeFolder == null) {
                        System.out.println("Native folder not found in JAR!");
                    } else {
                        System.out.println("Native folder exists in JAR");

                        NativeLibraryLoader.class.getClassLoader().resources("")
                            .forEach(url -> System.out.println("Found resource: " + url));
                    }
                }

                throw new FileNotFoundException("Native library not found: " + resourcePath);
            }
            Files.copy(in, targetPath, StandardCopyOption.REPLACE_EXISTING);
        }

        return targetPath;
    }
}