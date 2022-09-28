/******************************************************************************
* tcp_client.c
*
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "networks.h"

#define MAXBUF 1024

int main(int argc, char * argv[])
{
	int socket_num = 0;         //socket descriptor
	char *send_buf = NULL;         //data buffer
	int send_len = 0;        //amount of data to send
	int sent = 0;            //actual amount of data sent

	/* check command line arguments  */
	if (argc != 3)
	{
		printf("usage: %s host-name port-number \n", argv[0]);
		exit(1);
	}

	/* set up the TCP Client socket  */
	socket_num = tcpClientSetup(argv[1], argv[2]);


	//create packet buffer
	if ((send_buf = (char *) malloc(MAXBUF)) == NULL)
	{
		perror("malloc() error");
		exit(-1);
	}

	/* get the data and send it   */
	printf("Enter the data to send: ");
	
	send_len = 0;
	while ((send_buf[send_len] = getchar()) != '\n' && send_len < MAXBUF-1)
	{
		send_len++;
	}
	
	send_buf[send_len] = '\0';
	
	/* now send the data */
	sent =  send(socket_num, send_buf, send_len, 0);
	if(sent < 0)
	{
		perror("send call");
		exit(-1);
	}

	printf("String sent: %s \n", send_buf);
	printf("Amount of data sent is: %d\n", sent);
	
	close(socket_num);
	
	return 0;
}


