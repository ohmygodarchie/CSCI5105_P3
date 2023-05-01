/*
 * This is sample code generated by rpcgen.
 * These are only templates and you can use them
 * as a guideline for developing your own functions.
 */

#include "communicate.h"
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

NodeList *node_list;

CLIENT* trackingserver;

int server_port;
char *server_port_str;
char *server_ip;
char *dir;
int num_normal_nodes;
CLIENT **servers;
NodeList *global_node_list;
CLIENT *tracking_server;

int amPrimary = FALSE;


//------------------------------- CLIENT CREATE -------------------------------
CLIENT *setup_connection(char *con_server_ip, int con_server_port) {
	//printf("creating connection with ip %s, port %s\n", con_server_ip, con_server_port);

	// create this servers socket 
	// note, this server acts as a client to other servers
	// and has client connection "objects" for them
	int sockfd;
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("socket creation failed");
		return NULL;
	}
	
	// fill in information about the given server
	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	   
	servaddr.sin_family = AF_INET;
	int server_port_int = con_server_port;
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
	wait.tv_sec = 2;

	// create the object
	CLIENT *clnt = clntudp_create (&servaddr, COMMUNICATE_PROG, COMMUNICATE_VERSION, wait, &sockfd);
	if (clnt == NULL) {
		clnt_pcreateerror (con_server_ip);
		return NULL;
	}

	return clnt;
}

int setup_connections() { //most of this stuff is for the tracking server but every node needs to know if it is the primary or not
	// open the file:

	FILE *fp;

	fp = fopen("nodes.txt", "r");
	if (fp == NULL) {
		printf("Failed to setup connections, could not open nodes.txt\n");
		return FALSE;
	}

	int max_line_size = 255;
	char linebuf[max_line_size];


	// count the number of lines:

	int num_lines = 0;
	while(fgets(linebuf, sizeof(linebuf), fp)){
		num_lines++;
	}

	rewind(fp);


	// malloc space for connections

	// subtract 1 because primary con is dealt with separately

	num_normal_nodes = num_lines - 1;
	servers = malloc(num_normal_nodes * sizeof(CLIENT *));
	global_node_list = malloc(sizeof(NodeList));
	global_node_list->numNodes = 0;

	if (servers == NULL) {
		printf("malloc failed\n");
		fclose(fp);
		return FALSE;
	}


	// get connection information from file:

	bool_t foundPrimary = FALSE;

	for(int i = 0; i < num_lines; i++) {
		// each line contains a server ip and port, and then may contain the word Primary.
		fgets(linebuf, sizeof(linebuf), fp);

		char *con_server_ip = strtok(linebuf, " ");

		char *con_server_port_str = strtok(NULL, " ");
		int con_server_port_int = atoi(con_server_port_str);

		char *con_server_option = strtok(NULL, " ");

		//printf("%s %s", con_server_ip, con_server_port_str);

		// two cases either this is the primary line or it's a normal line
		if ( (con_server_option != NULL) && ((strncmp(con_server_option, "tracking", 8) == 0) || (strncmp(con_server_option, "tracking", 8) == 0)) ) {
			foundPrimary = TRUE;

			// two cases, either we are the primary or we must be able to connect to the primary
			if ( (strcmp(server_ip, con_server_ip) == 0) && (server_port == con_server_port_int) ) {
				amPrimary = TRUE;
			} else {
				tracking_server = setup_connection(con_server_ip, con_server_port_int);
				if (tracking_server == NULL) {
					clnt_pcreateerror (con_server_ip);
					free(servers);
					free(global_node_list);
					fclose(fp);
					return FALSE;
				}
			}

		} else {
			// two cases, either we are the normal server or we must be able to connect to the normal server
			if ( (strcmp(server_ip, con_server_ip) == 0) && (server_port == con_server_port_int) ) {
				continue;
			} else {
				servers[global_node_list->numNodes] = setup_connection(con_server_ip, con_server_port_int);
				if (servers[global_node_list->numNodes] == NULL) {
					clnt_pcreateerror (con_server_ip);
				}
				// save info about servers i
				Node node;
				strcpy(node.ip, con_server_ip);

				// saved server port may have a newline
				node.port = con_server_port_int;
				global_node_list->nodes[global_node_list->numNodes] = node;
				global_node_list->numNodes++;

			}
		}
		if (global_node_list->numNodes == num_normal_nodes){
			printf("All nodes in the arr\n");
		}

	} 

	// check if file didn't have primary
	if (foundPrimary == FALSE) {
		printf("nodes.txt does not have a tracking server info\n");
		free(servers);
		free(global_node_list);
		fclose(fp);
		return FALSE;
	}

	fclose(fp);

	return TRUE;
}

