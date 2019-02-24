#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include "shell-utils.h"

void call_execve(char **cmd1)
{
	int i = execvp(cmd1[0], cmd1);
	if(i < 0) 
	{
		printf("%s: %s\n", cmd1[0], "command not found");
		perror("Exec cmd broken");
		exit(errno);		
	}	
}




	