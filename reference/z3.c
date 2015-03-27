#include  "SMT.h"

Z3_ast smt_mk_var(Z3_context ctx, const char * name, Z3_sort ty) 
{
     Z3_symbol   s  = Z3_mk_string_symbol(ctx, name);
     return Z3_mk_const(ctx, s, ty);
}

Z3_ast smt_mk_int_var(Z3_context ctx, char * name) 
{
    Z3_sort ty = Z3_mk_int_sort(ctx);
    return smt_mk_var(ctx, name, ty);
}

void error_handler(Z3_context c, Z3_error_code e) 
{
     printf("Error code: %d\n", e);
//     exit(-1);
//incorrect use of Z3");
}

Z3_context mk_context_custom(Z3_config cfg, Z3_error_handler err) 
{
     Z3_context ctx;
    
     Z3_set_param_value(cfg, "MODEL", "true");
     ctx = Z3_mk_context(cfg);
     Z3_set_error_handler(ctx, err);
     
     return ctx;
}

Z3_context smt_mk_context() 
{
     Z3_config  cfg;
     Z3_context ctx;
     cfg = Z3_mk_config();
     ctx = mk_context_custom(cfg, error_handler);
     Z3_del_config(cfg);
     return ctx;
}

Z3_ast smt_mk_int(Z3_context ctx, int v) 
{
     Z3_sort ty = Z3_mk_int_sort(ctx);
     return Z3_mk_int(ctx, v, ty);
}

Z3_ast smt_mk_not(Z3_context ctx, Z3_ast f) {
     return Z3_mk_not(ctx, f);
}

Z3_ast smt_mk_and(Z3_context ctx, int c, Z3_ast args[]) {
     return Z3_mk_and(ctx, c, args);
}

Z3_ast smt_mk_or(Z3_context ctx, int c, Z3_ast args[]) {
     return Z3_mk_or(ctx, c, args);
}

void smt_insert_formula(Z3_context ctx, Z3_ast f) {
     Z3_assert_cnstr(ctx, f);
}

Z3_lbool smt_check_formula(Z3_context ctx) {
     return Z3_check(ctx);
}

Z3_context smt_reset_context(Z3_context ctx) {
     Z3_del_context(ctx);
     return smt_mk_context();
}

Z3_ast smt_mk_add(Z3_context ctx, int c, Z3_ast args[]) {
	return  Z3_mk_add(ctx, c, args);
}

Z3_ast smt_mk_sub(Z3_context ctx, int c, Z3_ast args[]) {
	return  Z3_mk_sub(ctx, c, args);
}

Z3_ast smt_mk_mul(Z3_context ctx, int c, Z3_ast args[]) {
	return  Z3_mk_mul(ctx, c, args);
}

Z3_ast smt_mk_le(Z3_context ctx, Z3_ast f1, Z3_ast f2) {
	return  Z3_mk_le(ctx, f1, f2);
}

Z3_ast smt_mk_lt(Z3_context ctx, Z3_ast f1, Z3_ast f2) {
	return  Z3_mk_lt(ctx, f1, f2);
}

Z3_ast smt_mk_ge(Z3_context ctx, Z3_ast f1, Z3_ast f2) {
	return  Z3_mk_ge(ctx, f1, f2);
}

Z3_ast smt_mk_gt(Z3_context ctx, Z3_ast f1, Z3_ast f2) {
	return  Z3_mk_gt(ctx, f1, f2);
}

Z3_ast smt_mk_eq(Z3_context ctx, Z3_ast f1, Z3_ast f2) {
	return  Z3_mk_eq(ctx, f1, f2);
}

void	smt_pp_expr(Z3_context ctx, Z3_ast e) {
	printf("%s\n", Z3_ast_to_string(ctx, e));
}
