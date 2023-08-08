//this code is from video_analytics_client_trashbit_2.py of date 230808. 
//for imporving speed, I have change .py -> .c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <netinet/tcp.h>

#include <asm/byteorder.h>
#include <linux/in.h>
#include <linux/in6.h>
#include <linux/socket.h>
#include <linux/types.h>

#define FRAME_RATE 30
#define DURATION 10
#define MPTCP_ENABLED 42

char filename[255], filename2[255];
unsigned long send_frame_size;

void* send_frames(void* arg) {
    int client_socket = *(int*)arg;
    int frame_size = send_frame_size;
    char data[frame_size];
    double timestamp;
    double c_wait_time = 1 / FRAME_RATE;

    // Initialize data with '0'
    memset(data, '0', frame_size);

    for (int i = 0; i < FRAME_RATE * DURATION; i++) {
        double tmp = (double) time(NULL);
        FILE* f = fopen(filename2, "a");
        if (f) {
            fprintf(f, "packet_index is %d and delayed_time is %f\n", i+1, tmp - timestamp - c_wait_time);
            fclose(f);
        }
        timestamp = (double) time(NULL);
        
        // Assume packing functions are already implemented (you need to make these!)
        
        //unsigned long frame_size = /* some value */;
        //int i = /* some value */;
        //unsigned char buffer[16]; // 8 bytes for double, 4 bytes for long, 4 bytes for int

        // Packing data into the buffer
        memcpy(data, &timestamp, sizeof(double));
        memcpy(data + sizeof(double), &send_frame_size, sizeof(unsigned long));
        memcpy(data + sizeof(double) + sizeof(unsigned long), &i, sizeof(int));
        // pack_data(timestamp, frame_size, i+1, data);
        
        send(client_socket, data, frame_size, 0);

        double delayed_time = (double) time(NULL) - timestamp;
        if (delayed_time > c_wait_time) {
            continue;
        } else {
            int wait_time = c_wait_time - delayed_time;
            sleep(wait_time);
        }
    }
    return NULL;
}

void* receive_frames(void* arg) {
    int client_socket = *(int*)arg;
    while (1) {
        char header_data[16];
        recv(client_socket, header_data, 16, 0);
        double sent_timestamp;
        long received_frame_size;
        int idx;
        
        // Unpacking functions need to be implemented
        memcpy(&sent_timestamp, header_data, sizeof(double));
        memcpy(&received_frame_size, header_data + sizeof(double), sizeof(unsigned long));
        memcpy(&idx, header_data + sizeof(double) + sizeof(unsigned long), sizeof(int));
        // unpack_data(header_data, &sent_timestamp, &received_frame_size, &idx);
        
        double received_timestamp = (double) time(NULL) + 0.02063;
        double received_send_delay = received_timestamp - sent_timestamp;
        printf("packet_idx %d, received_send_delay %f\n", idx, received_send_delay);
        
        time_t rawtime = (time_t)sent_timestamp;
        struct tm* timeinfo;
        timeinfo = localtime(&rawtime);
        FILE* f = fopen(filename, "a");
        if (f) {
            fprintf(f, "packet_index ,%d, sent_timestamp ,%s,received-send delay ,%f, and size ,%d,\n", idx, asctime(timeinfo), received_send_delay, received_frame_size);
            fclose(f);
        }
        usleep(1000); // Equivalent to time.sleep(0.001)
    }
    return NULL;
}

int main() {
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

    int client_socket;
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    //setsockopt(client_socket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
    //setsockopt(client_socket, IPPROTO_TCP, MPTCP_ENABLED, &(int){1}, sizeof(int));
    int enable =1;
    int ret = setsockopt(socket, SOL_TCP, MPTCP_ENABLED, &enable, sizeof(int));
	if(ret < 0){
		perror("[server] setsockopt(MPTCP_ENABLED) ");
		return -1;
	}
    
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8888);
    inet_aton("54.180.119.186", &server_address.sin_addr);
    connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address));

    // Reading frame sizes from file or setting default
    // Assume you've written read_frame_sizes() that returns the desired frame size
    // send_frame_size = read_frame_sizes();

    pthread_t send_thread, receive_thread;
    pthread_create(&receive_thread, NULL, receive_frames, &client_socket);
    pthread_create(&send_thread, NULL, send_frames, &client_socket);
    
    
    pthread_join(send_thread, NULL);
    pthread_join(receive_thread, NULL);

    close(client_socket);
    return 0;
}
