#include <stdio.h>
#include "ast.h"
#include "ast.tab.h"

int main()
{
	printf("> ");
	return yyparse();
}
