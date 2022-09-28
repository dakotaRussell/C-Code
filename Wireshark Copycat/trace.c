//Author: Dakota Dehn
//Class: CPE 464-01
//Date: 3 Oct, 2016

#include <stdio.h>
#include <pcap/pcap.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "checksum.h"

#define SUCCESS 1
#define TIMEOUT 0
#define ERROR -1
#define PACKETS_EXHAUSTED -2

#define REQUEST 0x0001
#define REPLY 0x0002
#define ECHO_REQUEST 0x08
#define ECHO_REPLY 0x00

#define ARP 0x0806
#define IP  0x0800
#define TCP 0x06
#define UDP 0x11
#define ICMP 0x01

#define HTTP 80
#define TELNET 23
#define FTP 21
#define POP3 110
#define SMTP 25
#define DNS 53

#define FIN_FLAG 0x0001
#define ACK_FLAG 0x0010
#define RST_FLAG 0x0004
#define SYN_FLAG 0x0002

struct ethHdr{
   uint8_t dest[6];
   uint8_t src[6];
   short type; 
}__attribute__((packed));

struct arpHdr {
   short hwType;
   short protoType;
   uint8_t hwSize;
   uint8_t protoSize;
   short opcode;
   uint8_t senderMAC[6];
   uint8_t senderIP[4];
   uint8_t targetMAC[6];
   uint8_t targetIP[4];
}__attribute__((packed));

struct ipHdr{
   uint8_t verAndLen; //version is upper nibble, len is lower nibble
   uint8_t TOSsubfields; //diffServe bits are [7:2], ECN is [1:0]
   short totalLength;
   short ID;
   uint8_t flags;
   uint8_t fragOffset;
   uint8_t TTL;
   uint8_t protocol;
   short unsigned checksum; 
   uint8_t sourceIP[4];
   uint8_t destIP[4];
}__attribute__((packed));

struct tcpHdr {
   short sourcePort;
   short destPort;
   uint32_t seqNum;
   uint32_t ackNum;
   short hdrLenAndFlags; 
   short windowSize;
   unsigned short checksum;
   short urgentPtr;
   uint8_t options[8];
} __attribute__((packed));

struct pseudoHdr {
   uint8_t sourceIP[4];
   uint8_t destIP[4];
   uint8_t zeroes;
   uint8_t protocol;
   unsigned short length;
}__attribute__((packed));

struct udpHdr {
   unsigned short sourcePort;
   unsigned short destPort;
   short length;
   short checksum;
}__attribute__((packed));

struct icmpHdr {
   uint8_t type;
   uint8_t code;
   short checksum;
   short identifier;
   short seqNum;
   uint8_t data[32];
}__attribute__((packed));

char errBuf[PCAP_ERRBUF_SIZE];
int packetCount;
short type; 
const unsigned char *pcapData;
unsigned char *pseudo;

void printArray(uint8_t *arr, int size) {
   int i;
   for (i = 0; i<size; i++) {
      printf("%x", arr[i]);
      if (i != size-1) {
         printf(":");
      }
   }
   printf("\n");
}

void printIpArray(uint8_t *arr, int size) {
   int i;
   for (i = 0; i<size; i++) {
      printf("%d", arr[i]);
      if (i != size-1) {
         printf(".");
      }
   }
   printf("\n");
}

void printPortNum(unsigned short num) {
   switch(num) {
      case (HTTP) :
         printf("HTTP\n");
         break;
      case (TELNET) :
         printf("Telnet\n");
         break;
      case (FTP):
         printf("FTP\n");
         break;
      case (POP3): 
         printf("POP3\n");
         break;
      case (SMTP):
         printf("SMTP\n");
         break;
      case (DNS):
         printf("DNS\n");
         break;
      default:
         printf("%u\n", num);
         break;
   }
}

void parsePacketHeader(struct pcap_pkthdr *pkthdr){
   printf("\nPacket number: %d  Packet Len: %d\n\n", packetCount, pkthdr->len);
} 

