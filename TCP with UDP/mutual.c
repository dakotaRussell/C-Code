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
#include <netinet/in.h>
#include <netdb.h>

#include "cpe464.h"
#include "mutual.h"

int32_t selectCall(int32_t socketNum, int32_t seconds, int32_t microseconds, 
   int32_t setNull){
   fd_set fdvar;
   struct timeval aTimeout;
   struct timeval *timeout = NULL;

   if (setNull == NOT_NULL) {
      aTimeout.tv_sec = seconds;
      aTimeout.tv_usec = microseconds;
      timeout = &aTimeout;
   }

   FD_ZERO(&fdvar); 
   FD_SET(socketNum, &fdvar); 

   if (select(socketNum + 1, (fd_set *)&fdvar, (fd_set *)0, (fd_set *)0, timeout) < 0) {
      perror("select");
      exit(-1);
   }
   
   if (FD_ISSET(socketNum, &fdvar)) {return 1;}
   else {return 0;}
   
}

int32_t sendBuf(uint8_t *buf, uint32_t len, Connection *connection, 
   uint8_t flag, uint32_t seqNum, uint8_t *packet){
   
   int32_t sentLen = 0; 
   int32_t sendingLen = 0; 

   if (len>0) {
      memcpy(&packet[sizeof(Header)], buf, len); 
   }

   sendingLen = createHeader(len, flag, seqNum, packet); 
   sentLen = safeSend(packet, sendingLen, connection);

   return sentLen; 
} 

int32_t safeSend(uint8_t *packet, uint32_t len, Connection *connection) {
   int sendLen = 0;

   if ((sendLen = sendtoErr(connection->sk_num, packet, len, 0, 
         (struct sockaddr *)&(connection->remote), connection->len)) < 0) {
      perror("in sendBuf, sendto() call");
      exit(-1);
   }

  return sendLen; 
}

int32_t safeRecv(int recvSk, char *data, int len, Connection *connection) {
   int recvLen = 0;
   uint32_t remoteLen = sizeof(struct sockaddr_in);

   if ((recvLen = recvfrom(recvSk, data, len, 0, 
      (struct sockaddr *)&(connection->remote), &remoteLen)) < 0) {
      perror("recvBuf, recvfrom");
      exit(-1); 
   }

   connection->len = remoteLen;


   return recvLen; 

} 

int32_t recvBuf(uint8_t *buf, int32_t len, int32_t recvNum, Connection *connection, 
   uint8_t *flag, uint32_t *seqNum){
   
   char data[MAXLEN]; 
   int32_t recvLen = 0; 
   int32_t dataLen = 0; 

   recvLen = safeRecv(recvNum, data, len, connection);

   dataLen = retrieveHeader(data, recvLen, flag, seqNum);

   if (dataLen > 0) {memcpy(buf, &data[sizeof(Header)], dataLen);}

   return dataLen; 
} 

int createHeader(uint32_t len, uint8_t flag, uint32_t seqNum, uint8_t *packet){
   Header *myHdr = (Header *)packet; 
   uint16_t checksum = 0; 

   seqNum = htonl(seqNum); 
   memcpy(&(myHdr->seqNum), &seqNum, sizeof(seqNum)); 

   myHdr->flag = flag;

   memset(&(myHdr->checksum), 0, sizeof(checksum));
   checksum = in_cksum((unsigned short *)packet, len + sizeof(Header));
   memcpy(&(myHdr->checksum), &checksum, sizeof(checksum));

   return len + sizeof(Header); 
} 

int retrieveHeader(char *data, int recvLen, uint8_t *flag, uint32_t *seqNum){
   Header *myHdr = (Header *)data;
   int returnValue = 0; 
   if (in_cksum((unsigned short *)data, recvLen) != 0) {
      returnValue = -1; 
   } else {
      *flag = myHdr->flag;
      memcpy(seqNum, &(myHdr->seqNum), sizeof(myHdr->seqNum));
      *seqNum = ntohl(*seqNum); 

      returnValue = recvLen - sizeof(Header);  
   }

   return returnValue; 
}

int processSelect(Connection *client, int *retryCount, int selectTO, int dataReadyState,
   int doneState) {
   int returnValue = dataReadyState;

   (*retryCount)++;
   if (*retryCount > MAX_TRIES) {
      returnValue = doneState;
   } else {
      if (selectCall(client->sk_num, SHORT_TIME, 0, NOT_NULL) == 1) {
         *retryCount = 0; 
         returnValue = dataReadyState;
      } else {
         returnValue = selectTO;
      }
   }

   return returnValue;

}
