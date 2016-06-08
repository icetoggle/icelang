#pragma once
extern int yylineno;
void yyerror(char *s, ...);

struct ast {
	int nodetype;
	struct ast *l;
	struct ast *r;
};

struct  numval {
	int nodetype;
	double number;
};

struct symbol {
	char *name;
	double value;
	struct ast *func;
	struct symlist *syms;
};

#define NHASH 9997
struct symbol symtab[NHASH];
struct symbol *lookup(char*);

struct symlist {
	struct symbol *sym;
	struct symlist *next;
};

struct symlist *newsymlist(struct symbol *sym, struct symlist *next);
void symlistfree(struct symlist *sl);


double eval(struct ast *);
void treefree(struct ast *);


enum bifs {
	B_sqrt = 1,
	B_exp,
	B_log,
	B_print
};

struct fncall {
	int nodetype;
	struct ast *l;
	enum bifs functype;
};

struct ufncall {
	int nodetype;
	struct ast *l;
	struct symbol *s;
};

struct flow {
	int nodetype;
	struct ast *cond;
	struct ast *tl;
	struct ast *el;
};

struct numval {
	int nodetype;
	double number;
};

struct symref {
	int nodetype;
	struct symbol *s;
};

struct symasgn {
	int nodetype;
	struct symbol *s;
	struct ast *v;
};
struct ast *newast(int nodetype, struct ast *l, struct ast *r);
struct ast *newnum(double d);
struct ast *newcmp(int cmptype, struct ast *l, struct ast *r);
struct ast *newfunc(int functype, struct ast *l);
struct ast *newcall(struct symbol *s, struct ast *l);
struct ast *newref(struct symbol *s);
struct ast *newasgn(struct symbol *s, struct ast *v);
struct ast *newflow(int nodetype, struct ast *cond, struct ast* tl, struct ast *tr);
