BIN  = bC  # name of thing to be built goes hereunknown class
CC   = g++

CPPFLAGS = -O0 -g -I./ -std=c++11
SRCS = parser.y parser.l treeUtils.cpp
HDRS = scanType.h treeNodes.h treeUtils.h
OBJS = lex.yy.o parser.tab.o treeUtils.o
LIBS = -lm 

$(BIN): $(OBJS)
	$(CC) $(CCFLAGS) $(OBJS) $(LIBS) -o $(BIN)

parser.tab.h parser.tab.c: parser.y scanType.h treeUtils.h treeUtils.cpp treeNodes.h
	bison -v -t -d --debug parser.y

lex.yy.c: parser.l parser.tab.h scanType.h
	flex parser.l

all:
	make

clean:
	rm -f $(OBJS) c- lex.yy.c parser.tab.h parser.tab.c c-.tar parser.output core

cleaner:
	make clean
	rm -f bC

test: $(BIN)
	./test.sh

tar:
	tar -cvf c-.tar $(SRCS) makefile 
