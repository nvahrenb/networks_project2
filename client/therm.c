/*
Chris Ray
Nathan Vahrenberg

CSE30264 - Computer Networks
Project 2 - Thermal Sensor Client
*/

/* Go! Temp reader code used in getSensorInfo() function
written by Jeff Sadowski <jeff.sadowski@gmail.com>
*/

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include <errno.h>

#define DPORT 9765

double getSensorInfo(int i){

	char* fileName;

	if( i == 1 ){
		fileName = "/dev/gotemp";
	}else if( i == 2){
		fileName = "/dev/gotemp2";
	}else{
		return 0;
	}
	
	struct packet {
		unsigned char measurements;
		unsigned char counter;
		int16_t measurement0;
		int16_t measurement1;
		int16_t measurement2; 
	};
	
	int fd = open(fileName,O_RDONLY);
	
	struct packet temp;
	
	read(fd,&temp,sizeof(temp));
	
	float conversion=0.0078125;
	return (temp.measurement0*conversion);
}

struct tempdata{
	char host[32];
	int nSensors;
	double sensorData;
	double lowVal;
	double highVal;
	char timestamp[32];
	int action;
};

int packAndSend(struct tempdata * package, int sock)
{
	int i, outBytes;
	uint32_t size;
	char * str = (char*)malloc(12);

	for(i = 0; i < 7; i++)
	{
		switch(i)
		{
			case 0:
			size = htonl(strlen(package->host));
			outBytes = send(sock, &size, sizeof(size), 0);
			if(outBytes < 0)
			{
				perror("Error sending buffer size");
				return -1;
			}

			outBytes = send(sock, package->host, sizeof(package->host), 0);
			if(outBytes < 0)
			{
				perror("Error sending buffer");
				return -1;
			}
			break;

			case 1:
			sprintf(str, "%d", package->nSensors);
			size = htonl(strlen(str));
			outBytes = send(sock, &size, sizeof(size), 0);
			if(outBytes < 0)
			{
				perror("Error sending buffer size");
				return -1;
			}

			outBytes = send(sock, str, sizeof(str), 0);
			if(outBytes < 0)
			{
				perror("Error sending buffer");
				return -1;
			}
			break;

			case 2:
			sprintf(str, "%f", package->sensorData);
			size = htonl(strlen(str));
			outBytes = send(sock, &size, sizeof(size), 0);
			if(outBytes < 0)
			{
				perror("Error sending buffer size");
				return -1;
			}

			outBytes = send(sock, str, sizeof(str), 0);
			if(outBytes < 0)
			{
				perror("Error sending buffer");
				return -1;
			}
			break;

			case 3:
			sprintf(str, "%f", package->lowVal);
			size = htonl(strlen(str));
			outBytes = send(sock, &size, sizeof(size), 0);
			if(outBytes < 0)
			{
				perror("Error sending buffer size");
				return -1;
			}

			outBytes = send(sock, str, sizeof(str), 0);
			if(outBytes < 0)
			{
				perror("Error sending buffer");
				return -1;
			}
			break;

			case 4:
			sprintf(str, "%f", package->highVal);
			size = htonl(strlen(str));
			outBytes = send(sock, &size, sizeof(size), 0);
			if(outBytes < 0)
			{
				perror("Error sending buffer size");
				return -1;
			}

			outBytes = send(sock, str, sizeof(str), 0);
			if(outBytes < 0)
			{
				perror("Error sending buffer");
				return -1;
			}
			break;

			case 5:
			size = htonl(strlen(package->timestamp));
			outBytes = send(sock, &size, sizeof(size), 0);
			if(outBytes < 0)
			{
				perror("Error sending buffer size");
				return -1;
			}

			outBytes = send(sock, package->timestamp, sizeof(package->timestamp), 0);
			if(outBytes < 0)
			{
				perror("Error sending buffer");
				return -1;
			}
			break;

			case 6:
			sprintf(str, "%d", package->action);
			size = htonl(strlen(str));
			outBytes = send(sock, &size, sizeof(size), 0);
			if(outBytes < 0)
			{
				perror("Error sending buffer size");
				return -1;
			}

			outBytes = send(sock, str, sizeof(str), 0);
			if(outBytes < 0)
			{
				perror("Error sending buffer");
				return -1;
			}
			break;
		}

		i++;
	}

	free(str);
	return 0;
}

