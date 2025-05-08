#define _GNU_SOURCE

#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    char** tokens;
    int len;
} cmd;

int countChar(char* str, char tgt){
    int cnt = 0;
    while ((*str) != '\n'){
        cnt += tgt == *str;
        str++;
    }
    return cnt;
}

void ExitWithCode(int errCode){
    if (errCode == 0) exit(0);
    else {
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    }
}

void ParseCommand(cmd* cmd_list, int cmd_list_len){
    for (int i=0; i<cmd_list_len; ++i){

        if (strcmp(cmd_list[i].tokens[0], "exit") == 0){
            ExitWithCode(0);
        }
        
        int rc = fork();
        if (rc == 0){
            execvp(cmd_list[i].tokens[0], cmd_list[i].tokens);
            ExitWithCode(1);
        }
    }
    for (int i=0; i<cmd_list_len; ++i) wait(NULL);
}

int main(){
    while (1){
        printf("prompt> ");

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

        int size = countChar(inp, ' ') + 1;
        char* tokens[size];
        len = 0;

        cmd cmd_list[size];
        int cmd_list_len = 0;
        cmd_list[0].len = 0;
        
        char delim = ' ';
        while ((tokens[len] = strsep(&inp, &delim)) != NULL){
            if (*tokens[len] == '\0') continue;
            else if (strcmp(tokens[len], "&") == 0){
                if (len == 0){
                    continue;
                }
                tokens[len] = NULL;
                cmd_list[cmd_list_len].tokens = calloc(sizeof(char*), len);

                for (int i = 0; i<len; ++i){
                    cmd_list[cmd_list_len].tokens[i] = calloc(sizeof(char), strlen(tokens[i]) + 1);
                    strcpy(cmd_list[cmd_list_len].tokens[i], tokens[i]);
                }

                cmd_list[cmd_list_len].len = len;
                len = 0;
                cmd_list_len++;
            }
            else len++;
        }
        if (len > 0){
            cmd_list[cmd_list_len].tokens = tokens;
            cmd_list[cmd_list_len].len = len;
            len = 0;
            cmd_list_len++;
        }
        
        ParseCommand(cmd_list, cmd_list_len);
        free(to_free);
    }
}