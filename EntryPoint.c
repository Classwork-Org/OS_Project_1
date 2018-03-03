#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CMDMAX 80
#define TOKENSMAX 50
#define TOKENLEN 1000
#define MAX_LEN 128

void tokenize(char* input, char* argv[])
{
	/*
		parse input into tokens with last string
		as NULL. Some limitations apply. Max number
		of tokens is 50, and max token length is
		1000
	*/
	char* tokens = strtok(input, " ");
	int tokenIndex = 0;

	while(tokens!=NULL)
	{
		argv[tokenIndex] = (char*)malloc(TOKENLEN*sizeof(char));
		strcpy(argv[tokenIndex++],tokens);
		tokens = strtok(NULL," \n");
	}

	argv[tokenIndex] = NULL;
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
	size_t inputSize = CMDMAX;

	if(getline(&*input, &inputSize, stdin) < 0)
	{
		printf("%s\n", "Unable to allocate input\
			string");
		exit(-1);
	}
}

void printStringArray(char* strArray[])
{
	int i = 0;
	while(strArray[i] != NULL)
	{
		printf("%s\n", strArray[i++]);
	}
}

int main(){


	printWelcomeMessage();

	char* input = NULL;
	getCommand(&input);

	char* argv[TOKENSMAX];
	tokenize(input, argv);

	pid_t pid;

	pid = fork();
	if (pid < 0)
	{
		printf("%s\n", "Fork Failed");
		exit(1);
	}
	else if (pid == 0)
	{
		printf("%s\n", "Child will run");
		printStringArray(argv);
		execvp(argv[0], argv);
		exit(0);
	}
	else
	{
		wait(0);
		printf("%s\n", "Child finished, parent will terminate");
	}


	return 0;
}

