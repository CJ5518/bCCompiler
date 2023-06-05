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

TreeNode* addSibling(TreeNode* t, TreeNode* s) {
	// make sure s is not null. If it is this s a major error. Exit the program!
	// Make sure t is not null. If ti is, just return s
	// look down t’s sibling list until you fin with with sibblin = null (the end o f the lsit) and add s there.
	return s;
}
// pass the static and type attribute down the sibling list
void setType(TreeNode* t, ExpType type, bool isStatic) {
	while (t) {
		// set t->type and t->isStatic
		// t = t->sibling;
	}
}
// the syntax tree goes here
TreeNode* syntaxTree;


%}
%union
{
	struct   TokenData tinfo ;
}
%token <tinfo> LT GT OP INT ID NUMCONST IF ELSE THEN TO DO FOR RETURN ERROR  PRECOMPILER  WHILE  NOT OR BOOL BREAK BY CHAR AND 
%token <tinfo> BOOLCONST STATIC 
%token <tinfo> EQ GEQ LEQ DEC DIVASS SUBASS ADDASS INC MULASS NEQ MAX MIN STRINGCONST CHARCONST COMMENT
%type <tinfo> term program
%%
program  :  program term
	|  term  {$$=$1;}
	;
term  : 
		LT {cout << "Line: " << yylval.tinfo.linenum << " Type: LT Token: " << yylval.tinfo.tokenstr << endl;}
	|  GT {cout << "Line: " << yylval.tinfo.linenum << " Type: GT Token: " << yylval.tinfo.tokenstr << endl;}
	|  OP {cout << "Line: " << yylval.tinfo.linenum << " Type: OP Token: " << yylval.tinfo.tokenstr << endl;}
	|  INT {cout << "Line: " << yylval.tinfo.linenum << " Type: INT Token: " << yylval.tinfo.tokenstr << endl;}
	|  ID {cout << "Line: " << yylval.tinfo.linenum << " Type: ID Token: " << yylval.tinfo.tokenstr << endl;}
	|  NUMCONST {cout << "Line: " << yylval.tinfo.linenum << " Type: NUMCONST Token: " << yylval.tinfo.tokenstr << endl;}
	|  IF {cout << "Line: " << yylval.tinfo.linenum << " Type: IF Token: " << yylval.tinfo.tokenstr << endl;}
	|  THEN {cout << "Line: " << yylval.tinfo.linenum << " Type: THEN Token: " << yylval.tinfo.tokenstr << endl;}
	|  RETURN {cout << "Line: " << yylval.tinfo.linenum << " Type: RETURN Token: " << yylval.tinfo.tokenstr << endl;}
	|  FOR {cout << "Line: " << yylval.tinfo.linenum << " Type: FOR Token: " << yylval.tinfo.tokenstr << endl;}
	|  DO {cout << "Line: " << yylval.tinfo.linenum << " Type: DO Token: " << yylval.tinfo.tokenstr << endl;}
	|  TO {cout << "Line: " << yylval.tinfo.linenum << " Type: TO Token: " << yylval.tinfo.tokenstr << endl;}
	|  WHILE {cout << "Line: " << yylval.tinfo.linenum << " Type: WHILE Token: " << yylval.tinfo.tokenstr << endl;}
	|  BOOLCONST {cout << "Line: " << yylval.tinfo.linenum << " Type: BOOLCONST Token: " << yylval.tinfo.tokenstr << endl;}
	|  STATIC {cout << "Line: " << yylval.tinfo.linenum << " Type: STATIC Token: " << yylval.tinfo.tokenstr << endl;}
	|  AND {cout << "Line: " << yylval.tinfo.linenum << " Type: AND Token: " << yylval.tinfo.tokenstr << endl;}
	|  NOT {cout << "Line: " << yylval.tinfo.linenum << " Type: NOT Token: " << yylval.tinfo.tokenstr << endl;}
	|  OR {cout << "Line: " << yylval.tinfo.linenum << " Type: OR Token: " << yylval.tinfo.tokenstr << endl;}
	|  BOOL {cout << "Line: " << yylval.tinfo.linenum << " Type: BOOL Token: " << yylval.tinfo.tokenstr << endl;}
	|  BREAK {cout << "Line: " << yylval.tinfo.linenum << " Type: BREAK Token: " << yylval.tinfo.tokenstr << endl;}
	|  BY {cout << "Line: " << yylval.tinfo.linenum << " Type: BY Token: " << yylval.tinfo.tokenstr << endl;}
	|  CHAR {cout << "Line: " << yylval.tinfo.linenum << " Type: CHAR Token: " << yylval.tinfo.tokenstr << endl;}
	|  ELSE {cout << "Line: " << yylval.tinfo.linenum << " Type: ELSE Token: " << yylval.tinfo.tokenstr << endl;}
	|  PRECOMPILER {cout << "Line: " << yylval.tinfo.linenum << " Type: PRECOMPILER Token: " << yylval.tinfo.tokenstr << endl;}
	|  EQ {cout << "Line: " << yylval.tinfo.linenum << " Type: EQ Token: " << yylval.tinfo.tokenstr << endl;}
	|  GEQ {cout << "Line: " << yylval.tinfo.linenum << " Type: GEQ Token: " << yylval.tinfo.tokenstr << endl;}
	|  LEQ {cout << "Line: " << yylval.tinfo.linenum << " Type: LEQ Token: " << yylval.tinfo.tokenstr << endl;}
	|  DEC {cout << "Line: " << yylval.tinfo.linenum << " Type: DEC Token: " << yylval.tinfo.tokenstr << endl;}
	|  DIVASS {cout << "Line: " << yylval.tinfo.linenum << " Type: DIVASS Token: " << yylval.tinfo.tokenstr << endl;}
	|  SUBASS {cout << "Line: " << yylval.tinfo.linenum << " Type: SUBASS Token: " << yylval.tinfo.tokenstr << endl;}
	|  ADDASS {cout << "Line: " << yylval.tinfo.linenum << " Type: ADDASS Token: " << yylval.tinfo.tokenstr << endl;}
	|  INC {cout << "Line: " << yylval.tinfo.linenum << " Type: INC Token: " << yylval.tinfo.tokenstr << endl;}
	|  MULASS {cout << "Line: " << yylval.tinfo.linenum << " Type: MULASS Token: " << yylval.tinfo.tokenstr << endl;}
	|  NEQ {cout << "Line: " << yylval.tinfo.linenum << " Type: NEQ Token: " << yylval.tinfo.tokenstr << endl;}
	|  MAX {cout << "Line: " << yylval.tinfo.linenum << " Type: MAX Token: " << yylval.tinfo.tokenstr << endl;}
	|  MIN {cout << "Line: " << yylval.tinfo.linenum << " Type: MIN Token: " << yylval.tinfo.tokenstr << endl;}
	|  STRINGCONST {cout << "Line: " << yylval.tinfo.linenum << " Type: STRINGCONST Token: " << yylval.tinfo.tokenstr << endl;}
	|  CHARCONST {cout << "Line: " << yylval.tinfo.linenum << " Type: CHARCONST Token: " << yylval.tinfo.tokenstr << endl;}
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

