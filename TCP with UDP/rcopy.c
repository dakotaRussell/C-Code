//RCOPY
//Author: Dakota Dehn
//Class: Networks CPE 464-01

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "cpe464.h"
#include "mutual.h"

//defines
#define NUM_ARGS 8

//globals
static int localFile; 
static char *remoteFile; 
static float percentError; 
static int windowSize; 
static char *remoteMachine; 
static int remotePort;
static uint32_t bufferSize;
static char **window;
static int *arr;
static int eofNum = -1;

typedef enum State STATE;

enum State {
   INITIAL, WAIT_ON_ACK, RECV_DATA, CLEANUP, DONE
};

//functions
void processArgs(int argc, char *argv[]) {
   if (argc != NUM_ARGS) {
      printf("Too few or too many arguements provided.\n");
      exit(-1); 
   }

   //opening new local file
   if (strlen(argv[1]) > 100) {
      printf("Local filename exceeds 100 characters.\n"); 
      exit(-1);
   }
   
   if ((localFile = open(argv[1], O_CREAT | O_TRUNC | O_RDWR, 0700)) < 0){
      printf("Error opening local file: %s.\n", argv[1]);
      exit(-1);
   } 

   //checking remote file
   if (strlen(argv[2]) > 100) {
      printf("Remote filename exceeds 100 characters.\n"); 
      exit(-1);
   } 
   remoteFile = argv[2]; 
   
   //setting remaining args
   bufferSize = atoi(argv[3]); 
   if (bufferSize < 400 || bufferSize > 1500) {
      printf("Buffer size is not between 400-1500 bytes.\n");
      exit(-1);
   }
  
   percentError = atof(argv[4]);
  
   windowSize = atoi(argv[5]);
   if (windowSize < 1) {
      printf("Window size has to be at least size 1.\n");
      exit(-1);
   }
 
   remoteMachine = argv[6]; 
   remotePort = atoi(argv[7]);  
}

