package com.reservoircode.net;

import com.sun.net.httpserver.HttpServer;
import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.Disabled;
import org.junit.jupiter.api.Test;
import com.reservoircode.net.SocketTester;

import java.io.IOException;
import java.net.InetSocketAddress;

// Unable to run without root rights, see docker compose file
// Give a try with Testcontainers (https://www.testcontainers.org/) project

@Disabled
class SocketTesterTest {

    @Test
    void shouldRawSocketToWebServer() throws IOException {
        final String srcIp = "127.0.0.1";
        final String dstIp = "127.0.0.1";
        final int port = 8000;
        final int readTimeoutMs = 100;
        final int writeTimeoutMs = 100;

        HttpServer.create(new InetSocketAddress(srcIp, port), 0);

        int result = new SocketTester(srcIp, dstIp, port, readTimeoutMs, writeTimeoutMs).run();

        Assertions.assertEquals(0, result);
    }

}