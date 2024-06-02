package com.reservoircode.net;

import lombok.extern.slf4j.Slf4j;
import picocli.CommandLine;

import java.util.concurrent.Callable;

@Slf4j
@CommandLine.Command(
        name = "Socket Tester",
        version = "1.0",
        mixinStandardHelpOptions = true,
        description = "Open a TCP Half Open connection again the given address.")
public class Main implements Callable<Integer> {

    @CommandLine.Option(names = {"-s", "--source"}, description = "The source address", showDefaultValue = CommandLine.Help.Visibility.ALWAYS)
    private String sourceAddress = "127.0.0.1";

    @CommandLine.Option(names = {"-d", "--destination"}, description = "The destination address", showDefaultValue = CommandLine.Help.Visibility.ALWAYS)
    private String destinationAddress = "127.0.0.1";

    @CommandLine.Option(names = {"-dp", "--destination-port"}, description = "The destination port", showDefaultValue = CommandLine.Help.Visibility.ALWAYS)
    private int destinationPort = 8080;

    @CommandLine.Option(names = {"-rto", "--read-timout"}, description = "The read timeout (ms)", showDefaultValue = CommandLine.Help.Visibility.ALWAYS)
    private int readTimeout = 100;

    @CommandLine.Option(names = {"-wto", "--write-timout"}, description = "The write timeout (ms)", showDefaultValue = CommandLine.Help.Visibility.ALWAYS)
    private int writeTimeout = 100;

    @Override
    public Integer call() {
        try {
            return new SocketTester(
                    sourceAddress,
                    destinationAddress,
                    destinationPort,
                    readTimeout,
                    writeTimeout).run();
        } catch (Throwable e) {
            log.error("Failed to run SocketTester caused by: {}", e.getMessage());
            return -1;
        }
    }

    public static void main(String... args) {
        System.exit(new CommandLine(new Main())
                .execute(args));
    }
}
