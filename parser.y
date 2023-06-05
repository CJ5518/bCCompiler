%{
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include "treeNodes.h"
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
%token <tokenData> OP INT ID NUMCONST IF ELSE THEN TO DO FOR RETURN ERROR PRECOMPILER
%token <tokenData> BOOLCONST STATIC OR BOOL BREAK BY CHAR AND CHARCONST COMMENT NOT WHILE
%token <tokenData> EQ GEQ LEQ DEC DIVASS SUBASS ADDASS INC MULASS NEQ MAX MIN STRINGCONST
%type <tokenData> term
%type <tree> program precomList declList decl varDecl scopedVarDecl varDeclList varDeclInit varDeclId typeSpec funDecl parms parmList parmTypeList
%type <tree> parmIdList parmId stmt matched iterRange unmatched expstmt compoundstmt localDecls stmtList returnstmt breakstmt
%type <tree> exp assignop simpleExp andExp unaryRelExp relExp relop minmaxExp
%type <tree> minmaxop sumExp sumop mulExp mulop unaryExp unaryop factor mutablel
%type <tree> immutable call args argList constant
%%
program  :  precomList declList {syntaxTree = $2;}
	;

precomList : precomList PRECOMPILER {$$ = $1;}
	| PRECOMPILER {printf("%s\n", yylval.tokenData->tokenstr);}
	| /*empty*/ {$$ = NULL;}

declList : declList decl {}
	| decl {}

decl : varDecl {}
	| funDecl {}

varDecl : typeSpec varDeclList ';' {}

scopedVarDecl : STATIC typeSpec varDeclList ';' {}
	| typeSpec varDeclList ';' {}

varDeclList : varDeclList ',' varDeclInit {}
	| varDeclInit {}

varDeclInit : varDeclId {}

varDeclId : ID {}
	| ID '[' NUMCONST '[' {}

typeSpec : INT {}
	| BOOL {}
	| CHAR {}

funDecl : typeSpec ID '(' parms ')' stmt {}
	| ID '(' parms ')' stmt {}

parms : parmList {}
	| /*empty*/ {}

parmList : parmList ';' parmTypeList {}
	| parmTypeList {}

parmTypeList : typeSpec parmIdList {}

parmIdList : parmIdList ',' parmId {}
	| parmId {}

parmId : ID {}
	| ID '[' ']' {}

stmt : matched {}
	| unmatched {}

matched : IF simpleExp THEN matched ELSE matched {}
	| WHILE simpleExp DO matched {}
	| FOR ID '=' iterRange DO matched {}
	| expstmt {}
	| compoundstmt {}
	| returnstmt {}
	| breakstmt {}

iterRange : simpleExp TO simpleExp {}
	| simpleExp TO simpleExp BY simpleExp {}

unmatched : IF simpleExp THEN stmt {}
	| IF simpleExp THEN matched ELSE unmatched {}
	| WHILE simpleExp DO unmatched {}
	| FOR ID '=' iterRange DO unmatched {}

expstmt : exp ';' {}

compoundstmt : '{' localDecls stmtList '}' {}

localDecls : localDecls scopedVarDecl {}
	| /* empty */ {}

stmtList : stmtList stmt // empty stmt test {}
	| /* empty */ {}

returnstmt : RETURN ';' {}
	| RETURN exp ';' {}

breakstmt : BREAK ';' {}

exp : mutablel assignop exp {}
	| mutablel INC {}
	| mutablel DEC {}
	| simpleExp {}
	| mutablel assignop error {}

assignop : '=' {}
	| ADDASS {}
	| SUBASS {}
	| MULASS {}
	| DIVASS {}

simpleExp : simpleExp OR andExp {}
	| andExp {}

andExp : andExp AND unaryRelExp {}
	| unaryRelExp {}

unaryRelExp : NOT unaryRelExp {}
	| relExp {}

relExp : minmaxExp relop minmaxExp {}
| minmaxExp {}

relop : LEQ {}
	| '>' {}
	| '<' {}
	| GEQ {}
	| EQ {}
	| NEQ {}

minmaxExp : minmaxExp minmaxop sumExp {}
	| sumExp {}

minmaxop : MAX {}
	| MIN {}

sumExp : sumExp sumop mulExp {}
	| mulExp {}

sumop : '+' {}
	| '-' {}
mulExp : mulExp mulop unaryExp {}
	| unaryExp {}
mulop : '*' {}
	| '/' {}
	| '%' {}
unaryExp : unaryop unaryExp {}
	| factor {}
unaryop : '-' {}
	| '*' {}
	| '?' {}
factor : immutable {}
	| mutablel {}
mutablel : ID {}
	| ID '[' exp ']' {}
immutable : '(' exp ')' {}
	| call {}
	| constant {}
call : ID '(' args ')' {}
args : argList {}
	| /* empty */ {}
argList : argList ',' exp {}
	| exp {}
constant : NUMCONST {}
	| CHARCONST {}
	| STRINGCONST {}
	| BOOLCONST {}


term  :
	  ERROR    {cout << "ERROR(" << yylval.tinfo.linenum << "): Invalid or misplaced input character: '" << yylval.tinfo.tokenstr << "'. Character Ignored.\n";}
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

