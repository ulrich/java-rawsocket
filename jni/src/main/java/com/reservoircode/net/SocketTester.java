package com.reservoircode.net;

import java.io.File;
import java.io.InputStream;
import java.nio.file.Files;
import java.nio.file.StandardCopyOption;

public class SocketTester {

    private static final String NATIVE_LIBRARY = "libSocketTester.so";

    static {
        try {
            File tempFile = File.createTempFile("libSocketTester", ".so");
            try (InputStream is = SocketTester.class.getClassLoader().getResourceAsStream(NATIVE_LIBRARY)) {
                if (is == null) {
                    throw new IllegalStateException("Native library not found: " + NATIVE_LIBRARY);
                }
                Files.copy(is, tempFile.toPath(), StandardCopyOption.REPLACE_EXISTING);
            }
            System.load(tempFile.getAbsolutePath());
        } catch (Exception e) {
            throw new IllegalStateException("Unable to load native library: " + NATIVE_LIBRARY);
        }
    }

    public native int socketTest(final String srcIp, final String dstIp, final int port, final int readTimeoutMs, final int writeTimeoutMs);

    private final String srcIp;
    private final String dstIp;
    private final int port;
    private final int readTimeoutMs;
    private final int writeTimeoutMs;

    public SocketTester(final String srcIp, final String dstIp, final int port, final int readTimeoutMs, final int writeTimeoutMs) {
        this.srcIp = srcIp;
        this.dstIp = dstIp;
        this.port = port;
        this.readTimeoutMs = readTimeoutMs;
        this.writeTimeoutMs = writeTimeoutMs;
    }

    public int run() {
        System.out.format("[INFO (J)] Testing %s:%s...\n", dstIp, port);

        final int result = new SocketTester(srcIp, dstIp, port, readTimeoutMs, writeTimeoutMs).socketTest(srcIp, dstIp, port, readTimeoutMs, writeTimeoutMs);

        switch (result) {
            case 0:
                System.out.println("[INFO (J)] Connection successful");
                return 0;
            case 1:
                System.out.println("[INFO (J)] Connection failed");
                return 1;
            case 2:
                System.out.println("[INFO (J)] Connection timeout");
                return 2;
            default:
                throw new IllegalStateException("Unknown return type.");
        }
    }

    public static void main(final String[] args) {
        if (args != null && args.length == 5) {
            new SocketTester(
                args[0],
                args[1],
                Integer.parseInt(args[2]),
                Integer.parseInt(args[3]),
                Integer.parseInt(args[4])).run();
            return;
        }
        new SocketTester(
            "127.0.0.1",
            "127.0.0.1",
            8000,
            100,
            100).run();
    }
}
