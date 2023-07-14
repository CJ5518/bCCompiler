%{
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include "treeNodes.h"
#include "treeUtils.h"
#include "scanType.h"
#include "semantics.h"
#include "symbolTable.h"
#include "yyerror.h"
#include "codegen.h"
using namespace std;

extern "C" int yylex();
extern "C" int yyparse();
extern "C" FILE *yyin;

extern int lineNum;
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

//For use in the root declList only
void setGlobal(TreeNode* t) {
	while (t) {
		// set t->type and t->isStatic
		// t = t->sibling;
		t->varKind = VarKind::Global;
		t = t->sibling;
	}
}

//For use in the root declList only
void setAsParameter(TreeNode* t) {
	while (t) {
		// set t->type and t->isStatic
		// t = t->sibling;
		t->varKind = VarKind::Parameter;
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
%type <tokenData> term assignop relop mulop minmaxop unaryop sumop
%type <tree> program precomList declList decl varDecl scopedVarDecl varDeclList varDeclInit varDeclId funDecl parms parmList parmTypeList
%type <tree> parmIdList parmId stmt matched iterRange unmatched expstmt compoundstmt localDecls stmtList returnstmt breakstmt
%type <tree> exp simpleExp andExp unaryRelExp relExp minmaxExp
%type <tree> sumExp mulExp unaryExp factor mutable
%type <type> typeSpec
%type <tree> immutable call args argList constant
%%
program  :  precomList declList {setGlobal($2);syntaxTree = $2;}
	;

precomList : precomList PRECOMPILER {printf("%s\n", yylval.tokenData->tokenstr);}
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
	| ID '[' NUMCONST ']' {$$ = newDeclNode(DeclKind::VarK, ExpType::UndefinedType, $1); $$->isArray = true; $$->size = $3->nvalue;}
	;

typeSpec : INT {$$ = ExpType::Integer;}
	| BOOL {$$ = ExpType::Boolean;}
	| CHAR {$$ = ExpType::Char;}
	;

funDecl : typeSpec ID '(' parms ')' stmt {$$ = newDeclNode(DeclKind::FuncK, $1, $2, $4, $6);}
	| ID '(' parms ')' stmt {$$ = newDeclNode(DeclKind::FuncK, ExpType::Void, $1, $3, $5);}
	;

parms : parmList {$$ = $1; setAsParameter($1);}
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

matched : IF simpleExp THEN matched ELSE matched {$$ = newStmtNode(StmtKind::IfK, $1, $2, $4, $6);}
	| WHILE simpleExp DO matched {$$ = newStmtNode(StmtKind::WhileK, $1, $2, $4);}
	| FOR ID '=' iterRange DO matched {$$ = newStmtNode(StmtKind::ForK, $1, newDeclNode(DeclKind::VarK, ExpType::Integer, $2), $4, $6);}
	| expstmt {$$ = $1;}
	| compoundstmt {$$ = $1;}
	| returnstmt {$$ = $1;}
	| breakstmt {$$ = $1;}
	;

iterRange : simpleExp TO simpleExp {$$ = newStmtNode(StmtKind::RangeK, $2, $1, $3);}
	| simpleExp TO simpleExp BY simpleExp {$$ = newStmtNode(StmtKind::RangeK, $2, $1, $3, $5);}
	;

unmatched : IF simpleExp THEN stmt {$$ = newStmtNode(StmtKind::IfK, $1, $2, $4);}
	| IF simpleExp THEN matched ELSE unmatched {$$ = newStmtNode(StmtKind::IfK, $1, $2, $4, $6);}
	| WHILE simpleExp DO unmatched {$$ = newStmtNode(StmtKind::WhileK, $1, $2, $4);}
	| FOR ID '=' iterRange DO unmatched {$$ = newStmtNode(StmtKind::ForK, $1, newDeclNode(DeclKind::VarK, ExpType::Integer, $2), $4, $6);}
	;

expstmt : exp ';' {$$ = $1;}
	| ';' {$$ = NULL;}
	;

compoundstmt : '{' localDecls stmtList '}' {$$ = newStmtNode(StmtKind::CompoundK, $1, $2, $3);}
	| '{' localDecls stmtList '}' ';' {$$ = newStmtNode(StmtKind::CompoundK, $1, $2, $3);}
	;

localDecls : localDecls scopedVarDecl {$$ = addSibling($1, $2);}
	| /* empty */ {$$ = NULL;}
	;

stmtList : stmtList stmt {$$ = addSibling($1, $2);}
	| /* empty */ {$$ = NULL;}
	;

returnstmt : RETURN ';' {$$ = newStmtNode(StmtKind::ReturnK, $1);}
	| RETURN exp ';' {$$ = newStmtNode(StmtKind::ReturnK, $1, $2);}
	;

breakstmt : BREAK ';' {$$ = newStmtNode(StmtKind::BreakK, $1);}
	;

exp : mutable assignop exp {$$ = newExpNode(ExpKind::AssignK, $2, $1, $3);}
	| mutable INC {$$ = newExpNode(ExpKind::AssignK, $2, $1);}
	| mutable DEC {$$ = newExpNode(ExpKind::AssignK, $2, $1);}
	| simpleExp {$$ = $1;}
	| mutable assignop error {}
	;

assignop : '=' {$$ = $1;}
	| ADDASS {$$ = $1;}
	| SUBASS {$$ = $1;}
	| MULASS {$$ = $1;}
	| DIVASS {$$ = $1;}
	;

simpleExp : simpleExp OR andExp {$$ = newExpNode(ExpKind::OpK, $2, $1, $3);}
	| andExp {$$ = $1;}
	;

andExp : andExp AND unaryRelExp {$$ = newExpNode(ExpKind::OpK, $2, $1, $3);}
	| unaryRelExp {$$ = $1;}
	;

unaryRelExp : NOT unaryRelExp {$$ = newExpNode(ExpKind::OpK, $1, $2);}
	| relExp {$$ = $1;}
	;

relExp : minmaxExp relop minmaxExp {$$ = newExpNode(ExpKind::OpK, $2, $1, $3);}
	| minmaxExp {$$ = $1;}
	;

relop : LEQ {$$ = $1;}
	| '>' {$$ = $1;}
	| '<' {$$ = $1;}
	| GEQ {$$ = $1;}
	| EQ {$$ = $1;}
	| NEQ {$$ = $1;}
	;

minmaxExp : minmaxExp minmaxop sumExp {$$ = newExpNode(ExpKind::OpK, $2, $1, $3);}
	| sumExp {$$ = $1;}
	;

minmaxop : MAX {$$ = $1;}
	| MIN {$$ = $1;}
	;

sumExp : sumExp sumop mulExp {$$ = newExpNode(ExpKind::OpK, $2, $1, $3);}
	| mulExp {$$ = $1;}
	;

sumop : '+' {$$ = $1;}
	| '-' {$$ = $1;}
	;

mulExp : mulExp mulop unaryExp {$$ = newExpNode(ExpKind::OpK, $2, $1, $3);}
	| unaryExp {$$ = $1;}
	;

mulop : '*' {$$ = $1;}
	| '/' {$$ = $1;}
	| '%' {$$ = $1;}
	;

unaryExp : unaryop unaryExp {$$ = newExpNode(ExpKind::OpK, $1, $2);}
	| factor {$$ = $1;}
	;

unaryop : '-' {$$ = $1;}
	| '*' {$$ = $1;}
	| '?' {$$ = $1;}
	;

factor : immutable {$$ = $1;}
	| mutable {$$ = $1;}
	;

mutable : ID {$$ = newExpNode(ExpKind::IdK, $1);}
	| ID '[' exp ']' {$$ = newExpNode(ExpKind::OpK, $2, newExpNode(ExpKind::IdK, $1), $3);}
	;

immutable : '(' exp ')' {$$ = $2;}
	| call {$$ = $1;}
	| constant {$$ = $1;}
	;

call : ID '(' args ')' {$$ = newExpNode(ExpKind::CallK, $1, $3);}
	;

args : argList {$$ = $1;}
	| /* empty */ {$$ = NULL;}
	;

argList : argList ',' exp {$$ = addSibling($1, $3);}
	| exp {$$ = $1;}
	;

constant : NUMCONST {$$ = newExpNode(ExpKind::ConstantK, $1); $$->type = ExpType::Integer;}
	| CHARCONST {$$ = newExpNode(ExpKind::ConstantK, $1); $$->type = ExpType::Char;}
	| STRINGCONST {$$ = newExpNode(ExpKind::ConstantK, $1); $$->type = ExpType::String; $$->isArray=true;}
	| BOOLCONST {$$ = newExpNode(ExpKind::ConstantK, $1); $$->type = ExpType::Boolean;}
	| ERROR    {emitTokenError(yylval.tokenData->linenum, yylval.tokenData->tokenstr[0]);}
	;


term  :
	|  COMMENT {}
	;
%%

//Token thingy for some of the weirder tokens
//Mostly made for the printing of the AST
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
	largerTokens[OR] = (char *)"or";
	largerTokens[NOT] = (char *)"not";
	largerTokens[GEQ] = (char *)">=";
	largerTokens[LEQ] = (char *)"<=";
	largerTokens['>'] = (char *)">";
	largerTokens['<'] = (char *)"<";
	largerTokens['*'] = (char *)"*";
	largerTokens['-'] = (char *)"-";
	largerTokens['+'] = (char *)"+";
	largerTokens['/'] = (char *)"/";
	largerTokens['?'] = (char *)"?";
	largerTokens['%'] = (char *)"%";
	largerTokens[EQ] = (char *)"==";
	largerTokens[NEQ] = (char *)"!=";
	largerTokens[MAX] = (char *)":>:";
	largerTokens[MIN] = (char *)":<:";
	largerTokens[INC] = (char *)"++";
	largerTokens[DEC] = (char *)"--";
	largerTokens['['] = (char *)"[";
}


int main(int argc, char **argv) {
	initErrorProcessing();
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
	char* infileName;
	for (index = optind; index < argc; index++) {
		yyin = fopen (argv[index], "r");
		infileName = argv[index];
		yyparse();
		fclose (yyin);
	}
	if (numErrors==0) {
		SymbolTable* symbolTable = new SymbolTable();
		//symbolTable->debug(true);
		int globalOffset = 0;
		syntaxTree = semanticAnalysis(syntaxTree, true, false, symbolTable, globalOffset);

		if (numErrors == 0)
			codegen(stdout, infileName, syntaxTree, symbolTable, globalOffset, true);
		//w03
		//printTree(stdout, syntaxTree, true, false);
		if(dotAST) {
			//IMPORTANT - I commented this out
			//printDotTree(stdout, syntaxTree, false, false);
		}
	}
	printf("Number of warnings: %d\n", numWarnings);
	printf("Number of errors: %d\n", numErrors);
	return 0;
}

