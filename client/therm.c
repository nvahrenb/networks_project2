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

//#define DEBUG

//function that returns a sensor temp reading in F
double getSensorInfo(int i){

	char * fileName;

	if( i == 1 ){
		fileName = "/dev/gotemp";	//read a different sensor based on the value of i
	}else if( i == 2){
		fileName = "/dev/gotemp2";
	}else{
		return 0;
	}
	
	struct packet {				//structure to hold sensor information
		unsigned char measurements;
		unsigned char counter;
		int16_t measurement0;
		int16_t measurement1;
		int16_t measurement2; 
	};
	
	int fd = open(fileName,O_RDONLY);	//opening the file associated with the desired sensor
	
	struct packet temp;
	
	read(fd,&temp,sizeof(temp));		//placing the relevant information into the temp structure
	
	double conversion=0.0078125;		//conversion factor from original code
	return (temp.measurement0*conversion * 9 / 5 + 32);	//returning the temperature in F
}

struct tempdata{	//structure to hold information about a host and its sensors
	char host[32];
	int nSensors;
	double sensorData;
	double lowVal;
	double highVal;
	char timestamp[32];
	int action;	
};

//function to set all numeric elements of a tempdata struct to 0 if you want a request status packet
void requestStatus(struct tempdata * temp)
{
	(temp->nSensors) = 0;
	(temp->sensorData) = 0;
	(temp->lowVal) = 0;
	(temp->highVal) = 0;
	(temp->action) = 1;
}

//function for serializing a struct in order to be sent accross the network
int packAndSend(struct tempdata * package, int sock)
{
	int i, outBytes;	
	uint32_t size;
	char * str = (char*)malloc(12);
	FILE *error_fp = fopen("/var/log/therm/error/g04_error_log","a"); //opening file where all error messages will be sent

	for(i = 0; i < 7; i++)	//iterate 0-6, converting all necessary data members of package to text before sending to the server
	{
		switch(i)
		{
			case 0:	//sending hostname of client
			size = htonl(sizeof(package->host));
			outBytes = send(sock, &size, sizeof(size), 0);	//sending size so server knows how many bytes to receive
			if(outBytes < 0)
			{
				perror("Error sending buffer size");
				fprintf(error_fp,"Error sending buffer size");
				return -1;
			}

			outBytes = send(sock, package->host, sizeof(package->host), 0); //sending actual data member
			if(outBytes < 0)
			{
				perror("Error sending buffer");
				fprintf(error_fp,"Error sending buffer");
				return -1;
			}
			break;

			case 1:	//sending number of sensors of client
			sprintf(str, "%d", package->nSensors);	//converting numeric data member to a string 
			size = htonl(sizeof(str));
			outBytes = send(sock, &size, sizeof(size), 0);
			if(outBytes < 0)
			{
				perror("Error sending buffer size");
				fprintf(error_fp,"Error sending buffer size");
				return -1;
			}

			outBytes = send(sock, str, sizeof(str), 0);
			if(outBytes < 0)
			{
				perror("Error sending buffer");
				fprintf(error_fp,"Error sending buffer");
				return -1;
			}
			break;

			case 2:	//sending sensor data (temp reading) from client 
			sprintf(str, "%f", package->sensorData);
			size = htonl(sizeof(str));
			outBytes = send(sock, &size, sizeof(size), 0);
			if(outBytes < 0)
			{
				perror("Error sending buffer size");
				fprintf(error_fp,"Error sending buffer size");
				return -1;
			}

			outBytes = send(sock, str, sizeof(str), 0);
			if(outBytes < 0)
			{
				perror("Error sending buffer");
				fprintf(error_fp,"Error sending buffer");
				return -1;
			}
			break;

			case 3: //sending low temperature value for this particular host
			sprintf(str, "%f", package->lowVal);
			size = htonl(sizeof(str));
			outBytes = send(sock, &size, sizeof(size), 0);
			if(outBytes < 0)
			{
				perror("Error sending buffer size");
				fprintf(error_fp,"Error sending buffer size");
				return -1;
			}

			outBytes = send(sock, str, sizeof(str), 0);
			if(outBytes < 0)
			{
				perror("Error sending buffer");
				fprintf(error_fp,"Error sending buffer");
				return -1;
			}
			break;

			case 4:	//sending high temperature value for this particular host
			sprintf(str, "%f", package->highVal);
			size = htonl(sizeof(str));
			outBytes = send(sock, &size, sizeof(size), 0);
			if(outBytes < 0)
			{
				perror("Error sending buffer size");
				fprintf(error_fp,"Error sending buffer size");
				return -1;
			}

			outBytes = send(sock, str, sizeof(str), 0);
			if(outBytes < 0)
			{
				perror("Error sending buffer");
				fprintf(error_fp,"Error sending buffer");
				return -1;
			}
			break;

			case 5:	//sending timestamp string 
			size = htonl(sizeof(package->timestamp));
			outBytes = send(sock, &size, sizeof(size), 0);
			if(outBytes < 0)
			{
				perror("Error sending buffer size");
				fprintf(error_fp,"Error sending buffer size");
				return -1;
			}

			outBytes = send(sock, package->timestamp, sizeof(package->timestamp), 0);
			if(outBytes < 0)
			{
				perror("Error sending buffer");
				fprintf(error_fp,"Error sending buffer");
				return -1;
			}
			break;

			case 6: //sending action value
			sprintf(str, "%d", package->action);
			size = htonl(sizeof(str));
			outBytes = send(sock, &size, sizeof(size), 0);
			if(outBytes < 0)
			{
				perror("Error sending buffer size");
				fprintf(error_fp,"Error sending buffer size");
				return -1;
			}

			outBytes = send(sock, str, sizeof(str), 0);
			if(outBytes < 0)
			{
				perror("Error sending buffer");
				fprintf(error_fp,"Error sending buffer");
				return -1;
			}
			break;
		}
	}
	
	fclose(error_fp);	//closing error log file
	free(str);	//freeing string used in numeric conversions
	return 0;
}

