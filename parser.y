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
	// printf("Add To: "); fflush(stdout);
	// printTreeNode(stdout, to, false, false); fflush(stdout);
	// printf("\nThis: "); fflush(stdout);
	// printTreeNode(stdout, newSibling, false, false); fflush(stdout);
	// printf("\n---------------------------------\n");
	//Check them both for NULL
	if (to == NULL) {
		return newSibling;
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
		next = next->sibling;
	}
	//Add the new sibling to the end of the linked list
	next->sibling = newSibling;

	return to;
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
	TokenData* tokenData;
	TreeNode *tree;
	ExpType type; // for passing type spec up the tree
}
%token <tokenData> INT ID NUMCONST IF ELSE THEN TO DO FOR RETURN ERROR PRECOMPILER
%token <tokenData> BOOLCONST STATIC OR BOOL BREAK BY CHAR AND CHARCONST COMMENT NOT WHILE
%token <tokenData> EQ GEQ LEQ DEC DIVASS SUBASS ADDASS INC MULASS NEQ MAX MIN STRINGCONST
%token <tokenData> '*' '+' '{' '}' '[' ']' ';' '-' '>' '<' '=' ':' ',' '/' '(' ')' '%' '?'
%type <tokenData> term assignop
%type <tree> program precomList declList decl varDecl scopedVarDecl varDeclList varDeclInit varDeclId funDecl parms parmList parmTypeList
%type <tree> parmIdList parmId stmt matched iterRange unmatched expstmt compoundstmt localDecls stmtList returnstmt breakstmt
%type <tree> exp simpleExp andExp unaryRelExp relExp relop minmaxExp
%type <tree> minmaxop sumExp sumop mulExp mulop unaryExp unaryop factor mutable
%type <type> typeSpec
%type <tree> immutable call args argList constant
%%
program  :  precomList declList {syntaxTree = $2;}
	;

precomList : precomList PRECOMPILER {$$ = $1;}
	| PRECOMPILER {printf("%s\n", yylval.tokenData->tokenstr);}
	| /*empty*/ {$$ = NULL;}
	;

declList : declList decl {$$ = addSibling($1, $2);}
	| decl {$$ = $1;}
	;

decl : varDecl {$$ = $1;}
	| funDecl {$$ = $1;}
	;

varDecl : typeSpec varDeclList ';' {$$ = $2; setType($2, $1, false); yyerrok;}
;

scopedVarDecl : STATIC typeSpec varDeclList ';' {$$ = $3; setType($$, $2, true); yyerrok;}
	| typeSpec varDeclList ';' {$$ = $2; setType($$, $1, false); yyerrok;}
	;

varDeclList : varDeclList ',' varDeclInit {$$ = $1; addSibling($1, $3);}
	| varDeclInit {$$ = $1;}
	;

varDeclInit : varDeclId {$$ = $1;}
	| varDeclId ':' simpleExp {$$ = $1; $$->child[0] = $3;}
	;

varDeclId : ID {$$ = newDeclNode(DeclKind::VarK, ExpType::UndefinedType, $1);}
	| ID '[' NUMCONST ']' {}
	;

typeSpec : INT {$$ = ExpType::Integer;}
	| BOOL {$$ = ExpType::Boolean;}
	| CHAR {$$ = ExpType::Char;}
	;

funDecl : typeSpec ID '(' parms ')' stmt {$$ = newDeclNode(DeclKind::FuncK, $1, $2, $4, $6);}
	| ID '(' parms ')' stmt {$$ = newDeclNode(DeclKind::FuncK, ExpType::Void, $1, $3, $5);}
	;

parms : parmList {$$ = $1;}
	| /*empty*/ {$$ = NULL;}
	;

parmList : parmList ';' parmTypeList {$$ = addSibling($1, $3);}
	| parmTypeList {$$ = $1;}
	;

parmTypeList : typeSpec parmIdList {$$ = $2; setType($2, $1, false);}
	;

parmIdList : parmIdList ',' parmId {$$ = addSibling($1, $3);}
	| parmId {$$ = $1;}
	;

parmId : ID {$$ = newDeclNode(DeclKind::ParamK, ExpType::UndefinedType, $1);}
	| ID '[' ']' {$$ = newDeclNode(DeclKind::ParamK, ExpType::UndefinedType, $1); $$->isArray = true;}
	;

stmt : matched {$$ = $1;}
	| unmatched {$$ = $1;}
	;

matched : IF simpleExp THEN matched ELSE matched {}
	| WHILE simpleExp DO matched {}
	| FOR ID '=' iterRange DO matched {}
	| expstmt {$$ = $1;}
	| compoundstmt {$$ = $1;}
	| returnstmt {$$ = $1;}
	| breakstmt {$$ = $1;}
	;

iterRange : simpleExp TO simpleExp {}
	| simpleExp TO simpleExp BY simpleExp {}
	;

unmatched : IF simpleExp THEN stmt {}
	| IF simpleExp THEN matched ELSE unmatched {}
	| WHILE simpleExp DO unmatched {}
	| FOR ID '=' iterRange DO unmatched {}
	;

