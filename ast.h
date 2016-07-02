#pragma once
typedef void *yyscan_t;
extern int yylineno;
extern int debug;
void yyerror(struct pcdata *, char *s, ...);

/*·ÖÎöÊı¾İ*/
struct pcdata {
	yyscan_t scaninfo;
	struct symbol *symtab;
	struct ast* ast;
};

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
struct symbol *lookup(struct pcdata *,char*);

struct symlist {
	struct symbol *sym;
	struct symlist *next;
};



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

struct symref {
	int nodetype;
	struct symbol *s;
};

struct symasgn {
	int nodetype;
	struct symbol *s;
	struct ast *v;
};
struct symlist *newsymlist(struct pcdata *, struct symbol *sym, struct symlist *next);
void symlistfree(struct pcdata *, struct symlist *sl);
double eval(struct pcdata *, struct ast *);
void treefree(struct pcdata *, struct ast *);
struct ast *newast(struct pcdata *, int nodetype, struct ast *l, struct ast *r);
struct ast *newnum(struct pcdata *, double d);
struct ast *newcmp(struct pcdata *, int cmptype, struct ast *l, struct ast *r);
struct ast *newfunc(struct pcdata *, int functype, struct ast *l);
struct ast *newcall(struct pcdata *, struct symbol *s, struct ast *l);
struct ast *newref(struct pcdata *, struct symbol *s);
struct ast *newasgn(struct pcdata *, struct symbol *s, struct ast *v);
struct ast *newflow(struct pcdata *, int nodetype, struct ast *cond, struct ast* tl, struct ast *tr);
void dodef(struct pcdata *, struct symbol *name, struct symlist *syms, struct ast *func);