int main( int argc, char *argv[] ){

	
	struct tempdata temp0, temp1;

	int sockfd, port;
	struct sockaddr_in serverAddr;
	struct hostent *hostname;
	FILE *fp;
	
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
	
	
	// Read sensor info to struct
	
	int numSensors = 0;
	char *cLowTemp = (char*)malloc(5); 
	char *cHighTemp = (char*)malloc(5);
	fp = fopen("/etc/t_client/client.conf","r");
	
	if(fp == NULL){ printf("File open error\n"); }
	char *line = NULL;
	size_t len;
	getline(&line, &len, fp);
	
	numSensors = atoi(line);
	temp0.nSensors = numSensors;
	temp1.nSensors = numSensors;
	
	struct tm *systime;
	time_t rtime;
	time( &rtime );
	systime = localtime( &rtime );
	char timestamp[32];
	strcpy(timestamp, asctime(systime));
	int i = 0;
  while(timestamp[i] != '\n'){
  	i++;
  }
  timestamp[i] = 0;
	
	if(numSensors >= 1){
		
		line = NULL;
		getline(&line, &len, fp);
		strncpy(cLowTemp, line, 4);
		strncpy(cHighTemp, line+5, 4);
		temp0.lowVal = atof(cLowTemp);
		temp0.highVal = atof(cHighTemp);
		
		temp0.nSensors = numSensors;
		gethostname(temp0.host, len);
		temp0.sensorData = getSensorInfo(1);
		strcpy(temp0.timestamp, timestamp);
		temp0.action = 0;
		
		#ifdef DEBUG
			printf("Sensor 0 info:\n");
			printf("Number of sensors: %d\n",temp0.nSensors);
			printf("Host: %s\n",temp0.host);
			printf("Temperature: %lf\n",temp0.sensorData);
			printf("Low Threshold: %lf\n",temp0.lowVal);
			printf("High Threshold: %lf\n",temp0.highVal);
			printf("Time: %s\n",temp0.timestamp);
			printf("Action: %s\n\n",(temp0.action == 0)?"send":"request status");
		#endif
	
	}
	
	if(numSensors == 2){
		
		line = NULL;
		getline(&line, &len, fp);
		strncpy(cLowTemp, line, 4);
		strncpy(cHighTemp, line+5, 4);
		temp1.lowVal = atof(cLowTemp);
		temp1.highVal = atof(cHighTemp);
		
		temp0.nSensors = numSensors;
		gethostname(temp0.host, len);
		temp0.sensorData = getSensorInfo(2);
		strcpy(temp0.timestamp, timestamp);
		temp0.action = 0;
	
	
	
		#ifdef DEBUG
			printf("Sensor 1 info:\n");
			printf("Number of sensors: %d\n",temp0.nSensors);
			printf("Host: %s\n",temp0.host);
			printf("Temperature: %lf\n",temp0.sensorData);
			printf("Low Threshold: %lf\n",temp0.lowVal);
			printf("High Threshold: %lf\n",temp0.highVal);
			printf("Time: %s\n",temp0.timestamp);
			printf("Action: %s\n\n",(temp0.action == 0)?"send":"request status");
		#endif
		
	}
	
	// Send struct(s) to server
	
	// Send struct 1
	if(numSensors >= 1){
		//write(sockfd, (char *)&temp0, sizeof(temp0));
		if(packAndSend(&temp0, sockfd) < 0)
		{
			perror("Error sending temp0");
			return 0;
		}
	}
	
	// Send struct 2
	if(numSensors == 2){
		//write(sockfd, (char *)&temp1, sizeof(temp1));
		if(packAndSend(&temp1, sockfd) < 0)
		{
			perror("Error sending temp0");
			return 0;
		}
	}
	
	fclose(fp);
	free(cLowTemp);
	free(cHighTemp);
}
