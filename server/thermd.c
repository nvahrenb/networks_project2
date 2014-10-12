/*
Chris Ray
Nathan Vahrenberg

CSE30264 - Computer Networks
Project 2 - Thermal Sensor Server
*/

#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#include <errno.h>

#define DPORT 9765

int main( int argc, char *argv[] ){

	int sockfd, port, conn;
	struct sockaddr_in clientAddr, serverAddr;
	socklen_t csize = sizeof(clientAddr);
	
	int exit = 0;
	
	// Open socket
	if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ){
		perror("Unable to open socket"); return 0;
	}
	
	// Set port number
	if( argc >= 2 ){
		port = atoi(argv[1]);
	}else{
		port = DPORT;
	}
	
	// Set server address info
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr=htonl(INADDR_ANY);
	serverAddr.sin_port=htons(port);
	
	// Set socket options
	int opt = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(int));
	
	// Bind to socket
	if( (bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr))) < 0 ){
		perror("Unable to bind"); return 0;
	}
	
	// Begin accepting connections to socket
	if( (listen(sockfd, 1024)) < 0 ){
		perror("Unable to accept connections"); return 0;
	}
	
	#ifdef DEBUG
		printf("Accepting connections on port %d\n",port);
	#endif
	
	// Begin loop and wait for client connection
	while(!exit){
	
		// Accept a connection
		conn = accept(sockfd, (struct sockaddr *)&clientAddr, &csize);
	
		#ifdef DEBUG
			printf("Client connected from %s\n",inet_ntoa(clientAddr.sin_addr));
		#endif
		
		// Get sensor info from client
	
	}


}
