#ifdef SMT_Z3
#include  "z3.h" 
#else
#ifdef SMT_YICES
#include "yices_c.h"
#else
#error "Please define an SMT solver !"
#endif
#endif


#ifdef SMT_Z3

typedef Z3_ast		smt_formula_t;
typedef Z3_context	smt_context_t;
typedef Z3_lbool	smt_bool_t;

#define SMT_TRUE	Z3_L_TRUE
#define SMT_FALSE	Z3_L_FALSE
#define SMT_UNDEF	Z3_L_UNDEF

Z3_context	smt_mk_context();
Z3_ast		smt_mk_int_var(Z3_context, char *);
Z3_ast		smt_mk_int(Z3_context, int);
Z3_ast		smt_not(Z3_context, Z3_ast);
void		smt_insert_formula(Z3_context, Z3_ast);
Z3_lbool	smt_check_formula(Z3_context);
Z3_context	smt_reset_context(Z3_context);
Z3_ast		smt_mk_add(Z3_context, int, Z3_ast[]);  
Z3_ast		smt_mk_sub(Z3_context, int, Z3_ast[]);  
Z3_ast		smt_mk_mul(Z3_context, int, Z3_ast[]);  
Z3_ast		smt_mk_le(Z3_context, Z3_ast, Z3_ast);  
Z3_ast		smt_mk_lt(Z3_context, Z3_ast, Z3_ast);  
Z3_ast		smt_mk_ge(Z3_context, Z3_ast, Z3_ast);  
Z3_ast		smt_mk_gt(Z3_context, Z3_ast, Z3_ast);  
Z3_ast		smt_mk_eq(Z3_context, Z3_ast, Z3_ast);  
Z3_ast		smt_mk_not(Z3_context, Z3_ast);
Z3_ast		smt_mk_and(Z3_context, int, Z3_ast[]);
Z3_ast		smt_mk_or(Z3_context, int, Z3_ast[]);
void		smt_pp_expr(Z3_context, Z3_ast);

#else
#ifdef SMT_YICES

typedef yices_expr	smt_formula_t;
typedef yices_context	smt_context_t;
typedef lbool		smt_bool_t;

#define SMT_TRUE	l_true
#define SMT_FALSE	l_false
#define SMT_UNDEF	l_undef

yices_context	smt_mk_context();
yices_expr	smt_mk_int_var(yices_context, char *);
yices_expr	smt_mk_int(yices_context, int);
yices_expr	smt_not(yices_context, yices_expr);
void		smt_insert_formula(yices_context, yices_expr);
lbool		smt_check_formula(yices_context);
yices_context	smt_reset_context(yices_context);
yices_expr	smt_mk_add(yices_context, int, yices_expr[]);  
yices_expr	smt_mk_sub(yices_context, int, yices_expr[]);  
yices_expr	smt_mk_mul(yices_context, int, yices_expr[]);  
yices_expr	smt_mk_le(yices_context, yices_expr, yices_expr);  
yices_expr	smt_mk_lt(yices_context, yices_expr, yices_expr);  
yices_expr	smt_mk_ge(yices_context, yices_expr, yices_expr);  
yices_expr	smt_mk_gt(yices_context, yices_expr, yices_expr);  
yices_expr	smt_mk_eq(yices_context, yices_expr, yices_expr);  
yices_expr	smt_mk_not(yices_context, yices_expr);
yices_expr	smt_mk_and(yices_context, int, yices_expr[]);
yices_expr	smt_mk_or(yices_context, int, yices_expr[]);
void		smt_pp_expr(yices_context, yices_expr);

#endif
#endif
