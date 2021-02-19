#include "microshell.h"

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

int exit_syscall(int previous_fd, int is_piped, int fildes[2], int ret)
{
	if (previous_fd != -1)
		close(previous_fd);
	if (is_piped)
	{
		close(fildes[0]);
		close(fildes[1]);
	}
	return(err_msg(ERR_SYS, ret, NULL));
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

	if (argc >= 2) 
	{
		while (++i <= argc) 
		{
			if (argv[i] && strcmp(argv[i], ";") != 0 && strcmp(argv[i], "|") != 0)
				nb_args++;
			else if (nb_args != 0) 
			{
				if (argv[i - nb_args] && strcmp(argv[i - nb_args], "cd") == 0) 
					ret = exec_cd(argv + i - nb_args) == 0;
				else if (nb_args) 

				{
					is_semicolon = argv[i] && strcmp(argv[i], ";") == 0;
					is_piped = argv[i] && strcmp(argv[i], "|") == 0;
					if (is_piped)
						if (pipe(fildes) == -1)
							return(exit_syscall(previous_fd, is_piped, fildes, 1));
					argv[i] = NULL;
					cpid = fork();
					if (cpid == -1)
						return(exit_syscall(previous_fd, is_piped, fildes, 1));

					if (cpid == 0) 
					{
						if (previous_fd != -1) {
							if(dup2(previous_fd, STDIN) == -1)
								exit(exit_syscall(previous_fd, is_piped, fildes, 75));
							close(previous_fd);
						}

						if (is_piped) {
							close(fildes[0]);
							if(dup2(fildes[1], STDOUT) == -1)
								exit(exit_syscall(previous_fd, is_piped, fildes, 75));
							//avec close(fildes[1]) ca ne passe pas non plus
						}

						if (execve(argv[i - nb_args], argv + i - nb_args, envp) == -1)
							exit(err_msg(ERR_EXEC, 1, argv[i - nb_args]));
					}
					else 
					{
						if(waitpid(cpid, &stat_loc, 0) == -1)
							return(exit_syscall(previous_fd, is_piped, fildes, 1));
						if(WIFEXITED(stat_loc))
						{
							ret = WEXITSTATUS(stat_loc);
							if (ret == 75)
							{
								if(previous_fd != -1)
									close(previous_fd);
								if (is_piped)
								{
									close(fildes[0]);
									close(fildes[1]);
								}
								return (1);
							}
						}
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
