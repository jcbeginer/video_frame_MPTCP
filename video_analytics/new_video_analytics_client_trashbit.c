/**
 * MPTCP Socket API Test App
 * video frame Sender(Client)
 * 
 * @date	: 2023-08-09
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
#include <sys/time.h>
#include <pthread.h>
#include <time.h>

#include "../header/mptcp.h"

#define FRAME_RATE 30
#define DURATION 10

int frame_sizes[8] = [15360,25600,30720,40960,71680,102400,133120,153600]; // # 15KB (index24), 25KB(index25), 30KB(index26), 40KB(index27), 70KB(index28), 100KB(index29), 130KB(index30), 150KB(index31)
int send_frame_size = frame_sizes[3];


/**
 * 기존의 TCP Client는 { socket() -> connect() -> recv(), send() -> close() }순서로 흘러간다.
 * 여기서 TCP Socket을 MPTCP Socket으로 설정하기 위해서는 socket()과 connect()사이에 setsockopt()을 사용한다.
 **/
void* send_frames(void* arg) {
    int client_socket = *(int*)arg;
    int frame_size = send_frame_size;
    char* data = (char*)malloc(sizeof(char)*frame_size);
    struct timeval tv;
    unsigned long timestamp;
    gettimeofday(&tv,NULL);
    timestamp = 1000000 * tv.tv_sec + tv.tv_usec;
    double c_wait_time = 1 / FRAME_RATE * 1000000; //[usec]
    
    // Initialize data with '0'
    memset(data, '0', frame_size);

    for (int i = 0; i < FRAME_RATE * DURATION; i++) {
        gettimeofday(&tv,NULL);
	unsigned long tmp = 1000000 * tv.tv_sec + tv.tv_usec;
        FILE* f = fopen(filename2, "a");
        if (f) {
            fprintf(f, "packet_index is %d and delayed_time is %ld [usec]\n", i+1, tmp - timestamp - c_wait_time);
            fclose(f);
        }
	    
        gettimeofday(&tv,NULL);
    	timestamp = 1000000 * tv.tv_sec + tv.tv_usec;
        
        // Assume packing functions are already implemented (you need to make these!)
        
        
        //unsigned char buffer[16]; // 8 bytes for double, 4 bytes for long, 4 bytes for int

        // Packing data into the buffer
        memcpy(data, &timestamp, sizeof(unsigned long));
        memcpy(data + sizeof(unsigned long), &send_frame_size, sizeof(int));
        memcpy(data + sizeof(unsigned long) + sizeof(int), &i, sizeof(int));
        // pack_data(timestamp, frame_size, i+1, data);
        
        send(client_socket, data, frame_size, 0);

        double delayed_time = (double) time(NULL) - timestamp;
        gettimeofday(&tv,NULL);
    	tmp = 1000000 * tv.tv_sec + tv.tv_usec;
        unsigned long delayed_time = tmp - timestamp;
        if (delayed_time > c_wait_time) {
            continue;
        } else {
            int wait_time = c_wait_time - delayed_time;
            usleep(wait_time);
        }
    }
    free(data)
    return NULL;
}

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

