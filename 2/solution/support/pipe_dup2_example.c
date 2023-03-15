#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int pipe_fd[2]; int out;
    pid_t pid;

    if (pipe(pipe_fd) < 0) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    if ((pid = fork()) < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {  // child process
        close(pipe_fd[0]);  // close the read end of the pipe
        dup2(pipe_fd[1], STDOUT_FILENO);  // redirect stdout to the write end of the pipe
        char *args[] = {"yes", "bigdata", NULL};
        execvp(args[0], args);
        perror("execlp");
        exit(EXIT_FAILURE);
    }
    out = pipe_fd[0];
    close(pipe_fd[1]);  // close the write end of the pipe

    if (pipe(pipe_fd) < 0) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    if ((pid = fork()) < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {  // child process
        close(pipe_fd[0]);
        dup2(out, STDIN_FILENO);  // redirect stdin to the read end of the pipe
        dup2(pipe_fd[1], STDOUT_FILENO);
        char *args[] = {"head", "-n", "10", NULL};
        execvp(args[0], args);
        perror("execlp");
        exit(EXIT_FAILURE);
    }
    close(out);
    close(pipe_fd[1]);
    out = pipe_fd[0];

    if (pipe(pipe_fd) < 0) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    if ((pid = fork()) < 0)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0)
    {
        close(pipe_fd[0]);
        dup2(out, STDIN_FILENO);
        char *args[] = {"wc", "-l", NULL};
        execvp(args[0], args);
        perror("execlp");
        exit(EXIT_FAILURE);
    }
    close(pipe_fd[1]);
    close(out);  // close the read end of the pipe

    // parent process waits for the child processes to finish
    waitpid(-1, NULL, 0);
    waitpid(-1, NULL, 0);
    waitpid(-1, NULL, 0);

    return 0;
}