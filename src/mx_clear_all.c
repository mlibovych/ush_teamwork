#include "ush.h"

static void mx_clear_export(t_export *list);

void mx_clear_all(t_shell *m_s) {
	mx_clear_export(m_s->exported);
	mx_clear_export(m_s->variables);
	mx_del_strarr(&m_s->history);
	free(m_s->pwd);
    //free(m_s);
}

static void mx_clear_export(t_export *list) {
    t_export *q = list;
    t_export *tmp = NULL;

    if (!(list) || !list)
        return;
    while (q) {
    	if (q->name)
        	free(q->name);
        if (q->value)
        	free(q->value);
        tmp = q->next;
        free(q);
        q = tmp;
    }
    //free(list);
    list = NULL;
}
