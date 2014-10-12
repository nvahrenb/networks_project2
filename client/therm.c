/*
Chris Ray
Nathan Vahrenberg

CSE30264 - Computer Networks
Project 2 - Thermal Sensor Client
*/

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

#include <errno.h>

#define DPORT 9765

int main( int argc, char *argv[] ){

	int sockfd, port;
	struct sockaddr_in serverAddr;
	struct hostent *hostname;
	
	int exit = 0;
	
	// Open socket
	if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ){
		perror( "Unable to open socket" ); return 0;
	}
	
	// Get host address from 1st arg
	if( (hostname = gethostbyname(argv[1])) == 0 ){
		perror( "Unable to get address of host"); return 0;
	}
	
	// Set port number
	if( argc >= 3 ){
		port = atoi(argv[2]);
	}else{
		port = DPORT;
	}
	
	// Set server address information
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	memcpy((void *)&serverAddr.sin_addr, hostname->h_addr_list[0], hostname->h_length);
	
	// Connect to host
	if( (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr))) < 0 ){
		perror("Unable to connect to host"); return 0;
	}
	
	#ifdef DEBUG
		printf("Connected to host %s (%s) on port %d\n",argv[1],inet_ntoa(serverAddr.sin_addr),port);
	#endif
	
	// Read sensor info and send to host

}