// initialization fcn, which is called from main in communicate_svc
void initialize(char *_server_ip, char *_server_port_str, char *_dir) {
	// set state to given args

	server_port_str = _server_port_str;
	server_port = atoi(_server_port_str);
	server_ip = _server_ip;

	// TODO: ADD CHECK FOR VALID DIR

	// setup the connections and primary information TODO
	if (setup_connections() == FALSE) {
		printf("failed to setup connections to servers\n");
		exit(EXIT_FAILURE);
	}

}


// ------------------------------ SERVER RPC SETUP ------------------------------
static struct timeval TIMEOUT = { 25, 0 };
NodeList *
find_1(char *filename,  CLIENT *clnt)
{
	static NodeList clnt_res;

	memset((char *)&clnt_res, 0, sizeof(clnt_res));
	if (clnt_call (clnt, Find,
		(xdrproc_t) xdr_wrapstring, (caddr_t) &filename,
		(xdrproc_t) xdr_NodeList, (caddr_t) &clnt_res,
		TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&clnt_res);
}

int *
download_1(char *filename,  CLIENT *clnt)
{
	static int clnt_res;

	memset((char *)&clnt_res, 0, sizeof(clnt_res));
	if (clnt_call (clnt, Download,
		(xdrproc_t) xdr_wrapstring, (caddr_t) &filename,
		(xdrproc_t) xdr_int, (caddr_t) &clnt_res,
		TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&clnt_res);
}

int *
getload_1(char *ip, int port,  CLIENT *clnt)
{
	getload_1_argument arg;
	static int clnt_res;

	memset((char *)&clnt_res, 0, sizeof(clnt_res));
	arg.ip = ip;
	arg.port = port;
	if (clnt_call (clnt, GetLoad, (xdrproc_t) xdr_getload_1_argument, (caddr_t) &arg,
		(xdrproc_t) xdr_int, (caddr_t) &clnt_res,
		TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&clnt_res);
}

FileList *
updatelist_1(char *ip, int port,  CLIENT *clnt)
{
	updatelist_1_argument arg;
	static FileList clnt_res;

	memset((char *)&clnt_res, 0, sizeof(clnt_res));
	arg.ip = ip;
	arg.port = port;
	if (clnt_call (clnt, UpdateList, (xdrproc_t) xdr_updatelist_1_argument, (caddr_t) &arg,
		(xdrproc_t) xdr_FileList, (caddr_t) &clnt_res,
		TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&clnt_res);
}

// ------------------------------ END SERVER RPC SETUP ------------------------------

typedef struct send_thread_args{
	int sockfd;
	char *filename;
} send_thread_args;
typedef struct download_thread_args{
	int sender_port;
	char *sender_ip;
	char *filename;
} download_thread_args;

int load = 0;
char this_peer_dir[] = {"tempname"};
FileList filelist;
// global char[120] server_type

void scan(char *dir){
	struct dirent *pDirent;
    DIR *pDir;

    pDir = opendir(this_peer_dir);
    if(pDir == NULL){
        printf("Cannot open directory %s.\n", this_peer_dir);
        return;
    }

    printf("Opened directory %s.\n", this_peer_dir);

    struct stat sb;

    int i = 0;
    while((pDirent = readdir(pDir)) != NULL){
        if(strcmp(pDirent->d_name, "..") != 0 && strcmp(pDirent->d_name, ".") != 0){
            //printf("[%s]\n", pDirent->d_name);
            file thisFile;
            strcpy(thisFile.name, pDirent->d_name);
            char filePath[256];
            strcpy(filePath, this_peer_dir);
            strcat(filePath, "/");
            strcat(filePath, pDirent->d_name);

            if(stat(filePath, &sb) == -1){
                perror("stat");
            }
            else{
                thisFile.size = (int) sb.st_size;
                filelist.files[i] = thisFile;
                i++;
            }
			if(i == 50) break;
        }
    }
	filelist.fileAmount = i;

    closedir(pDir);

    for(int j = 0; j < filelist.fileAmount; j++){
        printf("File: %s of size %d bytes.\n", filelist.files[j].name, filelist.files[j].size);
    }
    
    return;
}

void* download_thread(void *arg){
	int *sockfd = (int *) arg;
	load++; //MIGHT NEED TO LOCK THIS
	int client_socket;
	ssize_t len;
	struct sockaddr_in remote_addr;
	char buffer[BUFSIZ];
	int file_size;
	FILE *received_file;
	int remain_data = 0;

	download_thread_args *args = (download_thread_args *) arg;

	/* Zeroing remote_addr struct */
	memset(&remote_addr, 0, sizeof(remote_addr));

	/* Construct remote_addr struct */
	remote_addr.sin_family = AF_INET;
	inet_pton(AF_INET, args->sender_ip, &(remote_addr.sin_addr));
	remote_addr.sin_port = htons(args->sender_port);

	/* Create client socket */
	client_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (client_socket == -1)
	{
		fprintf(stderr, "Error creating socket");
		exit(EXIT_FAILURE);
	}

	/* Connect to the server */
	if (connect(client_socket, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr)) == -1)
	{
			fprintf(stderr, "Error on connect");

			exit(EXIT_FAILURE);
	}

	/* Receiving file size */
	recv(client_socket, buffer, BUFSIZ, 0);
	file_size = atoi(buffer);
	//fprintf(stdout, "\nFile size : %d\n", file_size);

	received_file = fopen(args->filename, "w");
	if (received_file == NULL)
	{
		fprintf(stderr, "Failed to open file foo");
		exit(EXIT_FAILURE);
	}

	remain_data = file_size;

	while ((remain_data > 0) && ((len = recv(client_socket, buffer, BUFSIZ, 0)) > 0))
	{
			fwrite(buffer, sizeof(char), len, received_file);
			remain_data -= len;
			fprintf(stdout, "Receive %ld bytes and we hope :- %d bytes\n", len, remain_data);
	}
	fclose(received_file);

	close(client_socket);

	return 0; //test
}

void* send_thread(void* arg){
	/* Listening to incoming connections */
	int server_socket, peer_socket, fd;
	struct sockaddr_in server_addr, peer_addr;
	socklen_t sock_len;
	int sent_bytes = 0;
	char file_size[256];
	ssize_t len;
	int remain_data;
	struct stat file_stat;
	int offset;
	char *filename_token;
	load++;

	send_thread_args *args = (send_thread_args *) arg;
	server_socket = args->sockfd;
	filename_token = args->filename;

	if ((listen(server_socket, 5)) == -1)
        {
                fprintf(stderr, "Error on listen");

                exit(EXIT_FAILURE);
        }

        fd = open(filename_token, O_RDONLY);
        if (fd == -1)
        {
                fprintf(stderr, "Error opening file");

                exit(EXIT_FAILURE);
        }

        /* Get file stats */
        if (fstat(fd, &file_stat) < 0)
        {
                fprintf(stderr, "Error fstat");

                exit(EXIT_FAILURE);
        }

        fprintf(stdout, "File Size: \n%ld bytes\n", file_stat.st_size);

        sock_len = sizeof(struct sockaddr_in);
        /* Accepting incoming peers */
        peer_socket = accept(server_socket, (struct sockaddr *)&peer_addr, &sock_len);
        if (peer_socket == -1)
        {
                fprintf(stderr, "Error on accept");

                exit(EXIT_FAILURE);
        }
        fprintf(stdout, "Accept peer --> %s\n", inet_ntoa(peer_addr.sin_addr));

        sprintf(file_size, "%ld", file_stat.st_size);

        /* Sending file size */
        len = send(peer_socket, file_size, sizeof(file_size), 0);
        if (len < 0)
        {
              fprintf(stderr, "Error on sending greetings" );

              exit(EXIT_FAILURE);
        }

        fprintf(stdout, "Server sent %ld bytes for the size\n", len);

        offset = 0;
        remain_data = file_stat.st_size;
        /* Sending file data */
        while (((sent_bytes = sendfile(peer_socket, fd, (long int *)&offset, BUFSIZ)) > 0) && (remain_data > 0))
        {
                fprintf(stdout, "1. Server sent %d bytes from file's data, offset is now : %d and remaining data = %d\n", sent_bytes, offset, remain_data);
                remain_data -= sent_bytes;
                fprintf(stdout, "2. Server sent %d bytes from file's data, offset is now : %d and remaining data = %d\n", sent_bytes, offset, remain_data);
        }

        close(peer_socket);
        close(server_socket);
		pthread_exit(NULL);

}

Node* peer_select(NodeList *list){
	// select a peer from the list based on load and latency

}

// void read_config(){
	//hardcoded file name/path
// }


NodeList *
find_1_svc(char *filename,  struct svc_req *rqstp)
{
	static NodeList result;

	/*
	 * insert server code here
	 */
	FileList* temp_files = malloc(sizeof(FileList));

	if (amPrimary == FALSE){

		NodeList *find_list = malloc(sizeof(NodeList));
		find_list = find_1(filename, trackingserver);
		if (find_list == NULL){
			//tracking server is down but we are not primary
			//FAIL
		}
		result = *find_list;
		return &result;
	}
	else{
		for (int i=0; i<num_normal_nodes; i++){
			//updates the file list for each node here we should check for status
			
			temp_files = updatelist_1(node_list->nodes[i].ip, node_list->nodes[i].port, servers[i]);
			if (temp_files == NULL){
				//set node status
			}
			node_list->nodes[i].files = *temp_files;
			
			for (int j=0; j<node_list->nodes[i].files.fileAmount; j++){
				if (strcmp(node_list->nodes[i].files.files[j].name, filename) == 0){
					result.nodes[result.numNodes] = node_list->nodes[i];
					result.numNodes++;
				}
			}
		}
		if (result.numNodes == 0){
			//no nodes have the file
			return NULL;
		}
	}
	return &result;
}

int *
download_1_svc(char *filename,  struct svc_req *rqstp)
{
	static int result;

	/*
	 * insert server code here
	 */

	//after download, call scan
	//split file name by space if there is a space

	char* send_token = strtok(filename, " ");
	if (strcmp(send_token, "send") != 0){
		NodeList find_list = *find_1(filename, trackingserver);
		if(&find_list == NULL){
			printf("Error: find_1 returned NULL.\n");
			result = -1;
			return &result;
		}
		Node* peer = peer_select(&find_list);


		if (peer== NULL){
			printf("Error: peer_select returned NULL.\n");
			result = -1;
			return &result;
		}
		//setup connection to peer
		CLIENT *clnt = setup_connection(peer->ip, peer->port);
		if(clnt == NULL){
			printf("Error: setup_connection returned NULL.\n");
			result = -1;
			return &result;
		}

		//call download on peer
		// add "send filename" to download_1 to indicate to send a file

		int* download_result = download_1(filename, clnt);
		if(download_result == NULL){
			printf("Error: download_1 returned NULL.\n");
			result = -1;
			return &result;
		}
		pthread_t thread;
		download_thread_args *args = (download_thread_args *) malloc(sizeof(download_thread_args));
		args->sender_port = *download_result;
		args->sender_ip = peer->ip;
		args->filename = filename;

		int theirPort = args->sender_port;

		int latency = 0;
		// open the file:

		FILE *fp;

		fp = fopen("latency.txt", "r");
		if (fp == NULL) {
			printf("Failed to find latency. Could not open latency.txt\n");
		}

		int max_line_size = 255;
		char linebuf[max_line_size];


		while(fgets(linebuf, sizeof(linebuf), fp) != NULL){
			char *incoming_server_ip = strtok(linebuf, " ");
			int incoming_server_port = atoi(strtok(NULL, " "));
			char *outgoing_server_ip = strtok(NULL, " ");
			int outgoing_server_port = atoi(strtok(NULL, " "));
			int connection_latency = atoi(strtok(NULL, " "));
			
			if(strcmp(server_port_str, incoming_server_ip) == 0 && server_port == incoming_server_port && strcmp(args->sender_ip, outgoing_server_ip) == 0 && theirPort == outgoing_server_port){
				latency = connection_latency;
				break;
			}
		}

		printf("Latency: %d\n", latency);
		// Insert latency here.

		fclose(fp);

		pthread_create(&thread, NULL, &download_thread, (void *) args); //this arguement should be the port number to download from
	
	}
	else if (strcmp(send_token, "send") == 0){ //if we are sending the file
		char* filename_token = strtok(NULL, " ");
		if (filename_token == NULL){
			printf("Error: strtok returned NULL.\n");
			result = -1;
			return &result;
		}
		//start sending a file on a port
		
		//set up port
		int server_socket;
        socklen_t       sock_len;
        struct sockaddr_in      server_addr;

        /* Create server socket */
        server_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (server_socket == -1)
        {
			fprintf(stderr, "Error creating socket" );
			exit(EXIT_FAILURE);
        }

        /* Zeroing server_addr struct */
        memset(&server_addr, 0, sizeof(server_addr));
        /* Construct server_addr struct */
        server_addr.sin_family = AF_INET;
        inet_pton(AF_INET, "127.0.0.1", &(server_addr.sin_addr));
        server_addr.sin_port = htons(0);

        /* Bind */
        if ((bind(server_socket, (struct sockaddr *)&server_addr, sizeof(struct sockaddr))) == -1)
        {
                fprintf(stderr, "Error on bind");
                exit(EXIT_FAILURE);
        }

		pthread_t thread;
		send_thread_args *args = malloc(sizeof(send_thread_args));
		args->filename = filename_token;
		args->sockfd = server_addr.sin_port;
		printf("args->sockfd: %d\n", args->sockfd);
		printf("args->filename: %s\n", args->filename);

		pthread_create(&thread, NULL, &send_thread, (void *)args); 
        
		result = args->sockfd;
        return &result;


	}

	//download file from peer
	//choose a peer from the list based on load and latency

	// if found
	// load++;
	// spawn a thread to download the file

	//else
	// return -1

	return &result;
}

int *
getload_1_svc(char *ip, int port, struct svc_req *rqstp)
{
	static int  result;

	/*
	 * insert server code here
	 */

	//find the node in the list

	// lock the load variable
	// read it 
	// unlock it
	// return it


	return &result;
}

FileList *
updatelist_1_svc(char *ip, int port, struct svc_req *rqstp)
{
	static FileList  result;

	/*
	 * insert server code here
	 */
	//scan directory for files

	return &result;
}
