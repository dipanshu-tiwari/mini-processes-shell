#define _GNU_SOURCE

#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

// structure to store each command
typedef struct {
    char** tokens;
    int len;
} cmd;

// counts the occurrence of the tgt char in string str
int countChar(char* str, char tgt){
    int cnt = 0;
    while ((*str) != '\n' && (*str) != '\0'){
        cnt += tgt == *str;
        str++;
    }
    return cnt;
}

// error handler
void ExitWithCode(int errCode){
    if (errCode == 0) exit(0);
    else if (errCode == 1) {
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    }
    else if (errCode == 2){
        char error_message[30] = "Executable not found\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(2);
    }
}

cmd* ParseCommand(char* inp, int* cmd_list_len){
    char* tokens[countChar(inp, ' ') + 2];
    int len = 0;

    cmd* cmd_list = malloc(sizeof(cmd) * (countChar(inp, '&') + 1));
    cmd_list[0].len = 0;
    
    // tokenization
    char delim = ' ';
    while ((tokens[len] = strsep(&inp, &delim)) != NULL){

        // ignoring empty tokens
        if (*tokens[len] == '\0') continue;

        else if (strcmp(tokens[len], "&") == 0){

            // ignoring empty commands
            if (len == 0){
                continue;
            }

            tokens[len] = NULL;
            cmd_list[*cmd_list_len].tokens = calloc(sizeof(char*), len + 1);

            for (int i = 0; i<len; ++i){
                cmd_list[*cmd_list_len].tokens[i] = calloc(sizeof(char), strlen(tokens[i]) + 1);
                strcpy(cmd_list[*cmd_list_len].tokens[i], tokens[i]);
            }

            cmd_list[*cmd_list_len].len = len;
            len = 0;
            (*cmd_list_len)++;
        }
        else len++;
    }

    // including the last command
    if (len > 0){
        cmd_list[*cmd_list_len].tokens = calloc(sizeof(char*), len + 1);
        for (int i = 0; i<len; ++i){
            cmd_list[*cmd_list_len].tokens[i] = calloc(sizeof(char), strlen(tokens[i]) + 1);
            strcpy(cmd_list[*cmd_list_len].tokens[i], tokens[i]);
        }

        cmd_list[*cmd_list_len].len = len;
        len = 0;
        (*cmd_list_len)++;
    }
    
    return cmd_list;
}

// execute parsed commands
void ExecuteCommand(cmd* cmd_list, int cmd_list_len, char** paths){

    for (int i=0; i<cmd_list_len; ++i){

        // built-in commands

        // exit
        if (strcmp(cmd_list[i].tokens[0], "exit") == 0){
            ExitWithCode(0);
        }

        // cd
        else if (strcmp(cmd_list[i].tokens[0], "cd") == 0){
            if (cmd_list[i].len > 2){
                char* tmp = strdup(cmd_list[i].tokens[1]);
                for (int j = 2; j < cmd_list[i].len; ++j){
                    strcat(tmp, " ");
                    strcat(tmp, cmd_list[i].tokens[j]);
                }
                if (chdir(tmp) != 0){
                    free(tmp);
                    ExitWithCode(1);
                }
                free(tmp);
            }
            else if (cmd_list[i].len == 2){
                if (chdir(cmd_list[i].tokens[1]) != 0) ExitWithCode(1);
            }
            else if (chdir(getenv("HOME")) != 0) ExitWithCode(1);
        }

        // executable files
        else {
            int rc = fork();
            if (rc == 0){
                
                // checking for the absolute path
                if (countChar(cmd_list[i].tokens[0], '/') > 0){
                    if (access(cmd_list[i].tokens[0], X_OK) == 0){
                        execv(cmd_list[i].tokens[0], cmd_list[i].tokens);
                        ExitWithCode(1);
                    }
                    else {
                        ExitWithCode(2);
                    }
                }

                // checking for relative path
                else {
                    int j=0;
                    for (; paths[j] != NULL; ++j){
                        char* tmp = strdup(paths[j]);
                        strcat(tmp, "/");
                        strcat(tmp, cmd_list[i].tokens[0]);
                        if (access(tmp, X_OK) == 0){
                            execv(tmp, cmd_list[i].tokens);
                            free(tmp);
                            ExitWithCode(1);
                        }
                        free(tmp);
                    }
                    ExitWithCode(2);
                }
            }
        }
    }

    // waiting for childs to finish
    for (int i=0; i<cmd_list_len; ++i) wait(NULL);
}

int main(){

    // all of the search directories
    char* paths[] = {"/bin", "/usr/bin", NULL};

    while (1){
        printf("prompt> ");

        // taking in input
        char* inp = NULL;
        char* to_free;
        size_t len = 0;
        len = getline(&inp, &len, stdin);
        to_free = inp;
        if (len == -1) {
            free(inp);
            ExitWithCode(1);
        }
        if (inp[len - 1] == '\n') inp[len - 1] = '\0';
        
        // Parses the raw input
        int cmd_list_len = 0;
        cmd* cmd_list = ParseCommand(inp, &cmd_list_len);

        // Executes the raw input
        ExecuteCommand(cmd_list, cmd_list_len, paths);

        // freeing memory
        free(to_free);
        for (int i = 0; i < cmd_list_len; ++i){
            for (int j = 0; j < cmd_list[i].len; ++j){
                free(cmd_list[i].tokens[j]);
            }
            free(cmd_list[i].tokens);
        }
    }
}