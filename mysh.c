#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "tokens.h"
#define MAX 1024

/* THIS CODE WAS MY OWN WORK , IT WAS WRITTEN WITHOUT CONSULTING ANY
SOURCES OUTSIDE OF THOSE APPROVED BY THE INSTRUCTOR. Artemis Kelly */

//Error Checks:
// [X] close
// [X] open
// [X] dup
// [X] dup2
// [X] fork

int parseCLine(char** args){
  int checker = 0;

  //for storing the tokenized args
  char* argsTokens = args[0];
  char* nextToken;

  //for iterating through
  int index = 0;
  //iterate through
  while (argsTokens != NULL) {

    //if the token is &, make sure that it is the last on the command line
    if (strcmp(argsTokens, "&") == 0) {
      //if its not print the error message
      if (args[index+1] != NULL) {
        fprintf(stderr, "Error: \"&\" must be last token on command line\n");
        checker = -1;
        return checker;
        //otherwise, move arg in array
      } else {
        args[index] = args[index+1];
        checker = 1;
      }
    }

    //if the token is a symbol:
    if (strcmp(argsTokens, "<") == 0 || strcmp(argsTokens, ">") == 0 || strcmp(argsTokens, ">>") == 0 || strcmp(argsTokens, "|") == 0) {

      //if the symbol is the first argument, invalid null command
      if (index == 0) {
        fprintf(stderr, "Error: Invalid null command.\n");
        checker = -1;
        return checker;
      }

      //if its not the first argument and token == a symbol

      //if this is the last arg or it is the &
      if (args[index+1] == NULL || strcmp(args[index+1], "&") == 0) {
        //no file name for input
        if (strcmp(argsTokens, "<") == 0) {
          fprintf(stderr, "Error: Missing filename for input redirection.\n");
          checker = -1;
          return checker;
          //invalid null command
        } else if(strcmp(argsTokens, "|") == 0){
          fprintf(stderr, "Error: Invalid null command.\n");
          checker = -1;
          return checker;
          //no file name for output
        } else {
          fprintf(stderr, "Error: Missing filename for output redirection.\n");
          checker = -1;
          return checker;
        }
      }

      //check if the symbol is followed by another symbol
      if(args[index+1] != NULL){
        nextToken = args[index+1];
        // if(args[index+3]){
        //   if((strcmp(argsTokens, "|") == 0) && (strcmp(args[index+3], "<") == 0) ){
        //     fprintf(stderr, "Error: Ambiguous input redirection.\n");
        //     checker = -1;
        //     return checker;
        //
        //   }
        // }

        if (strcmp(nextToken, "<") == 0 || strcmp(nextToken, ">") == 0 || strcmp(nextToken, ">>") == 0 || strcmp(nextToken, "|") == 0) {
          if(strcmp(argsTokens, "<") == 0){
            fprintf(stderr, "Error: Ambiguous input redirection.\n");
            checker = -1;
            return checker;
          }
          if(strcmp(argsTokens, "|") == 0){
            fprintf(stderr, "Error: Invalid null command.\n");
            checker = -1;
            return checker;
          }
          //if the first was not < or |, abiguous output
          fprintf(stderr, "Error: Ambiguous output redirection.\n");
          checker = -1;
          return checker;

        }

      }

    }
    if(args[index+2] != NULL){
      if(args[index+3] != NULL){
        if((strcmp(argsTokens, "|") == 0) && (strcmp(args[index+3], "<") == 0) ){
            fprintf(stderr, "Error: Ambiguous input redirection.\n");
            checker = -1;
            return checker;
      }
    }
  }
    index++;
    argsTokens = args[index];
  }

  return checker;
}

