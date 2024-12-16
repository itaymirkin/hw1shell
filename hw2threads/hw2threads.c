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
    printf("num of counters - %d \nnum of threads - %d\nlog enabled - %d\n", num_counters, num_threads, log_enabled);

    // Create countes files and init them
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
    worker_trds = calloc(num_threads, sizeof(pthread_t));
    thread_status = calloc(num_threads, 1);
    work_queue = calloc(1, sizeof(cmd_line_s));

    for (int i = 0; i < num_threads; i++)
    {
        thread_args *threadx = malloc(sizeof(thread_args));
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

    while (fgets(line, MAX_LINE_LENGTH, cmdfile))
    {

        // load a line
        line[strcspn(line, "\n")] = '\0';
        // Tokenize line and excute
        printf("LINE as line : %s\n", line);
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

            // Add to work queue
            work_queue = realloc(work_queue, num_jobs_pending * sizeof(cmd_line_s)); // Add command to the work queue
            work_queue[num_jobs_pending - 1] = *cmd;                                 // Need to verify this is correct

            pthread_cond_broadcast(&work_available); // Notify all threads
            pthread_mutex_unlock(&work_queue_lock);
             
        }
    }

    pthread_mutex_lock(&job_completion_lock);
    while (num_jobs_pending > 0) {
        printf("herereere\n");
        pthread_cond_wait(&all_jobs_done, &job_completion_lock);
       
    }
    pthread_mutex_unlock(&job_completion_lock);

    // Set terminate flag before cleanup
    terminate_threads = 1;
    pthread_cond_broadcast(&work_available);

    // Wait for all threads to finish
    for (int i = 0; i < num_threads; i++) {
        pthread_join(worker_trds[i], NULL);
    }

    // Cleanup

    // dispatcher_wait_for_all(num_threads);
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
    fprintf(stats_file, "sum of jobs turnaround time: %lld milliseconds\n", total_turnaround_time);
    fprintf(stats_file, "min job turnaround time: %lld milliseconds\n", min_turnaround_time);
    fprintf(stats_file, "average job turnaround time: %f milliseconds\n", avg_turnaround_time);
    fprintf(stats_file, "max job turnaround time: %lld milliseconds\n", max_turnaround_time);
    fprintf(stats_file, "jobs done: %d", job_count);
    return 0;
}
