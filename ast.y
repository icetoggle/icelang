%define api.pure
%parse-param { struct pcdata *pp }
%{
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
%{
#  include "ast.lex.h"
#  include "ast.h"
#define YYLEX_PARAM pp->scaninfo
%}

%token <d> NUMBER
%token <s> NAME
%token <fn> FUNC
%token EOL
%token IF THEN ELSE WHILE DO END DEF ELSEIF

%nonassoc <fn> CMP
%right '='
%left '+' '-'
%left '*' '/'
%nonassoc '|' UMINUS
%type <a> exp stmt list explist elseif
%type <sl> symlist

%start calclist
%%
stmt: IF exp THEN list END {$$ = newflow(pp,'I',$2,$4,NULL);}
	| IF exp THEN list elseif {$$ = newflow(pp, 'I',$2,$4, $5);}
	| WHILE exp DO list END {$$ = newflow(pp, 'W', $2,$4, NULL);}
	| list
	;

elseif : ELSE list END  {$$ = newflow(pp, 'E', NULL	, $2, NULL);}
	| ELSEIF exp THEN list elseif {$$ = newflow(pp, 'E', $2, $4, $5);}
	| ELSEIF exp THEN list END {$$ = newflow(pp, 'E', $2, $4, NULL);}

list:	{$$=NULL;}
	| exp ';' list {
		if($3 == NULL)
			$$ = $1;
		else
			$$ = newast(pp, 'L',$1,$3);
	}
	;

exp: exp CMP exp {$$ = newcmp(pp, $2, $1, $3);}
	| exp '+' exp {$$ = newast(pp, '+',$1, $3);}
	| exp '-' exp {$$ = newast(pp, '-',$1, $3);}
	| exp '*' exp {$$ = newast(pp, '*',$1, $3);}
	| exp '/' exp {$$ = newast(pp, '/',$1, $3);}
	| '|' exp {$$ = newast(pp, '|',$2, NULL);}
	| '(' exp ')' {$$ = $2;}
	| '-' exp %prec UMINUS {$$ = newast(pp, 'M', $2, NULL);}
	| NUMBER {$$ = newnum(pp, $1);}
	| NAME {$$ = newref(pp, $1);}
	| NAME '=' exp {$$ = newasgn(pp, $1,$3);}
	| FUNC '(' explist ')' {$$ = newfunc(pp, $1, $3);}
	| NAME '(' explist ')' {$$ = newcall(pp, $1, $3);}
	;

explist: exp 
	| exp ',' explist {$$ = newast(pp, 'L', $1, $3);}
	;

symlist: NAME {$$ = newsymlist(pp, $1, NULL);}
		| NAME ',' symlist {$$ = newsymlist(pp, $1, $3);}

calclist:
	stmt EOL {
		pp->ast = $1;
		YYACCEPT;	
	}
	| DEF NAME '(' symlist ')' list END {
		dodef(pp, $2, $4, $6);
		printf("%d: Defined %s\n> ", yyget_lineno(pp->scaninfo), $2->name);
		pp->ast = NULL;
		YYACCEPT;
		}
	;

%%