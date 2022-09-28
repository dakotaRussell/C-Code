/******************************************************************************
* tcp_server.c
*
* CPE 464 - Program 1
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

int main(int argc, char *argv[])
{
	int server_socket = 0;   //socket descriptor for the server socket
	int client_socket = 0;   //socket descriptor for the client socket
	char *buf = NULL;        //buffer for receiving from client
	int message_len = 0;     //length of the received message
	
	//create packet buffer
	if ((buf = (char *) malloc(MAXBUF)) == NULL)
	{
		perror("malloc() error");
		exit(-1);
	}
	
	//create the server socket
	server_socket = tcpServerSetup();
	
        //look for a client to serve
	client_socket = tcpAccept(server_socket);

	//now get the data on the client_socket
	if ((message_len = recv(client_socket, buf, MAXBUF, 0)) < 0)
	{
		perror("recv call");
		exit(-1);
	}

	printf("Message received, length: %d\n", message_len);
	printf("Data: %s\n", buf);
	
	/* close the sockets */
	close(server_socket);
	close(client_socket);
	
	return 0;
}
