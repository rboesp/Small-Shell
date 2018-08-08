#ifndef SMALLSH_H
#define SMALLSH_H

//debug print prints and flushes
#ifndef DPRINT
#define DPRINT(...) (printf(__VA_ARGS__), fflush(stdout))
#endif

//errror prints perror and exits(1)
#ifndef ERR_PRINT
#define ERR_PRINT(err_msg) (perror(err_msg), exit(1))
#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>
#include <fcntl.h>


//holds the tokens in user input
struct inp
{
	char * cmd;
	char * args[512];
	char * in_redir;
	char * out_redir;
	int bckgnd_yn; //either y or n (1 or 0)
	int args_arr_size; //how many elemetns to print
	int args_arr_len;  //actual number of elements array to delete alloc'd memory
};


//represents the built_in cmd's
enum builtIn {EXIT = 1, STATUS = 2, CD = 3};


//function prototypes
struct inp * init(struct inp * i);
void stripNewline(struct inp * i, int c);
char * getUserInput(char * line, size_t s);
void extract_cmd(char * line, int * comm_blank, struct inp * i);
void extract_args(char * line, struct inp * i);
void extract_rest(struct inp * i, int * redir);
void expand_pid(char * line);
void make_argv_arr(char * args[], struct inp * i);
void redir_stdout(struct inp * i);
void redir_stdin(struct inp * i);
void execute(char ** argv);
void spawnAndExecute(int status, struct inp * i, pid_t c_pid, pid_t pid, char ** args, int*);
void show_status(int success);
void run_it(int *, int built_in, char ** args);
void run_cmd(int *, struct inp * i, int built_in, char * args[], int * redir);
void Delete(struct inp * i, char ** args, char * line, int comm_blank);


#endif 
