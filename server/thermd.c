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

#include <pthread.h>

#define DPORT 9765

//#define DEBUG

struct tempdata{
	char host[32];
	int nSensors;
	double sensorData;
	double lowVal;
	double highVal;
	char timestamp[32];
	int action;
};

int unpack(struct tempdata * package, int sock)
{
	int i, inBytes;
	char * tmp;
	uint32_t size, totalBytes;

	for(i = 0; i < 7; i++)
	{
		switch(i)
		{
			case 0:
			inBytes = recv(sock, &size, sizeof(size), 0);
			if(inBytes < 0)
			{
				perror("Error receiving buffer size");
				return -1;
			}
			size = ntohl(size);

			totalBytes = 0;
			while(totalBytes < size)
			{
				inBytes = recv(sock, package->host, sizeof(package->host), 0);
				if(inBytes < 0)
				{
					perror("Error receiving buffer");
					return -1;
				}
				totalBytes += inBytes;
			}
			break;

			case 1:
			inBytes = recv(sock, &size, sizeof(size), 0);
			if(inBytes < 0)
			{
				perror("Error receiving buffer size");
				return -1;
			}
			size = ntohl(size);
			tmp = (char * ) malloc(size * sizeof(char));

			totalBytes = 0;
			while(totalBytes < size)
			{
				inBytes = recv(sock, tmp, sizeof(tmp), 0);
				if(inBytes < 0)
				{
					perror("Error receiving buffer");
					return -1;
				}
				totalBytes += inBytes;
			}

			(package->nSensors) = atoi(tmp);
			free(tmp);
			break;

			case 2:
			inBytes = recv(sock, &size, sizeof(size), 0);
			if(inBytes < 0)
			{
				perror("Error receiving buffer size");
				return -1;
			}
			size = ntohl(size);
			tmp = (char * ) malloc(size * sizeof(char));

			totalBytes = 0;
			while(totalBytes < size)
			{
				inBytes = recv(sock, tmp, sizeof(tmp), 0);
				if(inBytes < 0)
				{
					perror("Error receiving buffer");
					return -1;
				}
				totalBytes += inBytes;
			}
			(package->sensorData) = atof(tmp);
			free(tmp);
			break;

			case 3:
			inBytes = recv(sock, &size, sizeof(size), 0);
			if(inBytes < 0)
			{
				perror("Error receiving buffer size");
				return -1;
			}
			size = ntohl(size);
			tmp = (char * ) malloc(size * sizeof(char));

			totalBytes = 0;
			while(totalBytes < size)
			{
				inBytes = recv(sock, tmp, sizeof(tmp), 0);
				if(inBytes < 0)
				{
					perror("Error receiving buffer");
					return -1;
				}
				totalBytes += inBytes;
			}

			(package->lowVal) = atof(tmp);
			free(tmp);
			break;

			case 4:
			inBytes = recv(sock, &size, sizeof(size), 0);
			if(inBytes < 0)
			{
				perror("Error receiving buffer size");
				return -1;
			}
			size = ntohl(size);
			tmp = (char * ) malloc(size * sizeof(char));

			totalBytes = 0;
			while(totalBytes < size)
			{
				inBytes = recv(sock, tmp, sizeof(tmp), 0);
				if(inBytes < 0)
				{
					perror("Error receiving buffer");
					return -1;
				}
				totalBytes += inBytes;
			}

			(package->highVal) = atof(tmp);
			free(tmp);
			break;

			case 5:
			inBytes = recv(sock, &size, sizeof(size), 0);
			if(inBytes < 0)
			{
				perror("Error receiving buffer size");
				return -1;
			}
			size = ntohl(size);

			totalBytes = 0;
			while(totalBytes < size)
			{
				inBytes = recv(sock, package->timestamp, sizeof(package->timestamp), 0);
				if(inBytes < 0)
				{
					perror("Error receiving buffer");
					return -1;
				}
				totalBytes += inBytes;
			}
			break;

			case 6:
			inBytes = recv(sock, &size, sizeof(size), 0);
			if(inBytes < 0)
			{
				perror("Error receiving buffer size");
				return -1;
			}
			size = ntohl(size);
			tmp = (char * ) malloc(size * sizeof(char));

			totalBytes = 0;
			while(totalBytes < size)
			{
				inBytes = recv(sock, tmp, sizeof(tmp), 0);
				if(inBytes < 0)
				{
					perror("Error receiving buffer");
					return -1;
				}
				totalBytes += inBytes;
			}

			(package->action) = atoi(tmp);
			free(tmp);
			break;
		}
	}
	
	return 0;
}

int writeToFile(void * param)
{
	int * s = (int * ) param;
	FILE * fp;
	char * year, month, day, hour, minute, group;
	struct tempdata package1, package2;

	if(unpack(&package1, *s) < 0)
	{
		perror("Error unpacking structure");
		return -1;
	}

	if(package1.action == 0)
	{
		
	}
	

	if(package1.nSensors == 2)
	{
		if(unpack(&package2, *s) < 0)
		{
			perror("Error unpacking structure");
			return -1;
		}
	}

	
	
	#ifdef DEBUG
		printf("Received info:\n");
		printf("Number of sensors: %d\n",package2.nSensors);
		printf("Host: %s\n",package2.host);
		printf("Temperature: %lf\n",package2.sensorData);
		printf("Low Threshold: %lf\n",package2.lowVal);
		printf("High Threshold: %lf\n",package2.highVal);
		printf("Time: %s\n",package2.timestamp);
		printf("Action: %s\n\n",(package2.action == 0)?"send":"request status");
	#endif
	
}

int main( int argc, char *argv[] ){

	int sockfd, port, conn;
	pthread_t id1, id2;
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
		
		pthread_create(&id1, NULL, (void * ) writeToFile, (void * ) &conn);
		//pthread_create(&id2, NULL, (void * ) writeToFile, (void * ) &conn);
		//writeToFile((void *) &conn);
		
	}


}
