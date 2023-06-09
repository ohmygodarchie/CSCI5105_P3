/*
 * This is sample code generated by rpcgen.
 * These are only templates and you can use them
 * as a guideline for developing your own functions.
 */

#include "communicate.h"
#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

// rpc connection object
CLIENT *clnt;


// setup the rpc connection object:
CLIENT *setup_connection(char *con_server_ip, char *con_server_port) {
	
	// create this servers socket 
	// note, this server acts as a client to other servers
	// and has client connection "objects" for them
	printf("%s  %s\n", con_server_ip, con_server_port);
	int sockfd;
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("socket creation failed");
		return NULL;
	}
	
	// fill in information about the given server
	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	   
	servaddr.sin_family = AF_INET;
	int server_port_int = atoi(con_server_port);
	servaddr.sin_port = htons(server_port_int);

	//fill in the IP address as it's binary representation
	if (inet_pton(AF_INET, con_server_ip, &servaddr.sin_addr) <= 0) {
		printf("Invalid IP address\n");
		close(sockfd);
		return NULL;
	}

	//setup the timeout for rpc calls
	struct timeval wait;
	memset(&wait, 0, sizeof(wait));
	wait.tv_sec = 30;

	// create the object
	CLIENT *clnt = clntudp_create (&servaddr, COMMUNICATE_PROG, COMMUNICATE_VERSION, wait, &sockfd);
	if (clnt == NULL) {
		clnt_pcreateerror (con_server_ip);
		return NULL;
	}

	return clnt;
}

void cmd_loop() {
	int usrBuf_len = 200;
	char usrBuf[usrBuf_len];
	NodeList *result_1;
	int *result_2;
	int *result_3;
	FileList *result_4;

	// setup rpc args and vars.
	//cmd loop
	while(1) {
		//allow the client to provide a command
		
		printf("> ");


		//fill in the usrBuf with the command and command args
		memset(usrBuf, 0, usrBuf_len);
		fgets(usrBuf, usrBuf_len, stdin);
		//reply newline with null terminator
		for (int i = 0; i < usrBuf_len; i++) {
			if (usrBuf[i] == '\n') {
				usrBuf[i] = '\0';
				break;
			}
		}

		//printf("usrbuf: %s", usrBuf);

		//get the first token, which is the command
		char *token = strtok(usrBuf, " ");
		if (token == (char *)NULL) {
			//command is either ping or exit
			token = usrBuf;
		}

		// execute the needed rpc command

		if (strcmp(token, "exit") == 0){
			return;

		} else if (strcmp(token, "find") == 0){
			// rpc call
			char *text = strtok(NULL, "");
			if (text == NULL) {
				printf("find failed, no filename is provided\n");
				continue;
			}

			result_1 = find_1(text, clnt);
			
			if (result_1 == (NodeList *)NULL) {
				clnt_perror (clnt, "call failed");
			} 
			//need to print
			else {
				for (int i =0; i<result_1->numNodes; i++) {
					printf("%s %d\n", result_1->nodes[i].ip, result_1->nodes[i].port);
				}
			}


		} else if (strcmp(token, "download") == 0){
			//bug just get whole text

			char *text = strtok(NULL, "");
			if (text == NULL) {
				printf("download failed, no filename was provided\n");
				continue;
			}

			int* result_5 = download_1(text, clnt); //client here is the "server.c" associated with the this client.c


			// display rpc result

			if (result_5 == (int *) NULL) {
				clnt_perror (clnt, "call failed");
			} else {
				printf("downloaded successfully\n");
			}
			
		} else if (strcmp(token, "getload") == 0){
			// get arg from cmd

			char *peer_ip = strtok(NULL, " ");

			if (peer_ip == NULL) {
				printf("No IP provided\n");
				continue;
			}

			char *peer_port_str = strtok(NULL, " ");

			if (peer_port_str == NULL) {
				printf("No port provided\n");
				continue;
			}

			int peer_port = atoi(peer_port_str);

			// rpc call

			result_2 = getload_1(peer_ip, peer_port, clnt);

			if (result_2 == (int *) NULL) {
				clnt_perror (clnt, "call failed");
			} else {
				printf("load: %d\n", *result_2);
			}


		} else if (strcmp(token, "updatelist") == 0){
			// get arg from cmd
			char *peer_ip = strtok(NULL, " ");

			if (peer_ip == NULL) {
				printf("No IP provided\n");
				continue;
			}

			char *peer_port_str = strtok(NULL, " ");

			if (peer_port_str == NULL) {
				printf("No port provided\n");
				continue;
			}

			int peer_port = atoi(peer_port_str);
			result_4 = updatelist_1(peer_ip, peer_port, clnt); // need to update to take in these args
			// display rpc result


			if (result_4 == (FileList *) NULL) {
				clnt_perror (clnt, "call failed");
			} else {
				printf("updated successfully\n");
				//print the list
				for (int i = 0; i < result_4->fileAmount; i++) {
					printf("%s\n", result_4->files[i].name);
				}
			}

		} else {
			printf("command not found\n");
		}

	}
}



int
main (int argc, char *argv[])
{
	char *host;

	if (argc < 4) {
		printf ("usage: %s server_ip server_port directory\n", argv[0]);
		exit (1);
	}
	char *con_server_ip = argv[1];
	char *con_server_port_str = argv[2];

	char *directory = argv[3];

	clnt = setup_connection(con_server_ip, con_server_port_str);
	if (clnt == NULL) {
		clnt_pcreateerror (con_server_ip);
		exit(EXIT_FAILURE);
	}

	cmd_loop();

	//close the connection
	clnt_destroy(clnt);
	exit (0);
}
