package com.reservoircode.net;

import lombok.extern.slf4j.Slf4j;

import java.lang.foreign.Arena;
import java.lang.foreign.Linker;
import java.lang.foreign.MemorySegment;
import java.lang.foreign.SymbolLookup;
import java.lang.foreign.ValueLayout;
import java.lang.invoke.MethodHandle;

@Slf4j
public class SocketTester {
    private final String sourceAddress;
    private final String destinationAddress;
    private final int destinationPort;
    private final int readTimeout;
    private final int writeTimeout;

    public SocketTester(
            String sourceAddress,
            String destinationAddress,
            int destinationPort,
            int readTimeout,
            int writeTimeout
    ) {
        this.sourceAddress = sourceAddress;
        this.destinationAddress = destinationAddress;
        this.destinationPort = destinationPort;
        this.readTimeout = readTimeout;
        this.writeTimeout = writeTimeout;
    }

    public int run() throws Throwable {
        log.info("Running SocketTester for destination address {}:{}", destinationAddress, destinationPort);

        try (Arena confinedArena = Arena.ofConfined()) {
            SymbolLookup symbolLookup =
                    SymbolLookup.libraryLookup(Config.getNativeLibraryFile(), confinedArena);

            MemorySegment function =
                    symbolLookup.find(Config.getNativeFunctionName())
                            .orElseThrow(() -> new IllegalStateException("Unable to find the native function: " + Config.getNativeFunctionName()));

            MethodHandle methodHandle = Linker.nativeLinker()
                    .downcallHandle(
                            function,
                            Config.getNativeFunctionDescriptor());

            return (int) methodHandle.invoke(
                    confinedArena.allocateFrom(sourceAddress),
                    confinedArena.allocateFrom(destinationAddress),
                    confinedArena.allocateFrom(ValueLayout.OfInt.JAVA_INT, destinationPort),
                    confinedArena.allocateFrom(ValueLayout.OfInt.JAVA_INT, readTimeout),
                    confinedArena.allocateFrom(ValueLayout.OfInt.JAVA_INT, writeTimeout));
        }
    }
}
