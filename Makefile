TARGETS=mysh
CFLAGS=-std=gnu99
CC=gcc -g

mysh: mysh.o tokens.o
 
all: $(TARGETS)
