/*This is the server part of a time calibration and files upload system between server and smartphones*/
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

FILE *fp;


void *connection_handler(void *socket_desc){

	int done;
	int read_size, socket, rows, eof, endtx, i, k;
	char *msgbody, *response, *p, *p2, *backup;
	char *data[1000], *data2[1000];
	struct timeb responseTime;
	char response_time[14];
	char response_miltime[14];
	int flength;

	socket = *(int*)socket_desc;

	msgbody = (char*)malloc(PKTLEN*sizeof(char));
	bzero(msgbody, PKTLEN);
	backup = (char*)malloc(PKTLEN*sizeof(char));
	bzero(backup, PKTLEN);
	response = (char*)malloc(PKTLEN*sizeof(char));
	bzero(response, PKTLEN);

	//Receive a message from client
	while( (read_size = recv(socket, msgbody, PKTLEN, 0)) > 0 ){
		printf("The first while: Receive Message:%s",msgbody);

		if(strcmp(msgbody, "starttx\n") == 0){	
			bzero(msgbody, PKTLEN);
			fp = NULL;		
			printf("Start to save files.\n");
			while((read_size = recv(socket, msgbody, PKTLEN/2, 0)) > 0){
				//strcpy(backup, msgbody);
				//printf("The second while: Receive Message:%s\n",msgbody);
				done = 0;
				while(done==0){
					printf("Enter third while: %s\n", msgbody);
					bzero(backup, PKTLEN);
					strcpy(backup, msgbody);				
	
					p = strtok(msgbody, "\n");
					printf("split msgbody to p\n");
					rows = 0;
					while (p != NULL){
						data[rows++] = p;
						p = strtok (NULL, "\n");
					}
					printf("split finished\n");
				
					printf("row: %d\n", rows);
				
					if(strcmp(data[rows-1],"endtx") != 0){
						rows--;
						if(rows == 0){
							bzero(msgbody, PKTLEN);
							if((read_size = recv(socket, msgbody, PKTLEN/2, 0)) > 0){
								flength = strlen(backup);
								read_size = strlen(msgbody);
								for(i=0; i<read_size; i++)
									backup[flength+i] = msgbody[i];
								bzero(msgbody,PKTLEN);
								strcpy(msgbody, backup);
							}
							continue;
						}
					}
					else{
						printf("Done == 1.\n");
						if(rows==1){
							done = 1;
							continue;
						}
					}	
					printf("The number of rows is:%d\n", rows);
					
					for(eof = 0; eof < rows; eof++)
						if(strcmp(data[eof],"eof") == 0)
							break;
					printf("eof:%d\n",eof);
										
					for(endtx = 0; endtx < rows; endtx++)
						if(strcmp(data[endtx],"endtx") == 0)
							break;	
					printf("endtx:%d\n",endtx);
								
						
					p2 = strtok(data[0], ":");
					data2[0] = p2;
					p2 = strtok (NULL, "\n");
					data2[1] = p2;

					flength = 0;
					if(strcmp(data2[0],"filename") == 0){
						flength = strlen(data2[0])+strlen(data2[1])+2;
						fp = NULL;
						printf("Now save file:%s\n",data2[1]);
						fp = fopen(data2[1], "a");
						if(fp == NULL){
							printf("Cannot open the csv file for writing!");
							exit(1);
						}	
						//printf("The length of first row is:%d\n, data[1]:%s\n", flength,data[1]);
					}
				
					if(eof>0 && endtx>0 && flength==0)
						fprintf(fp,"%s\n",data[0]);
					
					for(k = 1;(k<eof)&&(k<endtx); k++)
						fprintf(fp,"%s\n",data[k]);

					if(eof < rows){
						fclose(fp);
						fp = NULL;
						printf("End the processing of saving a file.\n");
						if(endtx < rows && endtx == eof+1){
							done = 1;
							continue;
						}
					}

					if(flength>0){
						for(k=1; (k<eof+1) && (k<rows); k++)
							flength = flength+strlen(data[k])+1;
					}
					else if(flength == 0){
						for(k=0;(k<eof+1) && (k<rows);k++)
							flength = flength+strlen(data[k])+1;
					}
								
					for(k=0; k<PKTLEN-flength; k++)
						backup[k] = backup[k+flength];
					for(k=PKTLEN-flength; k<PKTLEN; k++)
						backup[k] = '\0';
					bzero(msgbody, PKTLEN);
					strcpy(msgbody, backup);

					if(eof==rows){
						bzero(msgbody, PKTLEN);
						if((read_size = recv(socket, msgbody, PKTLEN/2, 0)) > 0){
							flength = strlen(backup);
							for(i=0; i<read_size; i++)
								backup[flength+i] = msgbody[i];
							bzero(msgbody, PKTLEN);
							strcpy(msgbody, backup);
						}
						continue;
					}
				}	

				printf("All the files are saved");
				strcat(response, "All the files are saved!\r\n");
				write(socket, response, PKTLEN);
				
				bzero(msgbody, PKTLEN);
				bzero(response, PKTLEN);

				break;
			}			
		}
		else{	
			p = strtok(msgbody, ",");
			i = 0;
			while (p != NULL){
				data[i++] = p;
				p = strtok (NULL, ",");
			}

			//printf("\n%s\n",data[0]);

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
				//printf("Send Message:%s",response);

				write(socket, response, PKTLEN);
			}
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
