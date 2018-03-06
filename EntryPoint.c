#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define CMDMAX 80
#define HISTMAX 3
#define TOKENSMAX 50
#define TOKENLEN 1000
#define MAX_LEN 128
#define ALLOWPIPELINING 0

void clearAndResetStringArray(char* arr[], int arrMax, int strMax)
{
	int i = 0;
	for(i = 0; i<arrMax; i++)
	{
		if(arr[i] == NULL)
		{
			arr[i] = (char*)malloc(strMax*sizeof(char));
		}
	}
}

void test(){
}


void tokenize(char* input, char* argv[])
{
	/*
		parse input into tokens with last string
		as NULL. Some limitations apply. Max number
		of tokens is 50, and max token length is
		1000
	*/
	//reset argv array
	clearAndResetStringArray(argv, TOKENSMAX, TOKENLEN);

	//add extra byte for null character
	char* tempStorage = (char*)malloc(strlen(input)*sizeof(char)+1);
	strcpy(tempStorage, input);

	char* tokens = strtok(input, " \n\t");
	int tokenIndex = 0;


	while(tokens!=NULL)
	{
		strcpy(argv[tokenIndex++],tokens);
		tokens = strtok(NULL," \n\t");
	}

	if(argv[tokenIndex] != NULL)
	{
		free(argv[tokenIndex]);
		argv[tokenIndex] = NULL;
	}
	strcpy(input, tempStorage);
	free(tempStorage);
}

void printWelcomeMessage()
{
	//Load ascii logo
	char *filename = "ASCII.txt";
    FILE *fptr = NULL;
 
    if((fptr = fopen(filename,"r")) == NULL)
    {
        fprintf(stderr,"error opening %s\n",filename);
        exit(1);
    }

    //print logo
    char read_string[MAX_LEN];
 
    while(fgets(read_string,sizeof(read_string),fptr) != NULL)
        printf("%s",read_string);
 
 	//print welcome text
	char* welcomeMessage ="\nPointless-Terminal v1.0.0.\n\
\tThis is a terminal simulator...\
hence the Pointless part.\n";

	printf("%s\n", welcomeMessage);
}

void getCommand(char** input)
{
	/*
		Get reference to input string
		Dereference and pass to getline
		(contrived I know)
	*/
	fflush(stdout);
	size_t inputSize = CMDMAX;
	do{
		printf("%s", ">>>");
		if(getline(&*input, &inputSize, stdin) < 0)
		{
			printf("%s\n", "Unable to allocate input\
				string");
			exit(-1);
		}
	}while(!strcmp(*input, "\n"));
}

void printStringArray(char* strArray[])
{
	int i = 0;
	while(strArray[i] != NULL)
	{
		printf("%s\n", strArray[i++]);
	}
}

int notExit(char* input){
	return strcmp(input,"exit");
}

int isExit(char* input){
	return !strcmp(input,"exit");
}

void saveCommandToHistory(char* history[], char* cmd, int index){

	strcpy(history[index], cmd);
}

void deallocStringArray(char* arr[], int size)
{
	int i = 0;
	for(i = 0; i < size; i++)
	{
		if(arr[i]!= NULL){
			free(arr[i]);
		}
	}

}

int isHistoryRequest(char* cmd){
	return !(strcmp(cmd, "!!") && strcmp(cmd, "!"));
}

void setAllArrayPointersToNull(char* arr[], int size)
{
	int i = 0;
	for(i = 0; i<size; i++)
	{
		arr[i] = NULL;
	}
}

int isBackgroundProcess(char* argv[])
{
	int i = 0;
	while(argv[i] != NULL)
	{
		if(!strcmp(argv[i++],"&"))
		{
			free(argv[i-1]);
			argv[i-1] = NULL; //remove symbol
			return 1;
		}
	}
	return 0;
}

int main(){


	printWelcomeMessage();

	int backgroundFlag = 0;

	int historyIndex = 0;
	char* history[HISTMAX];
	setAllArrayPointersToNull(history,HISTMAX);
	clearAndResetStringArray(history, HISTMAX, CMDMAX);

	char* input = NULL;

	char* argv[TOKENSMAX];
	setAllArrayPointersToNull(argv, TOKENSMAX);
	clearAndResetStringArray(argv, TOKENSMAX, TOKENLEN);

	//get first command here. This is done to prevent 
	//calls to history first with nothing actually in it
	getCommand(&input);
	tokenize(input, argv);

	//doesn't make sense to make a history request here
	//so prevent it
	while(isHistoryRequest(argv[0]))
	{
		printf("%s\n", "No commands in history, please try again");
		getCommand(&input);
		tokenize(input, argv);
	}

	if(isBackgroundProcess(argv))
	{
		backgroundFlag = 1;
	}

	if(isExit(argv[0]))
	{
		return 0;
	}

	saveCommandToHistory(history, input, historyIndex);

	do{

		//process command

		pid_t pid;
		int fd[2];

		int p = pipe(fd);
		if(p<0)
		{
			printf("%s\n", "Failed to pipe");
			exit(1);
		}

		pid = fork();

		if (pid < 0)
		{
			printf("%s\n", "Fork Failed");
			exit(1);
		}
		else if (pid == 0)
		{	
			if(ALLOWPIPELINING)
			{
				//remove stdout from fdlist		
				close(1);
				//replace it with pipe write
				dup(fd[1]);
				//close pipe
				close(fd[0]);
				close(fd[1]);
				//initiate exec
			}

			int status = execvp(argv[0],argv);
			printf("%s\n", "Invalid Command");
			//if failed then program will not be replaced by argv[0]
			exit(status);
		}
		else
		{
			if(ALLOWPIPELINING)
			{
				char buffer[4096] = "";
				close(fd[1]);
				read(fd[0],buffer,sizeof(buffer));
				printf("%s\n", buffer);
			}
			else{
				if(backgroundFlag == 0)
				{
					while(wait(NULL)!=pid);
				}
				else
				{
					int status;
					waitpid(-1,&status,WNOHANG);
				}
				backgroundFlag = 0; //reset background flag if it was a background process
			}
		}

		//get new command
		int invalidHistoryIndex = 0;

		getCommand(&input);
		tokenize(input, argv);

		do{

			if(isHistoryRequest(argv[0]))
			{
				if(argv[1] == NULL) // just !!
				{
					invalidHistoryIndex = 0;
					printf("%s\n", history[historyIndex]);
					tokenize(history[historyIndex], argv);
				}
				else if(atoi(argv[1]) < HISTMAX) // ! with index
				{
					invalidHistoryIndex = 0;
					printf("%s\n", history[atoi(argv[1])]);
					tokenize(history[atoi(argv[1])], argv);
				}
				else
				{
					invalidHistoryIndex = 1;
					printf("%s\n", "Invalid history index");
					getCommand(&input);
					tokenize(input, argv);
				}
			}
			else
			{		
				if(historyIndex<HISTMAX-1)
				{
					historyIndex++;
				}
				else
				{
					historyIndex = 0;
				}
				saveCommandToHistory(history, input, historyIndex);
			}

		}while(invalidHistoryIndex);

		if(isBackgroundProcess(argv))
		{
			backgroundFlag = 1;
		}

	}while(notExit(argv[0]));	
	deallocStringArray(argv, TOKENSMAX);
	deallocStringArray(history, HISTMAX);
	free(input);
	return 0;
}