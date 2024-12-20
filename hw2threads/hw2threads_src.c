#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include "hw2threads_src.h"
#include <time.h>

pthread_t *worker_trds;
pthread_mutex_t work_queue_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t work_available = PTHREAD_COND_INITIALIZER;
int all_command_read = 0;
int *thread_status;
cmd_line_s **work_queue;

// Define global variables here
long long total_turnaround_time = 0;
long long min_turnaround_time = LLONG_MAX;
long long max_turnaround_time = 0;
int job_count = 0;
pthread_mutex_t stats_lock = PTHREAD_MUTEX_INITIALIZER;

void *dispatcher(void *arg)
{
    // Placeholder worker thread logic
    printf("Worker thread running\n");
    return NULL;
}

void dispatcher_wait(int num_threads)
{
    int command_in_background;
    while (1)
    {
        command_in_background = 0;
        // Check if any of the threads are currently working
        // pthread_mutex_lock(&work_queue_lock);
        for (int i = 0; i < num_threads; i++)
        {
            if (thread_status[i] == 1)
                command_in_background = 1;
        }
        // pthread_mutex_unlock(&work_queue_lock);
        if (command_in_background == 0)
        {
            printf("dispatcher finish wait\n");
            return;
        }
    }
};

int dispatcher_cmd_exec(cmd_line_s *cmd_line, int num_threads)
{
    for (int i = 0; i < cmd_line->num_of_cmds; i++)
    {
        if (cmd_line->cmds->type == DIS_WAIT)
        {
            dispatcher_wait(num_threads);
        }
        else if (cmd_line->cmds->type == CMD_MSLEEP)
            usleep(cmd_line->cmds->value * 1000); // The function sleeps in microseconds, thus we multiply by 10^3 to get miliseconds
        else
        {
            puts("Error in parsing line, worker command has been passed to the dispatcher");
            printf("Error in line:\n \"%s\"", cmd_line->line);
            return 1;
        }
    }
    return 0;
}


void *trd_func(void *arg)
{
    thread_args *threadx = (thread_args*) arg;
    int thread_id = threadx->thread_id;
    int log_enabled = threadx->log_enabled;
    #ifdef DEBUG_ON
        printf("Thread %d created\n", thread_id);
    #endif
    while (1)
    {
        pthread_mutex_lock(&work_queue_lock);
        if (terminate_threads && num_jobs_pending == 0) {
            pthread_mutex_unlock(&work_queue_lock);
            break;
        }
        while (num_jobs_pending == 0 && !terminate_threads)
        {
            thread_status[thread_id] = 0; // Turn off busy indication -IDLE
            pthread_cond_wait(&work_available, &work_queue_lock);
        }

        
         // Process a job
        if (num_jobs_pending > 0) {
            //cmd_line_s current_job = work_queue[0];
            thread_status[thread_id] = 1; // Turn on busy indication

            cmd_line_s *curr_job = work_queue[0];
            long long start_time = curr_job->start_time;
            #ifdef DEBUG_ON
                printf("START job - thread: %d ,time: %lld, pending jobs: %d\n", thread_id, start_time,num_jobs_pending);
            // printf("START job - pending jobs: %d\n", num_jobs_pending);
            #endif
            // Shift remaining jobs left
            for (int i = 0; i < num_jobs_pending - 1; i++) {
                work_queue[i] = work_queue[i + 1];
            }
            num_jobs_pending--;

            pthread_mutex_unlock(&work_queue_lock);
            

            if (num_jobs_pending == 0) {
                // Signal that all jobs are completed
                pthread_mutex_lock(&job_completion_lock);
                pthread_cond_signal(&all_jobs_done);
                pthread_mutex_unlock(&job_completion_lock);
            }
            pthread_mutex_unlock(&work_queue_lock);

            if (curr_job->is_dispatcher == 1)
            {
                puts("Error: thread has recieved a dispatcher command\n");
                printf("Error occured on line: %s\n", curr_job->line);
                EXIT_FAILURE;
            }

            // Iterate through the commands
            for (int cmd_idx = 0; cmd_idx < curr_job->num_of_cmds; cmd_idx++)
            {
                cmd_s curr_cmd = curr_job->cmds[cmd_idx];
                // Handle repeat command
                if (curr_cmd.type == CMD_REPEAT)
                {
                    // Repeat the following commands accoridng to the repeat value
                    for (int rpt_idx = 0; rpt_idx < curr_cmd.value; rpt_idx++)
                    {
                        // Iterate through the remaming commands
                        for (int cmd_idx_rpt = cmd_idx + 1; cmd_idx_rpt < curr_job->num_of_cmds; cmd_idx_rpt++)
                        {
                            int res = basic_cmd_exec(curr_job->cmds[cmd_idx_rpt],thread_id); // TODO: Replace with single line if possible
                            if (res != 0)
                                EXIT_FAILURE;
                        }
                    }
                    break; // TODO: Need to determine if after repeat the cmd finish or continues to the next commands
                }
                // Handle the rest of the commands
                int res = basic_cmd_exec(curr_cmd, thread_id); // TODO: Replace with single line if possible
                if (res == 0)
                    EXIT_FAILURE;
            }
            long long end_time = get_elapsed_time(program_start_time);
            // log to threads%id.txt file
            if (log_enabled)
            {
                pthread_mutex_lock(&work_queue_lock);
                char log_filename[50];
                sprintf(log_filename, "thread%02d.txt", thread_id);
                #ifdef DEBUG_ON
                    printf("%s\n", log_filename);
                #endif
                FILE *thread_log = fopen(log_filename, "a");
                if (thread_log == NULL)
                {
                    pthread_mutex_unlock(&work_queue_lock);
                    printf("CANT OPEN thread.txt");
                    continue;
                };

                fprintf(thread_log, "TIME %lld: START job %s\n", start_time, curr_job->line);
                fprintf(thread_log, "TIME %lld: END job %s\n", end_time, curr_job->line);
                fclose(thread_log);
                pthread_mutex_unlock(&work_queue_lock);
            }
            long long run_time = end_time - start_time;
            update_stats(run_time);
            printf("END job - thread: %d ,time : %lld,run time: %lld\n", thread_id, end_time, run_time);
            printf("END job - pending jobs: %d\n", num_jobs_pending);              
        }
        else {
            pthread_mutex_unlock(&work_queue_lock);
        }
        
    } 
    #ifdef DEBUG_ON
        printf("Thread %d has been terminated\n", thread_id);
    #endif
    free(arg);
    return NULL;
}

