#include "ush.h"
static void buildin_std_exec(t_shell *m_s, int (*builtin_functions[])
                             (t_shell *m_s, t_process *p), t_process *p);

static void buildin_fork(t_shell *m_s, int job_id, int (*builtin_functions[])
                         (t_shell *m_s, t_process *p), t_process *p);

int mx_launch_builtin(t_shell *m_s, t_process *p, int job_id) {
    int (*builtin_functions[])(t_shell *m_s, t_process *p) =
         {&mx_env, &mx_export, &mx_unset, &mx_echo, &mx_jobs, &mx_fg, &mx_bg,
         &mx_cd, &mx_pwd, &mx_which, &mx_exit, &mx_set, NULL};

    p->status = MX_STATUS_RUNNING;
    if (p->type == 4 || p->type == 5 || p->type == 6) {
        if(!p->pipe && p->foregrd && m_s->jobs[job_id]->first_pr->next == NULL)
            mx_remove_job_from_panel(m_s, job_id);  // not destroy!!!
    }
    if (p->pipe || !p->foregrd) {  // if pipe or in foregrd -> fork
        buildin_fork(m_s, job_id, builtin_functions, p);
    }
    else
        buildin_std_exec(m_s, builtin_functions, p);
//    if (p->type == 4 || p->type == 5 || p->type == 6) {
//        if(!p->pipe && p->foregrd && m_s->jobs[job_id]->first_pr->next == NULL)
//            mx_destroy_jobs(m_s, job_id);
//    }
    return p->exit_code;
}

static void buildin_fork(t_shell *m_s, int job_id, int (*builtin_functions[])
        (t_shell *m_s, t_process *p), t_process *p) {
    int shell_is_interactive = isatty(STDIN_FILENO);
    pid_t child_pid = fork();

    p->pid = child_pid;
    if (child_pid < 0) {
        perror("error fork");
        exit(1);
    }
    else if (child_pid == 0) {
        if (shell_is_interactive)
            mx_pgid(m_s, job_id, child_pid);
        mx_dup_fd(p); // dup to STD 0 1 2
        p->exit_code = builtin_functions[p->type](m_s, p);
        exit(p->exit_code);  // ?
    }
    else {
        if (shell_is_interactive) {
            if (m_s->jobs[job_id]->pgid == 0)
                m_s->jobs[job_id]->pgid = child_pid;
            setpgid (child_pid, m_s->jobs[job_id]->pgid);
        }
    }
}

static void buildin_std_exec(t_shell *m_s, int (*builtin_functions[])
                             (t_shell *m_s, t_process *p), t_process *p) {
    int defoult;

    if (p->output_path) {
        defoult = dup(1);
        if (p->outfile != STDOUT_FILENO) {
            lseek(p->outfile, 0, SEEK_END);
            dup2(p->outfile, STDOUT_FILENO);
            close(p->outfile);
        }
        if (p->infile != STDIN_FILENO) {
            dup2(p->infile, STDIN_FILENO);
            close(p->infile);
        }
    }
    p->exit_code = builtin_functions[p->type](m_s, p);
    p->status = MX_STATUS_DONE;
    if (p->output_path) {
        if (p->outfile != STDOUT_FILENO) {
            dup2(defoult, 1);
            close(defoult);
        }
    }
}


void mx_pgid(t_shell *m_s, int job_id, int child_pid) {
    if (m_s->jobs[job_id]->pgid == 0)
        m_s->jobs[job_id]->pgid = child_pid;
    setpgid(child_pid, m_s->jobs[job_id]->pgid);
    if (m_s->jobs[job_id]->foregrd)
        tcsetpgrp(STDIN_FILENO, m_s->jobs[job_id]->pgid);
    signal(SIGINT, MX_SIG_DFL);
    signal(SIGQUIT, MX_SIG_DFL);
    signal(SIGTSTP, MX_SIG_DFL);
    signal(SIGTTIN, MX_SIG_DFL);
    signal(SIGTTOU, MX_SIG_DFL);
    signal(SIGPIPE, mx_sig_h);
}