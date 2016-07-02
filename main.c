#include <stdio.h>
#include "ast.h"
#include "ast.tab.h"
#include "ast.lex.h"

int main()
{
	struct pcdata p = { NULL, 0, NULL };

	/* set up scanner */
	if (yylex_init_extra(&p, &p.scaninfo)) {
		perror("init alloc failed");
		return 1;
	}

	/* allocate and zero out the symbol table */
	if (!(p.symtab = calloc(NHASH, sizeof(struct symbol)))) {
		perror("sym alloc failed");
		return 1;
	}

	for (;;) {
		printf("> ");
		yyparse(&p);
		if (p.ast) {
			printf("= %4.4g\n", eval(&p, p.ast));
			treefree(&p, p.ast);
			p.ast = 0;
		}
	};
}
