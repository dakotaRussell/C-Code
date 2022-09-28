//Dakota's Header File

#include <stdint.h>
#include <stdlib.h>
#include <netinet/in.h>

#define MAXLEN 1550 
#define LONG_TIME 10
#define SHORT_TIME 1
#define MAX_TRIES 10
#define FILENAME_TRANS 6
#define DATA 1
#define RR_PACKET 3
#define SREJ_PACKET 4
#define FNAME_BAD 8
#define FNAME_OK 9
#define FILENAME_ACK_ACK 7
#define START_SEQ_NUM 0
#define END_OF_FILE 10

#pragma pack(1)

typedef struct connection Connection;

enum SELECT {
   SET_NULL, NOT_NULL
};

struct connection {
   int32_t sk_num;
   struct sockaddr_in remote;
   uint32_t len;
};

typedef struct header Header;

struct header {
   uint32_t seqNum; 
   uint16_t checksum; 
   uint8_t flag;
};

int32_t safeSend(uint8_t *packet, uint32_t len, Connection *connection);
int32_t safeRecv(int recvSk, char *data, int len, Connection *connection); 
int32_t selectCall(int32_t socketNum, int32_t seconds, int32_t microseconds, 
   int32_t setNull);
int32_t sendBuf(uint8_t *buf, uint32_t len, Connection *connection, 
   uint8_t flag, uint32_t seqNum, uint8_t *packet); 
int32_t recvBuf(uint8_t *buf, int32_t len, int32_t recvNum, Connection *connection, 
   uint8_t *flag, uint32_t *seqNum); 
int createHeader(uint32_t leng, uint8_t flag, uint32_t seqNum, uint8_t *packet); 
int retrieveHeader(char *data, int recvLen, uint8_t *flag, uint32_t *seqNum); 
int processSelect(Connection *client, int *retryCount, int selectTO, int dataReadyState,
   int doneState);

