//this code is from video_analytics_server_trashbit_2.py
//I have changed the language of code for improving performance!
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define FRAME_SIZE 8192

void receive_frame(int client_socket) {
    unsigned char frame_data[FRAME_SIZE];
    memset(frame_data, '0', FRAME_SIZE);
    
    while (1) {
        unsigned char header_data[20] = {0};
        int received = 0;
        while (received < 16) {
            int len = recv(client_socket, header_data + received, 16 - received, 0);
            if (len <= 0) {
                return;  // Disconnect or error
            }
            received += len;
        }

        double timestamp;
        unsigned long frame_size;
        int idx;
        memcpy(&timestamp, header_data, sizeof(double));
        memcpy(&frame_size, header_data + sizeof(double), sizeof(unsigned long));
        memcpy(&idx, header_data + sizeof(double) + sizeof(unsigned long), sizeof(int));

        frame_size -= 16;
        unsigned char received_data[frame_size];
        received = 0;
        while (received < frame_size) {
            int len = recv(client_socket, received_data + received, frame_size - received, 0);
            if (len <= 0) {
                return;  // Disconnect or error
            }
            received += len;
        }

        double received_timestamp = (double)time(NULL);
        double e2e_delay = received_timestamp - timestamp;

        printf("idx: %d, E2E delay: %lf, and size: %lu\n", idx, e2e_delay, frame_size);

        send(client_socket, header_data, 20, 0);

        char log_entry[200];
        snprintf(log_entry, sizeof(log_entry), "sender's timestamp ,%lf,received timestamp ,%lf, E2E delay ,%lf, and size ,%lu,\n", 
            timestamp, received_timestamp, e2e_delay, frame_size);

        FILE *file = fopen("./logging/video_analytics_server_log.txt", "a");
        if (file) {
            fputs(log_entry, file);
            fclose(file);
        }
    }
}

int main() {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    
    // Enable MPTCP (This might be different depending on your system)
    int enabled = 1;
    //setsockopt(server_socket, IPPROTO_TCP, 42, &mptcp_enabled, sizeof(mptcp_enabled));
    int ret = setsockopt(server_socket, SOL_TCP, 42, &enable, sizeof(int));
    if(ret < 0){
	perror("[server] setsockopt() ");
	return -1;
    }
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8888);
    inet_pton(AF_INET, "172.31.41.24", &server_address.sin_addr);

    bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address));

    listen(server_socket, 1);

    printf("Waiting for incoming connections...\n");
    
    struct sockaddr_in client_address;
    socklen_t client_len = sizeof(client_address);
    int client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_len);

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_address.sin_addr, client_ip, INET_ADDRSTRLEN);
    printf("Accepted connection from: %s\n", client_ip);

    struct stat st = {0};
    if (stat("./logging", &st) == -1) { 
        mkdir("./logging", 0700);  // Ensure the logging directory exists
    }

    FILE *file = fopen("./logging/video_analytics_server_log.txt", "a");
    if (file) {
        fputs("start--------------------------------------------\n", file);
        fclose(file);
    }

    receive_frame(client_socket);

    close(client_socket);
    close(server_socket);

    file = fopen("./logging/video_analytics_server_log.txt", "a");
    if (file) {
        fputs("END ----------------------------------------------------------------------------\n", file);
        fclose(file);
    }

    return 0;
}
