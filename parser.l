%{
#include "scanType.h"
#include "parser.tab.h"

#define YY_DECL extern "C" int yylex()

%}


%option noyywrap

letter      [a-zA-Z]
digit    [0-9]
quote    [\']
underbar [_]
letdig      {digit}|{letter}|{underbar}
limitters   [\;\,\(\)\{\}\[\]]
op    [\-\+\*\/\%\=\;\(\)\{\}\>\<\[\]\,\:\?]
whitespace  [\ \t]
%%
^#.+ { yylval.tinfo.tokenstr = yytext; return PRECOMPILER ; }
"int"      { yylval.tinfo.tokenstr = yytext; return INT ; }
"return"      { yylval.tinfo.tokenstr = yytext; return RETURN ; }
"if" { yylval.tinfo.tokenstr = yytext; return IF ; }
"then" { yylval.tinfo.tokenstr = yytext; return THEN ; }
"for" { yylval.tinfo.tokenstr = yytext; return FOR ; }
"to" { yylval.tinfo.tokenstr = yytext; return TO ; }
"do" { yylval.tinfo.tokenstr = yytext; return DO ; }
"while" { yylval.tinfo.tokenstr = yytext; return WHILE ; }
"and" { yylval.tinfo.tokenstr = yytext; return AND ; }
"not" { yylval.tinfo.tokenstr = yytext; return NOT ; }
"or" { yylval.tinfo.tokenstr = yytext; return OR ; }
"bool" { yylval.tinfo.tokenstr = yytext; return BOOL ; }
"break" { yylval.tinfo.tokenstr = yytext; return BREAK ; }
"by" { yylval.tinfo.tokenstr = yytext; return BY ; }
"char" { yylval.tinfo.tokenstr = yytext; return CHAR ; }
"else" { yylval.tinfo.tokenstr = yytext; return ELSE ; }
"=="      { yylval.tinfo.tokenstr = yytext; return EQ ; }
">="      { yylval.tinfo.tokenstr = yytext; return GEQ ; }
"<="      { yylval.tinfo.tokenstr = yytext; return LEQ ; }
"--"      { yylval.tinfo.tokenstr = yytext; return DEC ; }
"/="      { yylval.tinfo.tokenstr = yytext; return DIVASS ; }
"-="      { yylval.tinfo.tokenstr = yytext; return SUBASS ; }
"+="      { yylval.tinfo.tokenstr = yytext; return ADDASS ; }
"++"      { yylval.tinfo.tokenstr = yytext; return INC ; }
"*="      { yylval.tinfo.tokenstr = yytext; return MULASS ; }
"!="      { yylval.tinfo.tokenstr = yytext; return NEQ ; }
":>:"      { yylval.tinfo.tokenstr = yytext; return MAX ; }
":<:"      { yylval.tinfo.tokenstr = yytext; return MIN ; }
true|false { yylval.tinfo.tokenstr = yytext; return BOOLCONST ; }
"static" { yylval.tinfo.tokenstr = yytext; return STATIC ; }

\/\/.* { yylval.tinfo.tokenstr = yytext; return COMMENT ; }


\"((\\\")|([^\n\"]))*\"  { yylval.tinfo.tokenstr = yytext; return STRINGCONST ; }
\'..?\' { yylval.tinfo.tokenstr = yytext; return CHARCONST ; }

{op} { yylval.tinfo.tokenstr = yytext; return OP ; }
{digit}+     { yylval.tinfo.tokenstr = yytext; return NUMCONST ; }
{letdig}+     { yylval.tinfo.tokenstr = yytext; return ID ; }

\n    { yylval.tinfo.linenum++ ; }
{whitespace}+  ;
.     { yylval.tinfo.tokenstr = yytext; return ERROR; }

