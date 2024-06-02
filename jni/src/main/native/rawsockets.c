// java-rawsocket
#include "com_reservoircode_net_SocketTester.h"

// java-rawsocket
#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <sys/time.h>

#define SUCCESS 0
#define FAIL 1
#define TIMEOUT 2

// pseudo header needed for tcp header checksum calculation
struct pseudo_header {
    u_int32_t source_address;
    u_int32_t dest_address;
    u_int8_t placeholder;
    u_int8_t protocol;
    u_int16_t tcp_length;
};

#define DATAGRAM_LEN 4096
#define OPT_SIZE 20

// java-rawsocket
int socketTest(const char *srcIp, const char *dstIp, int dstPort, int readTimeoutMs, int writeTimeoutMs);

unsigned short checksum(const char *buf, unsigned size) {
    unsigned sum = 0, i;

    // accumulate checksum
    for (i = 0; i < size - 1; i += 2) {
        unsigned short word16 = *(unsigned short *) &buf[i];
        sum += word16;
    }
    // handle odd-sized case
    if (size & 1) {
        unsigned short word16 = (unsigned char) buf[i];
        sum += word16;
    }
    // fold to get the ones-complement result
    while (sum >> 16) sum = (sum & 0xFFFF) + (sum >> 16);

    // invert to get the negative in ones-complement arithmetic
    return ~sum;
}

void create_syn_packet(
        struct sockaddr_in *src,
        struct sockaddr_in *dst,
        char **out_packet,
        int *out_packet_len
) {
    // datagram to represent the packet
    char *datagram = calloc(DATAGRAM_LEN, sizeof(char));

    // required structs for IP and TCP header
    struct iphdr *iph = (struct iphdr *) datagram;
    struct tcphdr *tcph = (struct tcphdr *) (datagram + sizeof(struct iphdr));
    struct pseudo_header psh;

    // IP header configuration
    iph->ihl = 5;
    iph->version = 4;
    iph->tos = 0;
    iph->tot_len = sizeof(struct iphdr) + sizeof(struct tcphdr) + OPT_SIZE;
    iph->id = htonl(rand() % 65535); // id of this packet
    iph->frag_off = 0;
    iph->ttl = 64;
    iph->protocol = IPPROTO_TCP;
    iph->check = 0; // correct calculation follows later
    iph->saddr = src->sin_addr.s_addr;
    iph->daddr = dst->sin_addr.s_addr;

    // TCP header configuration
    tcph->source = src->sin_port;
    tcph->dest = dst->sin_port;
    tcph->seq = htonl(rand() % 4294967295);
    tcph->ack_seq = htonl(0);
    tcph->doff = 10; // tcp header size
    tcph->fin = 0;
    tcph->syn = 1;
    tcph->rst = 0;
    tcph->psh = 0;
    tcph->ack = 0;
    tcph->urg = 0;
    tcph->check = 0; // correct calculation follows later
    tcph->window = htons(5840); // window size
    tcph->urg_ptr = 0;

    printf("[INFO (C)] TCP header sequence number: %u\n", ntohl(tcph->seq));

    // TCP pseudo header for checksum calculation
    psh.source_address = src->sin_addr.s_addr;
    psh.dest_address = dst->sin_addr.s_addr;
    psh.placeholder = 0;
    psh.protocol = IPPROTO_TCP;
    psh.tcp_length = htons(sizeof(struct tcphdr) + OPT_SIZE);
    int psize = sizeof(struct pseudo_header) + sizeof(struct tcphdr) + OPT_SIZE;
    // fill pseudo packet
    char *pseudogram = malloc(psize);
    memcpy(pseudogram, (char *) &psh, sizeof(struct pseudo_header));
    memcpy(pseudogram + sizeof(struct pseudo_header), tcph, sizeof(struct tcphdr) + OPT_SIZE);

    // TCP options are only set in the SYN packet
    // ---- set mss ----
    datagram[40] = 0x02;
    datagram[41] = 0x04;
    int16_t mss = htons(48); // mss value
    memcpy(datagram + 42, &mss, sizeof(int16_t));
    // ---- enable SACK ----
    datagram[44] = 0x04;
    datagram[45] = 0x02;
    // do the same for the pseudo header
    pseudogram[32] = 0x02;
    pseudogram[33] = 0x04;
    memcpy(pseudogram + 34, &mss, sizeof(int16_t));
    pseudogram[36] = 0x04;
    pseudogram[37] = 0x02;

    tcph->check = checksum((const char *) pseudogram, psize);
    iph->check = checksum((const char *) datagram, iph->tot_len);

    *out_packet = datagram;
    *out_packet_len = iph->tot_len;
    free(pseudogram);
}

int receive_from(
        int sock,
        char *buffer,
        size_t buffer_length,
        struct sockaddr_in *dst,
        int readTimeoutMs
) {
    struct timespec start, end;
    unsigned short dst_port;
    int received;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    int diffTime;
    do {
        received = recvfrom(sock, buffer, buffer_length, 0, NULL, NULL);
        if (received < 0)
            return -1;

        memcpy(&dst_port, buffer + 22, sizeof(dst_port));
        clock_gettime(CLOCK_MONOTONIC_RAW, &end);
        diffTime = (int) (end.tv_sec - start.tv_sec) * 1000 + (end.tv_nsec - start.tv_nsec) / 1000000;
        if (diffTime >= readTimeoutMs) {
            printf("[INFO (C)] Read timeout after %d > %d ms\n", diffTime, readTimeoutMs);
            return -2;
        }
    } while (dst_port != dst->sin_port);

    printf("[INFO (C)] Received bytes: %d\n", received);
    printf("[INFO (C)] Destination port: %d\n", ntohs(dst->sin_port));

    return received;
}

