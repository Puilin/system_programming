#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>

void fatal(char *msg) {
    perror(msg);
    exit(1);
}

int main(void){
    while (1) {
        int fd;
        char *cwd;
        char cmd[1024];

        int amp = 0, in_red = 0, out_red = 0,
        out_override = 0, append = 0, here = 0, piping = 0, noclob = 0; // flags

        char *parsed_cmd[16]; // parsed commands buffer
        char *parsed; // temporary buffer for parsing

        cwd = getcwd(NULL, 1024);
        write(STDOUT_FILENO, "yunseok@termproj:", 18);
        write(STDOUT_FILENO, cwd, strlen(cwd));
        write(STDOUT_FILENO, "$ ", 3);
        //fflush(stdout);
        int num_read = read(0, cmd, 1024);
        cmd[num_read-1] = '\0';

        parsed = strtok(cmd, " ");
        int i = 0;
        while (parsed != NULL && i < 16) {
            parsed_cmd[i] = (char *)malloc(sizeof(char) * strlen(parsed));
            parsed_cmd[i++] = parsed;
            parsed = strtok(NULL, " ");
        }
        if (i == 16)
            fatal("More than 15 words cannot accepted");
        parsed_cmd[i] = NULL;

        int j, idx = -1;
        char c;
        for (j = 0; parsed_cmd[j]; j++) {
            char *temp = parsed_cmd[j];
            if (strcmp(temp, "noclobber") == 0) {
                noclob = 1;
            }
            if (strcmp(temp, ">|") == 0) {
                out_override = 1;
                idx = j;
            }
            else if (strcmp(temp, ">") == 0) {
                out_red = 1;
                idx = j;
            }
            else if (strcmp(temp, ">>") == 0) {
                append = 1;
                idx = j;
            }
            else if (strcmp(temp, "<") == 0) {
                in_red = 1;
                idx = j;
            }
            else if (strcmp(temp, "<<") == 0) {
                here = 1;
                idx = j;
            }
            if (strcmp(temp, "|") == 0)
                piping = 1;
                idx = j;
            if (temp[strlen(parsed_cmd[j])-1] == '&' || strcmp(temp, "&") == 0)
                amp = 1;
        }
            

        pid_t pid = fork();
        if (pid == 0) {
            /* output redirection */
            if (out_red == 1) {
                if((fd = open(parsed_cmd[idx], O_CREAT | O_WRONLY | O_TRUNC, 0644)) == -1){
                    perror("creat");
                    exit(1);
                }
                dup2(fd, 1);
                close(fd);
                parsed_cmd[idx-1] = NULL;
                execvp(parsed_cmd[0], parsed_cmd);
            }
            /* >> */
            if (append == 1) {
                if((fd = open(parsed_cmd[idx], O_CREAT | O_WRONLY | O_APPEND, 0644)) == -1){
                    perror("creat");
                    exit(1);
                }
                dup2(fd, 1);
                close(fd);
                parsed_cmd[idx-1] = NULL;
                execvp(parsed_cmd[0], parsed_cmd);
            }
            execvp(parsed_cmd[0], parsed_cmd);
        } else if (pid > 0) {
            waitpid(pid, NULL, 0);
        }
    }
    
    return 0;
}