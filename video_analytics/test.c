// Online C compiler to run C program online
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

int main() {
    // Write C code here
    //printf("Hello world");
    int frame_size = 40960;
    char* data = (char*)malloc(sizeof(char) * frame_size);
    //char data[frame_size];
    char str[1000];
    int i = 100;
    
    memset(data, '0', frame_size);
    
    struct timeval tv;
    
    printf("%d\n",strlen(data));
    
    
    gettimeofday(&tv,NULL);
    unsigned long tmp = 1000000 * tv.tv_sec + tv.tv_usec;
    struct tm* timeinfo;
    time_t rawtime = (time_t)tmp/1000000;
    timeinfo = localtime(&rawtime);
    printf("local time is %s",asctime(timeinfo));
    sleep(1);
    
    gettimeofday(&tv,NULL);
    unsigned long timestamp = 1000000 * tv.tv_sec + tv.tv_usec;
    
    sprintf(str, "%ld",timestamp);
    printf("the value of str is %s\n",str);
    printf("time delay value is %ld\n",timestamp-tmp);
    
    //printf("%lu\n",sizeof(str));
    printf("size of unsigned long is %lu\n",sizeof(unsigned long));
    
    memcpy(data, &timestamp, sizeof(unsigned long));
    //printf("%d\n",strlen(data));
    //printf("%d\n",sizeof(int));
    memcpy(data + sizeof(unsigned long), &frame_size, sizeof(int));
    //printf("%d\n",strlen(data));
    
    memcpy(data + sizeof(unsigned long) + sizeof(int), &i, sizeof(int));
    unsigned long tmp1;
    int tmp2;
    int tmp3;
    
    memcpy(&tmp1, data,sizeof(unsigned long));
    printf("tmp1 is %lu\n",tmp1);
    
    memcpy(&tmp2, data + sizeof(unsigned long),sizeof(int));
    memcpy(&tmp3, data + sizeof(unsigned long)+sizeof(int),sizeof(int));
    printf("tmp2 is %d\n",tmp2);
    printf("tmp3 is %d\n",tmp3);
    
    //printf("%s\n",data);
    //tmp1 
    
    
    //printf("%d\n",strlen(data));
    //printf("%d",sizeof(data[0:16]));
    //data[17]=NULL;
    //printf("%s",data);
    free(data);
    return 0;
}
