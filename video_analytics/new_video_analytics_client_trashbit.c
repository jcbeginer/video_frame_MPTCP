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

#include "./header/mptcp.h"

#define FRAME_RATE 30
#define DURATION 10
char filename[255], filename2[255];
int frame_sizes[8] = {15360,25600,30720,40960,71680,102400,133120,153600}; // # 15KB (index24), 25KB(index25), 30KB(index26), 40KB(index27), 70KB(index28), 100KB(index29), 130KB(index30), 150KB(index31)
int send_frame_size;
struct timeval tv;
struct tm* timeinfo;
/**
 * 기존의 TCP Client는 { socket() -> connect() -> recv(), send() -> close() }순서로 흘러간다.
 * 여기서 TCP Socket을 MPTCP Socket으로 설정하기 위해서는 socket()과 connect()사이에 setsockopt()을 사용한다.
 **/
void* send_frames(void* arg) {
    int client_socket = *(int*)arg;
    //initialization
    int send_frame_size = frame_sizes[3];
    int frame_size = send_frame_size;
    char* data = (char*)malloc(sizeof(char)*frame_size);
    unsigned long timestamp;
    gettimeofday(&tv,NULL);
    timestamp = 1000000 * tv.tv_sec + tv.tv_usec;
    double c_wait_time = 1000000/ FRAME_RATE; //[usec]
    
    // Initialize data with '0'
    memset(data, '0', frame_size);

    for (int i = 1; i <= FRAME_RATE * DURATION; i++) {
        gettimeofday(&tv,NULL);
	unsigned long tmp = 1000000 * tv.tv_sec + tv.tv_usec;
        FILE* f = fopen(filename2, "a");
        if (f) {
            fprintf(f, "packet_index is ,%d, and delayed_time is ,%lf, [usec]\n", i, tmp - timestamp - c_wait_time);
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
        // pack_data(timestamp, frame_size, i, data);
        
        send(client_socket, data, frame_size, 0);
	printf("video_frame sending... packet_idx %d, frame_size %d \n", i, send_frame_size);
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
    free(data);
    return NULL;
}
void* receive_frames(void* arg) {
    int client_socket = *(int*)arg;
    char header_data[16];
    unsigned long sent_timestamp;
    int received_frame_size;
    int idx;
    while (1) {
        
        recv(client_socket, header_data, 16, 0);
        
        
        // Unpacking
        memcpy(&sent_timestamp, header_data, sizeof(unsigned long));
        memcpy(&received_frame_size, header_data + sizeof(unsigned long), sizeof(int));
        memcpy(&idx, header_data + sizeof(unsigned long) + sizeof(int), sizeof(int));
        // unpack_data(header_data, &sent_timestamp, &received_frame_size, &idx);
	gettimeofday(&tv,NULL);
	unsigned long received_timestamp = 1000000 * tv.tv_sec + tv.tv_usec + 20630; //20.63ms maybe which is for inference time in server
        //double received_timestamp = (double) time(NULL) + 0.02063;
        unsigned long received_send_delay = received_timestamp - sent_timestamp; //[usec]
        printf("packet_idx %d, received_send_delay %ld [usec]\n", idx, received_send_delay);
        
        time_t rawtime = (time_t)sent_timestamp/1000000;
        timeinfo = localtime(&rawtime);
        FILE* f = fopen(filename, "a");
        if (f) {
            fprintf(f, "packet_index ,%d, sent_timestamp ,%s,received-send delay ,%ld,[usec] and size ,%d,\n", idx, asctime(timeinfo), received_send_delay, received_frame_size);
            fclose(f);
        }
        usleep(1); // Equivalent to time.sleep(0.001)
    }
    return NULL;
}

int main(int argc, char** argv)
{
	
	//for logging
	if (access("./logging", F_OK) != 0) {
        mkdir("./logging", 0700);
    	}
	time_t t = time(NULL);
    	struct tm tm = *localtime(&t);
    	sprintf(filename, "./logging/video_analytics_client_log%02d%02d%02d_minRTT_40KB.txt", tm.tm_year % 100, tm.tm_mon + 1, tm.tm_mday);
    	sprintf(filename2, "./logging/delayed_time_log%02d%02d%02d_2.txt", tm.tm_year % 100, tm.tm_mon + 1, tm.tm_mday);

	FILE* f = fopen(filename, "a");
    	if (f) {
        	fprintf(f, "start--------------------------------------------\n");
        	fclose(f);
    	}
	//for communication
	char* ADDR;
	int PORT;
	char* FILE_PATH;

	int sock;
	struct sockaddr_in addr;
	int ret;

	
	int enable = 1;
	char* path_manager = "fullmesh";

	
	ADDR = "54.180.119.186";//argv[1];
	PORT = atoi("8888"); //atoi(argv[2])

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


	
	//pthread part
	pthread_t send_thread, receive_thread;
    	pthread_create(&receive_thread, NULL, receive_frames, &sock);
    	pthread_create(&send_thread, NULL, send_frames, &sock);
    
    
    	pthread_join(send_thread, NULL);
    	pthread_join(receive_thread, NULL);

    	close(sock);
	

	return 0;
}

