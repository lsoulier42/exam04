#ifndef MAIN_H
# define MAIN_H
# include <unistd.h>
# include <stdlib.h>
# include <signal.h>
# include <sys/wait.h>
# include <string.h>

enum e_fd {
	STDIN,
	STDOUT,
	STDERR
};

enum e_err {
	ERR_SYS,
	ERR_CD_ARG,
	ERR_CD_DIR,
	ERR_EXEC
};

int ft_strlen(char *str);
void puterr(char *str);
int err_msg(int code, int ret, char *arg);
#endif
