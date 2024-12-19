#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include "hw2threads_src.h"
#include <time.h>

//* Globals *//

int num_threads;
int num_counters;
int log_enabled;
int num_jobs_pending = 0;

pthread_mutex_t job_completion_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t all_jobs_done = PTHREAD_COND_INITIALIZER;
int terminate_threads = 0;

struct timespec program_start_time;

int main(int argc, char *argv[])
{
    // Record hw2 start time
    if (clock_gettime(CLOCK_MONOTONIC, &program_start_time) == -1)
    {
        perror("clock_gettime failed");
        return 1;
    }
    // Validate num of args
    if (argc != 5)
    {
        fprintf(stderr, "not enough args\n");
        return 1;
    }
    // init main args
    int num_threads = atoi(argv[2]);
    int num_counters = atoi(argv[3]);
    int log_enabled = atoi(argv[4]);
    printf("Num of counters - %d \nNum of threads - %d\nLog enabled - %d\n", num_counters, num_threads, log_enabled);

    // Create countes files and init them
    char counter_filename[50];
    for (int i = 0; i < num_counters; i++)
    {
        sprintf(counter_filename, "count%02d.txt", i);
        #ifdef DEBUG_ON
        printf("File name - %s\n", counter_filename);
        #endif
        FILE *f = fopen(counter_filename, "w");
        if (f == NULL)
        {
            printf("Error in creating counter files\n");
            return 1;
        }
        fprintf(f, "0\n");
        fclose(f);
    }

    // Allocate for the number of threads
    worker_trds = calloc(num_threads, sizeof(pthread_t));
    thread_status = calloc(num_threads, sizeof(int));
    //work_queue = calloc(1, sizeof(cmd_line_s));

    for (int i = 0; i < num_threads; i++)
    {
        thread_args *threadx = malloc(2 * sizeof(int));
        //thread_args threadx;
        threadx->thread_id = i;
        threadx->log_enabled = log_enabled;
        pthread_create(&worker_trds[i], NULL, trd_func, threadx); // add Pointer with trd DATA struct to last null
    }

    /* Dispatcher Work*/

    // Parse cmd line
    FILE *cmdfile = fopen(argv[1], "r");
    if (cmdfile == NULL)
    {
        printf("CANT OPEN CMDFILE");
        return 1;
    }

    char line[MAX_LINE_LENGTH];
    cmd_line_s *cmd;

    restart_logs(num_threads);

    #ifdef DEBUG_ON
        printf("\n-----------------------------------DEBUG_ON------------------------------------------\n");
        printf("               Finished Setup - Dispatcher going thorugh command file\n\n");    
    #endif
    while (fgets(line, MAX_LINE_LENGTH, cmdfile))
    {
        // load a line
        line[strcspn(line, "\n")] = '\0';
        // Tokenize line and excute
        #ifdef DEBUG_ON
            printf("Next command line: %s\n", line);
        #endif
        cmd = parse_line(line);
        // logging to dispatcher.txt file
        if (log_enabled)
        {
            FILE *dispatcher_log = fopen("dispatcher.txt", "a");
            if (dispatcher_log == NULL)
            {
                printf("CANT OPENT dispatcher.txt");
                return 1;
            }
            long long elapsed_time = get_elapsed_time(program_start_time);
            fprintf(dispatcher_log, "TIME %lld: read cmd line: %s\n", elapsed_time, line);
            fclose(dispatcher_log);
        };

        if (cmd->is_dispatcher == 1)
        {
            if (dispatcher_cmd_exec(cmd, num_threads) == 1)
                return 1; // Need to check that this actually is running
        }

        // Offload to threads
        else
        {
            pthread_mutex_lock(&work_queue_lock);
            num_jobs_pending++;

            // Reallocate the queue to hold one more pointer
            work_queue = realloc(work_queue, num_jobs_pending * sizeof(cmd_line_s*));
            if (work_queue == NULL) {
                fprintf(stderr, "Failed to reallocate work queue\n");
                pthread_mutex_unlock(&work_queue_lock);
                return 1;
            }

            // Allocate space for the new command
            work_queue[num_jobs_pending - 1] = malloc(sizeof(cmd_line_s));
            if (work_queue[num_jobs_pending - 1] == NULL) {
                fprintf(stderr, "Failed to allocate space for command\n");
                pthread_mutex_unlock(&work_queue_lock);
                return 1;
            }

            // Copy the command
            memcpy(work_queue[num_jobs_pending - 1], cmd, sizeof(cmd_line_s));

            pthread_cond_broadcast(&work_available);
            pthread_mutex_unlock(&work_queue_lock);
        }
    }

    pthread_mutex_lock(&job_completion_lock);
    #ifdef DEBUG_ON
        printf("\n-----------------------------------DEBUG_ON------------------------------------------\n");
        printf("Dispatcher finished reading cmd file - waiting for all jobs to be accepted by threads\n");    
    #endif
    while (num_jobs_pending > 0) {
        pthread_cond_wait(&all_jobs_done, &job_completion_lock);
    }
    pthread_mutex_unlock(&job_completion_lock);

    // Set terminate flag before cleanup
    terminate_threads = 1;
    pthread_cond_broadcast(&work_available);

    #ifdef DEBUG_ON
        printf("\n-----------------------------------DEBUG_ON------------------------------------------\n");
        printf("         All jobs offloaded - waiting for all of the threads to terminate\n\n");    
    #endif

    // Wait for all threads to finish
    for (int i = 0; i < num_threads; i++) {
        pthread_join(worker_trds[i], NULL);
    }

    #ifdef DEBUG_ON
        printf("\n-----------------------------------DEBUG_ON------------------------------------------\n");
        printf("                    All Threads terminated - Starting cleanup\n\n");    
    #endif

    //-----------------Cleanup-------------------
    free(worker_trds);
    free(thread_status);
    free(work_queue);

    FILE *stats_file = fopen("stats.txt", "w");
    if (stats_file == NULL)
    {
        printf("CANT OPEN stats.txt");
        return 1;
    }

    // Calc stats
    long long total_runtime = get_elapsed_time(program_start_time);
    double avg_turnaround_time = job_count > 0 ? (double)total_turnaround_time / job_count : 0;
    // Write stats
    fprintf(stats_file, "total running time: %lld milliseconds\n", total_runtime);
    fprintf(stats_file, "Sum of jobs turnaround time: %lld milliseconds\n", total_turnaround_time);
    fprintf(stats_file, "Min job turnaround time: %lld milliseconds\n", min_turnaround_time);
    fprintf(stats_file, "Average job turnaround time: %f milliseconds\n", avg_turnaround_time);
    fprintf(stats_file, "Max job turnaround time: %lld milliseconds\n", max_turnaround_time);
    fprintf(stats_file, "Jobs done: %d", job_count);
    return 0;
}
