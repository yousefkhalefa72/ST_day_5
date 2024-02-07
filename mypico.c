#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

#define COMMAND      tokens[0]
#define E_LAST_CMD  -2
#define E_NOK       -1
#define E_OK         0

typedef signed int Std_Return;

Std_Return myecho(char* tokens[]);
Std_Return mypwd();
Std_Return mycd(char* tokens[]);


void sigintHandler(int sig_num)
{
    /* Reset handler to catch SIGINT next time.
    Refer http://en.cppreference.com/w/c/program/signal */
    signal(SIGINT, sigintHandler);
    printf("\n(ctrl+C) Cannot be kill \"$*#@!!\"\n");
    printf("\nJoe_shell->");
    fflush(stdout);
}

/**
 * @param commandline double ptr to your command line
 * @return Status of the function
 *          (int)   : successfully and return size of cmd line
 *          (E_NOK) : The function faild
 * @brief commandline need to free after finish
 */
Std_Return Get_Command_Line(char** commandline)
{
    char input_char=0;
    int command_line_size = 0;
    char* buf = (char *)malloc(sizeof(char));
    if(buf == NULL)
    {
        return E_NOK;
    }
    if(*commandline == NULL)
    {

    }
    else
    {
        command_line_size = sizeof(*commandline);
        char* temp = strdup(*commandline);
        free(*commandline);

        buf = (char *)realloc(buf,(2+command_line_size)*sizeof(char));
        if(buf == NULL)
        {
            return E_NOK;
        }
        buf = strdup(temp);
        free(temp);
    }

    /****************** Next Char ******************/
    while(1)
    {
        input_char = getchar();
        /*********************** End of line *******/
        if(input_char == '\n')
        {
            *commandline =(char *)malloc((command_line_size+1)*sizeof(char));
            if(*commandline == NULL)
            {
                return E_NOK;
            }
            /******************* up arrow ****************/
            if(strcmp(buf,"^[[A")==0)
            {
                free(buf);
                return  E_LAST_CMD;
            }

            *commandline = strdup(buf);
            commandline[command_line_size]=0;
            free(buf);
            return command_line_size;
        }
        /**************** handle "" *****************/
        else if(input_char == '\"')
        {
            buf = (char *)realloc(buf,(2+command_line_size)*sizeof(char));
            if(buf == NULL)
            {
                return E_NOK;
            }
            buf[command_line_size] = input_char;
            buf[command_line_size+1] = 0;
            command_line_size++;
            input_char = getchar();

            while(input_char != '"')
            {
                buf = (char *)realloc(buf,(2+command_line_size)*sizeof(char));
                if(buf == NULL)
                {
                    return E_NOK;
                }
                buf[command_line_size] = input_char;
                buf[command_line_size+1] = 0;
                command_line_size++;
                input_char = getchar();
            }
            buf = (char *)realloc(buf,(2+command_line_size)*sizeof(char));
            if(buf == NULL)
            {
                return E_NOK;
            }
            buf[command_line_size] = input_char;
            buf[command_line_size+1] = 0;
            command_line_size++;
        }
        /**************** handle '' *****************/
        else if(input_char == '\'')
        {
            buf = (char *)realloc(buf,(2+command_line_size)*sizeof(char));
            if(buf == NULL)
            {
                return E_NOK;
            }
            buf[command_line_size] = input_char;
            buf[command_line_size+1] = 0;
            command_line_size++;
            input_char = getchar();

            while(input_char != '\'')
            {
                buf = (char *)realloc(buf,(2+command_line_size)*sizeof(char));
                if(buf == NULL)
                {
                    return E_NOK;
                }
                buf[command_line_size] = input_char;
                buf[command_line_size+1] = 0;
                command_line_size++;
                input_char = getchar();
            }
            buf = (char *)realloc(buf,(2+command_line_size)*sizeof(char));
            if(buf == NULL)
            {
                return E_NOK;
            }
            buf[command_line_size] = input_char;
            buf[command_line_size+1] = 0;
            command_line_size++;
        }
        /*************** any char *******************/
        else
        {
            buf = (char *)realloc(buf,(2+command_line_size)*sizeof(char));
            if(buf == NULL)
            {
                return E_NOK;
            }
            buf[command_line_size] = input_char;
            buf[command_line_size+1] = 0;
            command_line_size++;
        }
    }
}

Std_Return printPrompt()
{
    printf("\nJoe_Shell >");
}

