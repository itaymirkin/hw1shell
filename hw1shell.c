#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/types.h>

#define MAX_LINE 1024
#define MAX_ARGS 64
#define MAX_BG_PROCESSES 4

typedef struct {
    pid_t pid;
    char command[MAX_LINE];
} BackgroundProcess;

BackgroundProcess bg_processes[MAX_BG_PROCESSES];
int num_bg_processes = 0;

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


int main() {
    char line[MAX_LINE];
    char *args[MAX_ARGS];

        
    while (1) {
                     
        // Print prompt
        printf("hw1shell$ ");
        fflush(stdout);
        
        // Read command
        
        if (fgets(line, MAX_LINE, stdin) == NULL) {
            if (feof(stdin)) {
                break;  // Exit on EOF
            }
            clean_bg();
            continue;
        }
        
        // remove trailing newline
        line[strcspn(line, "\n")] = 0;
        
        // skip empty lines
        if (strlen(line) == 0) {
            clean_bg();            
            continue;
        }
        
        // parse
        int argc = parse_command(line, args);
        if (argc == 0)  {
            clean_bg();
            continue;
        }
        
        
        // handle exit
        if (strcmp(args[0], "exit") == 0) {
            // Wait for all background processes before exiting
            while (num_bg_processes > 0) {
                clean_bg();
                if (num_bg_processes > 0) {
                    sleep(1);  // Wait a bit before checking again
                }
            }
            break;
        }
        
        // handle cd
        if (strcmp(args[0], "cd") == 0) {
            handle_cd(args);
            clean_bg();
            continue;
        }
        
        // handle job
        if (strcmp(args[0], "jobs") == 0) {
            handle_jobs();
            clean_bg();
            continue;
        }
        
        // check if it's a background command
        int is_background = is_bg(args, argc);
        
        // check if we can add another background process
        if (is_background && num_bg_processes >= MAX_BG_PROCESSES) {
            printf("hw1shell: too many background commands running\n");
            continue;
        }
        
        // fork a child process
        pid_t pid = fork();
        
        if (pid < 0) {
            printf("hw1shell: fork failed, errno is %d\n", errno);
            continue;
        }
        
        if (pid == 0) { //child
            execvp(args[0], args);
            // if execvp returns then error
            printf("hw1shell: invalid command\n");
            exit(1);
        } else {  //parent
            if (is_background) {
                if (add_to_bg(pid, line) == 0) {
                    printf("hw1shell: pid %d started\n", pid);
                    fflush(stdout);
                }
            } else {
                // wait foreground process completed
                int status;
                if (waitpid(pid, &status, 0) < 0) {
                    printf("hw1shell: waitpid failed, errno is %d\n", errno);
                }
            }
        }
        // Clean up any finished background processes
        clean_bg();
    }
    
    return 0;
}
