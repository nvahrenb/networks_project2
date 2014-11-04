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
#include <string.h>

#include <errno.h>

#include <pthread.h>

#define DPORT 9765

//#define DEBUG

struct tempdata{	//structure to hold information about a host and its sensors
	char host[32];
	int nSensors;
	double sensorData;
	double lowVal;
	double highVal;
	char timestamp[32];
	int action;
};

//function for receiving, converting, and placing information sent from a client into a temp data struct
int unpack(struct tempdata * package, int sock)
{
	int i, inBytes;
	char * tmp;
	uint32_t size, totalBytes;

	for(i = 0; i < 7; i++)	//iterate 0-6, filling in each data member of the struct as the information is received
	{
		switch(i)
		{
			case 0:	//receiving host name
			inBytes = recv(sock, &size, sizeof(size), 0);	//getting size of string
			if(inBytes < 0)
			{
				perror("Error receiving buffer size");
				return -1;
			}
			size = ntohl(size);

			totalBytes = 0;
			while(totalBytes < size)	//keep receiving information until the full string is received
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

			case 1:	//receiving sensor data
			inBytes = recv(sock, &size, sizeof(size), 0);
			if(inBytes < 0)
			{
				perror("Error receiving buffer size");
				return -1;
			}
			size = ntohl(size);
			tmp = (char * ) malloc(size * sizeof(char)); //allocating buffer space

			totalBytes = 0;
			while(totalBytes < size)	//storing string in tmp buffer
			{
				inBytes = recv(sock, tmp, sizeof(tmp), 0);
				if(inBytes < 0)
				{
					perror("Error receiving buffer");
					return -1;
				}
				totalBytes += inBytes;
			}

			(package->nSensors) = atoi(tmp); //converting text information back into numeric form
			free(tmp);	//freeing buffer
			break;

			case 2:	//receiving sensor data (temp reading)
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

			case 3:	//receiving low temp value
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

			case 4:	//receiving high temp value
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

			case 5:	//receiving time stamp string
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

			case 6:	//receiving action value
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

//function for converting the month given by time stamp into an integer
int convertMonth(char * month)
{
	if(!strcmp(month, "Jan"))
	{
		return 1;
	}
	else if(!strcmp(month, "Feb"))
	{
		return 2;
	}
	else if(!strcmp(month, "Mar"))
	{
		return 3;
	}
	else if(!strcmp(month, "Apr"))
	{
		return 4;
	}
	else if(!strcmp(month, "May"))
	{
		return 5;
	}
	else if(!strcmp(month, "Jun"))
	{
		return 6;
	}
	else if(!strcmp(month, "Jul"))
	{
		return 7;
	}
	else if(!strcmp(month, "Aug"))
	{
		return 8;
	}
	else if(!strcmp(month, "Sep"))
	{
		return 9;
	}
	else if(!strcmp(month, "Oct"))
	{
		return 10;
	}
	else if(!strcmp(month, "Nov"))
	{
		return 11;
	}
	else if(!strcmp(month, "Dec"))
	{
		return 12;
	}
	else
	{
		return 0;
	}
}

//function for writing to the appropriate file once a client is connected
void *  writeToFile(void * param)
{
	int s = * (int * ) param;
	FILE * fp;
	char * year, * month, * day, * weekDay, * hour, * minute, * second;
	char  * group;
	char fileName[50] = {0};
	char fileString[50] = {0};
	struct tempdata package1, package2;

	if(unpack(&package1, s) < 0)	//unpacking information about 1st sensor
	{
		perror("Error unpacking structure");
		return 0;
	}

	if(package1.nSensors == 2)
	{
		if(unpack(&package2, s) < 0)	//unpacking information about 2nd sensor if avaiable
		{
			perror("Error unpacking structure");
			return 0;
		}
		if(package1.action == 0)	//storing sensor information in the specified file if not a request status packet
		{
			day = strtok(package1.timestamp, " :");
			month = strtok(NULL, " :"); 
			weekDay = strtok(NULL, " :"); 
			hour = strtok(NULL, " :"); 
			minute = strtok(NULL, " :");
			second = strtok(NULL, " :"); 
			year = strtok(NULL, " :");
			group = "g04";
			sprintf(fileName, "/var/log/therm/temp_logs/%s_%s_%d_%s", group, year, convertMonth(month), package1.host);
			sprintf(fileString, "%s %d %s %s %s %5.2f %5.2f", year, convertMonth(month), weekDay, hour, minute, package1.sensorData, package2.sensorData);
			fp = fopen(fileName, "a");
			fprintf(fp, "%s \n", fileString);
			fclose(fp);
		
		}
	}
	else if(package1.action == 0)
	{
		day = strtok(package1.timestamp, " :");
		month = strtok(NULL, " :"); 
		weekDay = strtok(NULL, " :"); 
		hour = strtok(NULL, " :"); 
		minute = strtok(NULL, " :");
		second = strtok(NULL, " :"); 
		year = strtok(NULL, " :");
		group = "g04";
		sprintf(fileName, "/var/log/therm/temp_logs/%s_%s_%d_%s", group, year, convertMonth(month), package1.host);
		sprintf(fileString, "%s %d %s %s %s %f.2", year, convertMonth(month), weekDay, hour, minute, package1.sensorData);

		fp = fopen(fileName, "a");
		fprintf(fp, "%s \n", fileString);
		fclose(fp);
		
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

	
	free(param);	//freeing the socket identifier passed to this function
}

int main( int argc, char *argv[] ){

	int sockfd, port, conn, pid, * new_sock;
	pthread_t id1, id2, id3, id4;
	struct sockaddr_in clientAddr, serverAddr;
	socklen_t csize = sizeof(clientAddr);
	
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
	while((conn = accept(sockfd, (struct sockaddr *)&clientAddr, &csize))){
	
		// Accept a connection
		
		#ifdef DEBUG
			printf("Client connected from %s\n",inet_ntoa(clientAddr.sin_addr));
		#endif
		
		new_sock = malloc(1);
		*new_sock = conn;
		if(pthread_create(&id1, NULL, writeToFile, (void *) new_sock) < 0) //creating a new thread with each connection
		{
			perror("Could not create thread");
			return 0;
		}
	}

	if(conn < 0)
	{
		perror("Accept failed");
		return 0;
	}
}
