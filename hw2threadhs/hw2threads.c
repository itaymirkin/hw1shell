#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <string.h>

#define MAX_THREADS 4096
#define MAX_COUNTERS 100
#define MAX_LINE_LENGTH 1024

pthread_t* worker_trds;
pthread_mutex_t work_queue_lock;
pthread_cond_t work_available;

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
//* Globals *//

int num_threads;          
int num_counters;
int log_enabled;
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



int main(int argc, char *argv[]) {
    //Validate num of args
    if (argc != 5) {
        fprintf(stderr, "not enough args\n");
        return 1;
    }
    //init main args
    num_threads = atoi(argv[2]);
    num_counters = atoi(argv[3]);
    log_enabled = atoi(argv[4]);
    printf("num of counters - %d \nnum of threads - %d\nlog enabled - %d\n", num_counters, num_threads, log_enabled); 

    //Create countes files and init them 
    char counter_filename[50];
    for (int i = 0; i < num_counters; i++)
    {
        sprintf(counter_filename, "count%02d.txt", i);
        printf("file name - %s\n", counter_filename);
        FILE *f = fopen(counter_filename, "w");
        if (f == NULL)
        {
            printf("FILE CREATION ERROR HEREs");
            return 1;
        }
        fprintf(f, "0\n");
        fclose(f);
    }
    



        worker_trds = calloc(num_threads, sizeof(pthread_t));


        for (int i = 0; i < num_threads; i++)
        {
            pthread_create(&worker_trds[i], NULL, trd_func, NULL); // add Pointer with trd DATA struct to last null
        }

        /* Dispatcher Work*/

        //Parse cmd line 
        FILE *cmdfile = fopen(argv[1], "r");
        if (cmdfile == NULL) {
            printf("CANT OPEN CMDFILE");
            return 1;
        }

        char line[MAX_LINE_LENGTH];
        while (fgets(line, MAX_LINE_LENGTH, cmdfile)) {
            //loaded a line
            line[strcspn(line, "\n")] = '\0';
            //tokenize line and excute
            printf("LINE as line : %s\n", line);
            parse_line(line);
    }
    return 0;
}


