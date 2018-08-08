#include "smallsh.h"


//initalize memory for struct
struct inp * init(struct inp * i)
{
	i = malloc(sizeof(struct inp));
	i->cmd = NULL;
	i->in_redir = NULL;
	i->out_redir = NULL;
	i->args_arr_size = 0;
	i->args_arr_len = 0;
	i->bckgnd_yn = 0;
	return i; 
}


//strips ending newline off args
void stripNewline(struct inp * i, int c) 
{
	int size = strlen(i->args[c]);
	if(i->args[c][size-1] == '\n')
		i->args[c][size-1] = '\0';
}


//exctract the user cmd from input line
void extract_cmd(char * line, int * comm_blank, struct inp * i)
{
	char * cmd = strtok(line, " ");
	i->cmd = malloc(strlen(cmd)+1);
	if(cmd[strlen(cmd)-1] == '\n')
		cmd[strlen(cmd)-1] = '\0';
	strcpy(i->cmd, cmd);
	if(strstr(i->cmd, "#") != NULL) { *comm_blank = 1; }
	if(strcmp(i->cmd, "") == 0) { *comm_blank = 1; }
}


//extracts args with strtok to pass into execvp
void extract_args(char * line, struct inp * i)
{
	char * arg = strtok(NULL, " ");
	while(arg != NULL)
	{	
		i->args[i->args_arr_size] = malloc(strlen(arg)+1);
		strcpy(i->args[i->args_arr_size], arg);
	        stripNewline(i, i->args_arr_size);	
		i->args_arr_size++;
		i->args_arr_len++;
		arg = strtok(NULL, " ");
	}
}


//get in, out redir, check bckgnd or frgnd process
void extract_rest(struct inp * i, int * redir)
{
	int take_off = 0;	//counter of how many spots to chop off i->args after copying in, out, bck
	int x; 
	for(x = 0; x < i->args_arr_size; x++)
	{
		if(strcmp(i->args[x], "<") == 0) 
		{
			i->in_redir = malloc(strlen(i->args[x+1])+1);
			strcpy(i->in_redir, i->args[x+1]); //+1 because we want token after <
			take_off += 2;
			*redir = 1;
		}
		else if(strcmp(i->args[x], ">") == 0)
		{
			i->out_redir = malloc(strlen(i->args[x+1])+1);
			strcpy(i->out_redir, i->args[x+1]); //+1 because we want token after >
			take_off += 2;
			*redir = 1;
		}
		else if(strcmp(i->args[x], "&") == 0)
		{
			i->bckgnd_yn = 1;
			take_off += 1;
		}
	}
	i->args_arr_size -= take_off; //ex -2 = ["echo", "hello", ">", "file1"] --> ["echo", "hello"]
}

//gets the users input from cmd line
char * getUserInput(char * line, size_t s)
{
	s = 100;
	line = calloc(s, sizeof(char));
	printf(": "); fflush(stdout);
	getline(&line, &s, stdin);	//get entire line until \n
	return line;
}


//make array to pass to execvp()
void make_argv_arr(char * args[], struct inp * i)
{
	args[0] = malloc(strlen(i->cmd)+1);   //put cmd in first array spot
	strcpy(args[0], i->cmd);
	int p;
	for(p = 1; p <= i->args_arr_size; p++)
	{
		args[p] = malloc(strlen(i->args[p-1]) + 1);  //put args in middle
		strcpy(args[p], i->args[p-1]);
	}
	args[p] = NULL;   //make last spot NULL 
}


//calls execvp()
void execute(char ** argv)
{
	if(execvp(*argv, argv) < 0)
	{
		ERR_PRINT("Exec failed! --> ");
	}
}

//redirects stdin to file in inp->in_redir filename
void redir_stdout(struct inp * i) 
{
	//from lecture 4.3 slide 12
	int targetFD = open(i->out_redir, O_WRONLY | O_CREAT | O_TRUNC, 0644); //open file after >
	//if (targetFD == -1) { ERR_PRINT("targetFD_out");}
	int result = dup2(targetFD, 1); 
	//if (result == -1) { perror("dup2"); exit(2); }
	//fcntl(targetFD, F_SETFD, FD_CLOEXEC);
	//close(targetFD);
}

//redirects stdin to file in inp->in_redir filename
void redir_stdin(struct inp * i) 
{
	//from lecture 4.3 slide 12
	int targetFD = open(i->in_redir, O_RDONLY); //open file after <
	//if (targetFD == -1) { ERR_PRINT("targetFD_in");}
	int result = dup2(targetFD, 0); 
	//if (result == -1) { perror("dup2"); exit(2); }
	//fcntl(targetFD, F_SETFD, FD_CLOEXEC);
	//close(targetFD);
}

