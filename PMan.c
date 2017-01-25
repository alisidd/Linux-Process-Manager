#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <limits.h>

typedef struct node {
    pid_t process_id;
    char *cmd;
    bool is_stop;

    struct node *next;
} proc;

void create_process(char*);
void update_processes();

void add_process(pid_t, char[]);
void remove_process(pid_t);

void list_processes();

void kill_process(char*);
void stop_process(char*);
void start_process(char*);
void read_process(char*);

void get_lower(char []);

proc *g_root = NULL;

int main(int argc, char *argv[]) {

    // Set $PATH in program to current $PATH
    char *buffer = getenv("PATH");
    setenv("PATH", buffer, 1);

    g_root = malloc(sizeof(proc));

    if (g_root == NULL) {
        perror("Error: Out of memory");
        return 1;
    }

    while (1) {

        char input_command[2];
        char *tokens;

        printf("PMan:  >");
        fgets(input_command, 200, stdin);

        get_lower(input_command);
        // Get rid of new line feed in buffer
        strtok(input_command, "\n");

        // Tokenize the argument entered by user
        tokens = strtok(input_command, " ");

        if (strcmp(tokens, "bg") == 0) {

            create_process(tokens);

        } else if (strcmp(tokens, "bglist") == 0) {

            list_processes();

        } else if (strcmp(tokens, "bgkill") == 0) {

            kill_process(tokens);

        } else if (strcmp(tokens, "bgstop") == 0) {

            stop_process(tokens);

        } else if (strcmp(tokens, "bgstart") == 0) {

            start_process(tokens);

        } else if (strcmp(tokens, "pstat") == 0) {

            read_process(tokens);

        } else {
            if (strcmp(tokens, "\n") != 0) {
                printf("%s: command not found\n", tokens);
            }
        }

        update_processes();
    }
}

void create_process(char *tokens) {

    char *first_argument = tokens;
    char *arguments[200];
    int status;

    tokens = strtok(NULL, " ");

    int i = 0;
    // Add all the words after bg to "arguments"
    while (tokens != NULL) {
        arguments[i] = tokens;
        tokens = strtok(NULL, " ");
        i++;
    }

    arguments[i] = 0;

    pid_t pid = fork();

    if (pid == 0) {
        if (execvp(arguments[0], arguments) == -1) {
            perror("Error: Can't run command");
        }
        exit(0);
    } else if (pid > 0) {
        //Build cmd using collected arguments
        char cmd[200];
        sprintf(cmd, "%s %s", first_argument, *arguments);

        add_process(pid, cmd);
    		update_processes();
    } else {
        perror("Error: Can't fork process");
    }
}

void update_processes() {
    int status;

    while(1) {

        int id = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED);

        if (id > 0) {
            if (WIFEXITED(status) || WIFSIGNALED(status)) {

                remove_process(id);
                printf("Process %d terminated\n", id);

            } else if (WIFSTOPPED(status)) {

                // Change is_stop value of process that got stoped to true
                proc *head = g_root;

                if (head->process_id == id) {
                    head->is_stop = true;
                    return;
                }

                while (head->next != NULL) {
                    if (head->process_id == id) {
                        head->is_stop = true;
                    }
                    head = head->next;
                }

            } else if (WIFCONTINUED(status)) {

                // Change is_stop value of process that got continued to false
                proc *head = g_root;

                if (head->process_id == id) {
                    head->is_stop = false;
                    return;
                }

                while (head->next != NULL) {
                    if (head->process_id == id) {
                        head->is_stop = false;
                    }
                    head = head->next;
                }
            }
        } else {
            break;
        }
    }
}

void add_process(pid_t pid, char cmd[]) {
    proc *head = g_root;
    while (head->process_id != 0) {
        head = head->next;
    }
    head->process_id = pid;
    head->cmd = cmd;
    head->is_stop = false;

    head->next = malloc(sizeof(proc));

    if (head->next == NULL) {
        perror("Error: Out of memory");
        return;
    }

    head->next->process_id = 0;
}

