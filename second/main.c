#include "main.h"
#include <stdio.h>

int ft_strlen(char *str) {
	int i = 0;

	while(str[i++])
		;
	return (i);
}

void puterr(char *str) { 
	write(STDERR, str, ft_strlen(str)); 
}

int err_msg(int code, int ret, char *arg) {
	if (code == ERR_SYS)
		puterr("error: fatal\n");
	else if (code == ERR_CD_ARG)
		puterr("error: cd: bad arguments\n");
	else if (code == ERR_CD_DIR) {
		puterr("error: cd: cannot change directory ");
		puterr(arg);
		puterr("\n");
	} else {
		puterr("error: cannot execute ");
		puterr(arg);
		puterr("\n");
	}
	return (ret);
}

int exec_cd(char **args) {
	if (!(args[1]) || args[2])
		return (err_msg(ERR_CD_ARG, 0, NULL));
	if (chdir(args[1]) == -1)
		return (err_msg(ERR_CD_DIR, 0, args[1]));
	return (1);	
}

char **create_args(char **argv, int i, int nb_args) {
	char **args;
	int j = -1;

	if (!(args = (char**)malloc(sizeof(char*) * (nb_args + 1)))) {
		err_msg(ERR_SYS, 0, NULL);
		return (NULL);
	}
	while(++j < nb_args)
		args[j] = argv[(i - nb_args) + j];
	args[j] = NULL;
	return (args);
}

int main(int argc, char **argv, char **envp) {
	int previous_fd = -1;
	int fildes[2];
	char **args;
	int nb_args = 0;
	pid_t cpid;
	int i = 0;
	int is_piped = 0;
	int stat_loc;
	int ret;

	if (argc >= 2) {
		while (++i <= argc) 
		{
			if (argv[i] && strcmp(argv[i], ";") != 0 && strcmp(argv[i], "|") != 0)
				nb_args++;
			else 
			{
				if (!(args = create_args(argv, i, nb_args)))
					return (1);	
				nb_args = 0;
				if (args[0] && strcmp(args[0], "cd") == 0) 
					exec_cd(args);
				else 
				{
					if (argv[i] && strcmp(argv[i], "|") == 0) 
					{
						is_piped = 1;
						if (pipe(fildes) == -1)
							return (err_msg(ERR_SYS, 1, NULL));
					}

					cpid = fork();
					if (cpid == -1)
						return (err_msg(ERR_SYS, 1, NULL));

					if (cpid == 0) 
					{
						if (previous_fd != -1) {
							close(STDIN);
							if (dup(previous_fd) == -1)
								exit(err_msg(ERR_SYS, -1, NULL));
							close(previous_fd);
						}

						if (is_piped) {
							close(fildes[0]);
							close(STDOUT);
							if(dup(fildes[1]) == -1)
								exit(err_msg(ERR_SYS, -1, NULL));
						}

						if (execve(args[0], args, envp) == -1)
							err_msg(ERR_EXEC, 1, args[0]);
						close(fildes[1]);
						free(args);
						exit(1);
					}
					else 
					{
						if (previous_fd != -1)
							close(previous_fd);
						if (is_piped) {
							close(fildes[1]);
							previous_fd = fildes[0];
						}
						if (!(argv[i]) || (argv[i] && strcmp(argv[i], ";") == 0)) {
							if(waitpid(cpid, &stat_loc, 0) == -1)
							{
								printf("waitpid\n");
								return (err_msg(ERR_SYS, 1, NULL));
							}
							if (WIFEXITED(stat_loc)) {
								ret = WEXITSTATUS(stat_loc);
								if (ret == -1)
									return (1);
							}
						}
					}	
					if (argv[i] && strcmp(argv[i], ";") == 0) {
						previous_fd = -1;
						is_piped = 0;
					}
				}
				free(args);
			}
		}
	}
	return (ret);
}

