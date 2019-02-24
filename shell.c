#include <stdio.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "call_execve.c"
#include "shell-utils.h"

#define INPUT_BUFFER_SIZE 2048
#define NB_MAX_TOKENS 512

sigjmp_buf pointeur_retour;
int pid_child;

void TSTPhandler(int sig);
void TSTPhandler2(int sig);
void pipe_custom(char ** tube_tokens, char ** tokens);
void redirection(int side, char * file, char ** tokens);
void ecrire_dans_fichier(int pid_ecrivain);

void vider_buffer(void)
{
	int c;
	do
	{
		c = getchar();
	}while(c != '\n' && c != EOF);
}

/**
 * Handler gérant SIGINT
*/
void INThandler(int sig)
{
	signal(sig, INThandler);

	printf("\nVoulez-vous quitter le processus ? [o/n] : ");
	char c = getchar();
	
	if (c == 'o' || c == 'O') 
	{
		exit(0);
	}
	else
	{
		vider_buffer();
		signal(SIGINT, INThandler);
		siglongjmp(pointeur_retour, 1);
	}
}

void TSTPhandler2(int sig)
{
	signal(SIGTSTP, TSTPhandler);
	kill(pid_child,SIGCONT);	
}

/**
 * Handler gérant SIGTSTP
*/
void TSTPhandler(int sig)
{		
	signal(SIGTSTP, TSTPhandler2);
	siglongjmp(pointeur_retour, 2);
}


int main() {
	char line[INPUT_BUFFER_SIZE+1];
	int nb_tokens = 0;
	
	char* tokens[NB_MAX_TOKENS+1];	
	char* data;

	signal(SIGTSTP, TSTPhandler);
	signal(SIGINT, INThandler);

	system("clear");
		
	while(1)
	{		
		sigsetjmp(pointeur_retour, 2);
		printf("prompt $");

		data = fgets(line, INPUT_BUFFER_SIZE, stdin);

		if (data == NULL) 
		{
			/* Erreur ou fin de fichier : on quitte tout de suite */
			if (errno) 
			{
				perror("fgets");
			} 
			else 
			{
				fprintf(stderr, "EOF: exiting\n");
			}
			exit(errno);
		}

		if (strlen(data) == INPUT_BUFFER_SIZE-1) 
		{
			fprintf(stderr, "Input line too long: exiting\n");
			exit(1);
		}

		nb_tokens = split_tokens(tokens, data, NB_MAX_TOKENS);

		if (nb_tokens == NB_MAX_TOKENS) 
		{
			fprintf(stderr, "Too many tokens: exiting\n");
			exit(1);
		}

		if(strcmp(data, "exit") == 0)
		{
			exit(0);
		}			

		switch (fork())
		{
			case 0:
			;
				pid_child = getpid();

				char * redirection1 = trouve_redirection(tokens, ">");
				char * redirection2 = trouve_redirection(tokens, "<");
				
				if(redirection1 != NULL)
				{
					redirection(STDOUT_FILENO, redirection1, tokens);
					break;
				}

				else if(redirection2 != NULL)
				{
					redirection(STDIN_FILENO, redirection2, tokens);
					break;
				}	

				char ** tube_tokens = trouve_tube(tokens, "|");		
				if(tube_tokens != NULL)
				{ 
					pipe_custom(tube_tokens, tokens);
				}				
								
				else
				{
					call_execve(tokens);
				}			
				break;
			
			default:
				wait(NULL);
				break;
		}	
	}

}

void pipe_custom(char ** tube_tokens, char ** tokens)
{
	int pipe_fd[2];
	if(pipe(pipe_fd) == -1)
	{
		perror("pipe");
		exit(errno);
	}
	
	switch (fork())
	{
		case 0:			
			close(pipe_fd[1]);
			dup2(pipe_fd[0], STDIN_FILENO);
			call_execve(tube_tokens);
			exit(0);
			break;

		default:
			close(pipe_fd[0]);
			dup2(pipe_fd[1], STDOUT_FILENO);
			call_execve(tokens);
			wait(NULL);
			break;
	}
}

void redirection(int side, char * file, char ** tokens)
{
	char ** tube_tokens = trouve_tube(tokens, "|");	
	int file_fd = open(file, O_RDWR);	
	switch (fork())
	{
		case 0:
			dup2(file_fd, side);
			
			if (tube_tokens != NULL) 
			{
				pipe_custom(tube_tokens, tokens);
			}
			else
			{
				call_execve(tokens);
			}			
			exit(0);	
			break;

		default:
			close(file_fd);
			wait(NULL);
			break;
	}

}


void ecrire_dans_fichier(int pid_ecrivain)
{
	sleep(3);
	int fd;
	fd = open("txt", O_WRONLY);
	char texte[128];
	sprintf(texte, "Je suis : %d et j'écris dans le fichier\n", pid_ecrivain);
	printf("%s", texte);
	write(fd, texte, sizeof(texte));
	close(fd);
}
