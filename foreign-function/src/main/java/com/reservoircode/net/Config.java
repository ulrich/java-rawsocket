package com.reservoircode.net;

import java.io.File;
import java.io.InputStream;
import java.lang.foreign.FunctionDescriptor;
import java.lang.foreign.ValueLayout;
import java.nio.file.Files;
import java.nio.file.StandardCopyOption;

public record Config() {

    private static final String NATIVE_LIBRARY_FILE = "rawsockets.so";
    private static final String NATIVE_FUNCTION_NAME = "socket_tester";
    private static final FunctionDescriptor NATIVE_FUNCTION_DESCRIPTOR = FunctionDescriptor.of(
            ValueLayout.JAVA_INT,
            ValueLayout.ADDRESS,
            ValueLayout.ADDRESS,
            ValueLayout.ADDRESS,
            ValueLayout.ADDRESS,
            ValueLayout.ADDRESS);

    private static final String NATIVE_LIBRARY_FILE_PATH;

    static {
        try {
            File tempFile = File.createTempFile(NATIVE_LIBRARY_FILE, "_temp");

            try (InputStream is = SocketTester.class.getClassLoader().getResourceAsStream(NATIVE_LIBRARY_FILE)) {
                if (is == null) {
                    System.err.println("Native library not found: " + NATIVE_LIBRARY_FILE);
                    throw new RuntimeException();
                }
                Files.copy(is, tempFile.toPath(), StandardCopyOption.REPLACE_EXISTING);
            }
            NATIVE_LIBRARY_FILE_PATH = tempFile.getAbsolutePath();
        } catch (Exception e) {
            System.err.println("Unable to load native library: " + NATIVE_LIBRARY_FILE);
            throw new RuntimeException();
        }
    }

    public static String getNativeLibraryFile() {
        return NATIVE_LIBRARY_FILE_PATH;
    }

    public static String getNativeFunctionName() {
        return NATIVE_FUNCTION_NAME;
    }

    public static FunctionDescriptor getNativeFunctionDescriptor() {
        return NATIVE_FUNCTION_DESCRIPTOR;
    }
}