void parseEthernetHeader() {
   struct ethHdr myHdr; 
   memcpy(&myHdr, pcapData, sizeof(struct ethHdr));
   
   printf("        Ethernet Header\n");
   printf("                Dest MAC: ");
   printArray(myHdr.dest, 6);   
   printf("                Source MAC: ");
   printArray(myHdr.src, 6);
   
   switch (ntohs(myHdr.type)){
     case (ARP):
         printf("                Type: ARP\n");
         break;
     case(IP): 
         printf("                Type: IP\n");
         break;
      default:
         break;
   }
   pcapData += sizeof(struct ethHdr);
   type = ntohs(myHdr.type);
}

void parseARPHeader(){
   struct arpHdr myHdr; 
   memcpy(&myHdr, pcapData, sizeof(struct arpHdr));

   printf("\n        ARP header\n");
   printf("                Opcode: ");
   if (ntohs(myHdr.opcode) == REQUEST) {
      printf("Request\n");
   } else if (ntohs(myHdr.opcode) == REPLY){
      printf("Reply\n");
   }
   printf("                Sender MAC: ");
   printArray(myHdr.senderMAC, 6);
   printf("                Sender IP: ");
   printIpArray(myHdr.senderIP, 4);
   printf("                Target MAC: ");
   printArray(myHdr.targetMAC, 6);
   printf("                Target IP: ");
   printIpArray(myHdr.targetIP, 4);
   printf("\n");
 
   pcapData += sizeof(struct arpHdr);
   type = -1;
}

void parseIPHeader(){
   struct ipHdr myHdr; 
   struct pseudoHdr myPseudoHdr;
   unsigned short tempChecksum;
   memcpy(&myHdr, pcapData, sizeof(struct ipHdr));
   tempChecksum = myHdr.checksum;

   memcpy(&(myPseudoHdr.sourceIP), &(myHdr.sourceIP), sizeof(uint32_t));
   memcpy(&(myPseudoHdr.destIP), &(myHdr.destIP), sizeof(uint32_t));
   myPseudoHdr.zeroes = 0;
   myPseudoHdr.protocol = myHdr.protocol;
   myPseudoHdr.length = htons(ntohs(myHdr.totalLength) - ((myHdr.verAndLen & 0xF) * 4));

   memcpy(pseudo, &myPseudoHdr, sizeof(struct pseudoHdr));

   printf("\n        IP Header\n");
   printf("                IP Version: %d\n", myHdr.verAndLen >> 4);
   printf("                Header Len (bytes): %d\n", (myHdr.verAndLen & 0xF) * 4);
   printf("                TOS subfields:\n");
   printf("                     Diffserv bits: %d\n", myHdr.TOSsubfields >> 2);
   printf("                     ECN bits: %d\n", myHdr.TOSsubfields & 0x03);
   printf("                TTL: %d\n", myHdr.TTL);
   printf("                Protocol: ");

   if (myHdr.protocol == TCP) {printf("TCP\n");}
   else if(myHdr.protocol == UDP) {printf("UDP\n"); }
   else if(myHdr.protocol == ICMP) {printf("ICMP\n");}
   else { printf("Unknown\n");}

   myHdr.checksum = 0;
   myHdr.checksum = in_cksum((unsigned short *)&myHdr, sizeof(struct ipHdr));
   if (tempChecksum == myHdr.checksum) { printf("                Checksum: Correct (0x%04x)\n", ntohs(myHdr.checksum));}
   else { printf("                Checksum: Incorrect (0x%04x)\n", ntohs(tempChecksum));}
   
   printf("                Sender IP: ");
   printIpArray(myHdr.sourceIP, 4);
   printf("                Dest IP: ");
   printIpArray(myHdr.destIP, 4);
   printf("\n");

   pcapData += (myHdr.verAndLen & 0xF) * 4;
   type = myHdr.protocol;
}

