#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/types.h>

typedef struct {
    pid_t pid;
    char command[MAX_LINE];
} BackgroundProcess;


BackgroundProcess bg_processes[MAX_BG_PROCESSES];

// parse command line into args
int parse_command(char *line, char **args) {
    int argc = 0;
    int i = 0;
    
    while (line[i] != '\0') {
        while (line[i] == ' ' || line[i] == '\t' || line[i] == '\n') {
            i++;
        }

        if (line[i] == '\0') {
            break;
        }
        
        //new arg
        int capacity = 1;
        int j = 0;
        char *curr_arg = (char*)malloc(capacity + 1);  // +1 for null 
        if (!curr_arg) {
            //remove
            printf("Memory allocation failed\n");
            for (int k = 0; k < argc; k++) {
                free(args[k]);
            }
            return -1;
        }
        while (line[i] != ' ' && line[i] != '\t' && line[i] != '\n' && line[i] != '\0') {
            if (j >= capacity) {
                capacity *= 2;
                char *temp = (char*)realloc(curr_arg, capacity + 1);
                
                if (!temp) {
                    printf("Memory reallocation failed\n");
                    free(curr_arg);
                    for (int k = 0; k < argc; k++) {
                        free(args[k]);
                    }
                    return -1;
                }
                curr_arg = temp;
            }
            
            curr_arg[j++] = line[i++];
        }
        

        curr_arg[j] = '\0';
        //TODO: Remove - only for debug/////////////////////////
        //printf("arg number %d is %s\n", argc, curr_arg);
        if (argc < MAX_ARGS - 1) {
            args[argc++] = curr_arg;
        } else {
            free(curr_arg);
            break;
        }
    }
    // null terminate 
    args[argc] = NULL;
    return argc;
}

//command should is background?
int is_bg(char **args, int argc) {
    if (argc > 0 && strcmp(args[argc-1], "&") == 0) {
        args[argc-1] = NULL; //Remove & from agrs - Already been used   
        return 1;
    }
    return 0;
}

// clean_finished background processes
void clean_bg() {
    int i = 0;
    while (i < num_bg_processes) {
        int status;
        pid_t finish = waitpid(bg_processes[i].pid, &status, WNOHANG);
        
        if (finish > 0) {
            printf("\nhw1shell: pid %d finished\n", bg_processes[i].pid);
            fflush(stdout);
            //shifting remaining elems
            for (int j = i; j < num_bg_processes - 1; j++) {
                bg_processes[j] = bg_processes[j + 1];
            }
            num_bg_processes--;
            continue;
        } else if (finish < 0 && errno != ECHILD) {
            printf("\nhw1shell: waitpid failed, errno is %d\n", errno);
        } else {
            i++;
        }
        
    }
}

//add bg process to array
int add_to_bg(pid_t pid, char *command) {
    if (num_bg_processes >= MAX_BG_PROCESSES) {
        return -1;
    }
    
    bg_processes[num_bg_processes].pid = pid;
    strncpy(bg_processes[num_bg_processes].command, command, MAX_LINE - 1);
    bg_processes[num_bg_processes].command[MAX_LINE - 1] = '\0';
    num_bg_processes++;
    
    return 0;
}

//  cd command
void handle_cd(char **args) {
    char path[MAX_LINE];
    if (getcwd(path, sizeof(path)) != NULL) {
        if (args[1] == NULL)
        {
            printf("\nhw1shell: invalid command\n");
            fflush(stdout);
        }
        else {
             //printf("args[1]: %s\n", args[1]);
             if (chdir(args[1]) == 0)
             {
                 getcwd(path, sizeof(path));
                 //printf("Current working dir after change: %s\n", path);
             }
             else {
                printf("\nhw1shell: invalid command\n");
             }
        }
        
    }  
}


//jobs command
void handle_jobs() {
    for (int i = 0; i < num_bg_processes; i++) {
        printf("%d\t%s\n", bg_processes[i].pid, bg_processes[i].command);
    }
}