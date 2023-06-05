%{
#include "scanType.h"
#include "treeUtils.h"
#include "parser.tab.h"


#define YY_DECL extern "C" int yylex()

%}


%option noyywrap

letter      [a-zA-Z]
digit    [0-9]
underbar [_]
letdig      {digit}|{letter}|{underbar}
op    [\-\+\*\/\%\=\;\(\)\{\}\>\<\[\]\,\:\?]
whitespace  [\ \t]

%%
^#.+ { yylval.tokenData->tokenstr = yytext; return PRECOMPILER ; }
"int"      { yylval.tokenData->tokenstr = yytext; return INT ; }
"return"      { yylval.tokenData->tokenstr = yytext; return RETURN ; }
"if" { yylval.tokenData->tokenstr = yytext; return IF ; }
"then" { yylval.tokenData->tokenstr = yytext; return THEN ; }
"for" { yylval.tokenData->tokenstr = yytext; return FOR ; }
"to" { yylval.tokenData->tokenstr = yytext; return TO ; }
"do" { yylval.tokenData->tokenstr = yytext; return DO ; }
"while" { yylval.tokenData->tokenstr = yytext; return WHILE ; }
"and" { yylval.tokenData->tokenstr = yytext; return AND ; }
"not" { yylval.tokenData->tokenstr = yytext; return NOT ; }
"or" { yylval.tokenData->tokenstr = yytext; return OR ; }
"bool" { yylval.tokenData->tokenstr = yytext; return BOOL ; }
"break" { yylval.tokenData->tokenstr = yytext; return BREAK ; }
"by" { yylval.tokenData->tokenstr = yytext; return BY ; }
"char" { yylval.tokenData->tokenstr = yytext; return CHAR ; }
"else" { yylval.tokenData->tokenstr = yytext; return ELSE ; }
"=="      { yylval.tokenData->tokenstr = yytext; return EQ ; }
">="      { yylval.tokenData->tokenstr = yytext; return GEQ ; }
"<="      { yylval.tokenData->tokenstr = yytext; return LEQ ; }
"--"      { yylval.tokenData->tokenstr = yytext; return DEC ; }
"/="      { yylval.tokenData->tokenstr = yytext; return DIVASS ; }
"-="      { yylval.tokenData->tokenstr = yytext; return SUBASS ; }
"+="      { yylval.tokenData->tokenstr = yytext; return ADDASS ; }
"++"      { yylval.tokenData->tokenstr = yytext; return INC ; }
"*="      { yylval.tokenData->tokenstr = yytext; return MULASS ; }
"!="      { yylval.tokenData->tokenstr = yytext; return NEQ ; }
":>:"      { yylval.tokenData->tokenstr = yytext; return MAX ; }
":<:"      { yylval.tokenData->tokenstr = yytext; return MIN ; }
true|false { yylval.tokenData->tokenstr = yytext; return BOOLCONST ; }
"static" { yylval.tokenData->tokenstr = yytext; return STATIC ; }

\/\/.* { yylval.tokenData->tokenstr = yytext; return COMMENT ; }


\"((\\\")|([^\n\"]))*\"  { yylval.tokenData->tokenstr = yytext; return STRINGCONST ; }
\'..?\' { yylval.tokenData->tokenstr = yytext; return CHARCONST ; }

{op} { yylval.tokenData->tokenstr = yytext; return OP ; }
{digit}+     { yylval.tokenData->tokenstr = yytext; return NUMCONST ; }
{letdig}+     { yylval.tokenData->tokenstr = yytext; return ID ; }

\n    { yylval.tokenData->linenum++ ; }
{whitespace}+  ;
.     { yylval.tokenData->tokenstr = yytext; return ERROR; }

