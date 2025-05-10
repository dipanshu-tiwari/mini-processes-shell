#define _GNU_SOURCE

#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h> // for open()

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

// tokenizer
char* tokenizer(char** inp, char* delim, char* used_char){
    if (*inp == NULL || **inp == '\0') return NULL;
    char* token = *inp;
    while (**inp != '\0'){
        for (int i = 0; delim[i] != '\0'; ++i){
            if (**inp == delim[i]){
                **inp = '\0';
                (*inp)++;
                *used_char = delim[i];
                return token;
            }
        }
        (*inp)++;
    }
    *inp = NULL;
    *used_char = '\0';
    return token;
}

// parser
cmd* ParseCommand(char* inp, int* cmd_list_len){
    char* tokens[countChar(inp, ' ') + countChar(inp, '\t') + countChar(inp, '\v') + 2 * (countChar(inp, '>')) + 2];
    int len = 0;

    cmd* cmd_list = malloc(sizeof(cmd) * (countChar(inp, '&') + 1));
    cmd_list[0].len = 0;
    
    // tokenization
    char* delim = strdup(" \t\v>&");
    char used_char = '\0';
    while ((tokens[len] = tokenizer(&inp, delim, &used_char)) != NULL){

        // ignoring empty tokens
        if (*tokens[len] == '\0');
        else len++;

        if (used_char == ' ' || used_char == '\t' || used_char == '\v') continue;
        else if (used_char == '&'){
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
        else if (used_char == '>'){
            tokens[len] = strdup(">");
            len++; 
        }
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
    free(delim);
    return cmd_list;
}

// execute parsed commands
void ExecuteCommand(cmd* cmd_list, int cmd_list_len, char*** paths, int* run){

    for (int i=0; i<cmd_list_len; ++i){

        // built-in commands

        // exit
        if (strcmp(cmd_list[i].tokens[0], "exit") == 0){
            (*run) = 0;
            return;
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

        // path
        else if (strcmp(cmd_list[i].tokens[0], "path") == 0){
            int path_curr_len = 0;
            while ((*paths)[path_curr_len] != NULL){
                free((*paths)[path_curr_len]);
                path_curr_len++;
            }
            free((*paths)[path_curr_len]);
            free((*paths));

            (*paths) = (char **)calloc(sizeof(char*), (cmd_list[i].len));
            for (int j = 0; j < cmd_list[i].len - 1; ++j){
                (*paths)[j] = strdup(cmd_list[i].tokens[j + 1]);
            }
            (*paths)[cmd_list[i].len - 1] = NULL;
        }

        // executable files
        else {
            int rc = fork();
            if (rc == 0){

                // checking for output redirection
                int fd = STDOUT_FILENO;
                for (int j = 0; cmd_list[i].tokens[j] != NULL; ++j){
                    if (strcmp(cmd_list[i].tokens[j], ">") == 0){
                        if (fd != STDOUT_FILENO) ExitWithCode(1);
                        if (cmd_list[i].tokens[j+1] == NULL || strcmp(cmd_list[i].tokens[j+1], ">") == 0) ExitWithCode(1);
                        fd = open(cmd_list[i].tokens[j+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                        cmd_list[i].tokens[j] = NULL;
                        cmd_list[i].tokens[j+1] = NULL;
                        for (int k = j+2; cmd_list[i].tokens[k] != NULL; ++k){
                            cmd_list[i].tokens[k-2] = cmd_list[i].tokens[k];
                            cmd_list[i].tokens[k] = NULL;
                        }
                    }
                }
                if (fd != STDOUT_FILENO){
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                }
                
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
                    for (; (*paths)[j] != NULL; ++j){
                        char* tmp = strdup((*paths)[j]);
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

int main(int argc, char* argv[]){

    int to_prompt = 1;

    if (argc > 2){
        fprintf(stderr, "Usage: %s [batch_file]\n", argv[0]);
        exit(1);
    }
    else if (argc == 2){
        int fd = open(argv[1], O_RDONLY);
        if (fd < 0) ExitWithCode(0);
        dup2(fd, STDIN_FILENO);
        close(fd);
        to_prompt = 0;
    }

    // all of the search directories
    char** paths = calloc(sizeof(char*), 3);
    paths[0] = strdup("/bin");
    paths[1] = strdup("/usr/bin");

    int run = 1;

    while (run){
        if (to_prompt == 1) printf("prompt> ");

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
        ExecuteCommand(cmd_list, cmd_list_len, &paths, &run);

        // freeing memory
        free(to_free);
        for (int i = 0; i < cmd_list_len; ++i){
            for (int j = 0; j < cmd_list[i].len; ++j){
                free(cmd_list[i].tokens[j]);
            }
            free(cmd_list[i].tokens);
        }
    }

    // freeing path
    for (int i = 0; paths[i] != NULL; ++i){
        free(paths[i]);
    }
    free(paths);

    ExitWithCode(0);
}