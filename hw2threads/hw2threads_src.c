#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include "hw2threads_src.h"

void *dispatcher(void *arg) {
    // Placeholder worker thread logic
    printf("Worker thread running\n");
    return NULL;
}
void dispatcher_wait() {};

void *worker(void *arg) {
    // Placeholder worker thread logic
    printf("Worker thread running\n");
    return NULL;
}
void *trd_func(void *arg) {
    // Placeholder worker thread logic
    printf("Worker thread running\n");
    return NULL;
}

cmd_s parse_cmds(char *cmd_str) {
    
    cmd_s cmd;
    
    //char command[MAX_LINE_LENGTH];
    int value;
    
    if (sscanf(cmd_str, " msleep %d", &value) == 1) {
        cmd.type = CMD_MSLEEP;
        cmd.value = value;
    }
    else if (sscanf(cmd_str, " increment %d", &value) == 1) {
        cmd.type = CMD_INCREMENT;
        cmd.value = value;
    }
    else if (sscanf(cmd_str, " decrement %d", &value) == 1) {
        cmd.type = CMD_DECREMENT;
        cmd.value = value;
    }
    else if (sscanf(cmd_str, " repeat %d", &value) == 1) {
        cmd.type = CMD_REPEAT;
        cmd.value = value;
    }
    
    return cmd;
}



//tokenize cmd line by semi columns 
cmd_line_s*  parse_line(char *line) {
    cmd_line_s* cmd_line = (cmd_line_s *)malloc(sizeof(cmd_line_s));
    cmd_line->cmds = (cmd_s*)malloc(MAX_LINE_LENGTH);
    cmd_line->num_of_cmds = 0;
    cmd_line->line = strdup(line);

     if (strncmp(line, "dispatcher wait", 15) == 0) {
       cmd_line->is_dispatcher = 1;
       dispatcher_wait();
       cmd_s dis_cmd;
       if (strstr(line, "wait") != NULL) {
            dis_cmd.type = DIS_WAIT;  
            dis_cmd.value = 0; //just to assign something
        } else {
            dis_cmd = parse_cmds(line + 10); //10 = len dispatcehr  
        }
    cmd_line->cmds[0]= dis_cmd;
    cmd_line->num_of_cmds =1;
    return cmd_line;
    }

    cmd_line->is_dispatcher =0;
    char* token;
    char* line_copy = strdup(line);
    
    // skip woeker word
    token = strtok(line_copy, " ");
    token = strtok(NULL, ";");
    
    while (token != NULL) {
        cmd_line->cmds[cmd_line->num_of_cmds] = parse_cmds(token);
        cmd_line->num_of_cmds++;
        token = strtok(NULL, ";");
    }
    
    free(line_copy);
    return cmd_line;
}
