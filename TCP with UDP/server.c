//SERVER
//Author: Dakota Dehn
//Class: Networks CPE464-01

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include "cpe464.h"
#include "mutual.h"

typedef enum State STATE;

enum State {
   INITIAL, WAIT_ON_ACK_ACK, SEND_DATA, PROCESS_RR, CLOSED_WINDOW, 
   PROCESS_EOF, CLEANUP, FAKE_DONE, DONE
};

//globals
static float percentError; 
static int32_t lowerNum; 
static int32_t curNum;
static int32_t upperNum;
static int32_t endSeqNum;
static int windowOpen = 1; //1 is open, 0 is closed
static char **window; 

int processArgs(int argc, char* argv[]) {
   int portNumber = 0; 

   if (argc < 2 || argc > 3){
      printf("Incorrect number of arguments.\n");
      exit(-1);
   } 
   
   if (argc == 3) {portNumber = atoi(argv[2]);}
   
   percentError = atof(argv[1]); 

   return portNumber; 
}

uint32_t udpServer(int portNumber) {
   int sk = 0; //socket descriptor
   struct sockaddr_in local; //socket address for us
   uint32_t len = sizeof(local); //length of local address

   //create the socket
   if ((sk = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
      perror("socket in server"); 
      exit(-1); 
   } 

   //set up the socket
   local.sin_family = AF_INET; //internet family
   local.sin_addr.s_addr = INADDR_ANY; //wild card machine address
   local.sin_port = htons(portNumber); //let system choose the port

   //bind the name (address) to a port
   if (bindMod(sk,(struct sockaddr *)&local, sizeof(local)) < 0) {
      perror("udp_server, bind");
      exit(-1);
   }

   //get the port name
   getsockname(sk, (struct sockaddr *)&local, &len);
   printf("Using Port #: %d\n", ntohs(local.sin_port));

   return sk;
}

STATE filename(Connection *client, uint8_t *buf, uint32_t recvLen, int32_t *dataFile, 
 uint32_t *bufSize, uint32_t *windowSize, uint32_t *flag) {
   uint8_t response[1]; 
   char fname[MAXLEN];
   int i; 
   STATE returnValue = WAIT_ON_ACK_ACK; 

   memcpy(bufSize, buf, sizeof(uint32_t)); 
   *bufSize = ntohl(*bufSize);
   memcpy(windowSize, buf + sizeof(uint32_t), sizeof(uint32_t));
   *windowSize = ntohl(*windowSize); 
   memcpy(fname, buf + 2 * sizeof(uint32_t), recvLen - 2*sizeof(uint32_t)); 

   if ((client->sk_num = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
      perror("filename, open client socket");
      exit(-1);
   }

   if(((*dataFile) = open(fname, O_RDONLY)) < 0) {
      sendBuf(response, 0, client, FNAME_BAD, 0, buf); 
      *flag = FNAME_BAD;
      return CLEANUP;
   } else {
      sendBuf(response, 0, client, FNAME_OK, 0, buf);
      *flag = FNAME_OK;
   }

   //creating the window buffer from hell   
   window = (char **)malloc(*windowSize * sizeof(char *)); 
   for (i = 0; i < (*windowSize); i++) {
      window[i] = (char *)malloc(sizeof(char) * MAXLEN);
   }

   lowerNum = START_SEQ_NUM;
   curNum = START_SEQ_NUM;
   upperNum = *windowSize;
   endSeqNum = 0; 
   windowOpen = 1;

   return returnValue; 
}

STATE waitOnAckAck(Connection *client, int32_t origFileFlag, int *retryCount) {
   int returnValue = CLEANUP;
   uint8_t response[1]; 
   uint8_t packet[MAXLEN];
   uint8_t flag = 0;
   uint32_t seqNum = 0;
   uint32_t recvCheck = 0; 

   if ((returnValue = processSelect(client, retryCount, WAIT_ON_ACK_ACK, SEND_DATA, 
    CLEANUP)) == SEND_DATA) {
      recvCheck = recvBuf(packet, MAXLEN, client->sk_num, client, &flag, &seqNum);
      //check for bit flip
      if (recvCheck == -1) {
         returnValue = WAIT_ON_ACK_ACK;
      } else if(flag == FILENAME_ACK_ACK) { //recieved ACK ACK
         returnValue = SEND_DATA;
      } else if(flag == FILENAME_TRANS) { //recieved filename more than once
         returnValue = WAIT_ON_ACK_ACK; 

         //have to resend original ACK
         //assume ACK packet was lost to client
         sendBuf(response, 0, client, origFileFlag, 0, packet); 
      }
   }else if(returnValue == WAIT_ON_ACK_ACK) {
      sendBuf(response, 0, client, origFileFlag, 0, packet);
   }

   return returnValue; 
}

STATE sendData(Connection *client, int32_t fileName, uint32_t bufSize,
 uint8_t *packet, uint32_t windowSize) {
   STATE returnValue = SEND_DATA;
   int32_t lenRead = 0;
   uint8_t buf[MAXLEN];

   lenRead = read(fileName, buf, bufSize);
 
   switch (lenRead) {
      case -1:
         perror("sendData, read error");
         returnValue = CLEANUP;
         break;

      case 0:
         curNum++; 
         sendBuf(buf, 0, client, END_OF_FILE, curNum, packet);
         returnValue = PROCESS_EOF;
         endSeqNum = curNum;
         
         memcpy(window[curNum % windowSize], buf, lenRead);
         window[curNum % windowSize][lenRead] = '\0';
         break;

      default:
         if (windowOpen) { 
            curNum++;
            sendBuf(buf, lenRead, client, DATA, curNum, packet);
            returnValue = PROCESS_RR;
         }
         
         memcpy(window[curNum % windowSize], buf, lenRead);
         window[curNum % windowSize][lenRead] = '\0';
         break;
   }

   return returnValue;
}

void updateWindow(uint32_t seqNum, uint32_t windowSize){
   if (seqNum > lowerNum) {
      lowerNum = seqNum; 
      upperNum = seqNum + windowSize - 1;
   }    

   if (curNum != upperNum) {windowOpen = 1;} //window's open
   else {windowOpen = 0;}
}

STATE processRR(Connection *client, uint32_t windowSize){
   STATE returnValue = SEND_DATA;
   uint8_t buf[MAXLEN];
   uint8_t packet[MAXLEN];
   uint8_t flag = 0; 
   uint32_t seqNum = 0; 
   int32_t recvLen = 0;
    
   if (selectCall(client->sk_num, 0, 0, NOT_NULL) == 1) {
      recvLen = recvBuf(buf, MAXLEN, client->sk_num, client, &flag, &seqNum);
      //check for bit flip
      if (recvLen == -1) {
         returnValue = PROCESS_RR;
      } else if(flag == RR_PACKET) { //recieved RR
         updateWindow(seqNum, windowSize);
         returnValue = PROCESS_RR;  
      } else if(flag == SREJ_PACKET) { //recieved SREJ 
         updateWindow(seqNum, windowSize);
         returnValue = PROCESS_RR;
         memcpy(buf, window[(seqNum % windowSize)], sizeof(char) * 
          strlen(window[(seqNum % windowSize)])); 
         sendBuf(buf, strlen(window[(seqNum % windowSize)]), client, DATA, seqNum, packet);
      }
   }else { //nothing to process
      if (curNum != upperNum) {windowOpen = 1;}
      else {windowOpen = 0;}

      if (windowOpen) {returnValue = SEND_DATA;}
      else {returnValue = CLOSED_WINDOW;}
   }

    return returnValue;   
}

STATE closedWindow(Connection *client, uint32_t windowSize, int *retryCount) {
   STATE returnValue = CLOSED_WINDOW;
   uint8_t buf[MAXLEN]; 
   uint8_t packet[MAXLEN];
   uint8_t flag = 0;
   uint32_t seqNum = 0;
   uint32_t recvCheck = 0; 

   if ((returnValue = processSelect(client, retryCount, CLOSED_WINDOW, SEND_DATA, 
    CLEANUP)) == SEND_DATA) {
      recvCheck = recvBuf(packet, MAXLEN, client->sk_num, client, &flag, &seqNum);
      //check for bit flip
      if (recvCheck == -1) {
         returnValue = CLOSED_WINDOW;
      }else if(flag == RR_PACKET){ //recieved RR and broke deadlock
         updateWindow(seqNum, windowSize);
         if (windowOpen) {returnValue = SEND_DATA;}
         else {returnValue = CLOSED_WINDOW;}
      }else if(flag == SREJ_PACKET) {//recieved SREJ and broke deadlock
         updateWindow(seqNum, windowSize);
         if (windowOpen) {returnValue = SEND_DATA;}
         else {returnValue = CLOSED_WINDOW;}
         memcpy(buf, window[(seqNum % windowSize)], sizeof(char) * 
          strlen(window[(seqNum % windowSize)])); 
         sendBuf(buf, strlen(window[(seqNum % windowSize)]), client, DATA, seqNum, packet);
      }
   }else if (returnValue == CLOSED_WINDOW) {
      //send lowest packet to break deadlock
      memcpy(buf, window[lowerNum % windowSize], sizeof(char) * 
       strlen(window[lowerNum % windowSize])); 
      sendBuf(buf, strlen(window[lowerNum % windowSize]), client, DATA, lowerNum, packet);
   }

   return returnValue; 
}

STATE processEOF(Connection *client, int *retryCount, uint32_t windowSize) {
   STATE returnValue = CLEANUP;
   uint8_t buf[MAXLEN]; 
   uint8_t packet[MAXLEN];
   uint8_t flag = 0;
   uint32_t seqNum = 0;
   uint32_t recvCheck = 0; 

   if ((returnValue = processSelect(client, retryCount, PROCESS_EOF, CLEANUP, 
    FAKE_DONE)) == CLEANUP) {
      recvCheck = recvBuf(packet, MAXLEN, client->sk_num, client, &flag, &seqNum);
      //check for bit flip
      if (recvCheck == -1) {
         return PROCESS_EOF;
      }else if(flag == RR_PACKET){ //recieved RR and finished file transfer
         if (seqNum == endSeqNum){return CLEANUP;}
         else {returnValue = PROCESS_EOF;}
      }else if(flag == SREJ_PACKET) { 
         returnValue = PROCESS_EOF;
         memcpy(buf, window[seqNum % windowSize], sizeof(char) * 
          strlen(window[seqNum % windowSize])); 
         sendBuf(buf, strlen(window[seqNum % windowSize]), client, DATA, seqNum, packet);
      }
   }else if(returnValue == PROCESS_EOF) {
      sendBuf(buf, 0, client, END_OF_FILE, curNum, packet);
   }else if(returnValue == FAKE_DONE) {returnValue = CLEANUP;}
   return returnValue; 
}

void processClient(int32_t serverSk, uint8_t *buf, int32_t recvLen, Connection *client) {
   STATE state = INITIAL;
   int32_t dataFile = 0;
   uint8_t packet[MAXLEN];
   uint32_t bufSize = 0;
   uint32_t windowSize = 0;
   uint32_t flag = 0; 
   static int retryCount = 0;
   int firstTimeClosed = 0;
   int firstTimeProcEOF = 0;  
   int i; 

   while (state != DONE) {
      switch(state) {
         case INITIAL:
            state = filename(client, buf, recvLen, &dataFile, &bufSize, &windowSize, 
             &flag);
            break;

         case WAIT_ON_ACK_ACK:
            state = waitOnAckAck(client, flag, &retryCount); 
            break;

         case SEND_DATA:
            state = sendData(client, dataFile, bufSize, packet, windowSize);
            break;

         case PROCESS_RR:
            state = processRR(client, windowSize); 
            break;

         case CLOSED_WINDOW:
            if (firstTimeClosed == 0) {
               retryCount = 0;
               firstTimeClosed = 1; 
            }
            state = closedWindow(client, windowSize, &retryCount);
            break;

         case PROCESS_EOF:
            if (firstTimeProcEOF == 0) {
               retryCount = 0;
               firstTimeProcEOF = 1; 
            }
            state = processEOF(client, &retryCount, windowSize);
            break;

         case CLEANUP:
            for (i=0; i<windowSize; i++) {
               free(window[i]);
            }
            free(window);
            
            state = DONE;
            break;
      
         default:
            printf("In default and you should not be.\n");
            state = DONE;
            break;
      }
   }
}

void processServer(int serverNum) {
   pid_t pid = 0;
   int status = 0; 
   uint8_t buf[MAXLEN];
   Connection client;
   uint8_t flag = 0; 
   uint32_t seqNum = 0; 
   int32_t recvLen = 0;

   while (1) {
      if (selectCall(serverNum, LONG_TIME, 0, SET_NULL) == 1) {
         recvLen = recvBuf(buf, MAXLEN, serverNum, &client, &flag, &seqNum);
         if (recvLen != -1) {
            if ((pid = fork()) < 0) {
               perror("fork");
               exit(-1);
            }
            //child process
            if (pid == 0) { 
               processClient(serverNum, buf, recvLen, &client); 
               exit(0);
            }
         }
      
         //check to see if any children quit (only get here in the parent process)
         while (waitpid(-1, &status, WNOHANG) > 0){}
      }
   }
}

int main (int argc, char* argv[]) {
   uint32_t serverSkNum = 0;
   int portNumber = 0; 

   portNumber = processArgs(argc, argv); 

   sendtoErr_init(atof(argv[1]), DROP_ON, FLIP_ON, DEBUG_ON, RSEED_ON);
 
   serverSkNum = udpServer(portNumber); 

   processServer(serverSkNum); 

   return 0; 
}
