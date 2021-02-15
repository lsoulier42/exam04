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

int main(int argc, char **argv, char **envp) {
	int previous_fd = -1;
	int fildes[2];
	int nb_args = 0;
	pid_t cpid;
	int i = 0;
	int is_piped = 0;
	int stat_loc;
	int ret = 0;
	int is_semicolon = 0;

	if (argc >= 2) {
		while (++i <= argc) 
		{
			if (argv[i] && strcmp(argv[i], ";") != 0 && strcmp(argv[i], "|") != 0)
				nb_args++;
			else if (nb_args != 0) 
			{
				if (argv[i - nb_args] && strcmp(argv[i - nb_args], "cd") == 0) 
					exec_cd(argv + i - nb_args);
<<<<<<< HEAD:second/main.c
				else if (nb_args) 
=======
				else 
>>>>>>> 5dd6a6d0ce7458abaca1e59a947ce8e32f9422b3:second/microshell.c
				{
					is_semicolon = argv[i] && strcmp(argv[i], ";") == 0;
					is_piped = 0;
					if (argv[i] && strcmp(argv[i], "|") == 0) 
					{
						is_piped = 1;
						if (pipe(fildes) == -1)
							exit(err_msg(ERR_SYS, 1, NULL));
					}
					argv[i] = NULL;
					cpid = fork();
					if (cpid == -1)
						exit(err_msg(ERR_SYS, 1, NULL));

					if (cpid == 0) 
					{
						if (previous_fd != -1) {
							if(dup2(previous_fd, STDIN) == -1)
							exit(err_msg(ERR_SYS, 1, NULL));
						}

						if (is_piped) {
							close(fildes[0]);
							if(dup2(fildes[1], STDOUT) == -1)
								exit(err_msg(ERR_SYS, 1, NULL));
						}

						if (execve(argv[i - nb_args], argv + i - nb_args, envp) == -1)
<<<<<<< HEAD:second/main.c
							exit(err_msg(ERR_EXEC, 1, argv[i - nb_args]));
						exit(0);
=======
							err_msg(ERR_EXEC, 1, argv[i - nb_args]);
						exit(1);
>>>>>>> 5dd6a6d0ce7458abaca1e59a947ce8e32f9422b3:second/microshell.c
					}
					else 
					{
						waitpid(cpid, &stat_loc, 0);
						if (previous_fd != -1)
							close(previous_fd);
						if (is_piped) {
							close(fildes[1]);
							previous_fd = fildes[0];
						}
					}	
					if(is_semicolon)
						previous_fd = -1;
				}
				nb_args = 0;
			}
		}
	}
	return (ret);
}