// Excecute all other basic commands other than repeat
int basic_cmd_exec(cmd_s cmd, int id)
{
    int val = cmd.value;
    char counter_filename[50];
    char ctr_val[50];
    long long counter_value;

    // While writing to the counter file we lock the thread
    pthread_mutex_lock(&work_queue_lock);
    // If the command is CMD_MSLEEP, execute sleep immediately
    if (cmd.type == CMD_MSLEEP)
    {
        pthread_mutex_unlock(&work_queue_lock);
        #ifdef DEBUG_ON
            printf("Worker #%d Sleep: %d\n", id ,val);
        #endif 
        usleep(val * 1000); // The function sleeps in microseconds, thus we multiply by 10^3 to get miliseconds
        return 0;
    }

    sprintf(counter_filename, "count%02d.txt", val);
    FILE *file = fopen(counter_filename, "r");
    if (file == NULL)
    {
        pthread_mutex_unlock(&work_queue_lock);
        perror("Error opening file for reading");
        return 1;
    }
    while (fgets(ctr_val, sizeof(ctr_val), file) != NULL)
    {
        // This will keep overwriting last_line, so at the end it contains the last line
    }
    fclose(file);

    counter_value = strtoll(ctr_val, NULL, 10);
    #ifdef DEBUG_ON
        printf("Worker #%d update counterfile: %s, old count: %lld\n", id,counter_filename, counter_value);
    #endif
    // Open the file in append mode to add new data
    file = fopen(counter_filename, "a");
    if (file == NULL)
    {
        pthread_mutex_unlock(&work_queue_lock);
        printf("Error opening file for writing\n");
        return 1;
    }

    switch (cmd.type)
    {
    // case CMD_MSLEEP:
        // printf("basic_cmd_exec - sleep: %d", val);
        // usleep(val * 1000); // The function sleeps in microseconds, thus we multiply by 10^3 to get miliseconds
        // break;

    case CMD_INCREMENT:
        counter_value++;
        fprintf(file, "%lld\n", counter_value);
        break;

    case CMD_DECREMENT:
        counter_value--;
        fprintf(file, "%lld\n", counter_value);
        break;

    default:
        break;
    }
    fclose(file);
    pthread_mutex_unlock(&work_queue_lock);
    return 0;
}

