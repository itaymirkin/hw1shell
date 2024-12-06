#ifndef HW1SHELL_H
#define HW1SHELL_H

#define MAX_LINE 1024
#define MAX_ARGS 64
#define MAX_BG_PROCESSES 4

#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/types.h>

extern int num_bg_processes;


int parse_command(char *line, char **args);
int is_bg(char **args, int argc);
void clean_bg();
int add_to_bg(pid_t pid, char *command);
void handle_cd(char **args);
void handle_jobs();

#endif // HW1SHELL_H
