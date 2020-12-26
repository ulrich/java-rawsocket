# java-rawsocket

**java-rawsocket** is a helper aims for creating TCP Half Open connection in Java. It's based on C library linked by JNI.

## Overview

> For **more details** see [TCP_half-open article](https://en.wikipedia.org/wiki/TCP_half-open).

### Build

To build the project, execute this following command in your terminal (just for building the JAR file without executing test):
```bash
$ mvn clean compile jar:jar
```

The jar file contains the class and the compiled library.

> During the Maven build, the POM handles several plugins for executing the following actions:
> - Compile the SocketTester class
> - Generate the header file based necessary for JNI (com_reservoircode_net_SocketTester.h)
> - Run the GCC compiler to create the native library (libSocketTester.so)
> - Package the class and library in JAR file

### Run

To execute the SocketTester, you can use the *SocketTester#main* with hardcoded values:
- Source IP: 127.0.0.1
- Destination IP: 127.0.0.1
- Destination PORT: 8000
- Read timeout (ms): 100
- Write timeout (ms): 100

Start a Web server to simulate the endpoint. As example:
```bash
$ python3 -m http.server
```

Start the test:
```bash
$ sudo java -jar ./target/java-rawsocket.jar
```

Out example:
```bash
❯ sudo java -jar ./target/java-rawsocket.jar
[INFO (J)] Testing 127.0.0.1:8000...
[INFO (C)] Starting connection with following parameters. Source IP: 127.0.0.1, destination IP: 127.0.0.1, destination port: 8000, read timeout: 100, write timeout: 100
[INFO (C)] Selected source port number: 57916
[INFO (C)] Successfully sent 60 bytes SYN!
[INFO (C)] Received bytes: 48
[INFO (C)] Destination port: 57916
[INFO (C)] Successfully received 48 bytes
[INFO (C)] Received: syn= 1, ack= 1, rst= 0
[INFO (C)] SYN ACK received -> Success
[INFO (J)] Connection successful
```

On the output console you can see Java ([INFO (J)]) and C logs ([INFO (C)]).

> If you encounter some trouble during the test related to privilege issues, we have to execute the following command:
```bash
$ cd target
$ unzip java-rawsocket.jar
$ sudo setcap cap_net_raw+ep libSocketTester.so
```

### Testing (in progress)

An easy way to test the rawsocket helper with superuser rights is to run it in Docker environment. A start of Docker Compose project is located in the *docker* folder.

You can run the Docker Compose stack:
```bash
$ cd docker
$ mvn clean compile jar:jar -f ../pom.xml \
    && cp ../target/java-rawsocket.jar . \
    && docker build -t rawsocket . \
    && docker-compose up
```

In the IDE context we could test *Testcontainers* (https://www.testcontainers.org/).
