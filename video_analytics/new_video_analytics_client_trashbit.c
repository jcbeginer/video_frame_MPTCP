/**
 * MPTCP Socket API Test App
 * File Sender(Client)
 * 
 * @date	: 2023-07-13
 * @author	: Woosung(NETLAB)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <time.h>

#include "../header/mptcp.h"

int get_fsize(FILE* file);

/**
 * 기존의 TCP Client는 { socket() -> connect() -> recv(), send() -> close() }순서로 흘러간다.
 * 여기서 TCP Socket을 MPTCP Socket으로 설정하기 위해서는 socket()과 connect()사이에 setsockopt()을 사용한다.
 **/
int main(int argc, char** argv)
{
	char* ADDR;
	int PORT;
	char* FILE_PATH;

	int sock;
	struct sockaddr_in addr;
	int ret;

	FILE* file;
	char send_buff[1024] = { '\0', };
	int fsize = 0, nsize = 0;

	int enable = 1;
	char* path_manager = "fullmesh";

	if(argc != 4){
		fprintf(stderr, "usage: %s [host_address] [port_number] [file_path]\n", argv[0]);
		return -1;
	}
	ADDR = argv[1];
	PORT = atoi(argv[2]);
	FILE_PATH = argv[3];

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0){
		perror("[client] socket() ");
		return -1;
	}

	/* setsockopt()함수와 MPTCP_ENABLED(=42)상수를 사용하여 MPTCP Socket으로 Setup */
	ret = setsockopt(sock, SOL_TCP, MPTCP_ENABLED, &enable, sizeof(int));
	if(ret < 0){
		perror("[server] setsockopt(MPTCP_ENABLED) ");
		return -1;
	}

	memset(&addr, 0x00, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ADDR);
	addr.sin_port = htons(PORT);

	ret = connect(sock, (struct sockaddr *)&addr, sizeof(addr));
	if(ret < 0){
		perror("[client] connect() ");
		return -1;
	}
	printf("[client] connected\n");

	file = fopen(FILE_PATH, "rb");
	if(file == NULL){
		perror("[client] fopen() ");
		return -1;
	}

	fsize = get_fsize(file);

	printf("[client] sending file...(%s)\n", FILE_PATH); 
	// Open log file
   	char filename[1024] = "";
    	time_t now = time(NULL);
    	struct tm *t = localtime(&now);
	strftime(filename, sizeof(filename)-1, "./logging/file_transfer_client_log_%Y%m%d_%H%M%S.txt", t);
    	FILE *log_file = fopen(filename, "a");
    	if (log_file == NULL) {
        	printf("there is no file, so I make new one");
  	}	

	// Write log start entry
	//fprintf(log_file, "start--------------------------------------------\n");
	// capture start time
	struct timespec start, end;
    	clock_gettime(CLOCK_REALTIME, &start);

	
	while(nsize!=fsize){
		int fpsize = fread(send_buff, 1, 1024, file);
		nsize += fpsize;
		printf("[client] file size %dB | send to %dB\n", fsize, nsize);
		send(sock, send_buff, fpsize, 0);
	}
	// Capture end time
    	clock_gettime(CLOCK_REALTIME, &end);
    
    	// Write packet information
    	fprintf(log_file,"sent_timestamp: %ld.%ld, received_timestamp: %ld.%ld, received-send delay: %ld.%ld, size: %d\n", start.tv_sec, start.tv_nsec, end.tv_sec, end.tv_nsec, end.tv_sec-start.tv_sec,end.tv_nsec-start.tv_nsec, fsize);

    //...
	fclose(file);
	fclose(log_file);
	close(sock);

	return 0;
}

int get_fsize(FILE* file)
{
	int fsize;

	fseek(file, 0, SEEK_END);
	fsize=ftell(file);
	fseek(file, 0, SEEK_SET);	

	return fsize;
}
