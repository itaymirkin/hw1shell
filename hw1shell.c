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

// Function to parse command line into arguments
int parse_command(char *line, char **args) {
    int argc = 0;
    char *token = strtok(line, " \t\n");
    
    while (token != NULL && argc < MAX_ARGS - 1) {
        args[argc++] = token;
        token = strtok(NULL, " \t\n");
    }
    args[argc] = NULL;
    return argc;
}

// Function to check if command should run in background
int is_background_command(char **args, int argc) {
    if (argc > 0 && strcmp(args[argc-1], "&") == 0) {
        args[argc-1] = NULL;  // Remove & from arguments
        return 1;
    }
    return 0;
}

// Function to clean up finished background processes
void cleanup_finished_processes() {
    int i = 0;
    while (i < num_bg_processes) {
        int status;
        pid_t result = waitpid(bg_processes[i].pid, &status, WNOHANG);
        
        if (result > 0) {
            printf("hw1shell: pid %d finished\n", bg_processes[i].pid);
            // Remove process from array by shifting remaining elements
            for (int j = i; j < num_bg_processes - 1; j++) {
                bg_processes[j] = bg_processes[j + 1];
            }
            num_bg_processes--;
        } else if (result < 0 && errno != ECHILD) {
            printf("hw1shell: waitpid failed, errno is %d\n", errno);
        } else {
            i++;
        }
    }
}

// Function to add background process to array
int add_background_process(pid_t pid, char *command) {
    if (num_bg_processes >= MAX_BG_PROCESSES) {
        return -1;
    }
    
    bg_processes[num_bg_processes].pid = pid;
    strncpy(bg_processes[num_bg_processes].command, command, MAX_LINE - 1);
    bg_processes[num_bg_processes].command[MAX_LINE - 1] = '\0';
    num_bg_processes++;
    
    return 0;
}

// Function to implement built-in cd command
void handle_cd(char **args) {
    char path[MAX_LINE];
    if (getcwd(path, sizeof(path)) != NULL) {
        if (args[1] == NULL)
        {
            printf("hw1shell: invalid command\n");
        }
        else {
             //printf("args[1]: %s\n", args[1]);
             if (chdir(args[1]) == 0)
             {
                 getcwd(path, sizeof(path));
                 //printf("Current working dir after change: %s\n", path);
             }
        }
        
    }  
}


// Function to implement built-in jobs command
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
            continue;
        }
        
        // Remove trailing newline
        line[strcspn(line, "\n")] = 0;
        
        // Skip empty lines
        if (strlen(line) == 0) {
            continue;
        }
        
        // Parse command
        int argc = parse_command(line, args);
        if (argc == 0) continue;
        
        // Handle exit command
        if (strcmp(args[0], "exit") == 0) {
            // Wait for all background processes before exiting
            while (num_bg_processes > 0) {
                cleanup_finished_processes();
                if (num_bg_processes > 0) {
                    sleep(1);  // Wait a bit before checking again
                }
            }
            break;
        }
        
        // Handle cd command
        if (strcmp(args[0], "cd") == 0) {
            handle_cd(args);
            continue;
        }
        
        // Handle jobs command
        if (strcmp(args[0], "jobs") == 0) {
            handle_jobs();
            continue;
        }
        
        // Check if it's a background command
        int is_background = is_background_command(args, argc);
        
        // Check if we can add another background process
        if (is_background && num_bg_processes >= MAX_BG_PROCESSES) {
            printf("hw1shell: too many background commands running\n");
            continue;
        }
        
        // Fork a child process
        pid_t pid = fork();
        
        if (pid < 0) {
            printf("hw1shell: fork failed, errno is %d\n", errno);
            continue;
        }
        
        if (pid == 0) {  // Child process
            execvp(args[0], args);
            // If execvp returns, there was an error
            printf("hw1shell: invalid command\n");
            exit(1);
        } else {  // Parent process
            if (is_background) {
                // Add to background processes array
                if (add_background_process(pid, line) == 0) {
                    printf("hw1shell: pid %d started\n", pid);
                }
            } else {
                // Wait for foreground process to complete
                int status;
                if (waitpid(pid, &status, 0) < 0) {
                    printf("hw1shell: waitpid failed, errno is %d\n", errno);
                }
            }
        }
        
        // Clean up any finished background processes
        cleanup_finished_processes();
    }
    
    return 0;
}