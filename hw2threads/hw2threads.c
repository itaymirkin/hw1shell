#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include "hw2threads_src.h"


//* Globals *//

int num_threads;          
int num_counters;
int log_enabled;
int num_jobs_pending;

int main(int argc, char *argv[]) {
    //Validate num of args
    if (argc != 5) {
        fprintf(stderr, "not enough args\n");
        return 1;
    }
    //init main args
    int num_threads  = atoi(argv[2]);
    int num_counters = atoi(argv[3]);
    int log_enabled  = atoi(argv[4]);
    printf("num of counters - %d \nnum of threads - %d\nlog enabled - %d\n", num_counters, num_threads, log_enabled); 

    //Create countes files and init them 
    char counter_filename[50];
    for (int i = 0; i < num_counters; i++)
    {
        sprintf(counter_filename, "count%02d.txt", i);
        printf("file name - %s\n", counter_filename);
        FILE *f = fopen(counter_filename, "w");
        if (f == NULL)
        {
            printf("FILE CREATION ERROR HEREs");
            return 1;
        }
        fprintf(f, "0\n");
        fclose(f);
    }
    


    // Allocate for the number of threads
    worker_trds =   calloc(num_threads, sizeof(pthread_t));
    thread_status = calloc(num_threads, 1);
    work_queue    = calloc(1, sizeof(cmd_line_s));

    for (int i = 0; i < num_threads; i++)
    {
        pthread_create(&worker_trds[i], NULL, trd_func, NULL); // add Pointer with trd DATA struct to last null
    }

    /* Dispatcher Work*/

    //Parse cmd line 
    FILE *cmdfile = fopen(argv[1], "r");
    if (cmdfile == NULL) {
        printf("CANT OPEN CMDFILE");
        return 1;
    }

    char line[MAX_LINE_LENGTH];
    cmd_line_s* cmd;

    while (fgets(line, MAX_LINE_LENGTH, cmdfile)) {
        //load a line
        line[strcspn(line, "\n")] = '\0';
        //Tokenize line and excute
        printf("LINE as line : %s\n", line);
        cmd = parse_line(line);
        //TODO: Add logging to dispatcher.txt file
        if (cmd->is_dispatcher == 1) {
            if (dispatcher_cmd_exec(cmd, num_threads) == 1) return 1; //Need to check that this actually is running
        }

        // Offload to threads 
        else {
            pthread_mutex_lock(&work_queue_lock);
            num_jobs_pending++;

            // Add to work queue
            work_queue = realloc(work_queue,num_jobs_pending * sizeof(cmd_line_s)); // Add command to the work queue
            work_queue[num_jobs_pending-1] = *cmd; //Need to verify this is correct

            pthread_cond_broadcast(&work_available);                       // Notify all threads
            pthread_mutex_unlock(&work_queue_lock);
        }




    }
    return 0;
}


