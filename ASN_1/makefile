BIN  = bC  # name of thing to be built goes here
CC   = g++
# CFLAGS = -g
# CCFLAGS = -DCPLUSPLUS -g  # for use with C++ if file ext is .cc
#CFLAGS = -DCPLUSPLUS -g  # for use with C++ if file ext is .c

SRCS = parser.y parser.l token.h
OBJS = lex.yy.o parser.tab.o
LIBS = -lm 

$(BIN): $(OBJS)
	$(CC) $(CCFLAGS) $(OBJS) $(LIBS) -o $(BIN)

parser.tab.h parser.tab.c: parser.y
	bison -v -t -d parser.y  

lex.yy.c: parser.l parser.tab.h
	flex parser.l

all:    
	touch $(SRCS)
	make

clean:
	rm -f $(OBJS) c- lex.yy.c parser.tab.h parser.tab.c c-.tar parser.output core

tar:
	tar -cvf c-.tar $(SRCS) makefile 
