#ifndef Hw2THREADS_H
#define Hw2THREADS_H

#define MAX_THREADS 4096
#define MAX_COUNTERS 100
#define MAX_LINE_LENGTH 1024

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <limits.h>

typedef enum
{
    DIS_WAIT,
    DIS_SLEEP,
    CMD_MSLEEP,
    CMD_INCREMENT,
    CMD_DECREMENT,
    CMD_REPEAT
} CmdType;

typedef struct
{
    CmdType type;
    int value;
} cmd_s;

typedef struct
{
    cmd_s *cmds;
    int num_of_cmds;
    int is_dispatcher; // 1 - dispatcher, 1 - worker
    char *line;        // logging
    long long start_time;
} cmd_line_s;

typedef struct
{
    int thread_id;
    int log_enabled;
} thread_args;

extern pthread_t *worker_trds;
extern pthread_mutex_t work_queue_lock;
extern pthread_cond_t work_available;
extern int *thread_status;
extern cmd_line_s *work_queue;
extern int num_jobs_pending;

extern struct timespec program_start_time;
extern long long total_turnaround_time;
extern long long min_turnaround_time;
extern long long max_turnaround_time;
extern int job_count;

extern pthread_mutex_t stats_lock;

void *dispatcher(void *arg);

void dispatcher_wait(int num_threads);

int dispatcher_cmd_exec(cmd_line_s *cmd_line, int num_threads);

void *trd_func(void *arg);

int basic_cmd_exec(cmd_s cmd);

cmd_s parse_cmds(char *cmd_str);

cmd_line_s *parse_line(char *line);

void update_stats(long long turnaround_time);

long long get_elapsed_time(struct timespec start_time);

void dispatcher_wait_for_all(int num_threads);

void restart_logs(int num_threads);

extern pthread_mutex_t job_completion_lock;
extern pthread_cond_t all_jobs_done;
extern int terminate_threads;

// Add this function declaration
void signal_all_jobs_completed();
#endif // Hw2THREADS_H