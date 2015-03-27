#include  "SMT.h"

yices_expr smt_mk_int_var(yices_context ctx, char *name) 
{
	yices_var_decl	vdecl;
	yices_expr	v;
	yices_type	ty;

	vdecl = yices_get_var_decl_from_name (ctx, name);
	if (vdecl == 0) {
		ty = yices_mk_type(ctx, "int");
		vdecl = yices_mk_var_decl(ctx, name, ty);
	}
	v  = yices_mk_var_from_decl(ctx, vdecl);
	return v;
}

yices_context smt_mk_context() 
{
	return yices_mk_context();
}

yices_expr smt_mk_int(yices_context ctx, int v) 
{
	return yices_mk_num(ctx, v);
}

yices_expr smt_mk_not(yices_context ctx, yices_expr f) {
	return yices_mk_not(ctx, f);
}

yices_expr smt_mk_and(yices_context ctx, int c, yices_expr args[]) {
	return yices_mk_and(ctx, args, c);
}

yices_expr smt_mk_or(yices_context ctx, int c, yices_expr args[]) {
	return yices_mk_or(ctx, args, c);
}

void smt_insert_formula(yices_context ctx, yices_expr f) {
	yices_assert(ctx, f);
}

lbool smt_check_formula(yices_context ctx) {
	return yices_check(ctx);
}

yices_context smt_reset_context(yices_context ctx) {
	yices_reset(ctx);
	return ctx;
}

yices_expr smt_mk_add(yices_context ctx, int c, yices_expr args[]) {
	return  yices_mk_sum(ctx, args, c);
}

yices_expr smt_mk_sub(yices_context ctx, int c, yices_expr args[]) {
	return  yices_mk_sub(ctx, args, c);
}

yices_expr smt_mk_mul(yices_context ctx, int c, yices_expr args[]) {
	return  yices_mk_mul(ctx, args, c);
}

yices_expr smt_mk_le(yices_context ctx, yices_expr f1, yices_expr f2) {
	return  yices_mk_le(ctx, f1, f2);
}

yices_expr smt_mk_lt(yices_context ctx, yices_expr f1, yices_expr f2) {
	return  yices_mk_lt(ctx, f1, f2);
}

yices_expr smt_mk_ge(yices_context ctx, yices_expr f1, yices_expr f2) {
	return  yices_mk_ge(ctx, f1, f2);
}

yices_expr smt_mk_gt(yices_context ctx, yices_expr f1, yices_expr f2) {
	return  yices_mk_gt(ctx, f1, f2);
}

yices_expr smt_mk_eq(yices_context ctx, yices_expr f1, yices_expr f2) {
	return  yices_mk_eq(ctx, f1, f2);
}

void	smt_pp_expr(yices_context ctx, yices_expr e) {
	yices_pp_expr(e);
}
