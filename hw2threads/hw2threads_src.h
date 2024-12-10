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


typedef enum {
    DIS_WAIT,
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
    cmd_s * cmds;
    int num_of_cmds;
    int is_dispatcher; //1 - dispatcher, 1 - worker
    char * line;        //logging
} cmd_line_s;

void *dispatcher(void *arg);

void dispatcher_wait();

void *trd_func(void *arg);

cmd_s parse_cmds(char *cmd_str);

cmd_line_s *parse_line(char *line);

#endif //Hw2THREADS_H