void processArgs(char **args, int parseV) {
  int processID;

  processID = fork();

  int wstatus;
  int checker;

  //checker for failure on fork
  if (processID < 0) {
    perror("fork");
    return;

    //if fork works, pid is for a newly created child process so we have to wait for it to finish
  } else if (processID == 0) {
    int index = 0, fd;
    int stdinFD = dup(0);
    if(stdinFD == -1){
      perror("dup");
      return;
    }
    int stdoutFD = dup(1);
    if(stdoutFD == -1){
      perror("dup");
      return;
    }
    int stderrFD = dup(2);
    if(stderrFD == -1){
      perror("dup");
      return;
    }

    char* curToken = args[index];

    //iterate through the tokens
    while (curToken != NULL) {

      //if the token is <, open the file
      if (strcmp(curToken, "<") == 0) {
        if ((fd = open (args[index+1], O_RDONLY)) == -1) {
          fprintf(stderr, "Error: open(\"%s\"): %s\n", args[index+1], strerror(errno));
          exit(EXIT_FAILURE);
        }

        //copy fd info to 0
        checker = dup2 (fd, 0);
        if(checker == -1){
          perror("dup2");
          return;
        }
        checker = close (fd);
        if(checker == -1){
          perror("close");
          return;
        }


        char* nextToken = args[index+2];
        if (nextToken != NULL) {
          if (strcmp(nextToken, ">") == 0) {
            if ((fd = open(args[index+3], O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1) {
              fprintf(stderr, "Error: open(\"%s\"): %s\n", args[index+3], strerror(errno));
              exit(EXIT_FAILURE);
            }

            checker = dup2 (fd, 1);
            if(checker == -1){
              perror("dup2");
              return;
            }
            checker = dup2 (fd, 2);
            if(checker == -1){
              perror("dup2");
              return;
            }
            checker = close (fd);
            if(checker == -1){
              perror("close");
              return;
            }

          } else if (strcmp(nextToken, ">>") == 0) {
            if ((fd = open(args[index+3], O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1) {
              fprintf(stderr, "Error: open(\"%s\"): %s\n", args[index+3], strerror(errno));
              exit (EXIT_FAILURE);
            }

            checker = dup2 (fd, 1);
            if(checker == -1){
              perror("dup2");
              return;
            }
            checker = dup2 (fd, 2);
            if(checker == -1){
              perror("dup2");
              return;
            }
            checker = close (fd);
            if(checker == -1){
              perror("close");
              return;
            }

          } else if (strcmp(nextToken, "|") == 0) {
            int processID2;
            int fdpipe[2];

            if (pipe(fdpipe) == -1) {
              perror("pipe");
              exit(EXIT_FAILURE);
            }

            processID2 = fork();
            if(processID2 == -1){
              perror("fork");
              return;
            }

            if (processID2 > 0) {
              char **freshTokens=NULL;
              int fTokensCount=0;


              while (wait (&wstatus) != processID2)
              continue;

              checker = close(fdpipe[1]);
              if(checker == -1){
                perror("checker");
                return;
              }

              checker = dup2(fdpipe[0], 0);
              if(checker == -1){
                perror("dup2");
                return;
              }


              int index2 = index+3;
              freshTokens = (char**) malloc( sizeof(char*) );
              freshTokens[0] = NULL;

              while (args[index2] != NULL) {
                fTokensCount++;
                freshTokens = ( char ** ) realloc( freshTokens, (fTokensCount+1) * sizeof( char * ) );
                freshTokens[fTokensCount] = NULL;
                freshTokens[fTokensCount-1] = args[index2];
                index2++;
              }

              processArgs(freshTokens, parseV);

              for( int i=0; freshTokens[i]; i++ )
              free(freshTokens[i]);
              free(freshTokens);

              _exit (EXIT_SUCCESS);

            } else {
              checker = close(fdpipe[0]);
              if(checker == -1){
                perror("close");
                return;
              }

              checker = dup2(fdpipe[1], 1);
              if(checker == -1){
                perror("dup2");
                return;
              }

              checker = dup2(fdpipe[1], 2);
              if(checker == -1){
                perror("dup2");
                return;
              }

              checker = close(fdpipe[1]);
              if(checker == -1){
                perror("close");
                return;
              }
            }
          }
        }

        while (args[index]) {
          args[index] = NULL;
          index++;
        }
        break;

      } else if (strcmp(curToken, "|") == 0 && args[index+1] != NULL) {
        int fdpipe[2];
        int processID2;

        if (pipe(fdpipe) < 0) {
          perror("pipe");
          exit(EXIT_FAILURE);
        }

        processID2 = fork();
        if(processID2 == -1){
          perror("fork");
          exit(EXIT_FAILURE);
        }
        if (processID2 > 0) {
          int index2 = index+1;




          //wait for child
          while (wait (&wstatus) != processID2){
            continue;
          }

          checker = close(fdpipe[1]);
          if(checker == -1){
            perror("close");
            return;
          }

          checker = dup2(fdpipe[0], 0);
          if(checker == -1){
            perror("dup2");
            return;
          }

          int fTokensCount=0;
          char **freshTokens=NULL;
          freshTokens = (char**) malloc( sizeof(char*) );
          freshTokens[0] = NULL;

          while (args[index2] != NULL) {
            //update count
            fTokensCount++;
            //get space for it
            freshTokens = ( char ** ) realloc(freshTokens, (fTokensCount+1) * sizeof( char * ) );
            freshTokens[fTokensCount] = NULL;
            freshTokens[fTokensCount-1] = args[index2];
            index2++;
          }

          processArgs(freshTokens, parseV);

          //clear up the malloc
          for( int i=0; freshTokens[i]; i++ ){
            free(freshTokens[i]);
          }
          free(freshTokens);

          _exit (EXIT_SUCCESS);

        } else {

          checker = close(fdpipe[0]);
          if(checker == -1){
            perror("close");
            return;
          }

          checker = dup2(fdpipe[1], 1);
          if(checker == -1){
            perror("dup2");
            return;
          }

          checker = dup2(fdpipe[1], 2);
          if(checker == -1){
            perror("close");
            return;
          }

          checker = close(fdpipe[1]);
          if(checker == -1){
            perror("close");
            return;
          }

          //clean up
          while (args[index]) {
            args[index] = NULL;
            index++;
          }
          break;
        }
      } else if (((strcmp(curToken, ">") == 0) || (strcmp(curToken, ">>") == 0)) && (args[index+1] != NULL)) {
        if (strcmp(curToken, ">>") == 0) {
          if ((fd = open(args[index+1], O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1) {
            fprintf(stderr, "Error: open(\"%s\"): %s\n", args[index+1], strerror(errno));
            exit (EXIT_FAILURE);
          }
        } else {
          if ((fd = open(args[index+1], O_WRONLY | O_CREAT | O_EXCL , S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1) {
            fprintf(stderr, "Error: open(\"%s\"): %s\n", args[index+1], strerror(errno));
            exit(EXIT_FAILURE);
          }
        }

        checker = dup2 (fd, 1);
        if(checker == -1){
          perror("dup2");
          return;
        }
        checker = dup2 (fd, 2);
        if(checker == -1){
          perror("dup2");
          return;
        }

        checker = close (fd);
        if(checker == -1){
          perror("close");
          return;
        }

        char* nextToken = args[index+2];
        if (nextToken != NULL) {
          if (strcmp(nextToken, "<") == 0) {
            if ((fd = open (args[index+3], O_RDONLY)) == -1) {
              fprintf(stderr, "Error: open(\"%s\"): %s\n", args[index+3], strerror(errno));
              exit(EXIT_FAILURE);
            }

            checker = dup2 (fd, 0);
            if(checker == -1){
              perror("dup2");
              return;
            }
            checker = close (fd);
            if(checker == -1){
              perror("close");
              return;
            }
          }
        }

        while (args[index]) {
          args[index] = NULL;
          index++;
        }
        break;

      }

      //iterate to next
      index++;
      curToken = args[index];
    }
    //with execvp(): first argument is the program to be processArgsd,
    //second is array of arguments to go with it
    //copy back 3 main file descriptors
    if (execvp (args[0], args) == -1) {

      //stderr
      checker = dup2(stderrFD, 2);
      if(checker == -1){
        perror("dup2");
        return;
      }

      //stout
      checker = dup2(stdoutFD, 1);
      if(checker == -1){
        perror("dup2");
        return;
      }

      //stin
      checker = dup2(stdinFD, 0);
      if(checker == -1){
        perror("dup2");
        return;
      }

      fprintf(stderr, "Error: open(\"%s\"): %s\n", args[0], strerror(errno));
    }
    _exit (EXIT_FAILURE);
  } else if (processID > 0 && parseV == 0) {
    checker = wait(&wstatus);
    if(checker == -1){
      perror("wait");
      return;
    }
    while (checker != processID){
      //if it fails bc premature returns due to system interruption, call again
      if(checker == -1 && errno == EINTR ){
        checker = wait(&wstatus);
        if(checker == -1){
          perror("wait");
          return;
        }
      }
      checker = wait(&wstatus);
      if(checker == -1){
        perror("wait");
        return;
      }
      continue;
    }
  }
    //else if fork returned 0, this is a child process
 //else if (pid == 0)
}

int main(int argc, char *argv[]) {
  char *fileEnd;
  int printPrompt = 0;

  //if there were 2 arguments on the command line
  if (argc == 2) {
    //If “-” is given, do not print a prompt.
    if (strcmp(argv[1], "-") == 0) {
      printPrompt = 1;
    }
    else {
      fprintf(stderr, "Error: Usage: mysh [prompt]\n");
      exit(EXIT_FAILURE);
    }
    //should not take in any other arguments on the og command line so exit failure
  } else if (argc > 2) {
    fprintf(stderr, "Error: Usage: mysh [prompt]\n");
    exit(EXIT_FAILURE);
  }

  //process command line
  while (1) {
    char **commands;
    char buf[1024];

    //don't print a prompt if we set the variable above
    if (printPrompt) {
      fileEnd = fgets(buf, 1024, stdin);
      //checker for premature returns due to system interruption with fgets

      //if the return string is null, checker my errno. if set, run again
      if(fileEnd == NULL && errno == EINTR){
        fileEnd = fgets(buf, 1024, stdin);
      }

      //now checker for failure again
      if(fileEnd == NULL){
        perror("fgets");
      }
    } else {
      //otherwise, start by priting the prompt
      printf("mysh: ");
      fileEnd = fgets(buf, 1024, stdin);

      //if the return string is null, checker my errno. if set, run again
      if(fileEnd == NULL && errno == EINTR){
        fileEnd = fgets(buf, 1024, stdin);
      }

      //now checker for failure again
      if(fileEnd == NULL){
        perror("fgets");
      }
    }
    //checker for exit, exit on exit command
    if( fileEnd==NULL || strcmp(buf, "exit\n") == 0){
      exit(EXIT_SUCCESS);
    }

    //now buf contains command line from standard in
    commands = get_tokens(buf);

    //checke tokenized command prompts
    int parseCheck = parseCLine(commands);

    //IF parsecline returns -1, there was an issue
    if (parseCheck != -1) {
      processArgs(commands, parseCheck);
    }

    free_tokens(commands);
  }
}
