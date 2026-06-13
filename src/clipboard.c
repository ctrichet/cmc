#include "clipboard.h"
#include "exitcodes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

static int try_backend(buffer *b, const char *cmd, const char *const argv[])
{
    int pipefd[2];
    if (pipe(pipefd) != 0) return -1;

    pid_t pid = fork();
    if (pid < 0) {
        close(pipefd[0]);
        close(pipefd[1]);
        return -1;
    }

    if (pid == 0) {
        close(pipefd[1]);
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);
        execvp(cmd, (char *const *)argv);
        _exit(127);
    }

    close(pipefd[0]);
    size_t off = 0;
    while (off < b->len) {
        ssize_t n = write(pipefd[1], b->data + off, b->len - off);
        if (n < 0) {
            close(pipefd[1]);
            kill(pid, SIGTERM);
            waitpid(pid, NULL, 0);
            return -1;
        }
        off += (size_t)n;
    }
    close(pipefd[1]);

    int status;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
        return 0;
    return -1;
}

int copy_to_clipboard(buffer *b)
{
    const char *wayland_display = getenv("WAYLAND_DISPLAY");

    if (wayland_display && wayland_display[0]) {
        const char *const wl_argv[] = {"wl-copy", NULL};
        if (try_backend(b, "wl-copy", wl_argv) == 0)
            return 0;
    }

    const char *const xclip_argv[] = {"xclip", "-selection", "clipboard", NULL};
    if (try_backend(b, "xclip", xclip_argv) == 0)
        return 0;

    fprintf(stderr, "cmc: no clipboard backend available (tried wl-copy, xclip)\n");
    return EXIT_CLIPBOARD;
}
