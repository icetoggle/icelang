%{
#include "ast.h"
#include <stdio.h>
#include <stdlib.h>

%}

%union {
	struct ast *a;
	double d;
	struct symbol *s;
	struct symlist *sl;
	int fn;
}

%token <d> NUMBER
%token <s> NAME
%token <fn> FUNC
%token EOL
%token IF THEN ELSE WHILE DO LET

%nonassoc <fn> CMP
%right '='
%left '+' '-'
%left '*' '/'
%nonassoc '|' UMINUS
%type <a> exp stmt list explist
%type <sl> symlist

%start calclist
%%
calclist:
	| calclist exp EOL {
		printf("= %4.4g\n",eval($2));
		treefree($2);
		printf("> ");
	}
	| calclist EOL {printf("> ");}
	;

exp: factor
	|exp '+' factor { $$ = newast('+', $1, $3);}
	|exp '-' factor { $$ = newast('-', $1, $3);}
	;

factor: term
		| factor '*' term { $$ = newast('*', $1, $3);}
		| factor '/' term { $$ = newast('/', $1, $3);}
		;

term: NUMBER {$$ = newnum($1);}
		| '|' term {newast('|',$2, NULL);}
		| '(' exp ')' {$$ = $2;}
		| '-' term {$$ = newast('M', $2, NULL);}
		;
%%