void remove_process(pid_t pid) {
    proc *head = g_root;
    proc *prev = head;

    // Remove the process right away if it's the first one in the linked list
    if (head->process_id == pid) {
        head->process_id = head->next->process_id;
        head->cmd = head->next->cmd;
        head->is_stop = head->next->is_stop;
        proc *n = head->next;

        head->next = head->next->next;

        free(n);

        return;
    }

    // If process is not the first in the linkest list, find it and remove it
    while(prev->next->process_id != 0 && prev->next->process_id != pid) {
        prev = prev->next;
    }

    proc *n = prev->next;

    prev->next = prev->next->next;

    free(n);
}

void kill_process(char *tokens) {
    tokens = strtok(NULL, " ");

    if (tokens != NULL) {
        pid_t pidToKill = atoi(tokens);

        if (pidToKill == 0) {
            perror("Error: Process ID not valid");
            return;
        }

        int killStatus = kill(pidToKill, SIGTERM);

        if (killStatus == -1) { //if process doesn't terminate properly
            printf("Error: Process ID not valid\n");
        }

    } else {
        printf("Error: Process ID not provided\n");
    }
}

void stop_process(char *tokens) {
    tokens = strtok (NULL, " ");

    if (tokens != NULL) {
        pid_t pidToStop = atoi(tokens);

        if (pidToStop == 0) {
            perror("Error: Process ID not valid");
            return;
        }

        if (kill(pidToStop, SIGSTOP) == -1) {
            printf("Error: Process ID not valid\n");
        }

    } else {
        printf("Error: Process ID not provided\n");
    }
}

void start_process(char *tokens) {
    tokens = strtok (NULL, " ");

    if (tokens != NULL) {
        pid_t pidToStart = atoi(tokens);

        if (pidToStart == 0) {
            perror("Error: Process ID not valid");
            return;
        }

        if (kill(pidToStart, SIGCONT) == -1) {
            printf("Error: Process ID not valid\n");
        }

    } else {
        printf("Error: Process ID not provided\n");
    }
}

void list_processes() {
    proc *head = g_root;
    int backgroundJobs = 0;
    while (head->process_id != 0) {
        if (head->is_stop == false) {
            printf("%d: ", head->process_id);;

            //Read the /proc/[pid]/exe file for path name
            FILE *procFile;
            char path[100];
            char buffer[PATH_MAX + 1];

            sprintf(path, "/proc/%d/exe", head->process_id);

            if (readlink(path, buffer, sizeof(buffer)) != -1) {
              printf("%s\n", buffer);
			      } else {
              perror("Error: Can't fetch path");
            }

            backgroundJobs++;
        }
        head = head->next;
    }
    printf("Total background jobs: %d\n", backgroundJobs);
}

void read_process(char *tokens) {
    FILE *procFile;
    char path[100];

    tokens = strtok (NULL, " ");

    //Build the path
    sprintf(path, "/proc/%s/stat", tokens);

    procFile = fopen(path, "r");

    if (procFile) {
        char comm[100];
        char state;
        unsigned long utime;
        unsigned long stime;
        long rss;

        fscanf(procFile, "%*d %s %c %*s %*d %*d %*d %*u %*lu %*lu %*lu %*lu %lu %lu %*ld %*ld %*ld %*ld %*ld %*ld %*llu %*lu %ld", &comm, &state, &utime, &stime, &rss);

        printf("Comm: %s\nState: %c\nUTime: %lu\nSTime: %lu\nRSS: %ld\n", comm, state, utime, stime, rss);

        fclose(procFile);

    		sprintf(path, "/proc/%s/status", tokens);
    		procFile = fopen(path, "r");

    		char *line;
    		size_t length = 0;
    		int i = 0;

    		if (procFile) {
    			while (getline(&line, &length, procFile) != -1) {
    				if (i == 46 || i == 47) {
    					printf("%s", line);
    				}
    				i++;
    			}
    			free(line);
    			fclose(procFile);
    		}

    } else {
        printf("Error: Process %s does not exist\n", tokens);
    }
}

void get_lower(char str[]) {
    int counter = 0;
    while (str[counter] != '\0') {
        if (str[counter] >= 'A' && str[counter] <= 'Z') {
            str[counter] = str[counter] + 32;
        }
        counter++;
    }
}