//fork()'s and calls exec(), parent wiats for child 
void spawnAndExecute(int status, struct inp * i, pid_t c_pid, pid_t pid, char ** args, int * redir)
{
	c_pid = fork();  //spawn a new child process
	
//some help from here - https://stackoverflow.com/questions/9084099/re-opening-stdout-and-stdin-file-descriptors-after-closing-them/9084222
	int stdin_copy;
	int stdout_copy;
	if(*redir == 1)		//if user redirected
	{
		if(i->out_redir != NULL)  //stdout
		{
			stdout_copy = dup(1);
			close(1);
			redir_stdout(i);
		}
		if(i->in_redir != NULL) //stdin
		{
			stdin_copy = dup(0);
			close(0);
			redir_stdin(i);
		}
	}
	
	if (c_pid == 0)  //run the non built-in cmd
	{
		//DPRINT("IN CHILD --> REPLACING PROCESS NOW!\n");
		execute(args);
		ERR_PRINT("execvp err ");
	}
	
	else if (c_pid > 0)
	{
		//if bckgnd process
		if(!i->bckgnd_yn)
		{
			
			if((pid = waitpid(c_pid, &status, 0)) < 0) 
			{
				ERR_PRINT("parent wait err ");
			}
			
		}
		else
		{	
			DPRINT("Background PID: %i\n", c_pid);
			
			if((pid = waitpid(c_pid, &status, WNOHANG)) < 0) 
			{
				ERR_PRINT("parent wait err ");
			}
			
		}
	
		//DPRINT("BACK IN PARENT --> finished\n");  


		//restore stdin stdout file descriptor's here
		if(*redir == 1)
		{
			if(i->in_redir != NULL)
			{
				dup2(stdin_copy, 0);
				close(stdin_copy);
			}
			if(i->out_redir != NULL)
			{
				dup2(stdout_copy, 1);
				close(stdout_copy);
			}
		}
	}
	
	else
	{
		ERR_PRINT("fork failed");
	}	
}


//check if user typed-in  built in cmd
int checkBuiltIn(char ** args)
{
	if(strcmp(args[0], "exit") == 0)
		return 1;
	else if(strcmp(args[0], "status") == 0)
		return 2;
	else if(strcmp(args[0], "cd") == 0)
		return 3;
	else
		return 0;	
}



//showss status of program
void show_status(int success)
{
	if(WIFEXITED(success))
	{          
		//from lecture 1.3 27:00
        printf("exit value %i\n", WEXITSTATUS(success));
    }	 
}



//runs built-in cmds
void run_it(int * user_exit, int built_in, char ** args)
{
	if(built_in == EXIT) //1 = exit, 2 = status, 3 = whatever third is
	{
		//DPRINT("EXITING\n"); 
		*user_exit = 1;
	}	
	else if(built_in == STATUS)
	{
		//look at 27:34 in lecture 1.3 for help
		int success = 0;
		show_status(success);
	}
	else if(built_in == CD)
	{
		if(args[1] == NULL) 		//if cd typed alone
			chdir(getenv("HOME"));
		else 
			chdir(args[1]);		//else goto dir typed by user
		char dir[PATH_MAX];
		getcwd(dir, sizeof(dir)); 
		printf("%s\n", dir); 
	}
}


//runs what the user typed in as their cmd
void run_cmd(int * user_exit, struct inp * i, int built_in, char * args[], int * redir)
{	
	//0 = not built-in cmd
	if(!built_in) 
	{
		//fork and pass input to execvp()
		pid_t c_pid;
		pid_t pid;
		int status;
		spawnAndExecute(status, i, c_pid, pid, args, redir);
	}
	//anything but 0 = built-in
	else 
		run_it(user_exit, built_in, args);
}

//expands $$ into pid, ex: moop$$.txt --> moop44778.txt
void expand_pid(char * line)
{
	pid_t pid = getpid();
	char buff[50]; 
	sprintf(buff, "%d", pid);
	char * pid_pos;
	pid_pos = strstr(line, "$$");
	
	//got help with this piazza pid_post https://piazza.com/class/jc14n3c2sfz1wg?cid=430
	if(pid_pos)
	{
		strcpy(pid_pos + strlen(buff), pid_pos + 2); 
		strncpy(pid_pos, buff, strlen(buff));	
	}
}


//free alloc'd memory
void Delete(struct inp * i, char ** args, char * line, int comm_blank)
{
	int x;
	for(x = 0; x < i->args_arr_len; x++)
	{ 
		if(i->args[x] != NULL) 
			free(i->args[x]); 
	}
	
	if(comm_blank != 1) 
	{
		for(x = 0; x < i->args_arr_size+2; x++)
		{
			free(args[x]);
		}
	}
	
	if(i->cmd != NULL) { free(i->cmd); }
	if(i->in_redir != NULL) { free(i->in_redir); }
	if(i->out_redir != NULL) { free(i->out_redir); }
	if(i != NULL) { free(i); }
	if(line != NULL) { free(line); }
}
