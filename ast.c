#include "ast.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "ast.tab.h"

void
yyerror(struct pcdata *pp, char *s, ...)
{
    va_list ap;
    va_start(ap, s);
    
    fprintf(stderr, "%d: error: ", yyget_lineno(pp->scaninfo));
    vfprintf(stderr, s, ap);
    fprintf(stderr, "\n");
}

struct ast *
	newast(struct pcdata *pp, int nodetype, struct ast *l, struct ast *r)
{

	struct ast *a = malloc(sizeof(struct ast));

	if (!a) {
		yyerror(pp, "out of space");
		exit(0);
	}
	a->nodetype = nodetype;
	a->l = l;
	a->r = r;
	return a;
}

struct ast *
	newnum(struct pcdata *pp, double d)
{
	struct numval *a = malloc(sizeof(struct numval));

	if (!a) {
		yyerror(pp, "out of space");
		exit(0);
	}
	a->nodetype = 'K';
	a->number = d;
	return (struct ast *)a;
}
/* debugging: dump out an AST */
int debug = 0;
static double callbuiltin(struct pcdata *pp, struct fncall *f);
static double calluser(struct pcdata *pp, struct ufncall *f);
double eval(struct pcdata *pp, struct ast *a) {
	double v;
	switch (a->nodetype) {
	case 'K':
		v = ((struct numval *)a)->number;
		break;
	case 'N': 
		v = ((struct symref *)a)->s->value; break;
	case '+':
		v = eval(pp, a->l) + eval(pp, a->r); break;
	case '-':
		v = eval(pp, a->l) - eval(pp, a->r); break;
	case '*':
		v = eval(pp, a->l) * eval(pp, a->r); break;
	case '/':
		v = eval(pp, a->l) / eval(pp, a->r); break;
	case '|':
		v = fabs(eval(pp, a->l)); break;
	case 'M':
		v = -eval(pp, a->l);
		break;
	case '=': v = ((struct symasgn *)a)->s->value =
		eval(pp, ((struct symasgn *)a)->v); break;

	case '1': v = (eval(pp, a->l) > eval(pp, a->r)) ? 1 : 0; break;
	case '2': v = (eval(pp, a->l) < eval(pp, a->r)) ? 1 : 0; break;
	case '3': v = (eval(pp, a->l) != eval(pp, a->r)) ? 1 : 0; break;
	case '4': v = (eval(pp, a->l) == eval(pp, a->r)) ? 1 : 0; break;
	case '5': v = (eval(pp, a->l) >= eval(pp, a->r)) ? 1 : 0; break;
	case '6': v = (eval(pp, a->l) <= eval(pp, a->r)) ? 1 : 0; break;
	case 'I':
	{
		struct flow* f = (struct flow*)a;
		if (eval(pp, f->cond) != 0) {
			if (f->tl) {
				v = eval(pp, f->tl);
			}
			else {
				v = 0;
			}
		}
		else {
			if (f->el) {
				v = eval(pp, f->el);
			}
			else {
				v = 0;
			}
		}
		break;
	}
	case 'E':
	{
		struct flow* f = (struct flow*)a;
		if (f->cond == NULL || eval(pp, f->cond)) {
			if (f->tl) {
				v = eval(pp, f->tl);
			}
			else {
				v = 0;
			}
		}
		else {
			if (f->el) {
				v = eval(pp, f->el);
			}
			else {
				v = 0;
			}

		}
		break;
	}
	case 'W':
	{
		struct flow *f = (struct flow*)a;
		if (f->tl) {
			while (eval(pp, f->cond) != 0) {
				v = eval(pp, f->tl);
			}
		}
		break;
	}
	case 'L':
		eval(pp, a->l); v = eval(pp, a->r); break;
	case 'F':
		v = callbuiltin(pp, (struct fncall *)a); 
		break;
	case 'C':
		v = calluser(pp, (struct ufncall*)a);
		break;
	default:
		printf("internal error bad node %c \n", a->nodetype);
	}
	return v;
}


void treefree(struct pcdata *pp, struct ast *a) {
	switch (a->nodetype)
	{
	case '+':
	case '-':
	case '*':
	case '/':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case 'L':
		treefree(pp,a->r);
	case '|':
	case 'M': case 'C': case 'F': case 'E':
		treefree(pp, a->l);
	case 'K': case 'N':
		break;
	case '=':
		free( ((struct symasgn*)a)->v);
		break;
	case 'I': case 'W':
		treefree(pp, ((struct flow *)a)->cond);
		if (((struct flow*)a)->tl) {
			treefree(pp,  ((struct flow*)a)->tl);
		}
		if (((struct flow*)a)->el) {
			treefree(pp, ((struct flow*)a)->el);
		}
		break;
	default:
		printf("internal error: free bad node %c\n", a->nodetype);
	}

	free(a);
}



static unsigned symhash(char *sym) {
	unsigned int hash = 0;
	unsigned c;
	while (c = *sym++) hash = hash * 9 ^ c;
	return hash;
}

struct symbol* lookup(struct pcdata *pp, char *sym) {
	struct symbol *sp = &(pp->symtab)[symhash(sym)%NHASH];
	int scount = NHASH;
	while (--scount >= 0) {
		if (sp->name && !strcmp(sp->name, sym)) {
			return sp;
		}
		if (!sp->name) {
			sp->name = strdup(sym);
			sp->value = 0;
			sp->func = NULL;
			sp->syms = NULL;
			return sp;
		}
		if (++sp >= (pp->symtab + NHASH)) sp = pp->symtab;
	}
	yyerror(pp, "symbol table overflow\n");
	abort();

}

