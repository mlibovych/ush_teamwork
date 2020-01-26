#include "ush.h"

static char *check_path(char **arr, char *command);
static char *get_error(char **name, char *command);
static void print_error(char *command, char *error);

int mx_launch_process(t_shell *m_s, t_process *p, char *path, char **env) {

//    pid_t pgid = m_s->jobs[job_id]->pgid;
    p->status = STATUS_RUNNING;

    int shell_is_interactive = isatty(STDIN_FILENO);  //!!
    m_s->history_index = 0;
    if (shell_is_interactive) {
        p->pid = getpid();
        if (p->pgid == 0)
            p->pgid = p->pid;
        setpgid(p->pid, p->pgid);   //Процесс может присоединиться к группе или создать новую группу процессов
        if (p->foreground)
            tcsetpgrp(STDIN_FILENO, p->pgid);
    }
    signal(SIGINT, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);
    signal(SIGTTIN, SIG_DFL);
    signal(SIGTTOU, SIG_DFL);
    signal(SIGCHLD, SIG_DFL);
    p->pid = getpid();
    if (p->pgid > 0) {
        setpgid(0, p->pgid);
    } else {
        p->pgid = p->pid;
        setpgid(0, p->pgid);
    }
    if (p->infile != STDIN_FILENO) {
        dup2(p->infile, STDIN_FILENO);
        close(p->infile);
    }
    if (p->outfile != STDOUT_FILENO) {
        dup2(p->outfile, STDOUT_FILENO);
        close(p->outfile);
    }
    if (p->errfile != STDERR_FILENO) {
        dup2(p->errfile, STDERR_FILENO);
        close(p->errfile);
    }
    char **arr = mx_strsplit(path, ':');
    char *command = p->argv[0];
    path  = check_path(arr, command);
    char *error = get_error(&path, command); 
    if (execve(path, p->argv, env) < 0) {
        print_error(command, error);
        _exit(EXIT_FAILURE);
    }
    return (p->exit_code);
}


static char *check_path(char **arr, char *command) {
    int i = 0;
    char *name = NULL;
    int flag = 0;

    while (arr[i] != NULL && !flag) {
        DIR *dptr  = opendir(arr[i]);
        if (dptr != NULL) {
            struct dirent  *ds;
            while ((ds = readdir(dptr)) != 0) {
                if (strcmp(ds->d_name, command) == 0 && command[0] != '.') {
                    flag++;
                    name = mx_strjoin(arr[i], "/");
                    name = mx_strjoin(name, command);
                    break;
                }
            }
        closedir(dptr);
        }
        i++;
    }
    return name;
}

static char *get_error(char **name, char *command) {
    char *error = NULL;

    if (strstr(command, "/")) {
        *name = command;
        struct stat buff;
        if (lstat(*name, &buff) < 0) {
            error = strdup(": No such file or directory\n");
        }
        else {
            if (mx_get_type(buff) == 'd') {
                error = strdup(": is a directory\n");
            }
        }
    } 
    else
        error = strdup(": command not found\n");
    return error;
}

static void print_error(char *command, char *error) {
    mx_printerr("ush: ");
    if (error) {
        mx_printerr(command);
        mx_printerr(error);
    }
    else
        perror(command);
}
