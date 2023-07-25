#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <errno.h>
#include <sys/sem.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/time.h>
#define SEMKEY1 (key_t)1006
#define SEMKEY2 (key_t)2006
#define SEMKEY3 (key_t)3006
#define SHMKEY (key_t)01
#define YEAR 2022
// variable names for better visual understanding of code during ups and downs of the semaphores
#define lock 1
#define unlock 1

struct shared_mem{
    int req_segment;             // segment the child requests
    int line_of_segment;         // line of the segment the child requests
    int prev_segment;            // previous requested segment
    int num_of_requests;         // how many requests each child has to make
    int lines_per_segment;
    int lines_of_last_segment;
    int chpid;                   // child's pid that is is inside the shared memory at a time
    int num_of_segments;
    int count;                   // counts the number of requests already
    char text_seg[150][200];
    // segment of the text the parent serves to the children (on purpose much bigger in data size)    
};

long long find_milliseconds(void){
    
    struct timeval t;

    // get current time.
    gettimeofday(&t, NULL); 

    // calculate CURRENT milliseconds
    long long milliseconds = (t.tv_sec*1000LL + t.tv_usec/1000) % 1000; 
    return milliseconds;
};
