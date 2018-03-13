#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/timeb.h>

#define PORTNO 5030
#define PKTLEN 256

void *connection_handler(void *socket_desc){

	int read_size, socket, i;
	char *msgbody, *response, *p;
	char *data[4];
	struct timeb responseTime;
	char response_time[14];
	char response_miltime[14];

	socket = *(int*)socket_desc;

	msgbody = (char*)malloc(PKTLEN*sizeof(char));
	bzero(msgbody, PKTLEN);
	response = (char*)malloc(PKTLEN*sizeof(char));
	bzero(response, PKTLEN);

	//Receive a message from client
	while( (read_size = recv(socket, msgbody, PKTLEN, 0)) > 0 ){

		// split network pakets with ","
		printf("Receive Message:%s",msgbody);
		p = strtok(msgbody, ",");
		i = 0;
		while (p != NULL){
			data[i++] = p;
			p = strtok (NULL, ",");
		}

		//printf(data[0]);

		if (strcmp(data[0],"timesync") == 0){
			/*
				Do anything you want in here
			*/
			
			// response
			strcat(response, "timesync,");
			//UUID
			strcat(response, data[1]);
			strcat(response, ",");

			// server response time

			ftime(&responseTime);
			sprintf(response_time, "%ld", responseTime.time);
			sprintf(response_miltime, "%03d", responseTime.millitm);
			strcat(response, response_time);
			strcat(response, response_miltime);	
			strcat(response, "\n");
			printf("Send Message:%s",response);

			write(socket, response, PKTLEN);
		}

		bzero(msgbody, PKTLEN);
		bzero(response, PKTLEN);

	}

	free(msgbody);
	free(response);

	return 0;
}

int main(int argc , char *argv[]){

	int socket_desc, c;
	int *client_sock;
	struct sockaddr_in server , client;
	pthread_t *thread_id, *thread_mqtt;

	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1){
		puts("Could not create socket");
		return 1;
	}
	puts("Socket created.");

	int reuseaddr = 1;
	if (setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(int)) < 0)
		puts("Set socket reuse address failed");

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(PORTNO);

	//Bind
	if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0){
		//print the error message
		perror("bind failed. Error");
		return 1;
	}
	puts("Finished binding.");

	//Listen
	puts("Start listening.");
	while(0==0){
		listen(socket_desc, 3);
		//Accept and incoming connection
		puts("Waiting for incoming connections...");
		c = sizeof(struct sockaddr_in);

		thread_id = (pthread_t *)malloc(sizeof(pthread_t));
		client_sock = (int *)malloc(sizeof(int));

		while((*client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c))){
			puts("Connection accepted");

			if( pthread_create( thread_id , NULL ,  connection_handler , (void*) client_sock) < 0){
				perror("could not create thread");
				return 1;
			}

			puts("New thread created.");
		}

		if (*client_sock < 0){
			perror("accept failed");
			return 1;
		}

	}

	return 0;

}

// vim: ts=4 sw=4 noexpandtab
