/*
 * MADE BY HIA - DAE SUNG KIM
 * re - BOOK :: HAE HONG YAE ::
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <hacking.h>
#include <hacking_network.h>
#include <ctype.h>

#define PORT 80
#define webroot "./webroot"
#define SERVER_IP "192.168.10.19"

int session_cnt = 0;
void handle_con(int, struct sockaddr_in *);
time_t now;

int filesize(int);

int main() {
	int sock, new_sock[256], yes = 1;

	struct sockaddr_in host_addr, client_addr;
	int sin_size;

	char * server_ip = SERVER_IP;

	printf("Waiting for connection!!\n");

	sock = socket(PF_INET, SOCK_STREAM, 0);
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

	memset(&host_addr, 0, sizeof(host_addr));
	host_addr.sin_family = AF_INET;
	host_addr.sin_port = htons(PORT);
	host_addr.sin_addr.s_addr = inet_addr(server_ip);
	sin_size = sizeof(host_addr);
	bind(sock, (struct sockaddr *)&host_addr, sin_size);

	listen(sock, 20);

	while(1) {
		sin_size = sizeof(struct sockaddr_in);
		new_sock[session_cnt] = accept(sock, (struct sockaddr*)&client_addr, &sin_size);
		handle_con(new_sock[session_cnt], &client_addr);
		session_cnt++;
	}

	return 0;
}



void handle_con(int sockfd, struct sockaddr_in *client_addr_ptr) {
	unsigned char *ptr, req[500], res[500], *end;
	int fd, length, errfd;
	
	time(&now);
	FILE * log = fopen("/var/log/demoweb.log", "a");
	fprintf(log, "%s : connected client -> %s\n", ctime(&now), inet_ntoa(client_addr_ptr->sin_addr));
	printf("connected client : %s\n", inet_ntoa(client_addr_ptr->sin_addr));
	fclose(log);
	length = recv_line(sockfd, req);
	printf("%s\n", req);

	for(int i=0;i<strlen(req);i++) {
		printf("%d ", req[i]);
	}
	printf("\n");

	ptr = strstr(req, " HTTP/");//testing protocol HTTP
	*ptr = 0;
	ptr = NULL;
	if(strncmp(req, "GET ", 4) == 0) {
		ptr = req + 4;
	}
	if(strncmp(req, "HEAD ", 5) == 0) {
		ptr = req + 5;
	}
	printf("%s\n", req);
	printf("%s\n", ptr);
	if(ptr[strlen(ptr) - 1] == '/') {
		printf("index.html reqed!\n");
		strcat(ptr, "index.html");
	}
	else if(!strcmp(ptr, "/happy_kurisu")) {
		end = ptr + strlen(ptr) - 1;
		printf("error.html reqed!\n");
		while((*end) != '/') {
			*end = 0;
			end--;
		}
		strcat(end, "html5up-multiverse/index.html");
		ptr = end;
	}
	strcpy(res, webroot);
	strcat(res, ptr);
	printf("%s\n", res);
	fd = open(res, O_RDONLY, 0);
	if(fd == -1) {
		errfd = open("./webroot/error.html", O_RDONLY, 0);
		printf("returning 200\n");
		send_string(sockfd, "HTTP/1.0 200 OK\r\n");
		send_string(sockfd, "Server: Tiny webserver\r\n\r\n");
		if(ptr == req + 4) {
			length = filesize(errfd);
			ptr = (unsigned char *)malloc(length);
			read(errfd, ptr, length);
			send(sockfd, ptr, length, 0);
		//	printf("ptr : %s\n", ptr);
			free(ptr);
		}
		close(errfd);
	}

	else {
		printf("returning 200\n");
		send_string(sockfd, "HTTP/1.0 200 OK\r\n");
		send_string(sockfd, "Server: Tiny webserver\r\n\r\n");
		if(ptr == req + 4) {
			length = filesize(fd);
			ptr = (unsigned char *)malloc(length);
			read(fd, ptr, length);
			send(sockfd, ptr, length, 0);
		//	printf("ptr : %s\n", ptr);
			free(ptr);
		}
		close(fd);
	}
	shutdown(sockfd, SHUT_RDWR);
}

int filesize(int fd) {
	struct stat stat_struct;
	if(fstat(fd, &stat_struct) == -1) {
		return -1;
	}
	return (int) stat_struct.st_size;
}

	 	


