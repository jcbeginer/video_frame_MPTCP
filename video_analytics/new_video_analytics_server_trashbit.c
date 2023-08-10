/**
 * MPTCP Socket API Test App
 * video frame Recevier(Server)
 * 
 * @date	: 2023-08-10(Thu)
 * @author	: Woosung joo(NETLAB)
 * ref : Ji-Hun(INSLAB)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <time.h>
#include <sys/time.h>

#include "./header/mptcp.h"
FILE *file = fopen("./logging/video_analytics_server_log.txt", "a");

/**
 * 기존의 TCP Server는 { socket() -> bind() -> listen() -> accept() -> recv(), send() -> close() }순서로 흘러간다.
 * 여기서 TCP Socket을 MPTCP Socket으로 설정하기 위해서는 socket()과 bind()사이에 setsockopt()을 사용한다.
 **/

#define FRAME_SIZE 8192
char filename[255];
struct timeval tv;
struct tm* timeinfo;
void receive_frame(int client_socket) {
    char data[FRAME_SIZE];
    char header_data[16];
    char received_data[FRAME_SIZE-16];
    memset(data, '0', FRAME_SIZE);
    unsigned long received_timestamp;
    unsigned long sent_timestamp;
    int received_frame_size;

    int idx;
    
    while (1) {
        recv(client_socket, header_data, 16, 0);
        
        memcpy(&sent_timestamp, header_data, sizeof(unsigned long));
        memcpy(&received_frame_size, header_data + sizeof(unsigned long), sizeof(int));
        memcpy(&idx, header_data + sizeof(unsigned long) + sizeof(int), sizeof(int));

	int received = 0;        
        int check_frame_size = FRAME_SIZE - 16;
        while (received < check_frame_size) {
            int len = recv(client_socket, received_data + received, check_frame_size - received, 0);
            if (len <= 0) {
                return;  // Disconnect or error
            }
            received += len;
        }

	gettimeofday(&tv,NULL);
    	received_timestamp = 1000000 * tv.tv_sec + tv.tv_usec;
        double time_diff = received_timestamp - sent_timestamp;

        printf("idx: %d, one way delay: %lf [ms], and size: %lu\n", idx, time_diff/1000, received_frame_size);

        send(client_socket, header_data, 16, 0);

	time_t rawtime = (time_t)sent_timestamp/1000000;
        timeinfo = localtime(&rawtime);
        
        //snprintf(log_entry, sizeof(log_entry), "index of frame, %d, sender's timestamp, %d,sender's timestamp - received timestamp ,%lf,[ms], and size ,%lu,\n", 
            timestamp, , time_diff, frame_size);

        FILE *file = fopen(filename, "a");
        if (file) {
            fprintf(f, "packet_index ,%d, sent_timestamp ,%s,received-send delay ,%f,[usec], and size ,%d,\n", idx, asctime(timeinfo), time_diff, received_frame_size);
            fclose(file);
        }
    }
}


int main(int argc, char** argv)
{
  
  	//for logging
	if (access("./logging", F_OK) != 0) {
        mkdir("./logging", 0700);
    	}
	time_t t = time(NULL);
  	struct tm tm = *localtime(&t);
  	sprintf(filename, "./logging/video_analytics_server_log%02d%02d%02d_minRTT_40KB.txt", tm.tm_year % 100, tm.tm_mon + 1, tm.tm_mday);

  	FILE* f = fopen(filename, "a");
  	if (f) {
    		fprintf(f, "start--------------------------------------------\n");
    		fclose(f);
  	}
	int PORT;
	//const char* FILE_NAME = "recv_file";

	int server_sock, client_sock;
	struct sockaddr_in server_addr, client_addr;
	int len, addr_len, recv_len, ret;

	FILE *file;
	int fsize = 0, nsize = 0;
	char buffer[1024];

	int enable = 1;

	//PORT = atoi(argv[1]);
  	PORT = atoi("8888");

	server_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(server_sock < 0){
		perror("[server] socket() ");
		return -1;
	}

	/* setsockopt()함수와 MPTCP_ENABLED(=42)상수를 사용하여 MPTCP Socket으로 Setup */
	ret = setsockopt(server_sock, SOL_TCP, MPTCP_ENABLED, &enable, sizeof(int));
	if(ret < 0){
		perror("[server] setsockopt() ");
		return -1;
	}

	memset(&server_addr, 0x00, sizeof(struct sockaddr_in));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(PORT);

	ret = bind(server_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in));
	if(ret < 0){
		perror("[server] bind() ");
		return -1;
	}	

	ret = listen(server_sock, 5);
	if(ret < 0){
		perror("[server] listen() ");
		return -1;
	}

	addr_len = sizeof(struct sockaddr_in);
	client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_len);	
	if(client_sock < 0){
		perror("[server] accept() ");
		return -1;
	}
	printf("[server] connected to client\n");

  	receive_frame(client_socket);
  
	close(client_sock);
	close(server_sock);

	return 0;
}
