%{
#include "scanType.h"
#include "treeUtils.h"
#include "parser.tab.h"


#define YY_DECL extern "C" int yylex()

char *lastToken = (char*)"";
int lineNum = 1;

int setValue(int linenum, int tokenClass, char *svalue) {
	yylval.tokenData = new TokenData;
	yylval.tokenData->tokenclass = tokenClass;
	yylval.tokenData->linenum = linenum;
	yylval.tokenData->tokenstr = lastToken = strdup(svalue);
	yylval.tokenData->cvalue = svalue[0];
	//May cause overflow error
	yylval.tokenData->nvalue = atoi(svalue);
	yylval.tokenData->svalue = strdup(svalue);
	return tokenClass;
}

%}


%option noyywrap

letter      [a-zA-Z]
digit    [0-9]
underbar [_]
letdig      {digit}|{letter}|{underbar}
op    [\-\+\*\/\%\=\;\(\)\{\}\>\<\[\]\,\:\?]
whitespace  [\ \t]

%%
^#.+  { return setValue(lineNum, PRECOMPILER, yytext);}
"int"      { return setValue(lineNum, INT, yytext); }
"return"      { return setValue(lineNum, RETURN, yytext); }
"if" { return setValue(lineNum, IF, yytext); }
"then" { return setValue(lineNum, THEN, yytext); }
"for" { return setValue(lineNum, FOR, yytext); }
"to" { return setValue(lineNum, TO, yytext); }
"do" { return setValue(lineNum, DO, yytext); }
"while" { return setValue(lineNum, WHILE, yytext); }
"and" { return setValue(lineNum, AND, yytext); }
"not" { return setValue(lineNum, NOT, yytext); }
"or" { return setValue(lineNum, OR, yytext); }
"bool" { return setValue(lineNum, BOOL, yytext); }
"break" { return setValue(lineNum, BREAK, yytext); }
"by" { return setValue(lineNum, BY, yytext); }
"char" { return setValue(lineNum, CHAR, yytext); }
"else" { return setValue(lineNum, ELSE, yytext); }
"=="      { return setValue(lineNum, EQ, yytext); }
">="      { return setValue(lineNum, GEQ, yytext); }
"<="      { return setValue(lineNum, LEQ, yytext); }
"--"      { return setValue(lineNum, DEC, yytext); }
"/="      { return setValue(lineNum, DIVASS, yytext); }
"-="      { return setValue(lineNum, SUBASS, yytext); }
"+="      { return setValue(lineNum, ADDASS, yytext); }
"++"      { return setValue(lineNum, INC, yytext); }
"*="      { return setValue(lineNum, MULASS, yytext); }
"!="      { return setValue(lineNum, NEQ, yytext); }
":>:"      { return setValue(lineNum, MAX, yytext); }
":<:"      { return setValue(lineNum, MIN, yytext); }
true|false { return setValue(lineNum, BOOLCONST, yytext); }
"static" { return setValue(lineNum, STATIC, yytext); }

\/\/.* { }


\"((\\\")|([^\n\"]))*\"  { return setValue(lineNum, STRINGCONST, yytext); }
\'\\?.\' { return setValue(lineNum, CHARCONST, yytext); }

{op} { return setValue(lineNum, yytext[0], yytext); }
{digit}+     { return setValue(lineNum, NUMCONST, yytext); }
{letdig}+     { return setValue(lineNum, ID, yytext); }

\n    { lineNum++ ; }
{whitespace}+  ;
.     { return setValue(lineNum, ERROR, yytext); }

