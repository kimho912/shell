// The MIT License (MIT)
// 
// Copyright (c) 2016 Trevor Bakker 
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 12     // Mav shell supports up to 10 arguments

#define MAX_HISTORY_SIZE 15     // The maximum size of history list

int main()
{

  char * command_string = (char*) malloc( MAX_COMMAND_SIZE );
  //a stack of the command line history list up to 15
  char *history[MAX_HISTORY_SIZE] = {NULL};
  //counting history
  int history_count=0;
  //history pid list
  int history_pid[MAX_HISTORY_SIZE];
  //for !n command, saves 'n'
  int command_num=0;
  //if loop finished at ! function then iflag is 1
  int iflag=0;
  char *temp=malloc(MAX_COMMAND_SIZE);

  
  //mallocing history list
  for (int i =0; i < MAX_HISTORY_SIZE; i++)
  {
    history[i] = malloc(MAX_COMMAND_SIZE);
  }

  while( 1 )
  {
    int input_count=0;
    //if it is not from '!'function
    if(iflag==0)
    {
      // Print out the msh prompt
      printf ("msh> ");

      // Read the command from the commandline.  The
      // maximum command that will be read is MAX_COMMAND_SIZE
      // This while command will wait here until the user
      // inputs something since fgets returns NULL when there
      // is no input
      while( !fgets (command_string, MAX_COMMAND_SIZE, stdin) );
      //check if the input is less than 101
      for(int i=0; command_string[i]!='\0'; i++)
      {
        input_count++;
      }
      if(input_count>100)
      {
        printf("shall cannot exceed 100 characters...\n");
        continue;
      }
    }
    // if it is from '!'function then save replace command with the last history command 
    else 
    {
      strcpy(command_string,temp);
      iflag=0;
    }
    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];
    //reset token
    for( int i = 0; i < MAX_NUM_ARGUMENTS; i++ )
    {
      token[i] = NULL;
    }

    int   token_count = 0;                                 
                                                           
    // Pointer to point to the token
    // parsed by strsep
    char *argument_ptr = NULL;                                         
                                                           
    char *working_string  = strdup( command_string );                

    // we are going to move the working_string pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *head_ptr = working_string;

    // Tokenize the input strings with whitespace used as the delimiter
    while ( ( (argument_ptr = strsep(&working_string, WHITESPACE ) ) != NULL) && 
              (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup( argument_ptr, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
        token_count++;
    }
    if (token_count>MAX_NUM_ARGUMENTS)
    {
      printf("shall support up to 10 command line parameters in addition to the command\n");
      continue;
    }
    if (token[0]==NULL)
    {
      continue;
    }
    // Now print the tokenized input as a debug check

    //to identify if this command worked at built-in or not (-1 built-in, 0> child process)
    pid_t pid = -1;

    if(!strcmp(token[0],"exit") || !strcmp(token[0],"quit")) // if exit or quit, status is 0
    {
      exit(0);
    }
    else if(!strcmp(token[0],"cd"))                          //if the command is cd, move the directory 
    {
      if(token[1]==NULL)
      {
        printf("please add a directroy name\n");
        continue;
      }
      if (history_count==MAX_HISTORY_SIZE)                 //if the history count is max
      {
        for (int i=0; i<MAX_HISTORY_SIZE-1; i++)           //the 0 index of the list is released and new command saved at the last index  
        {
          strcpy(history[i],history[i+1]);
          strcpy(history[i+1],"");
          history_pid[i]=history_pid[i+1];
        }
        history_count--;
      }
      for (int i=0; token[i]!=NULL; i++)                   //copy the command-line and pass it to the history list 
      {
        strcat(history[history_count],token[i]);
        if (token[i+1]!=NULL)
        {
          strcat(history[history_count]," ");
        }
      }
      history_pid[history_count]=pid;                       //save the history pid in the history pid list
      history_count++;
      int cd = chdir(token[1]);                                        //move the directory to another directory
      if (cd==-1)
      {
        printf("Filename: %s does not exist\n",token[1]);
        continue;
      }
    }
    else if(!strcmp(token[0],"mkdir"))                        //make directory in the current folder
    {
      if (history_count==MAX_HISTORY_SIZE)                  //if the history count is max
      {
        for (int i=0; i<MAX_HISTORY_SIZE-1; i++)            //the 0 index of the list is released and new command saved at the last index 
        {
          strcpy(history[i],history[i+1]);
          strcpy(history[i+1],"");
          history_pid[i]=history_pid[i+1];
        }
        history_count--;
      }
      for (int i=0; token[i]!=NULL; i++)                    //copy the command-line and pass it to the history list
      {
        strcat(history[history_count],token[i]);
        if (token[i+1]!=NULL)
        {
          strcat(history[history_count]," ");
        }
      }
      history_pid[history_count]=pid;
      history_count++;
      
      mkdir(token[1],S_IRWXU);                               //make a new directory in the current folder
    }
    else if(!strcmp(token[0],"history") && token[1]==NULL)  //if history command is passed, print out the list
    {
      
      if (history_count==MAX_HISTORY_SIZE)
      {
        for (int i=0; i<MAX_HISTORY_SIZE-1; i++)
        {
          strcpy(history[i],history[i+1]);
          strcpy(history[i+1],"");
          history_pid[i]=history_pid[i+1];
        }
        history_count--;
      }
      for (int i=0; token[i]!=NULL; i++)
      {
        strcat(history[history_count],token[i]);
        if (token[i+1]!=NULL)
        {
          strcat(history[history_count]," ");
        }
      }
      history_pid[history_count]=pid;
      history_count++;
      
      for (int i=0; i < history_count; i++)                                         //printing out the list.
      {
        printf("[%d]: %s\n",i,history[i]);
      }
    }
    else if(!strcmp(token[0],"history") && !strcmp(token[1],"-p") && token[2]==NULL) //if history -p print out the history list and the pid list
    {
      if (history_count==MAX_HISTORY_SIZE)
      {
        for (int i=0; i<MAX_HISTORY_SIZE-1; i++)
        {
          strcpy(history[i],history[i+1]);
          strcpy(history[i+1],"");
          history_pid[i]=history_pid[i+1];
        }
        history_count--;
      }
      for (int i=0; token[i]!=NULL; i++)
      {
        strcat(history[history_count],token[i]);
        if (token[i+1]!=NULL)
        {
          strcat(history[history_count]," ");
        }
      }
      history_pid[history_count]=pid;
      history_count++;
      
      for (int i=0; i < history_count; i++)                             //same as history but with a pid.
      {
        printf("[%d]: [%d] %s\n",i,history_pid[i],history[i]);
      }
    }
    else if(token[0][0]=='!')                                           //if '!n'command is passed, do the nth command again
    {
      sscanf(token[0],"!%d",&command_num);                              //extract 'n'
      if(command_num>history_count-1 && command_num < MAX_HISTORY_SIZE) //is the number in the list?
      {
        printf("The Command is not in the history..\n");
        continue;
      }
      if (command_num >= MAX_HISTORY_SIZE || command_num<0)              //Is it between 0 and 14? 
      {
        printf("Command number must be between 0 and 14!\n");
        continue;
      }
      if (command_num>=0 && command_num<MAX_HISTORY_SIZE)                
      {
        strcpy(temp,history[command_num]);                             //copy the nth history to the temp char
        iflag=1;                                                       //iflag is on
        continue;                                                      //coming back to the beginning of the loop
      }
    }
    else
    {
      pid=fork();                                                      //make a child process to use system instructions
      if(pid == -1)
      {
        perror("fork failed");
        exit(EXIT_FAILURE);
      }
      else if(pid == 0)
      {
        int ret = execvp(token[0],token);                             //if it is valid, replace the process

        if(ret == -1)                                                 //if it is invalid, comment it
        {
          printf("%s: Command not found.\n",token[0]);
          fflush(NULL);
          exit(EXIT_SUCCESS);
        }
      }
      else
      {
        int status;
        
        if (history_count==MAX_HISTORY_SIZE)
        {
          for (int i=0; i<MAX_HISTORY_SIZE-1; i++)
          {
            strcpy(history[i],history[i+1]);
            strcpy(history[i+1],"");
            history_pid[i]=history_pid[i+1];
          }
          history_count--;
        }
        for (int i=0; token[i]!=NULL; i++)
        {
          strcat(history[history_count],token[i]);
          if (token[i+1]!=NULL)
          {
            strcat(history[history_count]," ");
          }
        }
        history_pid[history_count]=pid;
        history_count++;
        
        waitpid(pid,&status,0);
        fflush(NULL);
      }
    }

    // Cleanup allocated memory
    for( int i = 0; i < MAX_NUM_ARGUMENTS; i++ )
    {
      if( token[i] != NULL )
      {
        free( token[i] );
      }
    }

    free( head_ptr );

  }
  for (int i =0; i < MAX_HISTORY_SIZE; i++)
  {
    free(history[i]);
  }
  free(temp);
  free( command_string );


  return 0;
  // e2520ca2-76f3-90d6-0242ac120003
}