/**
 * @param command command line that will be tokens
 * @param tokens array of strins(tokens)
 * @return Status of the function
 *          (int)   : successfully and return num of tokens
 *          (E_NOK) : The function faild
 * @brief tokens[...,NULL] & tokens need to free after finish
 */
Std_Return parsing(char* command, char*** tokens)
{
    int token_num = 0;
    char** dynamicArray = (char**)malloc(sizeof(char *));
    if (dynamicArray == NULL)
    {
        return E_NOK;
    }
    while((*command != '\0'))
    {
        if(*command == '\'')
        {
            //take new token
            int i=0;
            char* buf = (char*)malloc(sizeof(char));
            if (buf == NULL)
            {
                return E_NOK;
            }
            buf[0]='\'';
            i++;
            command++;
            while(*command != '\'')
            {
                buf = (char*)realloc(buf,(2+i)*sizeof(char));
                if (buf == NULL)
                {
                    return E_NOK;
                }
                buf[i] = *command++;
                i++;
            }
            command++;
            buf[i]='\'';
            buf[i+1]='\0';
            if(token_num == 0)
            {
                dynamicArray[0]=strdup(buf);
            }
            else
            {
                dynamicArray = (char**)realloc(dynamicArray, (token_num+1)*sizeof(char*));
                if (dynamicArray == NULL)
                {
                    return E_NOK;
                }
                dynamicArray[token_num] =strdup(buf);
            }
            token_num++;
        }
        else if(*command == '\"')
        {
            //take new token
            int i=0;
            char* buf = (char*)malloc(sizeof(char));
            if (buf == NULL)
            {
                return E_NOK;
            }
            buf[0]='"';
            i++;
            command++;
            while(*command != '\"')
            {
                buf = (char*)realloc(buf,(2+i)*sizeof(char));
                if (buf == NULL)
                {
                    return E_NOK;
                }
                buf[i] = *command++;
                i++;
            }
            command++;
            buf[i]='\"';
            buf[i+1]='\0';
            if(token_num == 0)
            {
                dynamicArray[0]=strdup(buf);
            }
            else
            {
                dynamicArray = (char**)realloc(dynamicArray, (token_num+1)*sizeof(char*));
                if (dynamicArray == NULL)
                {
                    return E_NOK;
                }
                dynamicArray[token_num] =strdup(buf);
            }
            token_num++;
        }
        else if(*command == ' ')
        {
            command++;
        }
        else
        {
            	//take new token
            	int i=0;
            	char buf[20];

            	while((*command != ' ' ) && (*command != '\0' ))
            	{
                	buf[i] = *command++;
                	i++;
            	}
            	buf[i]='\0';
            	if(token_num == 0)
            	{
                	dynamicArray[0]=strdup(buf);
            	}
            	else
            		{
                		dynamicArray = (char**)realloc(dynamicArray, (token_num+1)*sizeof(char*));
                		if (dynamicArray == NULL)
                		{
                    		return E_NOK;
                		}
                		dynamicArray[token_num] =strdup(buf);
            		}
            		token_num++;
        	}
    	}
    	/***************** END DYNAMIC ARR BY NULL ******************************/
    	dynamicArray = (char**)realloc(dynamicArray, (token_num+1)*sizeof(char*));
    	if (dynamicArray == NULL)
    	{
        	return E_NOK;
    	}
    	dynamicArray[token_num] =NULL;
    	/******************** copy in original arr ************************/
    	(*tokens) = (char**)malloc((token_num+1)*sizeof(char*)); // Allocate memory for tokens
    	if ((*tokens) == NULL)
    	{
        	return E_NOK;
    	}

    	for(int i=0; dynamicArray[i] != NULL ; ++i)
    	{
        	(*tokens)[i]=strdup(dynamicArray[i]);
    	}
    	(*tokens)[token_num] =NULL;
    	/*********************** free **********************/
    	for(int i=0; dynamicArray[i] != NULL; ++i)
    	{
        	free(dynamicArray[i]);
    	}
	free(dynamicArray);
	return token_num;
}

