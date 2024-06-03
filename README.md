## Overview

**java-rawsocket** project is a helper aims for creating <u>TCP Half Open Connection</u> or <u>Embryonic Connection</u>
wrote in Java.

There are two implementations available. The first one is based on
the [JNI](https://docs.oracle.com/javase/8/docs/technotes/guides/jni/spec/jniTOC.html) (Java Native Interface) and the
second one is based on
the [Foreign Function](https://docs.oracle.com/en/java/javase/22/core/foreign-function-and-memory-api.html).

ℹ️ The used C code wasn't written by me, but it's plan I shall rewrite it from scratch.

## The Foreign Function implementation

### Build

Requirement : you have to install the Java 22 SDK at minimum to build and execute the socket-tester with the Foreign
Function
feature.

Afterward, go to the [foreign-function](foreign-function) folder and execute the following command in your terminal:

```bash
❯ mvn clean package
```

### Run

Start a Web server to simulate the endpoint. As example:

```bash
❯ python3 -m http.server
```

Display the options:

```bash
❯ sudo ~/.sdkman/candidates/java/22.0.1-oracle/bin/java \
        --enable-preview \
        --enable-native-access=ALL-UNNAMED \
        -jar ./target/foreign-function-jar-with-dependencies.jar --help
```

Start the test:

```bash
❯ sudo ~/.sdkman/candidates/java/22.0.1-oracle/bin/java \                                                                                                            
        --enable-preview \                                                                                           
        --enable-native-access=ALL-UNNAMED \                                                               
        ./target/java-rawsocket-ff-jar-with-dependencies.jar
```

Out example:

```bash
❯ sudo ~/.sdkman/candidates/java/22.0.1-oracle/bin/java \
        --enable-preview \
        --enable-native-access=ALL-UNNAMED \
        -jar ./target/foreign-function-jar-with-dependencies.jar       
19:20:47.774 [main] INFO com.reservoircode.net.SocketTester -- Running SocketTester for destination address 127.0.0.1:8080
[INFO (native)] Selected source port number: 35940
[INFO (native)] TCP header sequence number: 272214228
[INFO (native)] Successfully sent 60 bytes SYN!
[INFO (native)] Received bytes: 40
[INFO (native)] Destination port: 35940
[INFO (native)] Successfully received 40 bytes
[INFO (native)] Received syn: 0, ack: 1, rst: 1
[INFO (native)] TCP header sequence number response: 0
[INFO (native)] TCP header ack sequence number response: 272214229
[INFO (native)] tcph->syn: 0
[INFO (native)] tcph->ack: 16777216
[INFO (native)] SYN ACK received -> Success
```

## The JNI implementation

### Build

Go to the [jni](jni) folder and execute the following command in your terminal:

```bash
❯ mvn clean compile jar:jar
```

The jar file contains the class and the compiled library.

> During the Maven build, the POM handles several plugins for executing the following actions:
> - Compile the SocketTester class
> - Generate the header file based necessary for JNI (com_reservoircode_net_SocketTester.h)
> - Run the GCC compiler to create the native library (libSocketTester.so)
> - Package the class and library in JAR file

### Run

Start a Web server to simulate the endpoint. As example:

```bash
❯ python3 -m http.server
```

Start the test:

```bash
❯ sudo java -jar ./target/java-rawsocket-jni.jar
```

The default values used in the [SocketTester](jni/src/main/java/com/reservoircode/net/SocketTester.java) are:

- Source IP: 127.0.0.1
- Destination IP: 127.0.0.1
- Destination PORT: 8000
- Read timeout (ms): 100
- Write timeout (ms): 100

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

### Troubleshooting

> If you encounter some trouble during the test related to privilege issues, we have to execute the following command:

```bash
❯ cd target
❯ unzip java-rawsocket.jar
❯ sudo setcap cap_net_raw+ep libSocketTester.so
```
