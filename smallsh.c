#include "smallsh.h"

//main insertion point
int main(void)
{
	enum builtIn;  //init built-in cmd's to enum for easier reading later
	int user_exit = 0; //user exit flag

	while(!user_exit)
	{
		//init struct
		struct inp * i;
		i = init(i);
		
		//does input redir std in or out?
		int redir = 0;
		int comm_blank = 0;
		
		//get user input
		size_t size;
		char * line;
		line = getUserInput(line, size);
		
		//expand pid in user input
		expand_pid(line);

		//exctract tokens: cmd, args, in, out, bckgnd
		extract_cmd(line, &comm_blank, i);
		extract_args(line, i);
		extract_rest(i, &redir); //in, out, backgnd
		
		//put in null term array for execv()
		char * args[i->args_arr_size+2];//+2 to add cmd and NULL -> {cmd, arg1, ..., argn, NULL}
		
		//if not a comment or blank line
		if(comm_blank != 1)
		{
			//make the array to pass to execvp()
			make_argv_arr(args, i);	

			//checks if user typed a built in cmd
			int built_in = checkBuiltIn(args);

			//runs what user typed in
			run_cmd(&user_exit, i, built_in, args, &redir);
		}
		//delete allc'd memry
		Delete(i, args, line, comm_blank);
	}

	return 0;
}