int main( int argc, char *argv[] ){

	
	struct tempdata temp0, temp1;	//temp0 and temp1 will hold the information for the 2 sensors on a host

	int sockfd, port;
	struct sockaddr_in serverAddr;
	struct hostent *hostname;
	FILE * fp;
	FILE *error_fp = fopen("/var/log/therm/error/g04_error_log","a");	//opening error log file
	
	int exit = 0;	//this value should never change (i.e. server code should run indefinitely)
	
	// Open socket
	if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ){
		perror( "Unable to open socket" ); return 0;
		fprintf(error_fp,"Unable to open socket");
	}
	
	// Get host address from 1st arg
	if( (hostname = gethostbyname(argv[1])) == 0 ){
		perror( "Unable to get address of host"); return 0;
		fprintf(error_fp,"Unable to get address of host");
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
		perror("Unable to connect to host");
		fprintf(error_fp,"Unable to connect to host");
		return 0;
	}
	
	#ifdef DEBUG
		printf("Connected to host %s (%s) on port %d\n",argv[1],inet_ntoa(serverAddr.sin_addr),port);
	#endif
	
	
	// Read sensor info from config file
	int numSensors = 0;
	char *cLowTemp = (char*)malloc(5); 
	char *cHighTemp = (char*)malloc(5);
	fp = fopen("/etc/t_client/client.conf","r");
	
	if(fp == NULL){
		printf("File open error\n"); 
		fprintf(error_fp,"File open error");
	}
	char *line = NULL;
	size_t len;
	getline(&line, &len, fp);
	
	numSensors = atoi(line);
	temp0.nSensors = numSensors;	//adding number of sensors for the host
	temp1.nSensors = numSensors;
	
	struct tm *systime;		//generating timestamps for each struct
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
	
	if(numSensors >= 1){	//filling in the rest of the data members for 1 struct 
		
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
	
	if(numSensors == 2){ //filling in the data members for the other struct if a 2nd sensor is detected
		
		line = NULL;
		getline(&line, &len, fp);
		strncpy(cLowTemp, line, 4);
		strncpy(cHighTemp, line+5, 4);
		temp1.lowVal = atof(cLowTemp);
		temp1.highVal = atof(cHighTemp);
		
		temp1.nSensors = numSensors;
		gethostname(temp1.host, len);
		temp1.sensorData = getSensorInfo(2);
		strcpy(temp1.timestamp, timestamp);
		temp1.action = 0;
	
	
	
		#ifdef DEBUG
			printf("Sensor 1 info:\n");
			printf("Number of sensors: %d\n",temp1.nSensors);
			printf("Host: %s\n",temp1.host);
			printf("Temperature: %lf\n",temp1.sensorData);
			printf("Low Threshold: %lf\n",temp1.lowVal);
			printf("High Threshold: %lf\n",temp1.highVal);
			printf("Time: %s\n",temp1.timestamp);
			printf("Action: %s\n\n",(temp1.action == 0)?"send":"request status");
		#endif
		
	}
	
	// Send struct(s) to server
	
	// Send struct 1
	if(numSensors >= 1){	//we will only send the 1st struct if there is only 1 sensor (no structs are sent if 0 sensors)
		if(packAndSend(&temp0, sockfd) < 0)	//serializing the struct and sending it to the server
		{
			perror("Error sending temp0");
			fprintf(error_fp,"Error sending temp0");
			return 0;
		}
	}
	
	// Send struct 2
	if(numSensors == 2){	//sending the 2nd struct since there if there is a 2nd sensor
		if(packAndSend(&temp1, sockfd) < 0)
		{
			perror("Error sending temp1");
			fprintf(error_fp,"Error sending temp1");
			return 0;
		}
	}

	
	fclose(error_fp);	//closing the error log file
	fclose(fp);		//closing the config file
	free(cLowTemp);		//freeing the strings used for high and low temp
	free(cHighTemp);	
	close(sockfd);		//closing the socket
}
