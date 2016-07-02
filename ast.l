%option noyywrap nodefault yylineno reentrant bison-bridge
%option header-file="ast.lex.h"

%top{
#include "ast.h"
#include "ast.tab.h"
#include "math.h"
#include "string.h"
#include "memory.h"
}

%{
#define fileno _fileno
%}

/* float exponent */
EXP	([Ee][-+]?[0-9]+)

%%
%{
  struct pcdata *pp = yyextra;
%}
 /* single character ops */
"+" |
"-" |
"*" |
"/" |
"=" |
"|" |
"," |
";" |
"(" |
")"     { return yytext[0]; }

 /* comparison ops */
">"     { yylval->fn = 1; return CMP; }
"<"     { yylval->fn = 2; return CMP; }
"<>"    { yylval->fn = 3; return CMP; }
"=="    { yylval->fn = 4; return CMP; }
">="    { yylval->fn = 5; return CMP; }
"<="    { yylval->fn = 6; return CMP; }

 /* keywords */

"if"    { return IF; }
"then"  { return THEN; }
"else"  { return ELSE; }
"while" { return WHILE; }
"do"    { return DO; }
"end"	{ return END;}
"elseif" { return ELSEIF;}
"def"	{return DEF;}

 /* built in functions */
"sqrt"  { yylval->fn = B_sqrt; return FUNC; }
"exp"   { yylval->fn = B_exp; return FUNC; }
"log"   { yylval->fn = B_log; return FUNC; }
"print" { yylval->fn = B_print; return FUNC; }

 /* debug hack */
"debug"[0-9]+ { debug = atoi(&yytext[5]); printf("debug set to %d\n", debug); }

 /* names */
[a-zA-Z][a-zA-Z0-9]*  { yylval->s = lookup(pp, yytext); return NAME; }

[0-9]+"."[0-9]*{EXP}? |
"."?[0-9]+{EXP}? { yylval->d = atof(yytext); return NUMBER; }

"//".*  
[ \t]   /* ignore white space */ 
\\\n    printf("c> "); /* ignore line continuation */
"\n"    { return EOL; }

.	{ yyerror(pp, "Mystery character %c\n", *yytext); }

<<EOF>> { exit(0);}
%%