expstmt : exp ';' {$$ = $1;}
	;

compoundstmt : '{' localDecls stmtList '}' {$$ = newStmtNode(StmtKind::CompoundK, $1, $2, $3);}
	;

localDecls : localDecls scopedVarDecl {$$ = addSibling($1, $2);}
	| /* empty */ {$$ = NULL;}
	;

stmtList : stmtList stmt {$$ = addSibling($1, $2);}
	| /* empty */ {$$ = NULL;}
	;

returnstmt : RETURN ';' {}
	| RETURN exp ';' {}
	;

breakstmt : BREAK ';' {}
	;

exp : mutable assignop exp {$$ = newExpNode(ExpKind::AssignK, $2, $1, $3);}
	| mutable INC {}
	| mutable DEC {}
	| simpleExp {$$ = $1;}
	| mutable assignop error {}
	;

assignop : '=' {$$ = $1;}
	| ADDASS {$$ = $1;}
	| SUBASS {$$ = $1;}
	| MULASS {$$ = $1;}
	| DIVASS {$$ = $1;}
	;

simpleExp : simpleExp OR andExp {}
	| andExp {$$ = $1;}
	;

andExp : andExp AND unaryRelExp {}
	| unaryRelExp {$$ = $1;}
	;

unaryRelExp : NOT unaryRelExp {}
	| relExp {$$ = $1;}
	;

relExp : minmaxExp relop minmaxExp {}
	| minmaxExp {$$ = $1;}
	;

relop : LEQ {}
	| '>' {}
	| '<' {}
	| GEQ {}
	| EQ {}
	| NEQ {}
	;

minmaxExp : minmaxExp minmaxop sumExp {}
	| sumExp {$$ = $1;}
	;

minmaxop : MAX {}
	| MIN {}
	;

sumExp : sumExp sumop mulExp {}
	| mulExp {$$ = $1;}
	;

sumop : '+' {}
	| '-' {}
	;

mulExp : mulExp mulop unaryExp {}
	| unaryExp {$$ = $1;}
	;

mulop : '*' {}
	| '/' {}
	| '%' {}
	;

unaryExp : unaryop unaryExp {}
	| factor {$$ = $1;}
	;

unaryop : '-' {}
	| '*' {}
	| '?' {}
	;

factor : immutable {$$ = $1;}
	| mutable {$$ = $1;}
	;

mutable : ID {$$ = newExpNode(ExpKind::IdK, $1);}
	| ID '[' exp ']' {}
	;

immutable : '(' exp ')' {}
	| call {}
	| constant {$$ = $1;}
	;

call : ID '(' args ')' {}
	;

args : argList {$$ = $1;}
	| /* empty */ {$$ = NULL;}
	;

argList : argList ',' exp {$$ = addSibling($1, $3);}
	| exp {$$ = $1;}
	;

constant : NUMCONST {$$ = newExpNode(ExpKind::ConstantK, $1); $$->type = ExpType::Integer;}
	| CHARCONST {$$ = newExpNode(ExpKind::ConstantK, $1); $$->type = ExpType::Char;}
	| STRINGCONST {$$ = newExpNode(ExpKind::ConstantK, $1); $$->type = ExpType::String;}
	| BOOLCONST {$$ = newExpNode(ExpKind::ConstantK, $1); $$->type = ExpType::Boolean;}
	;


term  :
	  ERROR    {cout << "ERROR(" << yylval.tinfo.linenum << "): Invalid or misplaced input character: '" << yylval.tinfo.tokenstr << "'. Character Ignored.\n";}
	  COMMENT {}
	;
%%
void yyerror (const char *msg)
{ 
	cout << "Error: " <<  msg << endl;
}

//Token thingy for some of the weirder tokens
char *largerTokens[512];

void initTokenStrings() {
	const char* defaultMessage = "Unknown (or more likely unimplemented) largerTokens";
	for (int q = 0; q < 512; q++) {
		largerTokens[q] = (char*)defaultMessage;
	}
	largerTokens[ADDASS] = (char *)"+=";
	largerTokens[SUBASS] = (char *)"-=";
	largerTokens[MULASS] = (char *)"*=";
	largerTokens[DIVASS] = (char *)"/=";
	largerTokens[AND] = (char *)"and";
	largerTokens[BOOL] = (char *)"bool";
}

int main(int argc, char **argv) {
	initTokenStrings();
	yylval.tokenData = (TokenData*)malloc(sizeof(TokenData));
	yylval.tree = (TreeNode*)malloc(sizeof(TreeNode));
	yylval.tokenData->linenum = 1;
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
		printTree(stdout, syntaxTree, false, false);
		if(dotAST) {
			//IMPORTANT - I commented this out
			//printDotTree(stdout, syntaxTree, false, false);
		}
	}
	printf("Number of warnings: %d\n", numWarnings);
	printf("Number of errors: %d\n", numErrors);
	return 0;
}