uint32_t udpClientSetup(char *hostname, uint16_t portNum, Connection *connection){
   //return pointer to a sockaddr_in that it created or NULL if host not found
   //also passes back the socket number is sk

   struct hostent *hp = NULL; //address of remote host

   connection->sk_num = 0; 
   connection->len = sizeof(struct sockaddr_in);

   //create the socket
   if ((connection->sk_num = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
      perror("udpClientSetup, socket");
      exit(-1);
   }

   //designate the addressing family
   connection->remote.sin_family = AF_INET;

   //get the address of the remot host and store
   hp = gethostbyname(hostname);

   if (hp == NULL) {
      printf("Host not found: %s\n", hostname);
      return -1; 
   }

   memcpy(&(connection->remote.sin_addr), hp->h_addr, hp->h_length);

   //get the port used on the remote side and store
   connection->remote.sin_port = htons(portNum);

   return 0;
}

STATE initial(Connection *server){
   //Returns WAIT_ON_ACK if no error, otherwise DONE
   STATE returnValue = INITIAL;
   uint8_t packet[MAXLEN];
   int i;
   uint8_t buf[MAXLEN];
   int switchWindowSize = htonl(windowSize);
   int switchBufferSize = htonl(bufferSize);

   //if we have connected to server before, close it before reconnect
   if (server->sk_num > 0) {
      close(server->sk_num);
   }
 
   if (udpClientSetup(remoteMachine, remotePort, server) < 0) {
      returnValue = CLEANUP; 
   }else {
      returnValue = WAIT_ON_ACK;
      memcpy(buf, &switchBufferSize, sizeof(uint32_t));
      memcpy(&buf[sizeof(uint32_t)], &switchWindowSize, sizeof(uint32_t)); 
      memcpy(&buf[2*sizeof(uint32_t)], remoteFile, strlen(remoteFile));

      sendBuf(buf, strlen(remoteFile) + 2*sizeof(uint32_t), server, FILENAME_TRANS, 0,
       packet);
   }

   //creating the window buffer from hell   
   window = (char **)malloc(windowSize * sizeof(char *)); 
   for (i = 0; i < (windowSize); i++) {
      window[i] = (char *)malloc(sizeof(char) * MAXLEN);
   }

   arr = (int *)malloc(sizeof(int) * windowSize);
   for (i = 0; i<windowSize; i++) {arr[i] = 0;}

   return returnValue; 
}

STATE waitOnAck(Connection *server, int *retryCount) {
   int returnValue = CLEANUP;
   uint8_t packet[MAXLEN];
   uint8_t buf[MAXLEN];
   uint8_t flag = 0;
   uint32_t seqNum = 0;
   int32_t recvCheck = 0; 
   int switchWindowSize = htonl(windowSize);
   int switchBufferSize = htonl(bufferSize);

   if ((returnValue = processSelect(server, retryCount, WAIT_ON_ACK, RECV_DATA, 
    CLEANUP)) == RECV_DATA) {
      recvCheck = recvBuf(packet, MAXLEN, server->sk_num, server, &flag, &seqNum);
      //check for bit flip
      if (recvCheck == -1) {
         returnValue = WAIT_ON_ACK;
      }else if(flag == FNAME_BAD) {
         printf("Error during file open of %s on server.\n", remoteFile);
         returnValue = CLEANUP;
      }else if (flag == FNAME_OK){         
         //send ACK ACK
         sendBuf(buf, 0, server, FILENAME_ACK_ACK, 0, packet); 
      }
   } else if (returnValue == WAIT_ON_ACK) {
      //resend filename
      memcpy(buf, &switchBufferSize, sizeof(uint32_t));
      memcpy(&buf[sizeof(uint32_t)], &switchWindowSize, sizeof(uint32_t)); 
      memcpy(&buf[2*sizeof(uint32_t)], remoteFile, strlen(remoteFile));

      sendBuf(buf, strlen(remoteFile) + 2*sizeof(uint32_t), server, FILENAME_TRANS, 0,
       packet); 
   }

   return returnValue;        
}

STATE recvData(Connection *server) {
   int32_t dataLen = 0; 
   uint8_t buf[MAXLEN];
   uint8_t packet[MAXLEN];
   uint8_t flag = 0;
   uint32_t seqNum = 0; 
   static int32_t expectedSeqNum = 1; 
   int i;
   int myCount = 0;

   if (selectCall(server->sk_num, LONG_TIME, 0, NOT_NULL) == 0) {
      printf("Timeout after 10 seconds, server must be gone.\n"); 
      return CLEANUP; 
   }

   dataLen = recvBuf(buf, MAXLEN, server->sk_num, server, &flag, &seqNum);

   if (dataLen == -1) {return RECV_DATA;} //crc error
   
   if (flag == FNAME_OK) { //received ACK more than once
      //send ACK ACK again
      sendBuf(buf, 0, server, FILENAME_ACK_ACK, 0, packet); 
      return RECV_DATA;
   }
   
   if (seqNum == expectedSeqNum) {
      //expectedSeqNum++;
      //writing expected value to disk
      write(localFile, &buf, dataLen);
      for (i = 1; i<= windowSize; i++) {
         if (arr[(expectedSeqNum + i) % windowSize] == 0) {break;}
         arr[(expectedSeqNum + i) % windowSize] = 0; 
         write(localFile, window[(expectedSeqNum + i) % windowSize], 
          strlen(window[(expectedSeqNum + i) % windowSize]));
         myCount++;
   
         if ((expectedSeqNum + i) == eofNum) {
            expectedSeqNum = eofNum;
            eofNum = -1;
            for (i = 0; i<10; i++){
               sendBuf(buf, 0, server, RR_PACKET, expectedSeqNum, packet); 
            }
            return CLEANUP;
         }
      }
      if (flag == END_OF_FILE) {
         expectedSeqNum = seqNum;

         for (i = 0; i<10; i++){
            sendBuf(buf, 0, server, RR_PACKET, expectedSeqNum, packet); 
         }
         return CLEANUP;
      }
      expectedSeqNum = seqNum + myCount + 1;
      sendBuf(buf, 0, server, RR_PACKET, expectedSeqNum, packet); 
      myCount = 0;
   }else if (seqNum > expectedSeqNum) {
      //packet(s) lost somewhere, buffer the current one and SREJ appropriately
      memcpy(window[seqNum % windowSize], buf, dataLen);   
      window[seqNum % windowSize][dataLen] = '\0';
      if (flag == END_OF_FILE) {eofNum = seqNum;}
      
      arr[seqNum % windowSize] = 1;   //keeping track of original buffering    
      sendBuf(buf, 0, server, SREJ_PACKET, expectedSeqNum, packet);
   }else {sendBuf(buf, 0, server, RR_PACKET, expectedSeqNum, packet);}
   
   return RECV_DATA; 
}

int main (int argc, char* argv[]) {
   Connection server;
   static int retryCount = 0;
   STATE state = INITIAL;
   int i;

   processArgs(argc, argv);

   sendtoErr_init(percentError, DROP_ON, FLIP_ON, DEBUG_ON, RSEED_ON);
   
   while (state != DONE) {
      switch (state) {
         case INITIAL: 
            state = initial(&server); 
            break;

         case WAIT_ON_ACK:
            state = waitOnAck(&server, &retryCount); 
            break;

         case RECV_DATA:
            state = recvData(&server); 
            break;

         case CLEANUP:
            for (i=0; i<windowSize; i++) {
               free(window[i]);
            }
            free(window);
            free(arr);
            state = DONE;
            break;

         default: 
            printf("ERROR -- in default state.\n"); 
            break;
      }
   }   

   return 0; 
}

