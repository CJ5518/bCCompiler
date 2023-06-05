%{
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include "treeUtils.h"
#include "scanType.h"
using namespace std;

extern "C" int yylex();
extern "C" int yyparse();
extern "C" FILE *yyin;

void yyerror(const char *msg);

int numErrors;
int numWarnings;
extern int line;
extern int yylex();

//Add a sibling to a node
TreeNode* addSibling(TreeNode* to, TreeNode* newSibling) {
	//Check them both for NULL
	if (to == NULL) {
		printf("Bad argument to TreeNode, to is null");
		exit(1);
	}
	if (newSibling == NULL) {
		printf("Bad argument to TreeNode, newSibling is null");
		exit(1);
	}

	//Start here
	TreeNode* next = to;
	while (next) {
		//next is the last item in the linked list
		if (next->sibling == NULL) {
			break;
		}
	}
	//Add the new sibling to the end of the linked list
	next->sibling = newSibling;

	return newSibling;
}
// pass the static and type attribute down the sibling list
void setType(TreeNode* t, ExpType type, bool isStatic) {
	while (t) {
		// set t->type and t->isStatic
		// t = t->sibling;
		t->type = type;
		t->isStatic = isStatic;
		t = t->sibling;
	}
}
// the syntax tree goes here
TreeNode* syntaxTree;


%}
%union {
	TokenData *tokenData;
	TreeNode *tree;
	ExpType type; // for passing type spec up the tree
}
%token <tokenData> LT GT OP INT ID NUMCONST IF ELSE THEN TO DO FOR RETURN ERROR PRECOMPILER
%token <tokenData> BOOLCONST STATIC OR BOOL BREAK BY CHAR AND CHARCONST COMMENT NOT WHILE
%token <tokenData> EQ GEQ LEQ DEC DIVASS SUBASS ADDASS INC MULASS NEQ MAX MIN STRINGCONST
%type <tokenData> term
%type <tree> program
%%
program  :  program term
	|  term  {$$=$1;}
	;
term  : 
		LT {}
	|  GT {}
	|  OP {}
	|  INT {}
	|  ID {}
	|  NUMCONST {}
	|  IF {}
	|  THEN {}
	|  RETURN {}
	|  FOR {}
	|  DO {}
	|  TO {}
	|  WHILE {}
	|  BOOLCONST {}
	|  STATIC {}
	|  AND {}
	|  NOT {}
	|  OR {}
	|  BOOL {}
	|  BREAK {}
	|  BY {}
	|  CHAR {}
	|  ELSE {}
	|  PRECOMPILER {}
	|  EQ {}
	|  GEQ {}
	|  LEQ {}
	|  DEC {}
	|  DIVASS {}
	|  SUBASS {}
	|  ADDASS {}
	|  INC {}
	|  MULASS {}
	|  NEQ {}
	|  MAX {}
	|  MIN {}
	|  STRINGCONST {}
	|  CHARCONST {}
	|  COMMENT {}
	|  ERROR    {cout << "ERROR(" << yylval.tinfo.linenum << "): Invalid or misplaced input character: '" << yylval.tinfo.tokenstr << "'. Character Ignored.\n";}
	;
%%
void yyerror (const char *msg)
{ 
	cout << "Error: " <<  msg << endl;
}

int main(int argc, char **argv) {
	//yylval.tinfo.linenum = 1;
	int index;
	char *file = NULL;
	bool dotAST = false;             // make dot file of AST
	extern FILE *yyin;

	int ch;

	while ((ch = getopt(argc, argv, "d")) != -1) {
		switch (ch) {
			case 'd':
					  dotAST = true;
					  break;
			case '?':
			default:
					  //usage();
					;
		}
	}

	if ( optind == argc ) yyparse();
	for (index = optind; index < argc; index++) {
		yyin = fopen (argv[index], "r");
		yyparse();
		fclose (yyin);
	}
	if (numErrors==0) {
		printTree(stdout, syntaxTree, true, true);
		if(dotAST) {
			//IMPORTANT - I commented this out
			//printDotTree(stdout, syntaxTree, false, false);
		}
	}
	else {
		printf("-----------\n");
		printf("Errors: %d\n", numErrors);
		printf("-----------\n");
	}
	return 0;
}