cmd_s parse_cmds(char *cmd_str)
{
    cmd_s cmd;
    int value;

    if (sscanf(cmd_str, " msleep %d", &value) == 1)
    {
        cmd.type = CMD_MSLEEP;
        cmd.value = value;
    }
    else if (sscanf(cmd_str, " increment %d", &value) == 1)
    {
        cmd.type = CMD_INCREMENT;
        cmd.value = value;
    }
    else if (sscanf(cmd_str, " decrement %d", &value) == 1)
    {
        cmd.type = CMD_DECREMENT;
        cmd.value = value;
    }
    else if (sscanf(cmd_str, " repeat %d", &value) == 1)
    {
        cmd.type = CMD_REPEAT;
        cmd.value = value;
    }

    return cmd;
}

// Tokenize cmd line by semi columns
cmd_line_s *parse_line(char *line)
{
    cmd_line_s *cmd_line = (cmd_line_s *)malloc(sizeof(cmd_line_s));
    cmd_line->cmds = (cmd_s *)malloc(MAX_LINE_LENGTH);
    cmd_line->num_of_cmds = 0;
    cmd_line->line = strdup(line);
    cmd_line->start_time = get_elapsed_time(program_start_time);


    if (strncmp(line, "dispatcher", 10) == 0)
    {

        cmd_line->is_dispatcher = 1;
        cmd_s dis_cmd;
        if (strstr(line + 11, "wait") != NULL)
        {

            dis_cmd.type = DIS_WAIT;
            dis_cmd.value = 0; // just to assign something
        }
        else
        {
            dis_cmd = parse_cmds(line + 10); // 10 = len dispatcehr
        }
        cmd_line->cmds[0] = dis_cmd;
        cmd_line->num_of_cmds = 1;
        return cmd_line;
    }

    cmd_line->is_dispatcher = 0;
    char *token;
    char *line_copy = strdup(line);

    // skip worker word
    token = strtok(line_copy, " ");
    token = strtok(NULL, ";");

    while (token != NULL)
    {
        cmd_line->cmds[cmd_line->num_of_cmds] = parse_cmds(token);
        cmd_line->num_of_cmds++;
        token = strtok(NULL, ";");
    }

    free(line_copy);
    return cmd_line;
}

long long get_elapsed_time(struct timespec start_time)
{
    struct timespec current_time;
    clock_gettime(CLOCK_MONOTONIC, &current_time);
    return (current_time.tv_sec - start_time.tv_sec) * 1000LL +
           (current_time.tv_nsec - start_time.tv_nsec) / 1000000LL;
}

void update_stats(long long turnaround_time)
{
    
    pthread_mutex_lock(&stats_lock);
    total_turnaround_time += turnaround_time;
    if (turnaround_time < min_turnaround_time)
        min_turnaround_time = turnaround_time;
    if (turnaround_time > max_turnaround_time)
        max_turnaround_time = turnaround_time;
    job_count++;
    pthread_mutex_unlock(&stats_lock);
}

void dispatcher_wait_for_all(int num_threads)
{
    int jobs_pending;

    do
    {
        pthread_mutex_lock(&work_queue_lock);
        jobs_pending = num_jobs_pending;
        pthread_mutex_unlock(&work_queue_lock);

        if (jobs_pending > 0)
        {
            // Sleep briefly to allow threads to finish their work
            usleep(1000);
        }
    } while (jobs_pending > 0);

    // Ensure all threads are idle
    for (int i = 0; i < num_threads; i++)
    {
        while (thread_status[i] == 1)
        {
            usleep(1000);
        }
    }
}

void restart_logs(int num_threads)
{
    // Restart dispatcher log
    FILE *dispatcher_log = fopen("dispatcher.txt", "w");
    if (dispatcher_log == NULL)
    {
        perror("Error resetting dispatcher log");
        return;
    }
    fclose(dispatcher_log);

    // Restart logs for each thread
    for (int i = 0; i < num_threads; i++)
    {
        char thread_log_filename[50];
        sprintf(thread_log_filename, "thread%02d.txt", i);

        FILE *thread_log = fopen(thread_log_filename, "w");
        if (thread_log == NULL)
        {
            perror("Error resetting thread log");
            continue;
        }
        fclose(thread_log);
    }

    printf("Logs reset successfully for %d threads.\n", num_threads);
}
