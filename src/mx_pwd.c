#include "ush.h"

static int count_args(char **args, int n_options);
static void fill_options(int n_options, pwd_t *pwd_options, char **args);

int mx_pwd(t_shell *m_s, t_process *p) {
	char *dir;
	pwd_t pwd_options = {0, 0};
	int n_options = mx_count_options(p->argv, "LP", "pwd", " [-LP]");
	int n_args = count_args(p->argv, n_options);
	mx_set_variable(m_s->variables, "?", "1");

	fill_options(n_options, &pwd_options, p->argv);
	if (n_options <  0) return 1;
	if(n_args > 1) {
		mx_printerr("ush: pwd: too many arguments\n");
		return 1;
	}
	dir = getcwd(NULL, 20000);
	if(dir != NULL) {
		mx_set_variable(m_s->variables, "?", "0");
		if (pwd_options.P < 0) {
			printf("%s\n", m_s->pwd);
		} 
		else {
			printf("%s\n", dir);
		}
		free(dir);
	}
	else {
		perror("ush: pwd");
	}
    return 1;
}

static void fill_options(int n_options, pwd_t *pwd_options, char ** args) {
	int L_index = -1;
	int P_index = -1;

	for(int i = n_options; i > 0; i --) {
		for (int j = mx_strlen(args[i]); j > 0; j--) {
			if (args[i][j] == 'L' && P_index < 0) {
				L_index = 1;
			}
			if (args[i][j] == 'P' && L_index < 0) {
				P_index = 1;
			}
		}
	}
	pwd_options->L = L_index;
	pwd_options->P = P_index;
}

static int count_args(char **args, int n_options) {
	int n_args = 0;

	for (int i = n_options; args[i] != NULL; i++) {
		n_args++;
	}
	return n_args;
}
