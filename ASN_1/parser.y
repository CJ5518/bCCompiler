%{
#include <cstdio>
#include <iostream>
#include <unistd.h>
#include "scanType.h"
using namespace std;

extern "C" int yylex();
extern "C" int yyparse();
extern "C" FILE *yyin;

void yyerror(const char *msg);

%}
%union
{
   struct   TokenData tinfo ;
}
%token   <tinfo>  LT
%token   <tinfo>  GT
%token   <tinfo>  OP
%token   <tinfo>  INT
%token   <tinfo>  ID
%token   <tinfo>  NUMCONST
%token   <tinfo>  IF
%token   <tinfo>  ELSE
%token   <tinfo>  THEN
%token   <tinfo>  TO
%token   <tinfo>  DO
%token   <tinfo>  FOR
%token   <tinfo>  RETURN
%token   <tinfo>  ERROR 
%token   <tinfo>  PRECOMPILER 
%token   <tinfo>  WHILE 
%token   <tinfo>  NOT
%token   <tinfo>  OR
%token   <tinfo>  BOOL
%token   <tinfo>  BREAK
%token   <tinfo>  BY
%token   <tinfo>  CHAR
%token   <tinfo>  AND
%token   <tinfo>  BOOLCONST
%token   <tinfo>  STATIC
%token   <tinfo>  EQ
%token   <tinfo>  GEQ
%token   <tinfo>  LEQ
%token   <tinfo>  DEC
%token   <tinfo>  DIVASS
%token   <tinfo>  SUBASS
%token   <tinfo>  ADDASS
%token   <tinfo>  INC
%token   <tinfo>  MULASS
%token   <tinfo>  NEQ
%token   <tinfo>  MAX
%token   <tinfo>  MIN
%token   <tinfo>  STRINGCONST
%token   <tinfo>  CHARCONST
%token   <tinfo>  COMMENT
%type <tinfo>  term program
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
   yylval.tinfo.linenum = 1;
   int option, index;
   char *file = NULL;
   extern FILE *yyin;
   while ((option = getopt (argc, argv, "")) != -1)
      switch (option)
      {
      default:
         ;
      }
   if ( optind == argc ) yyparse();
   for (index = optind; index < argc; index++) 
   {
      yyin = fopen (argv[index], "r");
      yyparse();
      fclose (yyin);
   }
   return 0;
}

