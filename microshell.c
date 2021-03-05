#include "microshell.h"

int ft_strlen(char *str) {
	int i = 0;
	while(str[i++]) ;
	return (i);
}

void puterr(char *str) { 
	write(STDERR, str, ft_strlen(str)); 
}

int err_msg(int code, int ret, char *arg) {
    char* strs[4] = {"error: fatal", "error: cd: bad arguments",
        "error: cd: cannot change directory ", "error: cannot execute "};
	puterr(strs[code]);
    if(arg)
	    puterr(arg);
    puterr("\n");
	return (ret);
}

int exec_cd(char **args) {
	if (!args[1] || (args[1] && args[2]))
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
	int previous_fd = -1, nb_args = 0, i = 0, ret = 0, is_semicolon, is_piped, stat_loc;
	int fildes[2];
	pid_t cpid;

	if (argc == 1)
	    return (0);
    while (++i <= argc) {
        if (argv[i] && strcmp(argv[i], ";") != 0 && strcmp(argv[i], "|") != 0)
            nb_args++;
        else if (nb_args != 0) {
            is_semicolon = argv[i] && strcmp(argv[i], ";") == 0;
            is_piped = argv[i] && strcmp(argv[i], "|") == 0;
            argv[i] = NULL;
            if (argv[i - nb_args] && strcmp(argv[i - nb_args], "cd") == 0)
                ret = exec_cd(argv + i - nb_args) == 0;
            else {
                if (is_piped)
                    if (pipe(fildes) == -1)
                        return(exit_syscall(previous_fd, is_piped, fildes, 1));
                cpid = fork();
                if (cpid == -1)
                    return(exit_syscall(previous_fd, is_piped, fildes, 1));
                if (cpid == 0) {
                    if (previous_fd != -1) {
                        if(dup2(previous_fd, STDIN) == -1)
                            exit(exit_syscall(previous_fd, is_piped, fildes, ERR_CHILD));
                    }

                    if (is_piped) {
                        close(fildes[0]);
                        if(dup2(fildes[1], STDOUT) == -1)
                            exit(exit_syscall(previous_fd, is_piped, fildes, ERR_CHILD));
                    }

                    if (execve(argv[i - nb_args], argv + i - nb_args, envp) == -1) {
                        close(previous_fd);
                        exit(err_msg(ERR_EXEC, 1, argv[i - nb_args]));
                    }
                }
                else {
                    if(waitpid(cpid, &stat_loc, 0) == -1)
                        return(exit_syscall(previous_fd, is_piped, fildes, 1));
                    if(WIFEXITED(stat_loc)) {
                        ret = WEXITSTATUS(stat_loc);
                        if (ret == ERR_CHILD) {
                            if(previous_fd != -1)
                                close(previous_fd);
                            if (is_piped) {
                                close(fildes[0]);
                                close(fildes[1]);
                            }
                            exit(1);
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
	return (ret);
}