// java-rawsocket
JNIEXPORT jint

JNICALL
Java_com_reservoircode_net_SocketTester_socketTest(
        JNIEnv *env,
        jobject obj,
        jstring srcIp,
        jstring dstIp,
        jint dstPort,
        jint readTimeoutMs,
        jint writeTimeoutMs
) {
    const char *c_srcIp = (*env)->GetStringUTFChars(env, srcIp, 0);
    const char *c_dstIp = (*env)->GetStringUTFChars(env, dstIp, 0);
    const int c_dstPort = (int) dstPort;
    const int c_readTimeoutMs = (int) readTimeoutMs;
    const int c_writeTimeoutMs = (int) writeTimeoutMs;

    printf("[INFO (C)] Starting connection with following parameters. Source IP: %s, destination IP: %s, destination port: %d, read timeout: %d, write timeout: %d\n",
           c_srcIp, c_dstIp, c_dstPort, c_readTimeoutMs, c_writeTimeoutMs);

    return socketTest(c_srcIp, c_dstIp, c_dstPort, c_readTimeoutMs, c_writeTimeoutMs);
}

int socketTest(
        const char *srcIp,
        const char *dstIp,
        int dstPort,
        int readTimeoutMs,
        int writeTimeoutMs
) {
    srand(time(NULL));

    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);

    if (sock == -1) {
        printf("[ERROR (C)] Socket creation failed, you should be root\n");
        return FAIL;
    }
    struct timeval tv;

    tv.tv_sec = readTimeoutMs >= 1000 ? readTimeoutMs / 1000 : 0;
    tv.tv_usec = readTimeoutMs >= 1000 ? 0 : readTimeoutMs * 1000;

    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char *) &tv, sizeof tv) == -1) {
        printf("[INFO (C)] Setsockopt(SO_RCVTIMEO, %d + %d) failed: %s\n", (int) tv.tv_sec, (int) tv.tv_usec,
               strerror(errno));
        return FAIL;
    }
    tv.tv_sec = writeTimeoutMs >= 1000 ? writeTimeoutMs / 1000 : 0;
    tv.tv_usec = writeTimeoutMs >= 1000 ? 0 : writeTimeoutMs * 1000;
    if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char *) &tv, sizeof tv) == -1) {
        printf("[INFO (C)] Setsockopt(SO_SNDTIMEO,  %d + %d) failed: %s\n", (int) tv.tv_sec, (int) tv.tv_usec,
               strerror(errno));
        return FAIL;
    }
    // destination IP address configuration
    struct sockaddr_in daddr;
    daddr.sin_family = AF_INET;
    daddr.sin_port = htons(dstPort);
    if (inet_pton(AF_INET, dstIp, &daddr.sin_addr) != 1) {
        printf("[ERROR (C)] Destination IP configuration failed\n");
        return FAIL;
    }
    // source IP address configuration
    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(rand() % 65535); // random client port
    if (inet_pton(AF_INET, srcIp, &saddr.sin_addr) != 1) {
        printf("[ERROR (C)] Source IP configuration failed\n");
        return FAIL;
    }
    printf("[INFO (C)] Selected source port number: %d\n", ntohs(saddr.sin_port));

    // tell the kernel that headers are included in the packet
    int one = 1;
    const int *val = &one;

    if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) == -1) {
        printf("[ERROR (C)] Setsockopt(IP_HDRINCL, 1) failed\n");
        return FAIL;
    }
    // send SYN
    char *packet;
    int packet_len;

    create_syn_packet(&saddr, &daddr, &packet, &packet_len);

    int sent = sendto(sock, packet, packet_len, 0, (struct sockaddr *) &daddr, sizeof(struct sockaddr));

    if (sent == -1) {
        printf("[ERROR (C)] Sendto() failed\n");
        return FAIL;
    } else {
        printf("[INFO (C)] Successfully sent %d bytes SYN!\n", sent);
    }
    // receive SYN-ACK
    char recvbuf[DATAGRAM_LEN];

    int received = receive_from(sock, recvbuf, sizeof(recvbuf), &saddr, readTimeoutMs);

    if (received == -1) {
        printf("[ERROR (C)] Receive_from() failed\n");
        return FAIL;
    } else if (received == -2) {
        printf("[INFO (C)] Receive_from() timeout\n");
        return TIMEOUT;
    } else {
        printf("[INFO (C)] Successfully received %d bytes\n", received);

        struct iphdr *iph = (struct iphdr *) recvbuf;
        struct tcphdr *tcph = (struct tcphdr *) (recvbuf + sizeof(struct iphdr));

        printf("[INFO (C)] Received syn: %d, ack: %d, rst: %d\n", tcph->syn, tcph->ack, tcph->rst);
        printf("[INFO (C)] TCP header sequence number response: %u\n", ntohl(tcph->seq));
        printf("[INFO (C)] TCP header ack sequence number response: %u\n", ntohl(tcph->ack_seq));

        if (tcph->syn && tcph->ack) {
            printf("[INFO (C)] SYN ACK received -> Success\n");
            return SUCCESS;
        }
    }
    return FAIL;
}