struct ast* newcmp(struct pcdata *pp, int cmptype, struct ast *l, struct ast *r) {
	struct ast *a = malloc(sizeof(struct ast));
	if (!a) {
		yyerror(pp, "create new cmp error");
		abort();
	}
	a->nodetype = '0' + cmptype;
	a->l = l;
	a->r = r;
	return a;
}
struct symlist *newsymlist(struct pcdata *pp, struct symbol *sym, struct symlist *next) {
	struct symlist *s = malloc(sizeof(struct symlist));
	if (!s) {
		yyerror(pp, "create new symlist error");
		abort();
	}
	s->sym = sym;
	s->next = next;
	return s;
}	

struct ast *newfunc(struct pcdata *pp, int functype, struct ast *l) {
	struct fncall *f = malloc(sizeof(struct fncall));
	if (!f) {	
		yyerror(pp, "create new func error");
		abort();
	}
	f->nodetype = 'F';
	f->l = l;
	f->functype = functype;
	return (struct ast*)f;
}

struct ast *newcall(struct pcdata *pp, struct symbol *s, struct ast *l) {
	struct ufncall *f = malloc(sizeof(struct ufncall));
	if (!f) {
		yyerror(pp, "create new func error");
		abort();
	}
	f->nodetype = 'C';
	f->s = s;
	f->l = l;
	return (struct ast*)f;
}

struct ast *newref(struct pcdata *pp, struct symbol *s) {
	struct symref *ref = malloc(sizeof(struct symref));
	if (!ref) {
		yyerror(pp, "create new ref error");
		abort();
	}
	ref->nodetype = 'N';
	ref->s = s;
	return (struct ast*)ref;
}

struct ast *newasgn(struct pcdata *pp, struct symbol *s, struct ast *v) {
	struct symasgn *a = malloc(sizeof(struct symasgn));
	if (!a) {
		yyerror(pp, "create asgn error");
		exit(0);
	}
	a->nodetype = '=';
	a->s = s;
	a->v = v;
	return (struct ast*)a;
}

struct ast *newflow(struct pcdata *pp, int nodetype, struct ast *cond, struct ast* tl, struct ast *el) {
	struct flow *f = malloc(sizeof(struct flow));
	if (!f) {
		yyerror(pp, "create flow error");
		exit(0);
	}
	f->nodetype = nodetype;
	f->cond = cond;
	f->tl = tl;
	f->el = el;
	return (struct ast*)f;
}

void symlistfree(struct pcdata *pp, struct symlist *sl) {
	struct symlist *nsl;
	while (sl) {
		nsl = sl->next;
		free(sl);
		sl = nsl;
	}
}

void dodef(struct pcdata *pp, struct symbol *name, struct symlist *syms, struct ast *func)
{
	if (name->syms) {
		symlistfree(pp, name->syms);
	}
	if (name->func) {
		treefree(pp, name->func);
	}
	name->syms = syms;
	name->func = func;
}

static double callbuiltin(struct pcdata *pp, struct fncall *f)
{
	enum bifs functype = f->functype;
	double v = eval(pp, f->l);
	switch (functype) {
	case B_sqrt:
		return sqrt(v);
	case B_exp:
		return exp(v);
	case B_log:
		return log(v);
	case B_print:
		printf("= %4.4g\n", v);
		return v;
	default:
		yyerror(pp, "Unknow built-in function %d", functype);
		return 0.0;
	}
}

static double calluser(struct pcdata *pp, struct ufncall *f) {
	struct symbol *fn = f->s;	/* function name */
	struct symlist *sl;		/* dummy arguments */
	struct ast *args = f->l;	/* actual arguments */
	double *oldval, *newval;	/* saved arg values */
	double v;
	int nargs;
	int i;

	if (!fn->func) {
		yyerror(pp, "call to undefined function", fn->name);
		return 0;
	}

	/* count the arguments */
	sl = fn->syms;
	for (nargs = 0; sl; sl = sl->next)
		nargs++;

	/* prepare to save them */
	oldval = (double *)malloc(nargs * sizeof(double));
	newval = (double *)malloc(nargs * sizeof(double));
	if (!oldval || !newval) {
		yyerror(pp, "Out of space in %s", fn->name); return 0.0;
	}

	/* evaluate the arguments */
	for (i = 0; i < nargs; i++) {
		if (!args) {
			yyerror(pp, "too few args in call to %s", fn->name);
			free(oldval); free(newval);
			return 0;
		}

		if (args->nodetype == 'L') {	/* if this is a list node */
			newval[i] = eval(pp, args->l);
			args = args->r;
		}
		else {			/* if it's the end of the list */
			newval[i] = eval(pp, args);
			args = NULL;
		}
	}

	/* save old values of dummies, assign new ones */
	sl = fn->syms;
	for (i = 0; i < nargs; i++) {
		struct symbol *s = sl->sym;

		oldval[i] = s->value;
		s->value = newval[i];
		sl = sl->next;
	}

	free(newval);

	/* evaluate the function */
	v = eval(pp, fn->func);

	/* put the dummies back */
	sl = fn->syms;
	for (i = 0; i < nargs; i++) {
		struct symbol *s = sl->sym;

		s->value = oldval[i];
		sl = sl->next;
	}

	free(oldval);
	return v;
}