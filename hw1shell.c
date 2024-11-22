#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/types.h>
#include "shell.h"

int num_bg_processes = 0;


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
