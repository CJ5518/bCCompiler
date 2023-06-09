BIN  = bC  # name of thing to be built goes here
PARSE = parser
CC   = g++
# CFLAGS = -g
CPPFLAGS = -O0 -g -I./ # for use with C++ if file ext is .c

SRCS = $(PARSE).y $(PARSE).l treeUtils.cpp
OBJS = lex.yy.o $(PARSE).tab.o treeUtils.o
LIBS = -lm 

$(BIN): $(OBJS)
	$(CC) $(CCFLAGS) $(OBJS) $(LIBS) -o $(BIN)

$(PARSE).tab.h $(PARSE).tab.c: $(PARSE).y scanType.h treeUtils.h
	bison -v -t -d --debug $(PARSE).y

lex.yy.c: $(PARSE).l $(PARSE).tab.h scanType.h
	flex $(PARSE).l

all:
	make

clean:
	rm -f $(OBJS) c- lex.yy.c $(PARSE).tab.h $(PARSE).tab.c c-.tar $(PARSE).output core

cleaner:
	make clean
	rm -f bC

test: $(BIN)
	./test.sh

tar:
	tar -cvf c-.tar $(SRCS) makefile 
