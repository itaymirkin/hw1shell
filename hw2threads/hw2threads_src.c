#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include "hw2threads_src.h"

pthread_t *worker_trds;
pthread_mutex_t work_queue_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t work_available = PTHREAD_COND_INITIALIZER;
int *thread_status;
cmd_line_s *work_queue;

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
        for (int i = 0; i < num_threads; i++)
        {
            if (thread_status[i] == 1)
                command_in_background = 1;
        }

        if (command_in_background == 0)
            return;
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

// This is not in use
void *worker(void *arg)
{

    // Placeholder worker thread logic
    printf("Worker thread running\n");
    return NULL;
}

// TODO: Add loggging
void *trd_func(void *arg)
{
    int thread_id = *(int *)arg;
    while (1)
    {
        pthread_mutex_lock(&work_queue_lock);

        while (num_jobs_pending == 0)
        {
            thread_status[thread_id] = 0; // Turn off busy indication -IDLE
            pthread_cond_wait(&work_available, &work_queue_lock);
        }
        thread_status[thread_id] = 1; // Turn on busy indication
        printf("NUM OF PENDING JOBBS %d\n", num_jobs_pending);
      
        
        cmd_line_s cmd_line = work_queue[0];
        
        memmove(work_queue, &work_queue[1], (num_jobs_pending - 1) * sizeof(cmd_line_s));
        
        num_jobs_pending--;
        
        work_queue = realloc(work_queue, num_jobs_pending * sizeof(cmd_line_s));
        
        pthread_mutex_unlock(&work_queue_lock);
        
        printf("status of theads with idx %d is %d\n", thread_id, thread_status[thread_id]);
        if (cmd_line.is_dispatcher == 1)
        {
            puts("Error: thread has recieved a dispatcher command\n");
            printf("Error occured on line: %s", cmd_line.line);
            EXIT_FAILURE;
        }
        
        // Iterate through the commands
        // TODO: Need to determine of the work is done in parallel to other threads
        for (int cmd_idx = 0; cmd_idx < cmd_line.num_of_cmds; cmd_idx++)
        {
            cmd_s curr_cmd = cmd_line.cmds[cmd_idx];
            // Handle repeat command
            
            if (curr_cmd.type == CMD_REPEAT)
            {
                // Repeat the following commands accoridng to the repeat value
                for (int rpt_idx = 0; rpt_idx < curr_cmd.value; rpt_idx++)
                {
                    // Iterate through the remaming commands
                    for (int cmd_idx_rpt = cmd_idx + 1; cmd_idx_rpt < cmd_line.num_of_cmds; cmd_idx_rpt++)
                    {
                        int res = basic_cmd_exec(cmd_line.cmds[cmd_idx_rpt]); // TODO: Replace with single line if possible
                        if (res != 0)
                            EXIT_FAILURE;
                    }
                }
                break; // TODO: Need to determine if after repeat the cmd finish or continues to the next commands
            }
            
            // Handle the rest of the commands
            printf("%d", curr_cmd.type);
            int res = basic_cmd_exec(curr_cmd); // TODO: Replace with single line if possible
            
            if (res == 0)
                EXIT_FAILURE;
             
        }
        printf("got here");
        
        
    }
}

// Excecute all other basic commands other than repeat
int basic_cmd_exec(cmd_s cmd)
{
    int val = cmd.value;
    char counter_filename[50];
    char ctr_val[50];
    long long counter_value;
    // While writing to the counter file we lock the thread
    pthread_mutex_lock(&work_queue_lock);
    
    sprintf(counter_filename, "count%02d.txt", val);
    FILE *file = fopen(counter_filename, "r");
    if (file == NULL)
    {
        pthread_mutex_unlock(&work_queue_lock);
        printf("FAILED TO OPEN FILE FOR READING ERROR HEREs");
        return 1;
    }
    while (fgets(ctr_val, sizeof(ctr_val), file) != NULL) {
    // This will keep overwriting last_line, so at the end it contains the last line
    }
    fclose(file);

    printf("BEFORE INC - Last line was: %s FILE IS %s\n", ctr_val, counter_filename);

    counter_value = strtoll(ctr_val, NULL, 10);
    
    // Open the file in append mode to add new data
    file = fopen(counter_filename, "a");
    if (file == NULL) {
        pthread_mutex_unlock(&work_queue_lock);
        printf("Error opening file for writing\n");
        return 1;
    }

    
    switch (cmd.type)
    {
        // TODO: Confirm that sleep does not use cpu time
    case CMD_MSLEEP:
        usleep(val * 1000); // The function sleeps in microseconds, thus we multiply by 10^3 to get miliseconds
        break;

    case CMD_INCREMENT:
        counter_value++;
        fprintf(file, "%lld\n", counter_value);
        break;

    case CMD_DECREMENT:
        if (counter_value > 0)
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

    // char command[MAX_LINE_LENGTH];
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

// TODO: Need to add support for dispatcher sleep parsing, we handle the wait outside (already done)
// tokenize cmd line by semi columns
cmd_line_s *parse_line(char *line)
{
    cmd_line_s *cmd_line = (cmd_line_s *)malloc(sizeof(cmd_line_s));
    cmd_line->cmds = (cmd_s *)malloc(MAX_LINE_LENGTH);
    cmd_line->num_of_cmds = 0;
    cmd_line->line = strdup(line);

    // TODO: Change the condition for is_dispatcher based on the dispatcher flag and not the command
   
    if (strncmp(line, "dispatcher", 10) == 0)
    {
         
        cmd_line->is_dispatcher = 1;
        //    dispatcher_wait(); Removed
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
