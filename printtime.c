/*This is a printtime program*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/timeb.h>

#define PORTNO 5050
#define PKTLEN 10000

void main(){

	int done;
	int read_size, socket, rows, eof, endtx, i, k;
	char *msgbody, *response, *p, *p2, *backup;
	char *data[1000], *data2[1000];
	struct timeb responseTime;
	char response_time[14];
	char response_miltime[14];


	response = (char*)malloc(PKTLEN*sizeof(char));
	bzero(response, PKTLEN);

	while(1){

		bzero(response, PKTLEN);
		ftime(&responseTime);
        	sprintf(response_time, "%ld", responseTime.time);
        	sprintf(response_miltime, "%03d", responseTime.millitm);
        	strcat(response, response_time);
        	strcat(response, response_miltime);
        	strcat(response, "\n");
        	printf("%s",response);
	}

	free(msgbody);
	free(response);

}