void parseTCPHeader() {
   struct tcpHdr myHdr; 
   unsigned short tempChecksum;
   struct pseudoHdr myPseudoHdr;
   memcpy(&myHdr, pcapData, sizeof(struct tcpHdr));
   memcpy(&myPseudoHdr, pseudo, sizeof(struct pseudoHdr));
   tempChecksum = myHdr.checksum;

   myHdr.checksum = 0;
   memcpy(pseudo + sizeof(struct pseudoHdr), pcapData, ntohs(myPseudoHdr.length));
   memset(pseudo + sizeof(struct pseudoHdr) + 8*sizeof(short), 0, sizeof(short));

   printf("        TCP Header\n");
   printf("                Source Port: ");
   printPortNum(ntohs(myHdr.sourcePort));
   printf("                Dest Port: ");
   printPortNum(ntohs(myHdr.destPort));
   printf("                Sequence Number: %u\n", ntohl(myHdr.seqNum));
   printf("                ACK Number: %u\n", ntohl(myHdr.ackNum)); 
   printf("                Data Offset (bytes): %u\n", 
    ((ntohs(myHdr.hdrLenAndFlags)) >> 12)* 4);
   
   printf("                SYN Flag: ");
   if (ntohs(myHdr.hdrLenAndFlags) & SYN_FLAG) {printf ("Yes\n");}
   else {printf("No\n");}

   printf("                RST Flag: ");
   if (ntohs(myHdr.hdrLenAndFlags) & RST_FLAG) {printf ("Yes\n");}
   else {printf("No\n");}
      
   printf("                FIN Flag: ");
   if (ntohs(myHdr.hdrLenAndFlags) & FIN_FLAG) {printf ("Yes\n");}
   else {printf("No\n");}

   printf("                ACK Flag: ");
   if (ntohs(myHdr.hdrLenAndFlags) & ACK_FLAG) {printf ("Yes\n");}
   else {printf("No\n");} 

   printf("                Window Size: %d\n", ntohs(myHdr.windowSize));
   
   myHdr.checksum = in_cksum((unsigned short *)pseudo, sizeof(struct pseudoHdr) 
    + ntohs(myPseudoHdr.length));
   if (tempChecksum == myHdr.checksum) { printf("                Checksum: Correct (0x%04x)\n", 
    ntohs(myHdr.checksum));}
   else { printf("                Checksum: Incorrect (0x%04x)\n", ntohs(tempChecksum));}

   pcapData += sizeof(struct tcpHdr);
   type = -1;
}

void parseUDPHeader() {
   struct udpHdr myHdr;
   memcpy(&myHdr, pcapData, sizeof(struct udpHdr));

   printf("        UDP Header\n");
   printf("                Source Port: ");
   printPortNum(ntohs(myHdr.sourcePort));
   printf("                Dest Port: ");
   printPortNum(ntohs(myHdr.destPort));

   pcapData += sizeof(struct udpHdr);
   type = -1;
}

void parseICMPHeader() {
   struct icmpHdr myHdr;
   memcpy(&myHdr, pcapData, sizeof(struct icmpHdr));

   printf("        ICMP Header\n");
   if (myHdr.type == ECHO_REQUEST) {printf("                Type: Request\n");}
   else if (myHdr.type == ECHO_REPLY) {printf("                Type: Reply\n");}
   else {printf("              Type: %u\n", myHdr.type);}

   pcapData+= sizeof(struct icmpHdr);
   type = -1;
}

int main (int argc, char *argv[]) {
   //pcap necessary variables
   char *file = NULL;
   pcap_t *packet = NULL;
   pcapData = malloc(65535);
   pseudo = malloc(65535); //needs to accomodate TCP pseudo header and length of TCP Segment
   struct pcap_pkthdr *pkthdr = malloc(sizeof(struct pcap_pkthdr)); 
   
   //program management varibles
   packetCount = 0; //packet count for printf 
   int status = PACKETS_EXHAUSTED;

   //user error checking
   if (argc == 1) {
      printf("No pcap file provided. Nothing to parse\n");
      return 0;
   } else {file = argv[1];}

   //opening the file
   packet = pcap_open_offline(file, errBuf);
   if (!packet) {
      printf("Failed to open target file.\n");
      return 0;
   }

   //iterating through the packets
   status = pcap_next_ex(packet, &pkthdr, &pcapData);

   while (status == SUCCESS) {
      packetCount++;
      parsePacketHeader(pkthdr);
      parseEthernetHeader();
      if (type == ARP) {parseARPHeader();}
      else if (type == IP) {
         parseIPHeader();
         if (type == TCP) {parseTCPHeader();}
         else if(type == UDP) {parseUDPHeader();}
         else if(type == ICMP) {parseICMPHeader();}
      }
      status = pcap_next_ex(packet, &pkthdr, &pcapData);
   }
   
   return 0;
}
