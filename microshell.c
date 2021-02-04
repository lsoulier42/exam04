#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#define STDIN 0
#define STDOUT 1
#define STDERR 2

enum e_error {
	ERR_SYS,
	ERR_CD_ARG,
	ERR_CD_DIR,
	ERR_EXE
};

char **g_envp = NULL;
int g_signum = 0;

int ft_strlen(char *str)
{
	int i = 0;
	while (str[i++])
		;
	return (i);
}

void ft_putstr_fd(char *str, int fd)
{
	write(fd, str, ft_strlen(str));
}

typedef struct s_cmd {
	char *path;
	char **argv;
}				t_cmd;

int err_msg(int code, int ret, char *arg)
{
	if (code == ERR_SYS)
		ft_putstr_fd("error: fatal\n", STDERR);
	else if (code == ERR_CD_ARG)
		ft_putstr_fd("error: cd: bad arguments\n", STDERR);
	else if (code == ERR_CD_DIR)
	{
		ft_putstr_fd("error: cd: cannot change directory to ", STDERR);
		ft_putstr_fd(arg, STDERR);
		ft_putstr_fd("\n", STDERR);
	}
	else if (code == ERR_EXE)
	{
		ft_putstr_fd("error: cannot execute ", STDERR);
		ft_putstr_fd(arg, STDERR);
		ft_putstr_fd("\n", STDERR);
	}
	return (ret);
}

int exec_cd(t_cmd cmd)
{
	if (!cmd.argv[1] || cmd.argv[2])
		return (err_msg(ERR_CD_ARG, 0, NULL));
	if (chdir(cmd.argv[1]) == -1)
		return (err_msg(ERR_CD_DIR, 0, cmd.argv[1]));
	return (1);	
}

int main (int argc, char **argv, char **envp)
{
	int i = 0;
	g_envp = envp;
	char **arguments;
	int nb_arguments = 0;
	int j;
	t_cmd current;
	pid_t cpid;
	int fildes[2];
	int previous_fd = -1;
	int is_piped;

	while (++i <= argc)
	{
		if (argv[i] && strcmp(argv[i], "|") != 0 && strcmp(argv[i], ";") != 0)
			nb_arguments++;
		else
		{
			j = -1;
			current.path = argv[i - nb_arguments];
			current.argv = malloc(sizeof(char*) * (nb_arguments + 1));
			if (!(current.argv))
				return (err_msg(ERR_SYS, 1, NULL));
			while (++j < nb_arguments)
				current.argv[j] = argv[(i - nb_arguments) + j];
			current.argv[j] = NULL;			
			nb_arguments = 0;
			if (current.argv[0] && strcmp(current.argv[0], "cd") != 0)
			{
				is_piped = 0;
				if (argv[i] && strcmp(argv[i], "|") == 0)
				{
					is_piped = 1;
					if (pipe(fildes) == -1)
						return (err_msg(ERR_SYS, 1, NULL));
				}
				cpid = fork();
				if (cpid == -1)
					return (err_msg(ERR_SYS, 1, NULL));
				else if (cpid != 0)
				{
					if (previous_fd != -1)
					{
						close(STDIN);
						dup(previous_fd);
						close(previous_fd);
					}
					if (is_piped)
					{
						close(fildes[0]);
						close(STDOUT);
						dup(fildes[1]);
						close(fildes[1]);
					}
					if(execve(current.path, current.argv, envp) == -1)
						return (err_msg(ERR_EXE, 1, current.path));
				}
				else
				{
					if (is_piped)
					{
						close(fildes[1]);
						previous_fd = fildes[0];
					}
					else if (previous_fd != -1)
						close(previous_fd);
					waitpid(cpid, NULL, 0);
				}
			}
			else if (current.argv[0])
				if(!exec_cd(current))
					return (1);
			free(current.argv);
		}
	}
	return (0);
}