int main()
{
	signal(SIGINT, sigintHandler);
    	char* last_command = "hello to myshell";
    	char* command_line = NULL;
    	signed int command_line_size = 0;
    	signed int tokens_num = 0;
	char file_from_redirection = -1;
    	char** tokens = NULL;
    	
	while(1)
    	{
	/**************** Get Command Line ****************/
        printPrompt();
        command_line_size = Get_Command_Line(&command_line);
        if(command_line_size == 0)
        {
            continue;
        }
        else if(command_line_size == E_NOK)
        {
            printf("Can't get your command\n");
            continue;
        }
        else if(command_line_size == E_LAST_CMD)
        {
            free(command_line);
            command_line = (char*)malloc(strlen(last_command)*sizeof(char));
            command_line = strdup(last_command);
            printf("Joe_Shell >%s",command_line);
            command_line_size = Get_Command_Line(&command_line);
        }
        /*************** parsing ***************************/
        tokens_num = parsing(command_line, &tokens);
        
	if (tokens_num == E_NOK)
        {
            printf("Parsing failed\n");
            continue;
        }

	for(int i =0; tokens[i] != NULL;++i)
	{
		if(strcmp(tokens[i], "<") == 0)
		{
			file_from_redirection=open(tokens[i+1],O_WRONLY | O_CREAT | O_TRUNC, 0644);
			if(-1 == file_from_redirection)
			{
				printf(">>>> can't open file %s\n",tokens[i+1]);
				break;
			}
			else
			{
				char* buf = NULL;
				int num_read = read(file_from_redirection, buf, 1000);
				//check reading file
				if(num_read < 0)
				{
					printf(">>>> can't read file %s\n",tokens[i+1]);
					break;
				}
				tokens[i] = strdup(buf);
				tokens[i+1] = NULL;
				break;
			}
		}
		else if(strcmp(tokens[i], ">") == 0)
		{
			file_from_redirection=open(tokens[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
			if(-1 == file_from_redirection)
			{
				printf(">>>> can't open file %s\n",tokens[i+1]);
				break;
			}
			else
			{
				dup2(1,file_from_redirection);
			}
			tokens[i]= NULL;
			--i;
		}
		else if(strcmp(tokens[i], "|") == 0)
		{
			//command1 = tokens[0:i-1]
			//command2 = tokens[i+1],command1
		}
	}	
	if(tokens[1] == "=")
	{
		//var name = tokens[0]
		//var value = tokens[2]
	}
	/*************** empty command ***************/
        if(COMMAND == NULL)
        {
        	//nothing
        }

        /*************** myecho **********************/
        else if(strcmp(COMMAND,"myecho") == 0)
        {
        	myecho(tokens);
        }

	/*************** mypwd **********************/
       	else if(strcmp(COMMAND,"mypwd") == 0)
       	{
       		mypwd();
       	}

	/*************** mycd **********************/
       	else if(strcmp(COMMAND,"mycd") == 0)
       	{
       	    	mycd(tokens);
       	}

	/*************** EXIT **********************/
       	else if(strcmp(COMMAND,"exit") == 0)
       	{
		printf(">Goodbye\n");
       		return E_OK;
       	}

	/*************** unknown command ***************/
       	else
       	{
       		printf(">this command isn't exist in my shell\n");
	
		pid_t returned_pid = fork();
		/********************* PARENT *********************/
		if (returned_pid > 0)
		{
			int wstatus;
			wait(&wstatus);
		}
		/*********************** CHILD *********************/
		else if (returned_pid == 0)
		{	
			execvp(COMMAND, tokens);
			/***************** bad child *****************/
			printf("bash-> couldn't find \"%s\"\n",COMMAND);
			return E_OK;
		}
		/******************** didn't Fork *******************/
		else
		{
			printf("busy to execute out of myshell\n");
		}
       	}
	/***************** file_redirection ******************/
	if(file_from_redirection != -1)
	{
		dup2(file_from_redirection,1);
	}
        /***************** Free Memory **********************/
        last_command = strdup(command_line);
        free(command_line);
        command_line = NULL;
        for(int i=0; tokens[i] != NULL; ++i)
        {
            free(tokens[i]);
        }
        free(tokens);
    }
}

Std_Return myecho(char* tokens[])
{
	char counter = 1;
	printf(">");
	while(tokens[counter] != NULL)
	{
		printf("%s ",tokens[counter]);
		counter++;
	}
	printf("\n");
}

Std_Return mypwd()
{
 	char buf[500]; //buf for the directory path

	//handle getcwd error
	if(getcwd(buf,sizeof(buf)) == NULL)
       	{
		printf(">can't get current directory\n");
	}
	else
	{
        	printf(">Current directory : %s\n",buf);
	}
}

Std_Return mycd(char* tokens[])
{
        char cd_ret = chdir(tokens[1]);
	if(cd_ret != E_OK)
	{
		printf(">failed to chanage dir\n");
	}
}
