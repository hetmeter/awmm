#include  "SMT.h"
#include  "stdio.h"
#include  "string.h"
#include  "assert.h"
//#include  "malloc.h"
#include "stdlib.h"
#include  "sys/time.h"
// == caching ==
#include <glib.h>

#define MAX_LHS			30
#define MAX_LABEL		10
#define MAX_RHS			100
#define MAX_STMT		600
#define MAX_STMTS_ARRAYS	3
#define MAX_SIMPLE_PRED_COUNT	70
#define MAX_COMPOSED_PRED_COUNT	2800
#define MAX_LINE 		200
#define MAX_PRED_SIZE	100
#define MAX_DISJUNCTS	((2 * MAX_SIMPLE_PRED_COUNT) + MAX_COMPOSED_PRED_COUNT)
#define MAX_CONTEXTS	100
#define MAX_CUBESTRING_SIZE	1000000
#define MAX_VAR_COUNT 	100
#define MAX_VAR_SIZE	MAX_LHS
#define MAX_NR_PROC		2
#define MAX_BUFFER_SIZE	5

#define MAX_OPERANDS_EXPR       100

/* == activates composed predicates cubesize 1 mode == */
int extrapolate_flag;

int max_cubes;

#define MAX_TOTAL_PRED_COUNT ((extrapolate_flag) ? (MAX_COMPOSED_PRED_COUNT + MAX_SIMPLE_PRED_COUNT) : MAX_SIMPLE_PRED_COUNT)

#define ACTUAL_PRED_COUNT ((extrapolate_flag) ? (composed_pred_count + simple_pred_count) : simple_pred_count)

#define IFVAR		"ifvar"
#define ATOMIC_IFVAR	"at_ifvar"

#define TSO 1
#define PSO 2

#define FALSE_PREDICATE ((char *) "(1000 = 10000)")

#define MAX_GLOBAL_VARS 10

enum printType	{CUBE_PRINT_MODE = 1, TEMP_BOOL_PRINT_MODE, GLOBAL_BOOL_PRINT_MODE};

enum stmtType	{GOTO = 1, IF_CONDITIONAL, ELSE_CONDITIONAL, ENDIF_CONDITIONAL,
	STORE, LOAD, LOCAL, NOPST, NEW_PROCESS, BEG_INIT, END_INIT,
	ASSERT, FENCE, FLUSH, BEG_ATOMIC, END_ATOMIC, COMMENT,
	IF_CONDITIONAL_STAR, ELSE_CONDITIONAL_STAR, ENDIF_CONDITIONAL_STAR, ASSUME};
enum opType		{NOP = 1, ADD, SUB, MUL, LE, LT, GE, GT, EQ, DISEQ, AND, OR};
enum predType	{IGNORE = 0, POS, NEG};

typedef struct list {
	char *val;
	struct list *next;	
} *list_t;

list_t createElem(char *v) {
	list_t aux;

	aux = (list_t) malloc(sizeof(struct list));
	aux->val = v;
	aux->next = NULL; 
	return aux;
}

typedef struct queue{
	list_t head, tail;
} *queue_t;

queue_t createQueue() {
	queue_t aux;
	aux = (queue_t) malloc(sizeof(struct queue));
	aux->head = NULL;
	aux->tail = NULL;
	return aux;
}

void disposeElem(list_t *l) {
	list_t aux;

	aux = *l;
	free(aux->val);
	free(*l);
}

void disposeQ(queue_t *q) {
	assert((*q)->head == NULL);
	assert((*q)->tail == NULL);
	free(*q);
}

void enqueue(list_t e, queue_t q) {
	assert(e != NULL);
	if (q->tail == NULL) {
		assert(q->head == NULL);
		q->head = e;
		q->tail = e;
		assert(q->tail->next == NULL);
		assert(q->head->next == NULL);
	} else {
		assert(q->head != NULL);
		q->tail->next = e;
		q->tail = e;
	}
}

list_t dequeue(queue_t q) {
	list_t aux;
	if (q->head == NULL) {
		assert(q->tail == NULL);
		return NULL;
	} else if (q->head == q->tail) {
		assert(q->head->next == NULL);
		assert(q->tail->next == NULL);
		aux = q->head;
		aux->next = NULL;
		q->head = NULL;
		q->tail = NULL;
		return aux;
	} else {
		aux = q->head;
		q->head = q->head->next;
		aux->next = NULL;
		return aux;
	}
	assert(0);
	return NULL;
}

struct stmt {
	enum stmtType type;

	char  	label[MAX_LABEL];
	char  	lhs[MAX_LHS];
	char  	rhs[MAX_RHS];
	char  	condition[MAX_PRED_SIZE];

	char  	wlp[MAX_SIMPLE_PRED_COUNT][MAX_PRED_SIZE];

	/* the if syntactic scope that owns this statement. */
	int context;
	int context_depth;

	/* some labels for printing the program. */
	int elselabel;
	int endlabel;
	int ifvar_idx;

	/* process id */
	int pid;

	/* prime implicants of Predicate */
	enum predType	***cubes_pos;//[MAX_SIMPLE_PRED_COUNT][MAX_DISJUNCTS][MAX_TOTAL_PRED_COUNT];
	int			cubes_pos_count[MAX_SIMPLE_PRED_COUNT];

	/* Negation of the predicate */
	enum predType	***cubes_neg;//[MAX_SIMPLE_PRED_COUNT][MAX_DISJUNCTS][MAX_TOTAL_PRED_COUNT];
	int			cubes_neg_count[MAX_SIMPLE_PRED_COUNT];
};

typedef struct stmt stmt_t;

enum predType	***shared_cubes_pos, ***shared_cubes_neg;
int *shared_cubes_pos_count, *shared_cubes_neg_count;

stmt_t 		*stmts, *stmts2, *stmts3, *stmts4, *stmts5,
*stmts6, *stmts7, *stmts8, *stmts9, *stmts10;

char		preds[MAX_SIMPLE_PRED_COUNT][MAX_PRED_SIZE];
char		composed_preds[MAX_COMPOSED_PRED_COUNT][MAX_PRED_SIZE];

/* == each line i indicates which simple preds are found in the composed predicate i == */
enum predType	pred_components[MAX_COMPOSED_PRED_COUNT][MAX_SIMPLE_PRED_COUNT];

char 		vars[MAX_VAR_COUNT][MAX_VAR_SIZE];
char 		var_preds[MAX_VAR_COUNT][MAX_SIMPLE_PRED_COUNT];
char		pred_preds[MAX_SIMPLE_PRED_COUNT][MAX_SIMPLE_PRED_COUNT];
int			nr_preds_var[MAX_VAR_COUNT];

/* == how many global variables have buffered stores == */
int 	nr_weak_global_vars;

int 	var_count = 0;
int 	stmt_count = 0;
int 	simple_pred_count = 0;
int 	composed_pred_count = 0;

int 	current_pid;

char 	*global_vars[MAX_GLOBAL_VARS];
char	*global_vars_modified[MAX_NR_PROC][MAX_GLOBAL_VARS];

int 	global_var_count = 0;
int		global_var_modified_count[MAX_NR_PROC];

int 	context_stack[MAX_CONTEXTS];
int 	context_top = 0;
int 	max_depth = 0;

int 	beg_label;

/* == counters for ifvars (unique and shared) == */
int		ifvar_count = 0;
int 	atomic_ifvar_count = 0;

char 	*assume_disjunct;

/* == no label == */
char 	*no_lbl = (char *) "";

/* == true if nondeterministic flush sequence should be added == */
int 	add_random_flushes = 0;

/* == size of buffer used for writes to PSO shared variables == */
int 	k_buffer_size = 1;

queue_t invalid_cubes = NULL;

GHashTable *extrapolated_preds = NULL;


GHashTable *used_cubes_choose = NULL;
int SMT_0 = 0;
int SMT_1 = 1;

int SMT_stats[MAX_STMT][MAX_SIMPLE_PRED_COUNT];
int cache_hits[MAX_STMT][MAX_SIMPLE_PRED_COUNT];
int SMT_total_calls;

int *SMT_per_stmt;

smt_context_t SMT_ctx;

smt_formula_t buildExpression(char *pred, int *count);

int isCharacter(char c);
int isDigit(char c);

char* getComposedPredCube(enum predType cube[], enum printType pt);
char* getDisjunct(enum predType **cubes,
		int disjunct_count,
		enum printType pt);

void printStats(int stmt_idx, int pred_idx);

/* max source program label */
int max_label = 0;

int isCharacter(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_');
}

int isDigit(char c) {
	return (c >= '0' && c <= '9');
}

int isIdentifier(char *pred) {

	int i;

	if (!isCharacter(pred[0]))
		return 0;

	for (i = 1; i < strlen(pred); i += 1) {
		int c = pred[i];
		if (isCharacter(c) || isDigit(c))
			continue;	
		else
			break;
	}
	return i;
}

int isNumber(char *pred) {

	int i;

	if (!isDigit(pred[0]))
		return 0;

	for (i = 1; i < strlen(pred); i += 1) {
		int d = pred[i];
		if (isDigit(d))
			continue;	
		else if (isCharacter(d)) {
			printf("%s\n", pred);
			assert(0 && "chars cannot come right after digits");
		}
		break;
	}
	return i;
} 

/* remove trailing spaces and ";"'s from both left and right sides. */
void removeTrailingSpaces(char *str) {

	char  *temp;
	int   i, l, start, end;

	l = strlen(str);
	temp = (char *) malloc(l + 1);
	strcpy(temp, str);

	start = -1;
	end = -1;

	for (i = 0; i < l; i += 1) {
		if (str[i] != ' ') {
			start = i;
			break;
		}
	}

	for (i = l - 1; i >= 0; i -= 1) {
		if (str[i] != ' ' && str[i] != ';' && str[i] != '\n') {
			end = i;
			break;
		}
	}

	assert(start != -1 && end != -1);
	strncpy(str, &temp[start], end - start + 1);
	str[end - start + 1] = '\0';
	free(temp);
}

char *findFirstBracket(char *stmt) {

	int  i;

	for (i = 0; i < strlen(stmt); i += 1)
		if (stmt[i] == '(')
			return &stmt[i];
	return 0;
}

char *findLastBracket(char *stmt) {

	int  i;

	for (i = (strlen(stmt) - 1); i >= 0; i -= 1)
		if (stmt[i] == ')')
			return &stmt[i];
	return 0;
}

int arrayStrSearch(char **array, int arr_size, char *pattern) {
	int i;

	for(i = 0; i < arr_size; i++) {
		if(strcmp(array[i], pattern) == 0) {
			return i;
		}
	}
	return -1;
}

stmt_t* getStmt(int idx) {
	int arr_div, arr_mod;


	arr_div = idx / MAX_STMT;
	assert(arr_div < MAX_STMTS_ARRAYS);

	arr_mod = idx % MAX_STMT;

	switch(arr_div) {
	case 0 : return stmts + arr_mod;
	case 1 : return stmts2 + arr_mod;
	case 2 : return stmts3 + arr_mod;
	case 3 : return stmts4 + arr_mod;
	case 4 : return stmts5 + arr_mod;
	case 5 : return stmts6 + arr_mod;
	case 6 : return stmts7 + arr_mod;
	case 7 : return stmts8 + arr_mod;
	case 8 : return stmts9 + arr_mod;
	case 9 : return stmts10 + arr_mod;
	}

}

/* 1 if var is in pred */
int contains(char* pred, char *var) {
	char* chp;
	char* start;

	start =  pred;

	while ((chp = strstr(pred, var)) != NULL){

		int s = chp - start;
		int e = s + strlen(var);

		if (s > 0) {
			if (isDigit(pred[s-1]) || isCharacter(pred[s-1])) {
				pred = chp + 1;
				continue;
			}
		}
		if (e < strlen(pred)) {
			if (isDigit(pred[e]) || isCharacter(pred[e])) {
				pred = chp + 1;
				continue;
			}
		}

		return chp - start;
	}
	//printf("pred: %s, var: %s => 0\n", pred, var);
	return -1;
}

/* == returns the number of predicates in the cube == */
int cubeLength(list_t q) {
	int cnt, i;

	cnt = 0;
	for (i = 0; i < simple_pred_count; i++) {
		if (q->val[i] != 'i') {
			cnt++;
		}
	}
	return cnt;
}

int getUniqueLabel() {
	beg_label++;
	return beg_label;
}

int getUniqueIfvar(void) {
	ifvar_count++;
	return ifvar_count;
}

int getGlobalVarIdx(char*var) {
	int i;
	for (i = 0; i < global_var_count; i++) {
		if (strcmp(var, global_vars[i]) == 0) {
			return i;
		}
	}
	return -1;
}

void assertNoCarriadgeReturns(char *pred) {

	int i;
	for (i = 0 ; i < strlen(pred); i += 1) 
		assert(pred[i] != '\r');
}

/* compute wlp(assignStmt, pred). */
void computeWlpAssignment(stmt_t *assignStmt, int predIdx) {

	char 	*wlp;
	char 	*lhs;
	char 	*rhs;
	char 	*pred;
	int  	i, j, pos;

	wlp = (assignStmt->wlp)[predIdx];
	lhs = assignStmt->lhs;
	rhs = assignStmt->rhs;

	pred = preds[predIdx];

	j = 0;

	pos = contains(pred, lhs);
	if (pos == -1) {
		strcpy(wlp, pred);
		return;
	}

	for (i = 0; i < strlen(pred); ) {
		if (i == pos) {
			strcat(&wlp[j], "(");
			j += 1;

			strcat(&wlp[j], rhs);
			j = j + strlen(rhs);

			strcat(&wlp[j], ")");
			j += 1;

			pos = contains(pred + pos + strlen(lhs), lhs);
			i = i + strlen(lhs);
		}
		else {
			wlp[j] = pred[i];
			j += 1;
			i += 1;
		}
	}

	//printf("IN: %s = %s, PRED: %s, OUT %s \n", lhs, rhs, pred, wlp);
}

smt_formula_t buildConjunction(char charmap[], int charmap_count) {

	smt_formula_t args[MAX_SIMPLE_PRED_COUNT], e;
	int dummy, c, i;

	dummy = c = 0;

	/* Build Conjunction. */
	for (i = 0; i < charmap_count; i += 1) {

		if (charmap[i] == 'i') 
			continue;

		if (i < simple_pred_count) {
			e = buildExpression(preds[i], &dummy);
		} else {
			/* == in case c2bp' is active == */
			assert(extrapolate_flag);
			//printf("composed_pred[%d] = %s\n", (i - pred_count), composed_preds[i - pred_count]);
			e = buildExpression(composed_preds[i - simple_pred_count], &dummy);
		}

		if (charmap[i] == '-') 
			e = smt_mk_not(SMT_ctx, e);
		else
			assert(charmap[i] == '+');

		args[c++] = e;
	}

	if (c > 1)
		e = smt_mk_and(SMT_ctx, c, args);
	else
		e = args[0];

	return e;
}

char *buildKey(char *charmap, int charmap_count, int polarity, char *Qs){

	char *key;

	key = (char *) malloc(charmap_count + 1 + strlen(Qs) + 1);

	memcpy(key, charmap, charmap_count);
	key[charmap_count] = (polarity == 1)?'p':'n';
	memcpy(key + charmap_count + 1, Qs, strlen(Qs) + 1);
	//printf("key: %s\n", key);
	return key;
}

char *buildCubeKey(char *charmap, int charmap_count){

	char *key;
	int i, last_not_ignored, cube_size;

	cube_size = 0;
	last_not_ignored = 0;
	key = (char *) malloc(charmap_count + 1);
	for (i = 0; i < charmap_count; i++) {
		if (charmap[i] != 'i') {
			cube_size++;
			last_not_ignored = i;
		}
	}

	memcpy(key, charmap, charmap_count);
	if (cube_size == 1) {
		/* == to avoid printing a unit cube and its negation == */
		key[last_not_ignored] = '+';
	}
	key[charmap_count] = '\0';
	return key;
}


/* if polarity is  1: answer if P =>  Q is a valid formula where P is not false.
   if polarity is -1: answer if P => !Q is a valid formula where P is not false.
 */
int isImplicationValid(char charmap[], int charmap_count, int polarity, char *Qs, int stmt_idx, int pred_idx) {

	smt_formula_t Q, P, F, args[2];
	smt_bool_t sat;
	int dummy, i;
	//char *key;
	/* == caching == */
	//gpointer value;

	assert(polarity > 0 || polarity < 0);

	/* == check hashmap for cache == */
	/*key = buildKey(charmap, charmap_count, polarity, Qs);
	if ((value = g_hash_table_lookup(used_cubes_choose, key)) != NULL){
		free(key);
		dummy = *((int *)value);
		assert ((dummy == 0) || (dummy == 1));
		cache_hits[stmt_idx][pred_idx]++;
		return dummy;
	}*/
	/* == cache not hit == */

	/* need to build it again after reset (reset does not seem to reset as one would expect). */

	P = buildConjunction(charmap, charmap_count);
	//printf(" ==== \n");
	//smt_pp_expr(SMT_ctx, P);
	//printf("\n");
	/* build Q. For negative polarity, negate the Q. */
	Q = buildExpression(Qs, &dummy);

	if (polarity < 0)
		Q = smt_mk_not(SMT_ctx, Q);

	//printf("\n");

	/* build implication (P => Q), i.e. (not P or Q) and check it.*/
	P = smt_mk_not(SMT_ctx, P);
	args[0] = P;
	args[1] = Q;
	F = smt_mk_or(SMT_ctx, 2, args);

	/* take negation of formula so to check for satisfiability: F valid <=> F unsatisfiable  */
	F = smt_mk_not(SMT_ctx, F);

	/* insert formula into context. */
	smt_insert_formula(SMT_ctx, F);

	/* check for satisfiability. */

	sat = smt_check_formula(SMT_ctx);

	SMT_total_calls++;

	/* reset context. */
	SMT_ctx = smt_reset_context(SMT_ctx);
	//SMT_ctx = smt_mk_context();

	if (sat == SMT_TRUE) {
		/* == caching == */
		/*g_hash_table_insert(used_cubes_choose, key, &SMT_0);
		printf("NOT\n");
		printf("result false\n");*/
		return 0;
	} else if (sat == SMT_FALSE) {
		/* == caching == */
		/*int i, size = 0;
		for (i = 0; i < charmap_count; i++) {
			if (charmap[i] != 'i') {
				size ++;
			}
		}
		if (size > 1 && (strcmp(Qs, FALSE_PREDICATE) != 0)) {
			key = buildCubeKey(charmap, charmap_count);
			if ((value = g_hash_table_lookup(used_cubes_choose, key)) == NULL){
				g_hash_table_insert(used_cubes_choose, key, &SMT_0);
			} else {
				free(key);
			}
		}
		printf("result true\n");
		g_hash_table_insert(used_cubes_choose, key, &SMT_1);*/
		return 1;
	} else if (sat == SMT_UNDEF)
		assert(0 && "unclear why it is undefined");
}

void storeConjunct(int polarity, 
		enum predType ***cubes,
		int cubes_count[],
		int pred_idx,
		char charmap[],
		int charmap_count) {

	int c, i, xx;

	c = cubes_count[pred_idx];

	for(i = 0; i< ACTUAL_PRED_COUNT; i++) {
		if (cubes[pred_idx] == NULL) {
			printf("i = %d\n", i);
		}
		assert(cubes[pred_idx] != NULL && "cubes[pred_idx] != NULL");
		assert(cubes[pred_idx][c] != NULL && "cubes[pred_idx][c] != NULL");
		cubes[pred_idx][c][i] = IGNORE;
	}

	for (i = 0; i < charmap_count; i++) {
		if (charmap[i] == '-')  {
			cubes[pred_idx][c][i] = NEG;
		} else if (charmap[i] == '+') {
			cubes[pred_idx][c][i] = POS;
		} else if (charmap[i] == 'i') {
			cubes[pred_idx][c][i] = IGNORE;
		}
	}


	xx = 0;
	for(i = 0; i< ACTUAL_PRED_COUNT; i++) {
		if (cubes[pred_idx][c][i] != IGNORE) {
			assert((cubes[pred_idx][c][i] == POS) || (cubes[pred_idx][c][i] == NEG));
			xx++;

		}
	}

	//assert((xx == 1) || (xx == 0));

	cubes_count[pred_idx] = c + 1;	

	assert(cubes_count[pred_idx] < MAX_DISJUNCTS);
}

char *getEmptyCharmap(int size)
{
	char *emptyCube;
	int i;

	emptyCube = (char *) malloc(size * sizeof(char));

	for (i = 0; i < size; i++) {
		emptyCube[i] = 'i';
	}
	return emptyCube;
}

list_t getEmptyElem(int size) {
	char *cm;
	cm = getEmptyCharmap(size);
	return createElem(cm);
}

list_t copyAndExpand(list_t l, int i, char c, int size) {
	char *cm;

	cm = (char *) malloc(size * sizeof(char));
	memcpy(cm, l->val, size * sizeof(char));

	cm[i] = c;
	return createElem(cm);
}

void printElem(list_t e) {
	int i;
	for (i = 0; i < ((extrapolate_flag) ? (simple_pred_count + composed_pred_count) : simple_pred_count); i++) {
		printf("%c", e->val[i]);
	}
	printf("\n");
}

void printQueue(queue_t q) {
	list_t aux;
	aux = q->head;
	while(aux != NULL) {
		printElem(aux);
		aux = aux->next;
	}
}

void printAntet(FILE *f, char *lbl, int n) {
	int i;

	/* == print label == */
	if (strlen(lbl) != 0) {
		fprintf(f, "%s", lbl);
	}
	/* == print tabs == */
	for (i = 0; i < n; i++) {
		fprintf(f, "  ");
	}
}

int lastNotIgnored(list_t l, int size) {
	char *cm;
	int i;

	cm = l->val;
	for (i = size - 1; i >= 0; i--) {
		if (cm[i] != 'i') 
			return i;
	}
	assert(0);
	return 0;
}

queue_t expand(list_t l, int *unit_pos, int *unit_neg, int is_assumption) {
	int i; 
	int lNI; /* index of last predicate not ignored */
	queue_t res;
	res = createQueue();
	lNI = lastNotIgnored(l, simple_pred_count);
	for (i = lNI + 1; i < simple_pred_count; i++) {
		if (pred_preds[i][simple_pred_count] == '0' && !is_assumption) {
			continue;
		}
		enqueue(copyAndExpand(l, i, '+', simple_pred_count), res);
		enqueue(copyAndExpand(l, i, '-', simple_pred_count), res);
	}
	return res;
}



int myIsCovered(list_t e, queue_t q) {
	list_t aux;
	int i, is_covered;
	if (q == NULL) {
		return 0;
	}

	aux = q->head;

	while (aux != NULL) {
		is_covered = 1;
		for(i = 0; i < simple_pred_count; i++) {
			if ((aux->val[i] != 'i') && (aux->val[i] != e->val[i])) {
				is_covered = 0;
				break;
			}
			assert((aux->val[i] == 'i') || (aux->val[i] == e->val[i]));
		}
		if (is_covered)
			return 1;
		aux = aux->next;
	}
	return 0;
}

void myStoreConjuncts( int polarity, 
		enum predType ***cubes,
		int cubes_count[],
		int pred_idx,
		queue_t q,
		int nr_preds) {
	list_t e;

	e = q->head;
	//printf("STORING DISJUNCTS:\n");

	while (e != NULL) {

		storeConjunct(polarity, cubes, cubes_count, pred_idx, e->val, lastNotIgnored(e, nr_preds) + 1);
		//printf("%s\n", getDisjunct(cubes[pred_idx][cubes_count[pred_idx] - 1],
		//							TEMP_BOOL_PRINT_MODE));
		e = e->next;
	}

}

void addPredVar(int pred_idx, char *var){
	int i;
	for (i = 0; i < var_count; i++) {
		if (strcmp(vars[i], var) == 0) {
			var_preds[i][pred_idx] = '1';
			return;
		}
	}

	strcpy(vars[var_count], var);
	var_preds[var_count][pred_idx] = '1';
	var_count = var_count + 1;
	assert(var_count < MAX_VAR_COUNT);
}

/* == parses predicate and marks found vars in var_preds
 * returns the number of variables found == */
int parseAndStoreVars(char* pred, int idx) {
	int l, cnt;
	char var[MAX_VAR_SIZE];
	char *pred_end = pred + strlen(pred);
	//printf("pred: %s\n", pred);
	cnt = 0;
	while (pred < pred_end) {
		if ((l = isIdentifier(pred)) == 0) {
			pred = pred + 1;
		} else {
			strncpy(var, pred, l);
			var[l] = '\0';

			pred = pred + l;
			//printf("var: %s \n", var);
			addPredVar(idx, var);
			cnt++;
		}
	}
	return cnt;
}

void genPsoCounterPreds(FILE *fp) {
	int pid, var_idx, k;
	for (pid = 0; pid < current_pid; pid++) {
		for (var_idx = 0; var_idx < global_var_count; var_idx++) {
			if ((var_idx < nr_weak_global_vars) && (arrayStrSearch(global_vars_modified[pid],
					global_var_modified_count[pid],
					global_vars[var_idx]) != -1)) {
				for (k = 0; k <= k_buffer_size; k++) {
					fprintf(fp, "(%s_cnt_p%d = %d)\n", global_vars[var_idx], pid + 1, k);
				}
			}
		}
	}
}

void genTsoCounterPreds(FILE *fp) {
	int pid, k, var_idx;
	for (pid = 0; pid < current_pid; pid++) {
		if (global_var_modified_count[pid] > 0) {
			for (k = 0; k <= k_buffer_size; k++) {
				fprintf(fp, "(p%d_cnt = %d)\n", pid + 1, k);
				if (k != 0) {
					for (var_idx = 0; var_idx < global_var_count; var_idx++) {
						if ((var_idx < nr_weak_global_vars) && (arrayStrSearch(global_vars_modified[pid],
								global_var_modified_count[pid],
								global_vars[var_idx]) != -1)) {
							fprintf(fp, "(lhs_%d_p%d = %d)\n", k, pid + 1, var_idx);
						}
					}
				}
			}
		}
	}
}


char *replaceAll(char *original, char *motiv, char *replacement) {
	int orig_size, repl_size, motiv_size, next_motiv_pos;
	char *result, *result_iter, *orig_iter, *orig_end;

	orig_size = strlen(original);
	repl_size = strlen(replacement);
	motiv_size = strlen(motiv);

	result = (char *) malloc(MAX_PRED_SIZE * sizeof(char));

	orig_iter = original;
	orig_end = original + orig_size;
	assert(*orig_end == '\0');
	result_iter = result;

	while (orig_iter < orig_end) {
		next_motiv_pos = contains(orig_iter, motiv);

		if (next_motiv_pos != -1) {
			// copy original until motiv occurence
			strncpy(result_iter, orig_iter, next_motiv_pos);
			// copy replacement
			strncpy(result_iter + next_motiv_pos, replacement, repl_size);
			// update iterators
			result_iter = result_iter + next_motiv_pos + repl_size;
			orig_iter = orig_iter + next_motiv_pos + motiv_size;
		} else {
			// copy remaining of orig
			strncpy(result_iter, orig_iter, orig_end - orig_iter);
			result_iter[orig_end-orig_iter] = '\0';	
			break;
		}
	}
	return result;
}


void genLessPsoPreds(FILE *fp, char* pred) {
	int var_idx, proc_idx, buffer_idx;
	char *new_pred;
	char new_var[MAX_PRED_SIZE];

	for (var_idx = 0; 
			((var_idx < nr_weak_global_vars) && (var_idx < global_var_count));
			var_idx++) {
		if (contains(pred, global_vars[var_idx]) == -1) {
			continue;
		}
		for (proc_idx = 0; proc_idx < current_pid; proc_idx++) {
			if (arrayStrSearch(global_vars_modified[proc_idx],
					global_var_modified_count[proc_idx],
					global_vars[var_idx]) == -1) {
				continue;
			}
			for (buffer_idx = 0; buffer_idx < k_buffer_size; buffer_idx++) {
				sprintf(new_var, "%s_%d_p%d", 
						global_vars[var_idx], buffer_idx + 1, proc_idx + 1);
				new_pred = replaceAll(pred, global_vars[var_idx], new_var);

				//printf("%s\n", new_pred);
				fprintf(fp, "%s", new_pred);
				free(new_pred);
			}
		}
	}
}

void genLessTsoPreds(FILE *fp, char* pred) {
	int var_idx, proc_idx, buffer_idx;
	char *new_pred;
	char new_var[MAX_PRED_SIZE];

	gpointer value;

	for (var_idx = 0;
			((var_idx < nr_weak_global_vars) && (var_idx < global_var_count));
			var_idx++) {
		if (contains(pred, global_vars[var_idx]) == -1) {
			continue;
		}
		for (proc_idx = 0; proc_idx < current_pid; proc_idx++) {
			if (arrayStrSearch(global_vars_modified[proc_idx],
					global_var_modified_count[proc_idx],
					global_vars[var_idx]) == -1) {
				continue;
			}
			for (buffer_idx = 0; buffer_idx < k_buffer_size; buffer_idx++) {
				sprintf(new_var, "rhs_%d_p%d",
						buffer_idx + 1, proc_idx + 1);
				new_pred = replaceAll(pred, global_vars[var_idx], new_var);

				/* avoid duplicates when extrapolating */
				if ((value = g_hash_table_lookup(extrapolated_preds, new_pred)) == NULL){
					g_hash_table_insert(extrapolated_preds, new_pred, &SMT_0);
					fprintf(fp, "%s", new_pred);
				}
				//printf("%s\n", new_pred);

				free(new_pred);
			}
		}
	}
}

void genPsoPreds(FILE *fp, char* pred_start, int char_idx) {
	int pid, var_idx, l, i;
	char var[MAX_VAR_SIZE], *new_pred, *pred, *pred_end, temp[MAX_PRED_SIZE];

	pred_end = pred_start + strlen(pred_start);
	pred = pred_start + char_idx;
	while (pred < pred_end) {
		if ((l = isIdentifier(pred)) == 0) {
			pred = pred + 1;
		} else {
			strncpy(var, pred, l);
			var[l] = '\0';
			var_idx = getGlobalVarIdx(var);

			if((var_idx != -1) && (var_idx < nr_weak_global_vars)) {
				for (pid = 0; pid < current_pid; pid++) {
					if (arrayStrSearch(global_vars_modified[pid],
							global_var_modified_count[pid],
							var) != -1) {

						//printf("before loop\n");
						for (i = 1; i <= k_buffer_size; i++) {
							new_pred = (char *) malloc(MAX_PRED_SIZE * sizeof(char));
							strncpy(new_pred, pred_start, (pred-pred_start));
							new_pred[(pred-pred_start)] = '\0';
							//printf("1: %s\n", new_pred);

							sprintf(temp, "%s_%d_p%d", var, i, pid + 1);
							strcat(new_pred, temp);
							//printf("2: %s\n", new_pred);
							strcpy(new_pred + strlen(new_pred) , pred + l);
							//printf("hr\n");
							fprintf(fp, "%s", new_pred);
							genPsoPreds(fp, new_pred, pred + l - pred_start);
							free(new_pred);
						}
					}
				}
			}

			pred = pred + l;
			//printf("var: %s \n", var);
		}
	}
}



void computeConeOfInfluence(void){
	int var_idx, i, j, k;
	int changed;

	/* initialization */
	for(var_idx = 0; var_idx < var_count; var_idx++) {
		for (i = 0; i < simple_pred_count + 1; i++) {
			for (j = i + 1; j < simple_pred_count + 1; j++) {
				if ((var_preds[var_idx][i] == '1') && (var_preds[var_idx][j] == '1')) {
					pred_preds[i][j] = '1';
					pred_preds[j][i] = '1';
				}
			}
		}
	}
	/*printf("pred_preds: \n");
	for (i = 0; i < pred_count + 1; i++) {
		for (j = 0; j < pred_count + 1; j++) {
			printf("%c ", pred_preds[i][j]);
		}
		printf("\n");
	}*/

	/* compute fixpoint */
	do {
		changed = 0;
		for (i = 0; i < simple_pred_count + 1; i++) {
			for (j = 0; j < simple_pred_count + 1; j++) {
				for (k = 0; k < simple_pred_count + 1; k++) {
					if ((pred_preds[i][j] == '1') && (pred_preds[j][k] == '1')) {
						if (pred_preds[i][k] == '0') {
							changed = 1;
							pred_preds[i][k] = '1';
							pred_preds[k][i] = '1';
						}
					}
				}
			}
		}

	} while (changed);
	/*printf("pred_preds: \n");
	for (i = 0; i < pred_count + 1; i++) {
		for (j = 0; j < pred_count + 1; j++) {
			printf("%c ", pred_preds[i][j]);
		}
		printf("\n");
	}*/
}

/* == for c2bpp mycomputecubes should use all predicates,
 * both composed and simple, with cubes of size 1 only.
 * Updating the composed predicates can be left for the
 * wmm print function == */

void myComputeCubesPrime(stmt_t *stmt, int pred_idx, char *Q, int max_cubes, int stmt_idx) {
	queue_t results_pos, results_neg;
	list_t l, empty;
	int i, j, x, xx, cnt;
	int polarity;
	int is_valid, nr_vars;

	empty = getEmptyElem(simple_pred_count + composed_pred_count);
	results_pos = createQueue();
	results_neg = createQueue();

	cnt = 0;


	for (i = 0; i < (simple_pred_count + composed_pred_count); i++) {
		//printf("i = %d\n", i);

		l = copyAndExpand(empty, i, '+', simple_pred_count + composed_pred_count);
		polarity = 1;


		is_valid = isImplicationValid(l->val, i + 1, polarity, Q, stmt_idx, pred_idx);


		if (is_valid == 1) {
			enqueue(l, results_pos);
		} else {
			polarity = -1;
			is_valid = isImplicationValid(l->val, i + 1, polarity, Q, stmt_idx, pred_idx);
			if (is_valid == 1) {
				if ((stmt_idx == 1) && (pred_idx == 9)) {
					if (i < simple_pred_count) {
						//printf("enqueue: %s\n", preds[i]);
					} else {
						//printf("enqueue: %s\n", composed_preds[i - simple_pred_count]);
					}
					cnt++;
				}
				enqueue(l, results_neg);
			} else {
				disposeElem(&l);
			}
		}

		/* == for composed predicates, skip trying with the negation == */
		if (i >= simple_pred_count) {
			continue;
		}

		l = copyAndExpand(empty, i, '-', simple_pred_count + composed_pred_count);


		polarity = 1;
		//TODO: add impl_pos and_neg optimization
		is_valid = isImplicationValid(l->val, i + 1, polarity, Q, stmt_idx, pred_idx);
		if (is_valid == 1) {
			enqueue(l, results_pos);
		} else {
			polarity = -1;
			is_valid = isImplicationValid(l->val, i + 1, polarity, Q, stmt_idx, pred_idx);
			if (is_valid == 1) {
				if ((stmt_idx == 1) && (pred_idx == 9)) {
					if (i < simple_pred_count) {
						//printf("enqueue: %s\n", preds[i]);
					} else {
						//printf("enqueue: %s\n", composed_preds[i - simple_pred_count]);
					}
					cnt++;
				}
				enqueue(l, results_neg);
			} else {
				disposeElem(&l);
			}
		}
	}

	if ((stmt->type == IF_CONDITIONAL) || (stmt->type == ASSERT)) {
		assert((stmt->cubes_pos != NULL) && "cubes_pos is NULL");
		assert((stmt->cubes_neg != NULL) && "cubes_neg is NULL");

		assert((pred_idx == 0) && "pred_idx == 0");

		myStoreConjuncts(polarity, stmt->cubes_pos, stmt->cubes_pos_count,
				pred_idx, results_pos, simple_pred_count + composed_pred_count);

		myStoreConjuncts(polarity, stmt->cubes_neg, stmt->cubes_neg_count, pred_idx,
				results_neg, simple_pred_count + composed_pred_count);
	} else {

		assert((stmt->type == STORE) || (stmt->type == LOAD) || (stmt->type == LOCAL));

		myStoreConjuncts(polarity, shared_cubes_pos, shared_cubes_pos_count,
				pred_idx, results_pos, simple_pred_count + composed_pred_count);

		myStoreConjuncts(polarity, shared_cubes_neg, shared_cubes_neg_count, pred_idx,
				results_neg, simple_pred_count + composed_pred_count);
	}

	while((l = dequeue(results_pos)) != NULL) {
		disposeElem(&l);
	}
	disposeQ(&results_pos);

	while((l = dequeue(results_neg)) != NULL) {
		disposeElem(&l);
	}
	disposeQ(&results_neg);

}


void myComputeCubes(stmt_t *stmt, int pred_idx, char *Q, int max_cubes, int stmt_idx) {
	queue_t unit_q, q, expansion_q, results_pos, results_neg;
	list_t l, empty, exp;
	char *cm;
	int i, j;
	int unit_pos[simple_pred_count], unit_neg[simple_pred_count];
	int polarity;
	int is_valid, nr_vars, nr_preds;

	long worst_case, nom, den;

	struct timeval start_time, end_time;

	gettimeofday(&start_time, NULL);

	printf("compute cubes for: %s\n", Q);


	memset(unit_pos, 0, simple_pred_count * sizeof(int));
	memset(unit_neg, 0, simple_pred_count * sizeof(int));

	empty = getEmptyElem(simple_pred_count);
	unit_q = createQueue();
	q = createQueue();
	results_pos = createQueue();
	results_neg = createQueue();

	/* == reset state == */
	for (i = 0; i < simple_pred_count + 1; i++) {
		var_preds[i][simple_pred_count] = '0';
	}
	memset(pred_preds, '0', MAX_SIMPLE_PRED_COUNT * MAX_SIMPLE_PRED_COUNT);

	nr_vars = parseAndStoreVars(Q, simple_pred_count);
	computeConeOfInfluence();

	nr_preds = 0;
	for (i = 0; i < simple_pred_count; i++) {
		if (pred_preds[i][simple_pred_count] == '1') {
			nr_preds++;
		}
	}

	//printf("using %d/%d preds\n", nr_preds, pred_count);

	for (i = 0; i < simple_pred_count; i++) {

		/* == take care for cases like (1) = 2 or (1) = 1, that have no vars == */
		if (nr_vars & (pred_preds[i][simple_pred_count] == '0')) {
			continue;
		}

		l = copyAndExpand(empty, i, '+', simple_pred_count);
		enqueue(l, unit_q);

		l = copyAndExpand(empty, i, '-', simple_pred_count);
		enqueue(l, unit_q);
	}

	disposeElem(&empty);

	while ((l = dequeue(unit_q)) != NULL) {

		//printElem(l);
		polarity = 1;		
		//SMT_stats[stmt_idx][pred_idx]++;
		is_valid = isImplicationValid(l->val, lastNotIgnored(l, simple_pred_count) + 1, polarity, Q, stmt_idx, pred_idx);
		//printElem(l);
		if (is_valid == 1) {

			unit_pos[lastNotIgnored(l, simple_pred_count)] = 1;
			enqueue(l, results_pos);

		} else {
			polarity = -1;
			//SMT_stats[stmt_idx][pred_idx]++;
			is_valid = isImplicationValid(l->val, lastNotIgnored(l, simple_pred_count) + 1, polarity, Q, stmt_idx, pred_idx);
			if (is_valid == 1) { 

				unit_neg[lastNotIgnored(l, simple_pred_count)] = 1;
				enqueue(l, results_neg);
			} else {
				enqueue(l, q);	
			}
		}
	}

	disposeQ(&unit_q);

	while ((l = dequeue(q)) != NULL) {
		assert(cubeLength(l) <= max_cubes);
		/* == limit cubes size == */
		//printElem(l);
		if (cubeLength(l) == max_cubes) {
			disposeElem(&l);
			continue;
		}

		/* == strenghten the predicate == */
		expansion_q = expand(l, unit_pos, unit_neg, 0);

		/* == test the strenghtened predicates == */
		while ((exp = dequeue(expansion_q)) != NULL) {

			//printf("head: %d\n", q->head);
			polarity = 1;
			if (myIsCovered(exp, results_pos) ||
					myIsCovered(exp, results_neg) ||
					myIsCovered(exp, invalid_cubes)) {
				disposeElem(&exp);
				continue;
			}
			//SMT_stats[stmt_idx][pred_idx]++;
			is_valid = isImplicationValid(exp->val, lastNotIgnored(exp, simple_pred_count) + 1, polarity, Q, stmt_idx, pred_idx);
			//printElem(exp);
			if (is_valid == 1) {
				enqueue(exp, results_pos);

			} else {
				polarity = -1;
				//SMT_stats[stmt_idx][pred_idx]++;
				is_valid = isImplicationValid(exp->val, lastNotIgnored(exp, simple_pred_count) + 1, polarity, Q, stmt_idx, pred_idx);
				if (is_valid == 1) { 
					enqueue(exp, results_neg);

				} else {
					enqueue(exp, q);	
				}
			}	
		}
		disposeQ(&expansion_q);
		disposeElem(&l);		
	}
	gettimeofday(&end_time, NULL);




	if ((stmt->type == IF_CONDITIONAL) || (stmt->type == ASSERT)){
		assert((stmt->cubes_pos != NULL) && "cubes_pos is NULL");
		assert((stmt->cubes_neg != NULL) && "cubes_neg is NULL");

		assert((pred_idx == 0) && "pred_idx == 0");

		myStoreConjuncts(polarity, stmt->cubes_pos, stmt->cubes_pos_count,
					pred_idx, results_pos, simple_pred_count);

		myStoreConjuncts(polarity, stmt->cubes_neg, stmt->cubes_neg_count, pred_idx,
					results_neg, simple_pred_count);
	} else {

		assert((stmt->type == STORE) || (stmt->type == LOAD) 
			|| (stmt->type == LOCAL));

		myStoreConjuncts(polarity, shared_cubes_pos, shared_cubes_pos_count,
				pred_idx, results_pos, simple_pred_count);

		myStoreConjuncts(polarity, shared_cubes_neg, shared_cubes_neg_count, pred_idx,
				results_neg, simple_pred_count);
	}
	while((l = dequeue(results_pos)) != NULL) {
		disposeElem(&l);
	}
	disposeQ(&results_pos);

	while((l = dequeue(results_neg)) != NULL) {
		disposeElem(&l);
	}
	disposeQ(&results_neg);

	disposeQ(&q);

}


void myComputeAssumption(stmt_t *stmt, int pred_idx, char *Q, int max_cubes) {
	queue_t unit_q, q, expansion_q, results_pos;
	list_t l, empty, exp;
	char *cm;
	int i;
	int unit_pos[simple_pred_count], unit_neg[simple_pred_count];
	int polarity;
	int is_valid;

	memset(unit_pos, 0, simple_pred_count * sizeof(int));
	memset(unit_neg, 0, simple_pred_count * sizeof(int));

	//TODO: check if should be pred_count + composed_pred_count
	empty = getEmptyElem(simple_pred_count);
	unit_q = createQueue();
	q = createQueue();
	results_pos = createQueue();


	for (i = 0; i < simple_pred_count; i++) {
		l = copyAndExpand(empty, i, '+', simple_pred_count);
		enqueue(l, unit_q);
		l = copyAndExpand(empty, i, '-', simple_pred_count);
		enqueue(l, unit_q);
	}

	disposeElem(&empty);

	while ((l = dequeue(unit_q)) != NULL) {

		polarity = 1;
		is_valid = isImplicationValid(l->val, lastNotIgnored(l, simple_pred_count) + 1, polarity, Q, stmt_count + 1, simple_pred_count + 1);
		if (is_valid == 1) {

			unit_pos[lastNotIgnored(l, simple_pred_count)] = 1;
			enqueue(l, results_pos);
		} else {
			enqueue(l, q);
		}
	}

	while ((l = dequeue(q)) != NULL) {
		assert(cubeLength(l) <= max_cubes);
		/* == limit cubes size == */
		if (cubeLength(l) == max_cubes) {
			disposeElem(&l);
			continue;
		}

		expansion_q = expand(l, unit_pos, unit_neg, 1);
		while ((exp = dequeue(expansion_q)) != NULL) {
			polarity = 1;
			if (myIsCovered(exp, results_pos)) {
				disposeElem(&exp);
				continue;
			}
			is_valid = isImplicationValid(exp->val, lastNotIgnored(exp, simple_pred_count) + 1, polarity, Q, stmt_count + 1, simple_pred_count + 1);
			if (is_valid == 1) {

				enqueue(exp, results_pos);
			} else {
				enqueue(exp, q);
			}
		}
		disposeQ(&expansion_q);
		disposeElem(&l);
	}
	invalid_cubes = results_pos;

	myStoreConjuncts(polarity, stmt->cubes_pos, stmt->cubes_pos_count, pred_idx, results_pos, simple_pred_count);

	disposeQ(&unit_q);
	disposeQ(&q);
}

void computeCubes(int polarity, stmt_t *stmt, int pred_idx, char charmap[], int charmap_count, char *Q) {

	char *s;
	int i;

	if (charmap_count > simple_pred_count)
		return;

	assert(polarity > 0 || polarity < 0);

	if (charmap_count > 0) {
		char last = charmap[charmap_count - 1];
		if (last != 'i' && isImplicationValid(charmap, charmap_count, polarity, Q, 0, 0) == 1) {
			// Store the conjunct
			if (polarity > 0)
				storeConjunct(polarity, stmt->cubes_pos, stmt->cubes_pos_count, pred_idx, charmap, charmap_count);
			else
				storeConjunct(polarity, stmt->cubes_neg, stmt->cubes_neg_count, pred_idx, charmap, charmap_count);
			return;
		}
	}


	s = (char *) malloc(charmap_count + 1);
	for (i = 0; i < charmap_count; i += 1) 
		s[i] = charmap[i];

	s[i] = '+';
	computeCubes(polarity, stmt, pred_idx, s, charmap_count + 1, Q);

	s[i] = '-';
	computeCubes(polarity, stmt, pred_idx, s, charmap_count + 1, Q);

	s[i] = 'i';
	computeCubes(polarity, stmt, pred_idx, s, charmap_count + 1, Q);

	free(s);
}


void storeGlobalVarName(char *lhs, int pid) {

	int l, i, proc_gv_cnt;
	char *v;

	// Check if its stored already 

	int global_var_idx = arrayStrSearch(global_vars, global_var_count, lhs);

	if (pid != 0) {
		proc_gv_cnt = global_var_modified_count[pid - 1];
		if (arrayStrSearch(global_vars_modified[pid - 1],
				proc_gv_cnt, lhs) != -1) {
			return;
		} else {
			if (global_var_idx != -1) {
				global_vars_modified[pid - 1][proc_gv_cnt] =
						global_vars[global_var_idx];
				global_var_modified_count[pid - 1]++;
			} else {
				/* == not reachable because shared variables
				 * are all discovered in the initialization area == */
				//assert(0);
			}
		}
	} else {
		l = strlen(lhs);
		v = (char *) malloc(l + 1);
		strcpy(v, lhs);
		global_vars[global_var_count] = v;
		global_var_count += 1;
	}
}

void freeCubes(enum predType ****c, int d1, int d2) {
	int i, j;
	for (i = 0; i < d1; i++) {
		for (j = 0; j < d2; j++) {
			free((*c)[i][j]);
		}
		free((*c)[i]);
	}
	free(*c);
}

void freeCubesMemory(stmt_t *stmt, int d1, int d2) {
	freeCubes(&(stmt->cubes_pos), d1, d2);
	freeCubes(&(stmt->cubes_neg), d1, d2);
}

enum predType ***allocateCubes(int d1, int d2, int d3) {
	enum predType ***res;
	int i, j;

	res = (enum predType***) malloc(d1 * sizeof(enum predType**));

	if (res == NULL) {
		printf("could not allocate memory\n");
		exit(EXIT_FAILURE);
	}
	for (i = 0; i < d1; i++) {
		res[i] = (enum predType**) malloc(d2 * sizeof(enum predType*));
		if (res[i] == NULL) {
			printf("could not allocate memory\n");
			exit(EXIT_FAILURE);
		}
		for (j = 0; j < d2; j++) {
			res[i][j] = (enum predType *)calloc(d3, sizeof(enum predType));
			if (res[i][j] == NULL) {
				printf("could not allocate memory\n");
				exit(EXIT_FAILURE);
			}
		}
	}

	return res;
}

void allocateCubesMemory(stmt_t *stmt, int d1, int d2, int d3) {
	stmt->cubes_pos = allocateCubes(d1, d2, d3);
	stmt->cubes_neg = allocateCubes(d1, d2, d3);
}

int parseAndStoreStatement(char *stmt, int idx, int *in_atomic, int *atomic_nested_ifs) {

	char *pos, *lbl, *lhs, *rhs, *label;
	enum stmtType *type;
	int ilbl;

	lhs   = getStmt(idx)->lhs;
	rhs   = getStmt(idx)->rhs;
	label = getStmt(idx)->label;
	type  = &(getStmt(idx)->type);

	assertNoCarriadgeReturns(stmt);
	/* store label if there */
	if ((strstr(stmt, "process") == 0) && (strstr(stmt, "/*") == 0)) {
		if ((lbl = strstr(stmt, ":")) != 0) {
			/* store label */
			strncpy(label, stmt, lbl - stmt + 1);
			label[lbl - stmt + 1] = '\0';

			removeTrailingSpaces(label);

			ilbl = atoi(label);
			if (ilbl > max_label) {
				max_label = ilbl;
			}
			stmt = lbl + 1;
		}
	}

	if (strstr(stmt, "/*") != 0) {
		*type = COMMENT;
		strncpy(getStmt(idx)->condition, stmt, strlen(stmt));
	}
	/* store/load */
	else if (((pos = strstr(stmt, "=")) != 0) && (strstr(stmt, "if") == 0) && (strstr(stmt, "assert") == 0) && (strstr(stmt, "assume") == 0)) {


		char *start, *keyword;
		int  n;

		start = stmt;

		if ((keyword = strstr(stmt, "store")) != 0) {
			*type = STORE;
			start = keyword + strlen("store");
		}
		else if ((keyword = strstr(stmt, "load")) != 0) {
			*type = LOAD;
			start = keyword + strlen("load");
		}
		else {
			*type = LOCAL;
		}
		n = pos - start;
		strncpy(lhs, start, n);
		lhs[n] = '\0';

		removeTrailingSpaces(lhs);
		/* remember global var */
		if ((strstr(stmt, "store")) != NULL) {
			storeGlobalVarName(lhs, current_pid);
		}

		/* store rhs */
		strncpy(rhs, pos + 1, strlen(pos + 1) - 1);
		rhs[strlen(pos + 1) - 1] = '\0';
		removeTrailingSpaces(rhs);
		if ((strstr(stmt, "load")) != 0) {
			//printf("rhs global var: %s \n", rhs);
		}

		//printf("statement: %s%s=%s\n", label, lhs, rhs);
	}
	/* goto */
	else if (strstr(stmt, "goto") != 0) {

		strncpy(lhs, stmt, strlen(stmt) - 1);
		lhs[strlen(stmt)-1] = '\0';
		removeTrailingSpaces(lhs);

		*type = GOTO;
	}
	/* conditional */
	else if (strstr(stmt, "if") != 0 && strstr(stmt, "endif") == 0) {
		char *s, *e;

		s = findFirstBracket(stmt);
		e = findLastBracket(stmt);
		assert(e != 0 && s != 0 && e > s);
		strncpy(getStmt(idx)->condition, s, e - s + 1);
		getStmt(idx)->condition[e - s + 1] = '\0';

		if (strcmp(getStmt(idx)->condition, "(*)") == 0) {
			*type = IF_CONDITIONAL_STAR;
		} else {
			*type = IF_CONDITIONAL;

			if (*in_atomic) {
				*atomic_nested_ifs = (*atomic_nested_ifs) + 1;
				if ((*atomic_nested_ifs) > atomic_ifvar_count) {
					atomic_ifvar_count = (*atomic_nested_ifs);
				}
			} else {
				getStmt(idx)->ifvar_idx = getUniqueIfvar();
			}
		}

		//printf(" cond = %s \n", getStmt(idx)->condition);

		context_stack[context_top] = idx;			
		context_top += 1;
		if (context_top > max_depth) {
			max_depth = context_top;
		}
		/* == allow nested ifs == */
		//assert(context_top <= 1 && "Nested If's are not allowed !");
	}
	else if (strstr(stmt, "else") != 0) {

		assert(context_top > 0);
		getStmt(idx)->context = context_stack[context_top - 1];
		if (getStmt(getStmt(idx)->context)->type == IF_CONDITIONAL_STAR) {
			*type = ELSE_CONDITIONAL_STAR;
		} else {
			assert(getStmt(getStmt(idx)->context)->type == IF_CONDITIONAL);
			*type = ELSE_CONDITIONAL;
		}
	}
	else if (strstr(stmt, "endif") != 0) {



		assert(context_top > 0);
		getStmt(idx)->context = context_stack[context_top - 1];
		if (getStmt(getStmt(idx)->context)->type == IF_CONDITIONAL_STAR) {
			*type = ENDIF_CONDITIONAL_STAR;
		} else {
			assert(getStmt(getStmt(idx)->context)->type == IF_CONDITIONAL);
			*type = ENDIF_CONDITIONAL;
			if (*in_atomic) {
				*atomic_nested_ifs = (*atomic_nested_ifs) - 1;
			}
		}
		context_top -= 1;
	}
	else if (strstr(stmt, "nop") != 0) {

		strncpy(lhs, stmt, strlen(stmt) - 1);
		lhs[strlen(stmt)-1] = '\0';
		removeTrailingSpaces(lhs);

		*type = NOPST;
	}
	else if (strstr(stmt, "process") != 0) {

		int pid;

		sscanf(stmt, "process %d", &pid);			
		getStmt(idx)->pid = pid;
		current_pid = pid;
		*type = NEW_PROCESS;
	}
	else if (strstr(stmt, "beginit") != 0) {
		*type = BEG_INIT;
	}
	else if (strstr(stmt, "endinit") != 0) {
		*type = END_INIT;
	}
	else if (strstr(stmt, "assert") != 0) {

		//allocateCubesMemory(getStmt(idx), 1, MAX_DISJUNCTS, MAX_TOTAL_PRED_COUNT);
		char *s, *e;

		s = findFirstBracket(stmt);
		e = findLastBracket(stmt);
		assert(e != 0 && s != 0 && e > s);
		strncpy(getStmt(idx)->condition, s, e - s + 1);
		getStmt(idx)->condition[e - s + 1] = '\0';

		strncpy(lhs, stmt, s - stmt);
		lhs[s - stmt] = '\0';
		removeTrailingSpaces(lhs);
		//printf("lhs: %s \n", lhs);
		//printf("cond: %s \n", getStmt(idx)->condition);
		*type = ASSERT;
	}
	else if (strstr(stmt, "assume") != 0) {

		char *s, *e;

		*type = ASSUME;

		s = findFirstBracket(stmt);
		e = findLastBracket(stmt);
		assert(e != 0 && s != 0 && e > s);
		strncpy(getStmt(idx)->condition, s, e - s + 1);
		getStmt(idx)->condition[e - s + 1] = '\0';

	}
	else if (strstr(stmt, "fence") != 0) {
		*type = FENCE;
	}
	else if ((pos = strstr(stmt, "flush")) != 0) {
		char *e;
		*type = FLUSH;
		stmt = pos + strlen("flush");
		if ((e = strstr(stmt, ";")) == 0) {
			e = stmt + strlen(stmt);
		}
		strncpy(lhs, stmt, e - stmt);
		lhs[e - stmt + 1] = '\0';
		removeTrailingSpaces(lhs);
	}
	else if (strstr(stmt, "begin_atomic") != 0) {
		*type = BEG_ATOMIC;
		*in_atomic = 1;
	}
	else if (strstr(stmt, "end_atomic") != 0) {
		*type = END_ATOMIC;
		*in_atomic = 0;
	}

	/* fail */
	else {
		assert(strlen(stmt) < 2);
		return 0;
	}
	getStmt(idx)->pid = current_pid;
	getStmt(idx)->context_depth = context_top;
	return 1;
}

int isOperator(char *pred, enum opType *op) {

	int size = 0;
	*op = NOP;

	if (pred[0] == '+') {
		*op = ADD;
		size = 1;
	}
	else if (pred[0] == '-') {
		*op = SUB;
		size = 1;
	}
	else if (pred[0] == '*') {
		*op = MUL;
		size = 1;
	}
	else if (pred[0] == '<') {
		if (pred[1] == '=') {
			*op = LE;
			size = 2;
		}
		else {
			*op = LT;
			size = 1;
		}
	}
	else if (pred[0] == '>') {

		if (pred[1] == '=') {
			*op = GE;
			size = 2;
		}
		else {
			*op = GT;
			size = 1;
		}
	}
	else if (pred[0] == '=') {
		*op = EQ;
		size = 1;
	}
	else if (pred[0] == '!' && pred[1] == '=') {
		*op = DISEQ;
		size = 2;
	}
	else if (pred[0] == '&') {
		*op = AND;
		size = 1;
	}
	else if (pred[0] == '|') {
		*op = OR;
		size = 1;
	}

	return size;
}

smt_formula_t buildExpression(char *pred, int *count) {

	smt_formula_t	args[MAX_OPERANDS_EXPR], fe;
	int			curr_arg, i, size;
	enum 		opType	op, new_op;

	curr_arg = 0;
	for (i = 0; i < MAX_OPERANDS_EXPR; i++) {
		args[i] = 0;
	}
	op = NOP;
	new_op = NOP;

	//printf("start: pred = %s\n", pred);

	for (i = 0; i < strlen(pred); ) {

		//printf("current: %c\n", pred[i]); fflush(stdout);

		if ((pred[i] == '!') && (pred[i + 1] == '(')) {
			smt_formula_t e;
			int c;

			e = smt_mk_not(SMT_ctx, buildExpression(&pred[i+2], &c));
			//printf("i = %d count = %d \n", i, c); fflush(stdout);
			args[curr_arg++] = e;
			//printf("PRED = %s\n", pred);  smt_pp_expr(SMT_ctx, e); printf("\n");
			//printf("Remaining = %s %d\n", &pred[i + c + 1], i + c + 1);
			i = i + c + 2;
			continue;
		} else if (pred[i] == ')') {
			i = i + 1;
			break;   
		}
		else if (pred[i] == '(') {
			smt_formula_t e;
			int c;

			e = buildExpression(&pred[i+1], &c);
			//printf("i = %d count = %d \n", i, c); fflush(stdout);
			args[curr_arg++] = e;
			//printf("PRED = %s\n", pred);  //smt_pp_expr(SMT_ctx, e); printf("\n");
			//printf("Remaining = %s %d\n", &pred[i + c + 1], i + c + 1);
			i = i + c + 1;
			continue;
		}
		else if  ((size = isOperator(&pred[i], &new_op)) > 0)  {
			if (op == NOP) {
				op = new_op;
			} else {
				assert(op == new_op);
			}
			//printf("OP: i  = %d size = %d\n", i, size);
			i += size;
		}
		else if ((size = isIdentifier(&pred[i])) > 0)
		{
			char *id;
			smt_formula_t	v;

			id = (char *) malloc(size + 1);
			strncpy(id, &pred[i], size);
			id[size] = '\0';

			v = smt_mk_int_var(SMT_ctx, id);

			free(id);

			assert(curr_arg < MAX_OPERANDS_EXPR);
			assert(args[curr_arg] == 0);
			args[curr_arg++] = v;					

			//printf("ID: i  = %d size = %d\n", i, size);
			i += size;
		}
		else if ((size = isNumber(&pred[i])) > 0)
		{
			char *nums;
			int   num;
			smt_formula_t n;

			nums = (char *) malloc(size + 1);
			strncpy(nums, &pred[i], size);
			nums[size] = '\0';
			sscanf(nums, "%d", &num);
			free(nums);

			n = smt_mk_int(SMT_ctx, num);

			assert(curr_arg < MAX_OPERANDS_EXPR);
			assert(args[curr_arg] == 0);
			args[curr_arg++] = n;					

			//printf("NUM: i  = %d size = %d\n", i, size);

			i += size;
		}
		else 
		{
			//if (pred[i] != ' ') printf("%s[%d] = %d:%d:%d:%d", pred, i, pred[i],'\0', '\n','=');
			assert(pred[i] == ' ' || pred[i] == '\n'); // must be empty space / end of string if none of the other kinds.
			i += 1;
		}
	}

	*count = i;

	/* make the final expression. */
	if (args[0] == 0)
		assert(0 && "left argument cannot be 0");
	else if (args[0] != 0 && args[1] == 0)
	{
		assert(op == NOP);
		fe = args[0];
	}
	else  {
		// both arguments are not 0
		assert(op != NOP);
		switch(op) {

		case ADD:
			fe = smt_mk_add(SMT_ctx, 2, args);  break;
		case SUB:
			fe = smt_mk_sub(SMT_ctx, 2, args);  break;
		case MUL:
			fe = smt_mk_mul(SMT_ctx, 2, args);  break;
		case LE:
			fe = smt_mk_le(SMT_ctx, args[0], args[1]); break;
		case LT:
			fe = smt_mk_lt(SMT_ctx, args[0], args[1]); break;
		case GE:
			fe = smt_mk_ge(SMT_ctx, args[0], args[1]); break;
		case GT:
			fe = smt_mk_gt(SMT_ctx, args[0], args[1]); break;
		case EQ:
			fe = smt_mk_eq(SMT_ctx, args[0], args[1]); break;
		case DISEQ:
		{
			smt_formula_t tmp;
			tmp = smt_mk_eq(SMT_ctx, args[0], args[1]); 
			fe = smt_mk_not(SMT_ctx, tmp);
			break;
		}
		case AND:
			fe = smt_mk_and(SMT_ctx, curr_arg, args); break;
		case OR:
			fe = smt_mk_or(SMT_ctx, curr_arg, args); break;
		default:
			assert(0);
		};
	}
	//smt_pp_expr(SMT_ctx, fe);
	//printf("\n");
	return fe;
}

/* Store predicates, remove newline characters. */
void parseAndStorePredicate(char *pred, int idx) {

	int l;

	assertNoCarriadgeReturns(pred);

	l = strlen(pred);
	assert(l > 1 && pred[l - 1] == '\n');
	strncpy(preds[idx], pred, strlen(pred) - 1);
	preds[idx][strlen(pred) - 1] = '\0';

	parseAndStoreVars(preds[idx], idx);
}

void parseAndStoreComposedPredicate(char *pred, int idx) {

	int l, i;
	char tmp[MAX_PRED_SIZE];

	assertNoCarriadgeReturns(pred);


	/* == identify which predicates form the composed pred == */
	for (i = 0; i < simple_pred_count; i++) {
		sprintf(tmp, "!%s", preds[i]);
		if (strstr(pred, tmp)) {
			pred_components[idx][i] = NEG;
		} else if (strstr(pred, preds[i])) {
			pred_components[idx][i] = POS;
		}
	}

	l = strlen(pred);
	assert(l > 1 && pred[l - 1] == '\n');
	strncpy(composed_preds[idx], pred, strlen(pred) - 1);
	composed_preds[idx][strlen(pred) - 1] = '\0';

	//TODO: does cone of influence bring such improvement?
	//parseAndStoreVars(preds[idx], idx);
}

void printPredicates(int count) {

	int i;

	printf("\nPredicates: \n----------- \n");
	for (i = 0; i < count; i += 1) {
		printf("\tB%d: %s", i, preds[i]); fflush(stdout);
		if (i < count - 1)
			printf("\n");
	}
	printf("\n\n");
}

void printStatements(int count) {

	int i;
	enum stmtType type;

	printf("Statements: \n");
	for (i = 0; i < count; i += 1)
	{	
		type = getStmt(i)->type;

		if (type == LOAD || type == STORE || type == LOCAL)
			printf("%s=%s\n", getStmt(i)->lhs, getStmt(i)->rhs);
		else if (type == GOTO)
			printf("%s \n", getStmt(i)->lhs);
		else if (type == IF_CONDITIONAL)
			printf("if %s \n", getStmt(i)->condition);
		else if (type == ELSE_CONDITIONAL)
			printf("else %s \n", getStmt(getStmt(i)->context)->condition);
		else if (type == ENDIF_CONDITIONAL)
			printf("endif %s \n", getStmt(getStmt(i)->context)->condition);
		else if (type == NOPST)
			printf("%s \n", getStmt(i)->lhs);
		else if (type == NEW_PROCESS)
			printf("Process %d \n", getStmt(i)->pid);
		else if (type == BEG_INIT)
			printf("beginit \n");
		else if (type == END_INIT)
			printf("endinit \n");
		else if (type == ASSERT)
			printf("%s \n", getStmt(i)->lhs);
	}
	printf("\n");
}

void initState() {

	int i, j, k, m;
	stmt_t *stmt;

	stmts = (stmt_t *) malloc(MAX_STMT * sizeof(stmt_t));

	stmts2 = (stmt_t *) malloc(MAX_STMT * sizeof(stmt_t));
	stmts3 = (stmt_t *) malloc(MAX_STMT * sizeof(stmt_t));
	//stmts4 = (stmt_t *) malloc(MAX_STMT * sizeof(stmt_t));
	//stmts5 = (stmt_t *) malloc(MAX_STMT * sizeof(stmt_t));
	//stmts6 = (stmt_t *) malloc(MAX_STMT * sizeof(stmt_t));
	//stmts7 = (stmt_t *) malloc(MAX_STMT * sizeof(stmt_t));
	//stmts8 = (stmt_t *) malloc(MAX_STMT * sizeof(stmt_t));
	//stmts9 = (stmt_t *) malloc(MAX_STMT * sizeof(stmt_t));
	//stmts10 = (stmt_t *) malloc(MAX_STMT * sizeof(stmt_t));

	if (!stmts  || !stmts2 || !stmts3 ){//|| !stmts4 || !stmts5 ){//|| !stmts6 || !stmts7 || 
			//!stmts8 || !stmts9 || !stmts10) {
		printf("memory allocation error\n");
		exit(1);
	}

	SMT_ctx = smt_mk_context();

	SMT_per_stmt = (int *) calloc(MAX_STMT * MAX_STMTS_ARRAYS, sizeof(int));

	shared_cubes_pos = allocateCubes(MAX_SIMPLE_PRED_COUNT, MAX_DISJUNCTS, MAX_TOTAL_PRED_COUNT);
	shared_cubes_neg = allocateCubes(MAX_SIMPLE_PRED_COUNT, MAX_DISJUNCTS, MAX_TOTAL_PRED_COUNT);
	shared_cubes_pos_count = (int *) calloc(MAX_SIMPLE_PRED_COUNT, sizeof(int));
	shared_cubes_neg_count = (int *) calloc(MAX_SIMPLE_PRED_COUNT, sizeof(int));

	for (i = 0; i < (MAX_STMT * MAX_STMTS_ARRAYS); i++) {

		stmt = getStmt(i);

		stmt->lhs[0] = '\0';
		stmt->rhs[0] = '\0';
		stmt->label[0] = '\0';

		stmt->elselabel = -1;
		stmt->endlabel = -1;
		stmt->context = -1;

		for (j = 0; j < MAX_SIMPLE_PRED_COUNT; j++) {

			stmt->cubes_pos_count[j] = 0;
			stmt->cubes_pos = NULL;

			stmt->cubes_neg_count[j] = 0;
			stmt->cubes_neg = NULL;

			assert( IGNORE == 0 );
		}
	}

	memset(var_preds, '0', MAX_VAR_COUNT * MAX_SIMPLE_PRED_COUNT);
	memset(pred_preds, '0', MAX_SIMPLE_PRED_COUNT * MAX_SIMPLE_PRED_COUNT);

	for (i = 0; i < MAX_COMPOSED_PRED_COUNT; i++) {
		for (j = 0; j < MAX_SIMPLE_PRED_COUNT; j++) {
			pred_components[i][j] = IGNORE;
		}
	}

	context_top = 0;
	stmt_count = 0;
	simple_pred_count = 0;

	SMT_total_calls = 0;

	/* == initialize hashmap for caching SMT calls == */
	used_cubes_choose = g_hash_table_new(g_str_hash, g_str_equal);
	extrapolated_preds = g_hash_table_new(g_str_hash, g_str_equal);

	memset(SMT_stats, 0, MAX_STMT * MAX_SIMPLE_PRED_COUNT);
	memset(cache_hits, 0, MAX_STMT * MAX_SIMPLE_PRED_COUNT);

	for (i = 0; i < MAX_NR_PROC; i++) {
		global_var_modified_count[i] = 0;
	}
}

char* getComposedPredCube(enum predType cube[], enum printType pt) {
	int pc, cc, i, is_first_cube, j;
	char temp[MAX_CUBESTRING_SIZE], *cubestring;
	enum predType v;

	assert(pt != CUBE_PRINT_MODE);

	cubestring = (char *) malloc(MAX_CUBESTRING_SIZE);
	cubestring[0] = '\0';
	/* count how many conjuncts we have. */
	cc = 0;
	for (i = 0; i < ACTUAL_PRED_COUNT; i += 1) {
		if (cube[i] != IGNORE)
			cc += 1;
	}

	if (cc == 0)  {
		strcpy(cubestring, "false");
		return cubestring;
	}

	assert(cc == 1);

	/* get conjuncts. */
	pc = 0;
	cubestring[0] = '\0';

	for (i = 0; i < ACTUAL_PRED_COUNT; i += 1) {

		if (cube[i] == IGNORE)
			continue;

		assert(cube[i] == NEG || cube[i] == POS);

		pc += 1;

		if (i < simple_pred_count) {
			/* == get simple predicate == */
			//printf("%s\n", preds[i]);

			if (pt == TEMP_BOOL_PRINT_MODE) {
				if (cube[i] == NEG) {
					sprintf(temp, "t%d == 0", i);
				} else {
					sprintf(temp, "t%d != 0", i);
				}
				strcat(cubestring, temp);

			} else if (pt == GLOBAL_BOOL_PRINT_MODE) {
				/* == used for printing assertions == */
				if (cube[i] == NEG)
					sprintf(temp, "B%d == 0", i);
				else
					sprintf(temp, "B%d != 0", i);
				strcat(cubestring, temp);
			}

		} else {
			/* == composed predicate, print its components (Yuri translation) == */
			is_first_cube = 1;

			//printf("%s\n", composed_preds[i-pred_count]);

			for (j = 0; j < simple_pred_count; j++) {
				//printf("pred_components[%d][%d] = %c\n", i - pred_count, j, pred_components[i - pred_count][j]);
				v = pred_components[i - simple_pred_count][j];
				if (v == POS) {
					//printf("  PRED: %s\n", preds[j]);
					if (pt == TEMP_BOOL_PRINT_MODE) {
						sprintf(temp, "t%d != 0", j);
					} else if (pt == GLOBAL_BOOL_PRINT_MODE) {
						sprintf(temp, "B%d != 0", j);
					}
				} else if (v == NEG) {
					//printf("  PRED: !%s\n", preds[j]);
					if (pt == TEMP_BOOL_PRINT_MODE) {
						sprintf(temp, "t%d == 0", j);
					} else if (pt = GLOBAL_BOOL_PRINT_MODE) {
						sprintf(temp, "B%d == 0", j);
					}
				} else {
					assert(v == IGNORE);
					continue;
				}
				if (!is_first_cube) {
					strcat(cubestring, " && ");
				} else {
					is_first_cube = 0;
				}
				strcat(cubestring, temp);
			}
		}

		if (pc < cc)
			strcat(cubestring, " && ");
	}
	assert (pc == cc);

	return cubestring;
}

char* getCube(enum predType cube[], enum printType pt) {

	int pc, cc, i;
	char temp[MAX_CUBESTRING_SIZE], *cubestring;

	cubestring = (char *) malloc(MAX_CUBESTRING_SIZE);
	cubestring[0] = '\0';

	/* count how many conjuncts we have. */
	cc = 0;
	for (i = 0; i < ACTUAL_PRED_COUNT; i += 1) {
		if (cube[i] != IGNORE) 
			cc += 1;
	}

	if (cc == 0)  {
		strcpy(cubestring, "false");
		return cubestring;
	}

	/* get conjuncts. */
	pc = 0;	
	cubestring[0] = '\0';
	for (i = 0; i < ACTUAL_PRED_COUNT; i += 1) {

		if (cube[i] == IGNORE)
			continue;

		assert(cube[i] == NEG || cube[i] == POS);

		pc += 1;

		if (pt == TEMP_BOOL_PRINT_MODE) {
			if (cube[i] == NEG)
				sprintf(temp, "t%d == 0", i);
			else 
				sprintf(temp, "t%d != 0", i);
			strcat(cubestring, temp);
		} else if (pt == GLOBAL_BOOL_PRINT_MODE) {
			/* == used for printing assertions == */
			if (cube[i] == NEG)
				sprintf(temp, "B%d == 0", i);
			else
				sprintf(temp, "B%d != 0", i);
			strcat(cubestring, temp);
		} else {
			if (cube[i] == NEG) {
				strcat(cubestring, "!");
			}
			strcat(cubestring, preds[i]);
		}

		if (pc < cc)
			strcat(cubestring, " && ");
	}

	return cubestring;
}

char* myGetCube(char* cube) {

	int pc, cc, i;
	char *cubestring;
	int cube_length;

	cube_length = strlen(cube);
	cubestring = (char *) malloc(MAX_CUBESTRING_SIZE);
	cubestring[0] = '\0';

	/* count how many conjuncts we have. */
	cc = 0;
	for (i = 0; i < cube_length; i += 1) {
		if (cube[i] != 'i')
			cc += 1;
	}

	if (cc == 0)  {
		strcpy(cubestring, "false");
		return cubestring;
	}

	pc = 0;
	cubestring[0] = '\0';
	for (i = 0; i < cube_length; i += 1) {

		if (cube[i] == 'i')
			continue;

		assert(cube[i] == '-' || cube[i] == '+');

		pc += 1;

		if (cube[i] == '-')
			strcat(cubestring, "!");
		strcat(cubestring, preds[i]);


		if (pc < cc)
			strcat(cubestring, " & ");
	}

	return cubestring;
}

int isCovered(enum predType *toBeCovered,
		int toBeCoveredIndex,
		enum predType **cubes,
		int count) {

	int success, i, j;
	enum predType *cover;

	success = 0;

	//printf("START\n");
	for (i = 0; i < toBeCoveredIndex; i += 1) {

		cover = cubes[i];
		success = 1;
		//printf("\nChecking :\n");
		for (j = 0; j < simple_pred_count; j += 1) {
			//printf("cover[%d] = %d, toBeCovered[%d] = %d\n", j, cover[j], j, toBeCovered[j]);
			if ((cover[j] != toBeCovered[j]) && (cover[j] != IGNORE)) {
				//printf("Not Covered \n");
				success = 0;
				break;
			}
		}
		//printf("---- \n");
		if (success == 1)
			return 1;
	}

	//printf("END\n");
	assert(success == 0);
	return 0;
}

/* == in C2BPP, each cube corresponds to a predicate
 * This function takes a cube and returns the idx of the corresponding predicate
 * The result is in [0 .. (simple_pred_count + composed_pred_count)]== */
int getPredIdx(enum predType cube[]) {
	int i, res;
	assert(extrapolate_flag);

	res = -1;
	for (i = 0 ; i < ACTUAL_PRED_COUNT; i++) {
		if (cube[i] != IGNORE) {
			if (i >= simple_pred_count) {
				assert(cube[i] == POS);
			}
			res = i;
			break;
		}
	}
	for(i = ACTUAL_PRED_COUNT; i > res; i--) {
		assert(cube[i] == IGNORE);
	}
	assert(res != -1);
	return res;
	//assert(0 && "cube cannot be empty");
	//return 0;
}

int simplePredCoversComposedPred(int cover_pred_idx, enum predType cover_polarity, int coveree_pred_idx) {

	assert((cover_polarity == POS) || (cover_polarity == NEG));
	return (pred_components[coveree_pred_idx][cover_pred_idx] == cover_polarity);
}

/* == the indices are in [0 .. composed_pred_count]== */
int composedPredCoversComposedPred(int cover_pred_idx, int coveree_pred_idx) {
	int j;

	enum predType *cover_compo, *coveree_compo;

	cover_compo = pred_components[cover_pred_idx];
	coveree_compo = pred_components[coveree_pred_idx];

	for (j = 0; j < simple_pred_count; j++) {
		if ((cover_compo[j] != coveree_compo[j]) && (cover_compo[j] != IGNORE)) {
			return 0;
		}
	}
	return 1;
}

int isCoveredComposedPred(enum predType *target_cube,
		int target_cube_disj_idx,
		enum predType **cubes,
		int count) {

	int target_pred_idx, cover_pred_idx, i, j;
	enum predType *cover;

	assert(extrapolate_flag);

	target_pred_idx = getPredIdx(target_cube);

	if (target_pred_idx < simple_pred_count) {
		/* == nothing covers a simple predicate == */
		//printf("nothing covers a simple pred\n");
		return -1;
	}
	/*== target is a composed pred ==*/

	for (i = 0; i < count; i++) {
		/* == forall cubes in the disjunct == */

		if (i == target_cube_disj_idx) {
			/* == avoid self-cover case == */
			continue;
		}
		cover = cubes[i];
		cover_pred_idx = getPredIdx(cover);
		assert((cover[cover_pred_idx] == POS) || (cover[cover_pred_idx] == NEG));
		if (cover_pred_idx < simple_pred_count) {
			/* == cover is a simple pred == */
			if (simplePredCoversComposedPred(cover_pred_idx,
					cover[cover_pred_idx],
					(target_pred_idx - simple_pred_count))) {
				return cover_pred_idx;
			}
		} else {
			/* == cover is a composed pred == */
			//printf("Coverage Test: \n");
			//printf("  cover: %s \n", composed_preds[cover_pred_idx - simple_pred_count]);
			//printf("  coveree: %s \n", composed_preds[target_pred_idx - simple_pred_count]);

			if (composedPredCoversComposedPred((cover_pred_idx - simple_pred_count),
					(target_pred_idx - simple_pred_count))) {
				return cover_pred_idx;
			}
		}
	}
	return -1;
}

int isTrue(enum predType **cubes){
	int i;

	for (i = 1; i < simple_pred_count; i++) {
		if ((cubes[0][i] != IGNORE) || (cubes[1][i] != IGNORE)) {
			return 0;
		}
	}
	if ((cubes[0][0] == POS) && (cubes[1][0] == NEG)) {
		return 1;
	}
	return 0;
}


char* getAssumption(enum predType **cubes, int disjunct_count, enum printType pt) {

	char *disjunct, *cube;
	int i, at_least_one_cube; /* at least one cube was already added to the disjunct */

	at_least_one_cube = 0; /* initially false */
	disjunct = (char *) malloc(MAX_CUBESTRING_SIZE);
	disjunct[0] = '\0';

	if (disjunct_count == 0) {
		strcat(disjunct, "false");
		return disjunct;
	}

	if ((1 < disjunct_count) && (isTrue(cubes))) {
		strcat(disjunct, "true");
		return disjunct;
	}
	for (i = 0; i < disjunct_count; i++)  {

		if (isCovered(cubes[i], i, cubes, disjunct_count)) {
			continue;
		}

		if (at_least_one_cube) {
			strcat(disjunct, " || ");
		}

		strcat(disjunct, "(");

		cube = getCube(cubes[i], pt);

		strcat(disjunct, cube);
		strcat(disjunct, ")");
		free(cube);
		at_least_one_cube = 1; /* one cube was added */
	}
	return disjunct;
}


char* getDisjunct(enum predType **cubes,
		int disjunct_count,
		enum printType pt) {

	char *disjunct, *cube, *key_cube;
	int i, at_least_one_cube; /* at least one cube was already added to the disjunct */

	gpointer value;

	at_least_one_cube = 0; /* initially false */
	disjunct = (char *) malloc(MAX_CUBESTRING_SIZE);

	disjunct[0] = '\0';

	if (disjunct_count == 0) {
		if (pt == CUBE_PRINT_MODE)
			strcpy(disjunct, "\t");
		strcat(disjunct, "false");
		return disjunct;
	}

	if ((1 < disjunct_count) && (isTrue(cubes))) {
		strcat(disjunct, "true");
		return disjunct;
	}
	assert(disjunct_count < MAX_DISJUNCTS);
	//printf("ALL DISJUNCTS:\n");
	for (i = 0; i < disjunct_count; i++) {
		//printf("%s\n", getComposedPredCube(cubes[i], pt));
	}

	for (i = 0; i < disjunct_count; i++)  {

		if (extrapolate_flag) {
			if (isCoveredComposedPred(cubes[i], i, cubes, disjunct_count) != -1)  {
				continue;
			}
		} else if (isCovered(cubes[i], i, cubes, disjunct_count)) {
			continue;
		}

		if (at_least_one_cube) {
			strcat(disjunct, " || ");
			if ((i != (disjunct_count - 1)) && (pt == CUBE_PRINT_MODE)) {
				strcat(disjunct, " \n ");
			}
		}

		if (pt == CUBE_PRINT_MODE)
			strcat(disjunct, "\t");
		else
			strcat(disjunct, "(");

		cube = (extrapolate_flag) ? getComposedPredCube(cubes[i], pt) : getCube(cubes[i], pt);

		/* == add cube to hashmap == */
		if (!extrapolate_flag) {


			key_cube = getCube(cubes[i], CUBE_PRINT_MODE);
			//printf("key_cube: %s\n", key_cube);

			if (strstr(key_cube, "&")) {
				if ((value = g_hash_table_lookup(used_cubes_choose, key_cube)) == NULL){
					g_hash_table_insert(used_cubes_choose, key_cube, &SMT_0);
				} else {
					free(key_cube);
				}
			}
		}

		strcat(disjunct, cube);

		free(cube);
		at_least_one_cube = 1; /* one cube was added */

		if (pt != CUBE_PRINT_MODE)
			strcat(disjunct, ")");
	}

	return disjunct;
}

void printCubes() {

	int	 	i, j, type;
	char	*disjunct;

	printf("\nCubes: \n------\n \n ");

	for (i = 0; i < stmt_count; i += 1) {

		type = getStmt(i)->type;

		if (type == IF_CONDITIONAL) {
			printf("if %s \n", getStmt(i)->condition);
			printf("   condition: %s, implies (negation of): \n", getStmt(i)->condition);
			disjunct = getDisjunct(getStmt(i)->cubes_neg[0],
					getStmt(i)->cubes_neg_count[0],
					CUBE_PRINT_MODE);
			printf("%s", disjunct);
			printf("\n");
			free(disjunct);
		}
		else if (type == ELSE_CONDITIONAL) {
			int ctx;

			ctx = getStmt(i)->context;
			assert(ctx != -1);
			printf("else \n");
			printf("   condition: !%s, implies (negation of): \n", getStmt(ctx)->condition);
			disjunct = getDisjunct(getStmt(ctx)->cubes_pos[0],
					getStmt(ctx)->cubes_pos_count[0],
					CUBE_PRINT_MODE);
			printf("%s", disjunct);
			printf("\n");
			free(disjunct);
		}
		else if (type == LOAD || type == STORE || type == LOCAL) {

			printf("%s = %s\n", getStmt(i)->lhs, getStmt(i)->rhs);

			for (j = 0; j < simple_pred_count; j += 1) {
				if (contains(preds[j], getStmt(i)->lhs) == -1) {
					continue;
				}

				/* Print positive predicates. */
				printf("   pred:  %s, wlp:  %s, implied by: \n", preds[j], getStmt(i)->wlp[j]);
				disjunct = getDisjunct(getStmt(i)->cubes_pos[j],
						getStmt(i)->cubes_pos_count[j],
						CUBE_PRINT_MODE);
				printf("%s\n", disjunct);
				free(disjunct);

				/* Print negative predicates. */
				printf("   pred: !%s, wlp: !%s, implied by: \n", preds[j], getStmt(i)->wlp[j]);
				disjunct = getDisjunct(getStmt(i)->cubes_neg[j],
						getStmt(i)->cubes_neg_count[j],
						CUBE_PRINT_MODE);
				printf("%s\n", disjunct);
				free(disjunct);
			}
			printf("\n");
		}
	}
}



void reset_local_vars(FILE *f) {

	int i;

	fprintf(f, "/* Reset local variables */ \n");

	/* Reset local variables. */
	for (i = 0; i < simple_pred_count; i++) {
		fprintf(f, "%d: t%d = 0; \n", getUniqueLabel(), i);
	}

}

char* computeWeakModelProgram(int model, int k) {

	char *lhs, *rhs, freevar[MAX_VAR_SIZE];
	int  type, i, j, nextlabel, endlabel;

	for (i = 0; i < stmt_count; i += 1) {

		type = getStmt(i)->type;
		rhs = getStmt(i)->rhs;

		if (type == STORE) {

			if (k == 0) {
				printf("%s store %s = %s;\n", getStmt(i)->label, getStmt(i)->lhs, getStmt(i)->rhs);
			}
			else {
				lhs = getStmt(i)->lhs;
				sprintf(freevar, "fv_%s", lhs);

				endlabel = getUniqueLabel();
				nextlabel = getUniqueLabel();

				for (j = 0; j < k; j++) {

					if (j == 0)
						printf("%s if ((%s = %d)) \n", getStmt(i)->label, freevar, j);
					else
						printf("%d: if ((%s = %d)) \n", nextlabel, freevar, j);


					printf("  %s_%d = %s; \n", lhs, j, rhs);
					printf("  %s = %s + 1; \n", freevar, freevar);
					printf("  goto %d; \n", endlabel);
					printf("else \n");

					if (j == (k - 1))
						nextlabel = endlabel;
					else 
						nextlabel = getUniqueLabel();

					printf("  goto %d; \n", nextlabel);
				}
				printf("%d: nop; \n", endlabel);
			}
		}
	}
	return 0;
}

void printInitBooleanProgram(FILE *f) {

	int 	i;

	/* Print shared variables. */
	fprintf(f, "shared ");
	for (i = 0; i < simple_pred_count; i++) {
		fprintf(f, "B%d", i);
		if (i < (simple_pred_count - 1))
			fprintf(f, ", ");
	}
	fprintf(f, ";\n");

	/* Print local variables. */
	fprintf(f, "local ");
	for (i = 0; i < simple_pred_count; i++) {
		fprintf(f, "t%d", i);
		if (i < (simple_pred_count - 1))
			fprintf(f, ", ");
	}

	fprintf(f, ";\n\n");

	/* Initialize shared vars. */
	fprintf(f, "init\n");
	for (i = 0; i < simple_pred_count; i++)
		fprintf(f, "%d: store B%d = *;\n", getUniqueLabel(), i);
	fprintf(f, "\n");

	/* === add assumption to exclude false states === */

	/* load into temporaries. */
	for (i = 0; i < simple_pred_count; i += 1)
		fprintf(f, "%d: load t%d = B%d;\n", getUniqueLabel(), i, i);
	fprintf(f, "\n");

	/* compute assume disjunct */
	assume_disjunct = getAssumption(getStmt(stmt_count)->cubes_pos[0],
			getStmt(stmt_count)->cubes_pos_count[0],
			GLOBAL_BOOL_PRINT_MODE);

	// fprintf(f, "%d: assume(!(%s)); \n", getUniqueLabel(), assume_disjunct);
}

char *getComposedPredUpdate(int composed_pred_idx, int polarity) {
	int i, first_pred;
	char *disjunct;
	char temp[MAX_CUBESTRING_SIZE];
	enum predType c;

	disjunct = (char *) malloc(MAX_CUBESTRING_SIZE * sizeof(char));
	disjunct[0] = '\0';
	first_pred = 1;

	for(i = 0; i < MAX_SIMPLE_PRED_COUNT; i++) {
		c = pred_components[composed_pred_idx][i];
		if(c != IGNORE) {
			if (polarity == 1) {
				if (!first_pred) {
					strcat(disjunct, " && ");
				}
				if (c == POS) {
					sprintf(temp, "(t%d != 0)", i);
				} else {
					assert(c == NEG);
					sprintf(temp, "(t%d == 0)", i);
				}
				strcat(disjunct, temp);
			} else {
				if (!first_pred) {
					strcat(disjunct, " || ");
				}
				assert(polarity == -1);
				if (c == POS) {
					sprintf(temp, "(t%d == 0)", i);
				} else {
					assert(c == NEG);
					sprintf(temp, "(t%d != 0)", i);
				}
				strcat(disjunct, temp);
			}
			first_pred = 0;
		}
	}

	return disjunct;
}


struct print_context {
	int in_atomic;
	int atomic_if_depth;
	int in_fence;
	int in_flush;
};

typedef struct print_context print_context_t;

void printStmt(FILE *f, stmt_t *stmt, print_context_t *print_ctxt) {
	enum stmtType type;
	char *disjunct;
	int label, ctx, m, n, i, j, k;
	int elselabel, halflabel, endlabel, nr_preds_updated;
	char *disjunctIf, *disjunctElse;
	int temps_to_load[MAX_SIMPLE_PRED_COUNT];
	int first_print;

	type = stmt->type;

	switch (type) {
	case NEW_PROCESS:
		fprintf(f, "\nprocess %d", stmt->pid);
		break;

	case BEG_INIT:
		//set in_atomic
		print_ctxt->in_atomic = 1;
		break;

	case END_INIT:
		//unset in_atomic
		print_ctxt->in_atomic = 0;
		reset_local_vars(f);

		/* add assume after init phase */
		fprintf(f, "%d: assume(!(%s)); \n", getUniqueLabel(), assume_disjunct);

		break;

	case GOTO:
		/* print statement label. */
		if (strlen(stmt->label) != 0) {
			fprintf(f, "/* Statement %s goto %s */ \n", stmt->label, stmt->lhs);
		} else {
			fprintf(f, "/* Statement goto %s */ \n", stmt->lhs);
		}
		if (strlen(stmt->label) == 0)
			fprintf(f, "%d: ", getUniqueLabel());
		else
			fprintf(f, "%s ", stmt->label);
		fprintf(f, "  if (true) %s; \n", stmt->lhs);
		break;

	case IF_CONDITIONAL_STAR:
		if (strlen(stmt->label) == 0)
			fprintf(f, "%d: nop; \n", getUniqueLabel());
		else
			fprintf(f, "%s nop; \n", stmt->label);
		fprintf(f, "\n/* Statement if %s */ \n", stmt->condition);
		stmt->elselabel = getUniqueLabel();
		stmt->endlabel = getUniqueLabel();
		fprintf(f, "\n%d: if (*) goto %d;\n", getUniqueLabel(), stmt->elselabel);
		break;

	case IF_CONDITIONAL:

		/* print statement label. */
		if (strlen(stmt->label) == 0) {
			fprintf(f, "%d: nop; \n", getUniqueLabel());
		} else {
			fprintf(f, "%s nop; \n", stmt->label);
		}
		fprintf(f, "\n/* Statement if %s */ \n", stmt->condition);
		label = getUniqueLabel();
		stmt->elselabel = getUniqueLabel();
		stmt->endlabel = getUniqueLabel();

		disjunct = getDisjunct(stmt->cubes_neg[0],
				stmt->cubes_neg_count[0],
				GLOBAL_BOOL_PRINT_MODE);

		assert(stmt->elselabel != -1);
		fprintf(f, "\n%d: if (*) goto %d;\n", getUniqueLabel(), stmt->elselabel);

		fprintf(f, "%d: assume(!(%s)); \n", label, disjunct);
		/* == if in atomic region, use ifvar coresponding to the depth of the if == */
		if (print_ctxt->in_atomic) {
			print_ctxt->atomic_if_depth = print_ctxt->atomic_if_depth + 1;
		} 
		free(disjunct);
		break;

	case ELSE_CONDITIONAL_STAR:
		ctx = stmt->context;
		assert(ctx != -1);
		fprintf(f, "%d: if (true) goto %d;\n", getUniqueLabel(), getStmt(ctx)->endlabel);
		fprintf(f, "%d: nop;\n", getStmt(ctx)->elselabel);
		break;

	case ELSE_CONDITIONAL:
		ctx = stmt->context;
		assert(ctx != -1);
		fprintf(f, "%d: if (true) goto %d;\n", getUniqueLabel(), getStmt(ctx)->endlabel);
		fprintf(f, "%d: nop;\n", getStmt(ctx)->elselabel);

		disjunct = getDisjunct(getStmt(ctx)->cubes_pos[0],
				getStmt(ctx)->cubes_pos_count[0],
				GLOBAL_BOOL_PRINT_MODE);

		fprintf(f, "%d: assume(!(%s)); \n", getUniqueLabel(), disjunct);

		free(disjunct);
		/* after else is printed, no need to store the cubes anymore */
		freeCubesMemory(getStmt(ctx), 1, MAX_DISJUNCTS);
		break;

	case ENDIF_CONDITIONAL_STAR:
	case ENDIF_CONDITIONAL:
		ctx = stmt->context;
		assert(ctx != -1);
		assert(getStmt(ctx)->endlabel != -1);
		fprintf(f, "%d: nop;\n", getStmt(ctx)->endlabel);
		break;

	case LOAD:
	case STORE:
	case LOCAL:

		memset(temps_to_load, 0, MAX_SIMPLE_PRED_COUNT * sizeof(int));

		/* print statement label. */
		if (strlen(stmt->label) == 0)
			fprintf(f, "%d: nop; \n", getUniqueLabel());
		else
			fprintf(f, "%s nop; \n", stmt->label);
		fprintf(f, "\n/* Statement %s = %s */\n", stmt->lhs, stmt->rhs);

		if (!print_ctxt->in_atomic) {
			fprintf(f, "%d: begin_atomic;\n", getUniqueLabel());
		}

		for (j = 0; j < simple_pred_count; j++) {
			if(contains(preds[j], stmt->lhs) == -1) {
				continue;
			}

			if(isTrue(shared_cubes_pos[j])) {
				continue;
			}

			for (k = 0; k < shared_cubes_pos_count[j]; k++) {
				for (m = 0; m < simple_pred_count; m++) {
					if(shared_cubes_pos[j][k][m] != IGNORE) {
						temps_to_load[m] = 1;
					}
				}
				for (m = 0; m < composed_pred_count; m++) {
					if (shared_cubes_pos[j][k][m + simple_pred_count] != IGNORE) {
						for (n = 0; n < simple_pred_count; n++) {
							if (pred_components[m][n] != IGNORE) {
								assert((pred_components[m][n] == POS) || (pred_components[m][n] == NEG));
								temps_to_load[n] = 1;
							}
						}
					}
				}
			}

			if(isTrue(shared_cubes_neg[j])) {
				continue;
			}

			for (k = 0; k < shared_cubes_neg_count[j]; k++) {

				for (m = 0; m < simple_pred_count; m++) {
					if(shared_cubes_neg[j][k][m] != IGNORE) {
						temps_to_load[m] = 1;
					}
				}

				for (m = 0; m < composed_pred_count; m++) {
					if(shared_cubes_neg[j][k][m + simple_pred_count] != IGNORE) {
						for (n = 0; n < simple_pred_count; n++) {
							if (pred_components[m][n] != IGNORE) {
								assert((pred_components[m][n] == POS) || (pred_components[m][n] == NEG));
								temps_to_load[n] = 1;
							}
						}
					}
				}
			}
		}

		/* == load into temporaries. == */
		for (k = 0; k < simple_pred_count; k += 1) {
			if (!temps_to_load[k]) {
				continue;
			}
			fprintf(f, "%d:  load t%d = B%d;\n", getUniqueLabel(), k, k);
		}
		fprintf(f, "\n");

		/* == update predicates == */
		for (j = 0; j < simple_pred_count; j += 1) {
			if(contains(preds[j], stmt->lhs) == -1) {
				continue;
			}
			disjunctIf   = getDisjunct(shared_cubes_pos[j],
					shared_cubes_pos_count[j],
					TEMP_BOOL_PRINT_MODE);

			disjunctElse = getDisjunct(shared_cubes_neg[j],
					shared_cubes_neg_count[j],
					TEMP_BOOL_PRINT_MODE);

			fprintf(f, " /* update predicate - B%d: %s */\n", j, preds[j]);

			label = getUniqueLabel();
			fprintf(f, "%d:   store B%d = choose(%s, %s);\n", label, j, disjunctIf, disjunctElse);
			free(disjunctIf);
			free(disjunctElse);
			if (j < (simple_pred_count - 1))
				fprintf(f, "\n\n");
		}

		/* == reset temporaries. == */
		for (k = 0; k < simple_pred_count; k += 1) {
			if (!temps_to_load[k]) {
				continue;
			}
			fprintf(f, "%d: t%d = 0;\n", getUniqueLabel(), k);
		}
		fprintf(f, "\n");


		/* == print assume statement after each store/load, except the init section == */
		if(stmt->pid != 0) {
			fprintf(f, "%d: assume(!(%s)); \n", getUniqueLabel(), assume_disjunct);
		}

		if (!print_ctxt->in_atomic) {
			fprintf(f, "\n%d: end_atomic;", getUniqueLabel());
		}
		break;

	case NOPST:
		if (strlen(stmt->label) == 0)
			fprintf(f, "%d: ", getUniqueLabel());
		else
			fprintf(f, "%s ", stmt->label);

		fprintf(f, "%s; \n", stmt->lhs);
		break;

	case ASSERT:
		disjunct = getDisjunct(stmt->cubes_pos[0],
				stmt->cubes_pos_count[0],
				GLOBAL_BOOL_PRINT_MODE);

		fprintf(f, "%s %s; \n", stmt->lhs, disjunct);
		free(disjunct);
		break;

	case BEG_ATOMIC:
		if (strlen(stmt->label) != 0) {
			if (!print_ctxt->in_atomic) {
				fprintf(f, "%s begin_atomic;\n", stmt->label);
				print_ctxt->in_atomic = 1;
			} else {
				fprintf(f, "%s nop;\n", stmt->label);
			}
		} else {
			if (!print_ctxt->in_atomic) {
				fprintf(f, "%d: begin_atomic;\n", getUniqueLabel());
				print_ctxt->in_atomic = 1;
			}
		}
		break;

	case END_ATOMIC:
		assert(print_ctxt->in_atomic == 1);
		if (strlen(stmt->label) == 0) {
			fprintf(f, "%d: end_atomic;\n", getUniqueLabel());
		} else {
			fprintf(f, "%s end_atomic;\n", stmt->label);
		}
		print_ctxt->in_atomic = 0;
		break;

	case COMMENT:
		fprintf(f, "%s\n", stmt->condition);
		if (strstr(stmt->condition, "fence_start") != 0) {
			assert(print_ctxt->in_atomic);
			print_ctxt->in_fence = 1;
		}
		else if (strstr(stmt->condition, "fence_end") != 0) {
			assert(print_ctxt->in_fence);
			print_ctxt->in_fence = 0;
		}
		if (strstr(stmt->condition, "flush_start") != 0) {
			print_ctxt->in_flush = 1;
		}
		else if (strstr(stmt->condition, "flush_end") != 0) {
			assert(print_ctxt->in_flush);
			print_ctxt->in_flush = 0;
		}
		break;

	case ASSUME:
		/* Particular case: assumes are fences */
		first_print = 1;

		for (j = 0; j < simple_pred_count; j += 1) {
			if(strstr(stmt->condition, preds[j]) != 0) {
				if (first_print) {
					first_print = 0;
					fprintf(f, "%d: assume((B%d != 0)", getUniqueLabel(), j);
				} else {
					fprintf(f, " && (B%d != 0)", j);
				}
			}
		}
		if (!first_print) {
			fprintf(f, ");\n");
		}
		break;

	default:
		assert(0 && "Encountered an unexpected statement type");
		break;
	}
	fprintf(f, "\n");
}

void printFlush(FILE *f, int depth, char* var, int proc_nr) {
	int i;

	printAntet(f, no_lbl, depth);

	printAntet(f, no_lbl, depth);
	fprintf(f, "%s = %s_1_p%d;\n", var, var, proc_nr);
	for (i = 2; i <= k_buffer_size; i++){
		printAntet(f, no_lbl, depth);
		fprintf(f, "if (%s_cnt_p%d >= %d)\n", var, proc_nr, i);
		printAntet(f, no_lbl, depth + 1);
		fprintf(f, "%s_%d_p%d = %s_%d_p%d;\n", var, i - 1, proc_nr, var, i, proc_nr);
		printAntet(f, no_lbl, depth);
		fprintf(f, "else\n");
		printAntet(f, no_lbl, depth);
		fprintf(f, "endif\n");
	}
	printAntet(f, no_lbl, depth);
	fprintf(f, "%s_cnt_p%d = %s_cnt_p%d - 1;\n", var, proc_nr, var, proc_nr);
}

void printRandomTsoFlushes(FILE *f, int depth, int proc_nr, int put_atomics) {
	int i;
	int ilbl = getUniqueLabel();
	int end_lbl = getUniqueLabel();

	fprintf(f, "\n /* flush_start */\n");
	if (put_atomics) {
		fprintf(f, "begin_atomic\n");
	}

	printAntet(f, no_lbl, depth);
	fprintf(f, "if (*)\n");
	printAntet(f, no_lbl, depth + 1);
	fprintf(f, "goto %d;\n", end_lbl);
	printAntet(f, no_lbl, depth);
	fprintf(f, "else\n");
	printAntet(f, no_lbl, depth);
	fprintf(f, "endif;\n");
	
	fprintf(f, "%d: nop;\n", ilbl);

	printAntet(f, no_lbl, depth);
	fprintf(f, "if (p%d_cnt > 0)\n", proc_nr);

	for (i = 0; i < global_var_count && (i < nr_weak_global_vars); i++) {
		if (arrayStrSearch(global_vars_modified[proc_nr - 1],
				global_var_modified_count[proc_nr - 1],
				global_vars[i]) == -1) {
			continue;
		}
		printAntet(f, no_lbl, depth);
		fprintf(f, "if (lhs_1_p%d = %d)\n", proc_nr, i);
		printAntet(f, no_lbl, depth + 1);
		fprintf(f, "%s = rhs_1_p%d\n", global_vars[i], proc_nr);
		printAntet(f, no_lbl, depth);
		fprintf(f, "else\n");
		printAntet(f, no_lbl, depth);
		fprintf(f, "endif\n");
	}

	for (i = 1; i < k_buffer_size; i++) {
		printAntet(f, no_lbl, depth + i);
		fprintf(f, "if (p%d_cnt > %d)\n", proc_nr, i);
		printAntet(f, no_lbl, depth + i + 1);
		fprintf(f, "lhs_%d_p%d = lhs_%d_p%d\n", i, proc_nr, i + 1, proc_nr);
		printAntet(f, no_lbl, depth + i + 1);
		fprintf(f, "rhs_%d_p%d = rhs_%d_p%d\n", i, proc_nr, i + 1, proc_nr);

	}

	for (i = k_buffer_size - 1; i >= 1; i--) {
		printAntet(f, no_lbl, depth + i);
		fprintf(f, "else\n");
		printAntet(f, no_lbl, depth + i);
		fprintf(f, "endif\n");
	}

	printAntet(f, no_lbl, depth + i);
	fprintf(f, "p%d_cnt = p%d_cnt -1\n", proc_nr, proc_nr);

	/* == several possible flushes == */
	printAntet(f, no_lbl, depth);
	fprintf(f, "if (*)\n");
	printAntet(f, no_lbl, depth + 1);
	fprintf(f, "goto %d;\n", ilbl);
	printAntet(f, no_lbl, depth);
	fprintf(f, "else\n");
	printAntet(f, no_lbl, depth);
	fprintf(f, "endif;\n");
	/* == end from if (cnt > 0) == */
	printAntet(f, no_lbl, depth);
	fprintf(f, "else\n");
	printAntet(f, no_lbl, depth);
	fprintf(f, "endif;\n");

	printAntet(f, no_lbl, depth);
	fprintf(f, "%d: nop;\n", end_lbl);

	if (put_atomics) {
		printAntet(f, no_lbl, depth);
		fprintf(f, "end_atomic\n");
	}
	fprintf(f, " /* flush_end */\n");
}

void printRandomFlushes(FILE *f, int depth, int proc_nr, int in_atomic) {
	int i;
	int ilbl = getUniqueLabel();

	fprintf(f, "\n /* flush_start */\n");


	fprintf(f, "%d: nop;\n", ilbl);


	for (i = 0; i < global_var_count && (i < nr_weak_global_vars); i++) {

		if (arrayStrSearch(global_vars_modified[proc_nr - 1],
				global_var_modified_count[proc_nr - 1],
				global_vars[i]) == -1) {
			continue;
		}
		if (!in_atomic) {
			fprintf(f, "begin_atomic\n");
		}
		printAntet(f, no_lbl, depth);
		fprintf(f, "if (%s_cnt_p%d != 0)\n", global_vars[i], proc_nr);
		printAntet(f, no_lbl, depth + 1);
		/* == nondeterministic buffer flush == */
		fprintf(f, "if (*)\n");
		printFlush(f, depth + 2, global_vars[i], proc_nr);
		printAntet(f, no_lbl, depth + 1);
		fprintf(f, "else\n");
		printAntet(f, no_lbl, depth + 1);
		fprintf(f, "endif\n");
		printAntet(f, no_lbl, depth);
		fprintf(f, "else\n");
		printAntet(f, no_lbl, depth);
		fprintf(f, "endif\n");
		if (!in_atomic) {
			printAntet(f, no_lbl, depth);
			fprintf(f, "end_atomic\n");
		}
	}
	printAntet(f, no_lbl, depth);
	/* == several possible flushes == */
	if (!in_atomic) {
		fprintf(f, "begin_atomic\n");
	}
	fprintf(f, "if (*)\n");
	printAntet(f, no_lbl, depth + 1);
	fprintf(f, "goto %d;\n", ilbl);
	printAntet(f, no_lbl, depth);
	fprintf(f, "else\n");
	printAntet(f, no_lbl, depth);
	fprintf(f, "endif;\n");
	if (!in_atomic) {
		printAntet(f, no_lbl, depth);
		fprintf(f, "end_atomic\n");
	}
	fprintf(f, " /* flush_end */\n");
}


/* Print in c2bp format. */
void printTsoProgram(char *file_name) {

	int	i, j, k, label, put_atomics, type;
	int 	beg_init = 0;
	char 	*lbl;
	int 	depth, pid, p;
	FILE 	*f;

	f = fopen(file_name, "w");
	if (!f) {
		printf("could not open file: %s\n", file_name);
		exit(EXIT_FAILURE);
	}

	put_atomics = 1;

	for (i = 0; i < stmt_count; i += 1) {

		type = getStmt(i)->type;
		lbl = getStmt(i)->label;
		depth = getStmt(i)->context_depth;
		pid = getStmt(i)->pid;

		if (type == IF_CONDITIONAL || type == ELSE_CONDITIONAL || type == ENDIF_CONDITIONAL) {
			printAntet(f, lbl, depth - 1);
		} else {
			printAntet(f, lbl, depth);
		}

		if (type == NEW_PROCESS) {
			fprintf(f, "process %d:\n\n", pid);
		}

		else if (type == BEG_INIT) {
			fprintf(f, "beginit\n");
			beg_init = 1;
		}

		else if (type == END_INIT) {
			/* init buffer counters */
			for (p = 0; p < current_pid; p++) {
				fprintf(f, "p%d_cnt = 0;\n", p + 1);
			}

			/* init overflow var */
			fprintf(f, "overflow = 0;\n\n");

			printAntet(f, lbl, depth);
			fprintf(f, "endinit\n");
			beg_init = 0;
		}

		else if (type == GOTO) {
			fprintf(f, "%s; \n", getStmt(i)->lhs);
		}
		else if (type == IF_CONDITIONAL) {
			fprintf(f, "if (%s)\n", getStmt(i)->condition);
		}
		else if (type == ELSE_CONDITIONAL) {
			fprintf(f, "else\n");
		}
		else if (type == ENDIF_CONDITIONAL) {
			fprintf(f, "endif;\n");
		} else if (type == LOAD || type == STORE || type == LOCAL) {

			int     elselabel, halflabel, endlabel, var_idx;
			char	*disjunctIf, *disjunctElse;

			if (type == STORE) {
				if (beg_init) {
					fprintf(f, "store %s = %s;\n", getStmt(i)->lhs, getStmt(i)->rhs);
				} else {

					/* == determine index of the lhs global variable ==*/
					var_idx = getGlobalVarIdx(getStmt(i)->lhs);

					if (var_idx >= nr_weak_global_vars ||
							arrayStrSearch(global_vars_modified[pid - 1], global_var_modified_count[pid - 1], getStmt(i)->lhs) == -1) {
						fprintf(f, "store %s = %s;\n", getStmt(i)->lhs, getStmt(i)->rhs);
					} else {
						if (put_atomics) {
							fprintf(f, "begin_atomic\n");

							printAntet(f, no_lbl, depth);
						}
						fprintf(f, "/* Statement: store %s = %s; */\n", getStmt(i)->lhs, getStmt(i)->rhs);

						printAntet(f, no_lbl, depth);
						fprintf(f, "if (p%d_cnt = %d)\n", pid, k_buffer_size);
						printAntet(f, no_lbl, depth + 1);
						fprintf(f, "overflow = 1;\n");
						printAntet(f, no_lbl, depth);
						fprintf(f, "else\n");
						printAntet(f, no_lbl, depth);
						fprintf(f, "endif;\n");

						fprintf(f, "p%d_cnt = p%d_cnt + 1;\n", pid, pid);

						for (j = 1; j <= k_buffer_size; j++) {
							printAntet(f, no_lbl, depth);
							fprintf(f, "if (p%d_cnt = %d)\n", pid, j);
							printAntet(f, no_lbl, depth + 1);
							fprintf(f, "lhs_%d_p%d = %d;\n", j, pid, var_idx);

							printAntet(f, no_lbl, depth + 1);
							fprintf(f, "rhs_%d_p%d = %s;\n", j, pid, getStmt(i)->rhs);

							printAntet(f, no_lbl, depth);
							fprintf(f, "else\n");
							printAntet(f, no_lbl, depth);
							fprintf(f, "endif;\n");
						}
						if (put_atomics) {
							printAntet(f, no_lbl, depth);
							fprintf(f, "end_atomic\n");
						}
					}
				}
			} else if (type == LOAD) {

				fprintf(f, "nop;\n");

				var_idx = getGlobalVarIdx(getStmt(i)->rhs);

				if (var_idx >= nr_weak_global_vars ||
						arrayStrSearch(global_vars_modified[pid - 1], global_var_modified_count[pid - 1], getStmt(i)->rhs) == -1) {
					fprintf(f, "load %s = %s;\n", getStmt(i)->lhs, getStmt(i)->rhs);
				} else {
					int end_lbl = getUniqueLabel();
					int lbls[MAX_BUFFER_SIZE];

					printAntet(f, no_lbl, depth);
					fprintf(f, "/* Statement: load %s = %s; */\n", getStmt(i)->lhs, getStmt(i)->rhs);
					if (put_atomics) {
						printAntet(f, no_lbl, depth);
						fprintf(f, "begin_atomic\n");
					}

					for (j = 0; j <= k_buffer_size; j++) {

						lbls[j] = getUniqueLabel();
						printAntet(f, no_lbl, depth + j);
						fprintf(f, "if (p%d_cnt = %d)\n", pid, j);
						printAntet(f, no_lbl, depth + j + 1);
						fprintf(f, "goto %d\n", lbls[j]);

						printAntet(f, no_lbl, depth + j + 1);
						fprintf(f, "else\n");
						printAntet(f, no_lbl, depth + j);
						fprintf(f, "endif\n");
					}

					for (j = k_buffer_size; j > 0; j--) {
						printAntet(f, no_lbl, depth);
						fprintf(f, "%d: nop;\n", lbls[j]);
						printAntet(f, no_lbl, depth);
						fprintf(f, "if (lhs_%d_p%d = %d)\n", j, pid, var_idx);
						printAntet(f, no_lbl, depth + 1);
						fprintf(f, "%s = rhs_%d_p%d\n", getStmt(i)->lhs, j, pid);
						printAntet(f, no_lbl, depth + 1);
						fprintf(f, "goto %d\n", end_lbl);
						printAntet(f, no_lbl, depth);
						fprintf(f, "else\n");
						printAntet(f, no_lbl, depth);
						fprintf(f, "endif\n");
					}

					printAntet(f, no_lbl, depth);
					fprintf(f, "%d: load %s = %s;\n", lbls[0], getStmt(i)->lhs, getStmt(i)->rhs);
					printAntet(f, no_lbl, depth);
					fprintf(f, "%d: nop;\n", end_lbl);
					if (put_atomics) {
						printAntet(f, no_lbl, depth);
						fprintf(f, "end_atomic\n");
					}
				}
			} else if (type == LOCAL) {
				fprintf(f, "%s = %s;\n", getStmt(i)->lhs, getStmt(i)->rhs);
			}
		}
		else if (type == NOPST)  {
			fprintf(f, "%s;\n", getStmt(i)->lhs);
		}
		else if (type == ASSERT) {
			fprintf(f, "%s %s;\n", getStmt(i)->lhs, getStmt(i)->condition);
		}
		else if (type == FENCE) {
			int end_lbl = getUniqueLabel();
			int lbls[MAX_BUFFER_SIZE];
			if (put_atomics) {
				fprintf(f, "begin_atomic\n");
			/* == flushes only variables with buffered stores == */
				printAntet(f, no_lbl, depth);
			}

			fprintf(f, "/* Statement: fence_start; */\n");

			printAntet(f, no_lbl, depth);

			fprintf(f, "assume (p%d_cnt = 0);\n", pid);

			printAntet(f, no_lbl, depth);
			fprintf(f, "/* fence_end */\n");
			if (put_atomics) {
				printAntet(f, no_lbl, depth);
				fprintf(f, "end_atomic\n");
			}
		}
		else if (type == FLUSH) {
			if (put_atomics) {
				fprintf(f, "begin_atomic\n");

				printAntet(f, no_lbl, depth);
			}
			fprintf(f, "%s = %s_1_p%d;\n", getStmt(i)->lhs, getStmt(i)->lhs, pid);

			for (j = 1; j < k_buffer_size; j++) {
				printAntet(f, no_lbl, depth + j - 1);
				fprintf(f, "if (%s_cnt_p%d != %d)\n", getStmt(i)->lhs, pid, j);
				printAntet(f, no_lbl, depth + j);
				fprintf(f, "%s_%d_p%d = %s_%d_p%d;\n", getStmt(i)->lhs, j, pid,
						getStmt(i)->lhs, j + 1, pid);

			}
			for (j = k_buffer_size - 1; j > 0; j++) {
				printAntet(f, no_lbl, depth + j - 1);
				fprintf(f, "else\n");
				printAntet(f, no_lbl, depth + j - 1);
				fprintf(f, "endif;\n");
			}

			printAntet(f, no_lbl, depth);
			fprintf(f, "if (");
			for (j = 0; j < k_buffer_size; j++) {
				fprintf(f, "(%s_cnt_p%d != %d)", getStmt(i)->lhs, pid, j);
				if (j != k_buffer_size - 1) {
					fprintf(f, " && ");
				}
			}
			fprintf(f, ")\n");
			printAntet(f, no_lbl, depth + 1);
			fprintf(f, "%s_cnt_p%d = %d;\n", getStmt(i)->lhs, pid, k_buffer_size - 1);
			printAntet(f, no_lbl, depth);
			fprintf(f, "else\n");
			printAntet(f, no_lbl, depth + 1);
			fprintf(f, "%s_cnt_p%d = %s_cnt_p%d - 1;\n", getStmt(i)->lhs, pid, getStmt(i)->lhs, pid);
			printAntet(f, no_lbl, depth);
			fprintf(f, "endif;\n");
			if (put_atomics) {
				printAntet(f, no_lbl, depth);
				fprintf(f, "end_atomic\n");
			}
		}
		else if (type == COMMENT) {
			fprintf(f, "%s\n", getStmt(i)->condition);
		}
		else if (type == BEG_ATOMIC) {
			put_atomics = 0;
			fprintf(f, "begin_atomic\n");
		}
		else if (type == END_ATOMIC) {
			put_atomics = 1;
			fprintf(f, "end_atomic\n");
		}
		else
			assert(0 && "Encountered an unexpected statement type");

		if (((type != BEG_ATOMIC) && (type != END_ATOMIC) && (type != COMMENT) && (type != FENCE) && (type != BEG_INIT) && (type != END_INIT) && (type != ASSERT) && !beg_init) && add_random_flushes) {
			printRandomTsoFlushes(f, depth, pid, put_atomics);
		}

		fprintf(f, "\n");
	}
	fclose(f);
}


/* Print in c2bp format. */
void printPsoProgram(char *file_name) {
	FILE *f;
	int in_atomic;

	f = fopen(file_name, "w");
	if (!f) {
		printf("could not create file %s\n", file_name);
		exit(EXIT_FAILURE);
	}

	int	i, j, k, p, label, type;
	int 	beg_init = 0;
	char 	*lbl;
	int 	depth, pid;

	in_atomic = 0;

	for (i = 0; i < stmt_count; i += 1) {

		type = getStmt(i)->type;
		lbl = getStmt(i)->label;
		depth = getStmt(i)->context_depth;
		pid = getStmt(i)->pid;

		if (type == IF_CONDITIONAL || type == ELSE_CONDITIONAL || type == ENDIF_CONDITIONAL) {
			printAntet(f, lbl, depth - 1);
		} else {
			printAntet(f, lbl, depth);
		}

		if (type == NEW_PROCESS) {
			fprintf(f, "process %d:\n\n", pid);
		}

		else if (type == BEG_INIT) {
			fprintf(f, "beginit\n");
			beg_init = 1;
		}

		else if (type == END_INIT) {

			/* initialize counters for vars with buffered stores */
			for (p = 0; p < current_pid; p++) {
				for (j = 0; (j < global_var_count) && (j < nr_weak_global_vars); j++) {
					if (arrayStrSearch(global_vars_modified[p],
							global_var_modified_count[p],
							global_vars[j]) == -1) {
						continue;
					}
					printAntet(f, no_lbl, depth);
					fprintf(f, "%s_cnt_p%d = 0;\n", global_vars[j], p + 1);
				}
			}

			/* initialize overflow var */
			fprintf(f, "overflow = 0;\n\n");
			printAntet(f, lbl, depth);
			fprintf(f, "endinit\n");
			beg_init = 0;
		}
		else if (type == GOTO) {
			fprintf(f, "%s; \n", getStmt(i)->lhs);
		}
		else if (type == IF_CONDITIONAL) {
			fprintf(f, "if (%s)\n", getStmt(i)->condition);
		}
		else if (type == ELSE_CONDITIONAL) {
			fprintf(f, "else\n");
		}
		else if (type == ENDIF_CONDITIONAL) {
			fprintf(f, "endif;\n");
		} else if (type == LOAD || type == STORE || type == LOCAL) {

			int     elselabel, halflabel, endlabel, var_idx;
			char	*disjunctIf, *disjunctElse;

			if (type == STORE) {
				if (beg_init) {
					fprintf(f, "store %s = %s;\n", getStmt(i)->lhs, getStmt(i)->rhs);
				} else {

					/* == determine index of the lhs global variable ==*/
					var_idx = getGlobalVarIdx(getStmt(i)->lhs);

					if (var_idx >= nr_weak_global_vars ||
							arrayStrSearch(global_vars_modified[pid - 1], global_var_modified_count[pid - 1], getStmt(i)->lhs) == -1) {
						fprintf(f, "store %s = %s;\n", getStmt(i)->lhs, getStmt(i)->rhs);
					} else {

						if (!in_atomic) { 
							fprintf(f, "begin_atomic\n");
							printAntet(f, no_lbl, depth);
						}
						fprintf(f, "/* Statement: store %s = %s; */\n", getStmt(i)->lhs, getStmt(i)->rhs);

						/* == print case when buffer overflows == */
						printAntet(f, no_lbl, depth);

						fprintf(f, "if (%s_cnt_p%d = %d)\n", getStmt(i)->lhs, pid, k_buffer_size);

						printAntet(f, no_lbl, depth + 1);
						fprintf(f, "overflow = 1;\n");
						printAntet(f, no_lbl, depth);
						fprintf(f, "else\n");
						printAntet(f, no_lbl, depth);
						fprintf(f, "endif;\n");

						fprintf(f, "%s_cnt_p%d = %s_cnt_p%d + 1;\n", getStmt(i)->lhs, getStmt(i)->pid, getStmt(i)->lhs, pid);

						for (j = 1; j <= k_buffer_size; j++) {
							printAntet(f, no_lbl, depth);
							fprintf(f, "if (%s_cnt_p%d = %d)\n", getStmt(i)->lhs, pid, j);
							printAntet(f, no_lbl, depth + 1);
							fprintf(f, "%s_%d_p%d = %s;\n", getStmt(i)->lhs, j, pid, getStmt(i)->rhs);
							printAntet(f, no_lbl, depth);
							fprintf(f, "else\n");
							printAntet(f, no_lbl, depth);
							fprintf(f, "endif;\n");
						}

						if (!in_atomic) {
							printAntet(f, no_lbl, depth);
							fprintf(f, "end_atomic\n");
						}
					}
				}
			} else if (type == LOAD) {

				fprintf(f, "nop;\n");

				var_idx = getGlobalVarIdx(getStmt(i)->rhs);

				if (var_idx >= nr_weak_global_vars ||
						arrayStrSearch(global_vars_modified[pid - 1], global_var_modified_count[pid - 1], getStmt(i)->rhs) == -1) {
					fprintf(f, "load %s = %s;\n", getStmt(i)->lhs, getStmt(i)->rhs);
				} else {

					printAntet(f, no_lbl, depth);
					fprintf(f, "/* Statement: load %s = %s; */\n", getStmt(i)->lhs, getStmt(i)->rhs);
					if (!in_atomic) {
						printAntet(f, no_lbl, depth);
						fprintf(f, "begin_atomic\n");
					}
					printAntet(f, no_lbl, depth);
					fprintf(f, "if (%s_cnt_p%d = 0)\n", getStmt(i)->rhs, pid);
					printAntet(f, no_lbl, depth + 1);
					fprintf(f, "load %s = %s;\n", getStmt(i)->lhs, getStmt(i)->rhs);
					printAntet(f, no_lbl, depth);
					fprintf(f, "else\n");
					printAntet(f, no_lbl, depth);
					fprintf(f, "endif;\n");
					for (j = 1; j <= k_buffer_size; j++) {
						printAntet(f, no_lbl, depth);
						fprintf(f, "if (%s_cnt_p%d = %d)\n", getStmt(i)->rhs, pid, j);
						printAntet(f, no_lbl, depth + 1);
						fprintf(f, "%s = %s_%d_p%d;\n", getStmt(i)->lhs, getStmt(i)->rhs, j, pid);
						printAntet(f, no_lbl, depth);
						fprintf(f, "else\n");
						printAntet(f, no_lbl, depth);
						fprintf(f, "endif;\n");
					}

					if (!in_atomic) {
						printAntet(f, no_lbl, depth);
						fprintf(f, "end_atomic\n");
					}
				}
			} else if (type == LOCAL) {
				fprintf(f, "%s = %s;\n", getStmt(i)->lhs, getStmt(i)->rhs);
			}
		}
		else if (type == NOPST)  {
			fprintf(f, "%s;\n", getStmt(i)->lhs);
		}
		else if (type == ASSERT) {
			fprintf(f, "%s %s;\n", getStmt(i)->lhs, getStmt(i)->condition);
		}
		else if (type == FENCE) {
			if (!in_atomic) {
				fprintf(f, "begin_atomic\n");
			/* == flushes only variables with buffered stores == */
				printAntet(f, no_lbl, depth);
			}
			fprintf(f, "/* Statement: fence_start; */\n");

			int first_print = 1;

			for (j = 0; (j < global_var_count) && (j < nr_weak_global_vars); j++) {

				if (arrayStrSearch(global_vars_modified[pid - 1],
						global_var_modified_count[pid - 1],
						global_vars[j]) == -1) {
					continue;
				}
				if (first_print) {
					first_print = 0;
					fprintf(f, "assume((%s_cnt_p%d = 0)", global_vars[j], pid);
				} else {
					fprintf(f, "& (%s_cnt_p%d = 0)", global_vars[j], pid);
				}
/*
				printAntet(f, no_lbl, depth);
				//printf("if (%s_cnt_p%d != 0)\n", global_vars[j], pid);

				for (k = 1; k <= k_buffer_size; k++) {
					printAntet(f, no_lbl, depth + k );
					fprintf(f, "if (%s_cnt_p%d = %d)\n", global_vars[j], pid, k);
					printAntet(f, no_lbl, depth + k + 1);
					fprintf(f, "%s = %s_%d_p%d;\n", global_vars[j], global_vars[j], k, pid);
					printAntet(f, no_lbl, depth + k );
					fprintf(f, "else\n");
					printAntet(f, no_lbl, depth + k );
					fprintf(f, "endif;\n");
				}


				printAntet(f, no_lbl, depth + 1);
				fprintf(f, "%s_cnt_p%d = 0;\n", global_vars[j], pid);
*/

			}
			if (!first_print) {
				fprintf(f, ");\n");
			}

			printAntet(f, no_lbl, depth);
			fprintf(f, "/* fence_end */\n");
			if (!in_atomic) {
				printAntet(f, no_lbl, depth);
				fprintf(f, "end_atomic\n");
			}
		}
		else if (type == FLUSH) {

			if (!in_atomic) {
				fprintf(f, "begin_atomic\n");
				printAntet(f, no_lbl, depth);
			}

			fprintf(f, "%s = %s_1_p%d;\n", getStmt(i)->lhs, getStmt(i)->lhs, pid);

			for (j = 1; j < k_buffer_size; j++) {
				printAntet(f, no_lbl, depth + j - 1);
				fprintf(f, "if (%s_cnt_p%d != %d)\n", getStmt(i)->lhs, pid, j);
				printAntet(f, no_lbl, depth + j);
				fprintf(f, "%s_%d_p%d = %s_%d_p%d;\n", getStmt(i)->lhs, j, pid,
						getStmt(i)->lhs, j + 1, pid);

			}
			for (j = k_buffer_size - 1; j > 0; j++) {
				printAntet(f, no_lbl, depth + j - 1);
				fprintf(f, "else\n");
				printAntet(f, no_lbl, depth + j - 1);
				fprintf(f, "endif;\n");
			}


			printAntet(f, no_lbl, depth );
			fprintf(f, "%s_cnt_p%d = %s_cnt_p%d - 1;\n", getStmt(i)->lhs, pid, getStmt(i)->lhs, pid);
			//printAntet(no_lbl, depth);
			//printf("endif;\n");
			if (!in_atomic) {
				printAntet(f, no_lbl, depth);
				fprintf(f, "end_atomic\n");
			}
		}
		else if (type == COMMENT) {
			fprintf(f, "%s\n", getStmt(i)->condition);
		}
		else if (type == BEG_ATOMIC) {
			in_atomic = 1;
			printAntet(f, no_lbl, depth);
			fprintf(f, "begin_atomic\n");
		}
		else if (type == END_ATOMIC) {
			printAntet(f, no_lbl, depth);
			fprintf(f, "end_atomic\n");
			in_atomic = 0;
		}
		else
			assert(0 && "Encountered an unexpected statement type");

		if (((type != BEG_ATOMIC) && (type != END_ATOMIC) && (type != COMMENT) && (type != FENCE) && (type != BEG_INIT) && (type != END_INIT) && (type != ASSERT) && !beg_init) && add_random_flushes) {
			printRandomFlushes(f, depth, pid, in_atomic);
		}

		fprintf(f, "\n");
	}
	fclose(f);
}

void printStats(int stmt_idx, int pred_idx) {
	enum stmtType type;
	char	*disjunct;

	type = getStmt(stmt_idx)->type;
	printf("=================\n");
	if ((type == LOAD) || (type == STORE) || (type == LOCAL)) {

		printf("pred: %s\n", getStmt(stmt_idx)->wlp[pred_idx]);
		printf("%d calls 2 Y, %d cache hits\n",
				SMT_stats[stmt_idx][pred_idx],
				cache_hits[stmt_idx][pred_idx]);
		printf("   pred:  %s, wlp:  %s, implied by: \n",
				preds[pred_idx],
				getStmt(stmt_idx)->wlp[pred_idx]);
		disjunct = getDisjunct(getStmt(stmt_idx)->cubes_pos[pred_idx],
				getStmt(stmt_idx)->cubes_pos_count[pred_idx],
				CUBE_PRINT_MODE);
		printf("%s\n", disjunct);
		free(disjunct);

		/* Print negative predicates. */
		printf("   pred: !%s, wlp: !%s, implied by: \n",
				preds[pred_idx],
				getStmt(stmt_idx)->wlp[pred_idx]);
		disjunct = getDisjunct(getStmt(stmt_idx)->cubes_neg[pred_idx],
				getStmt(stmt_idx)->cubes_neg_count[pred_idx],
				CUBE_PRINT_MODE);
		printf("%s\n", disjunct);
		free(disjunct);
	}
	else if (type == IF_CONDITIONAL) {
		printf("if cond: %s\n", getStmt(stmt_idx)->condition);
		printf("%d calls 2 Y, %d cache hits\n", SMT_stats[stmt_idx][0], cache_hits[stmt_idx][0]);
	}
	else if (type == ASSERT) {
		printf("assert cond: %s\n", getStmt(stmt_idx)->condition);
		printf("%d calls 2 Y, %d cache hits\n", SMT_stats[stmt_idx][0], cache_hits[stmt_idx][0]);
	}
}

void parseCodeFile(char *code_file_name) {
	FILE 	*code_file;
	char  	line[MAX_LINE];
	int 	in_atomic, atomic_nested_ifs;

	/* open code file */
	code_file = fopen(code_file_name, "r");
	if (!code_file) {
		printf("%s code file not found\n", code_file_name);
		exit(EXIT_FAILURE);
	}

	/* parse program */
	while(fgets(line, sizeof(line), code_file)) {
		if (parseAndStoreStatement(line, stmt_count, &in_atomic, &atomic_nested_ifs) != 0) {
			stmt_count += 1;
		}
	}
	fclose(code_file);
	//printf("prog parse done\n");

	/* == starting value used for generating fresh labels == */
	beg_label = max_label + 1;
}

void parsePredFile(char *pred_file_name) {
	FILE 	*pred_file;
	char  	line[MAX_LINE];

	/* open predicate file */
	pred_file = fopen(pred_file_name, "r");
	if (!pred_file) {
		printf("%s predicate file not found\n", pred_file_name);
		exit(EXIT_FAILURE);
	}

	while(fgets(line, sizeof(line), pred_file)) {
		if (strlen(line) < 2) {
			//assert(extrapolate_flag);
			/* == simple preds are separated by a newline from composed preds == */
			while(fgets(line, sizeof(line), pred_file)) {
				parseAndStoreComposedPredicate(line, composed_pred_count);
				composed_pred_count += 1;
				//printf("composed pred: %d\n", composed_pred_count);
			}
		} else {
			parseAndStorePredicate(line, simple_pred_count);
			simple_pred_count += 1;
			//printf("simple pred: %d\n", pred_count);
		}
	}

	fclose(pred_file);
}

void genBoolProg(char *bool_file_name) {
	FILE 			*bool_file;
	print_context_t 	*print_ctxt;
	stmt_t 			*stmt;
	int 			SMT_in_flush, SMT_before, i, j;
	struct timeval 		start_time, end_time;

	gettimeofday(&start_time, NULL);

	/* open predicate file */
	bool_file = fopen(bool_file_name, "w");
	if (!bool_file) {
		printf("could not create file: %s\n", bool_file_name);
		exit(EXIT_FAILURE);
	}

	int s = 0;

	/* init print context */

	print_ctxt = (print_context_t *) malloc(sizeof(print_context_t));
	print_ctxt->in_atomic = 0;
	print_ctxt->in_fence = 0;
	print_ctxt->in_flush = 0;
	print_ctxt->atomic_if_depth = 1;

	printInitBooleanProgram(bool_file);

	//printf("%s\n", assume_disjunct);

	SMT_before = 0;

	for (i = 0; i < stmt_count; i += 1) {

		//printf("line %d\n", i);
		//fflush(stdout);
		stmt = getStmt(i);

		switch(stmt->type) {
		case LOAD:
		case STORE:
		case LOCAL:

			/* reset shared memory */
			memset(shared_cubes_pos_count, 0, sizeof(int) * MAX_SIMPLE_PRED_COUNT);
			memset(shared_cubes_neg_count, 0, sizeof(int) * MAX_SIMPLE_PRED_COUNT);

			for (j = 0; j < simple_pred_count; j += 1) {

				if(!contains(preds[j], stmt->lhs)) {
					continue;
				}
				computeWlpAssignment(stmt, j);
				if (extrapolate_flag) {
					//printf("wlp: %s\n", getStmt(i)->wlp[j]);
					myComputeCubesPrime(stmt, j, stmt->wlp[j], max_cubes, i);
				} else {
					myComputeCubes(stmt, j, stmt->wlp[j], max_cubes, i);
				}
			}
			break;

		case IF_CONDITIONAL:

			allocateCubesMemory(stmt, 1, MAX_DISJUNCTS, MAX_TOTAL_PRED_COUNT);

			if (extrapolate_flag) {
				myComputeCubesPrime(stmt, 0, stmt->condition, max_cubes, i);
			} else {
				myComputeCubes(stmt, 0, stmt->condition, max_cubes, i);
			}
			break;

		case ASSERT:

			allocateCubesMemory(stmt, 1, MAX_DISJUNCTS, MAX_TOTAL_PRED_COUNT);

			/* == compute cubes that imply the assertion predicate == */
			if (extrapolate_flag) {
				myComputeCubesPrime(stmt, 0, stmt->condition, max_cubes, i);
			} else {
				myComputeCubes(stmt, 0, stmt->condition, max_cubes, i);
			}
		}

		printStmt(bool_file, stmt, print_ctxt);

		SMT_per_stmt[i] = SMT_total_calls - SMT_before;
		SMT_before = SMT_total_calls;

		if (print_ctxt->in_flush) {
			SMT_in_flush += SMT_per_stmt[i];
		}
	}

	fclose(bool_file);
	gettimeofday(&end_time, NULL);

	char stats_file_name[500];
	sprintf(stats_file_name, "%s.stats", bool_file_name);
	FILE *f = fopen(stats_file_name, "w");

	/* print stats */

	for (i = 0; i < MAX_NR_PROC; i++) {
		fprintf(f, "proc %d modifies:\n", i);
		for (j = 0; j < global_var_modified_count[i]; j++) {
			fprintf(f, "%s\n", global_vars_modified[i][j]);
		}
	}

	/* == caching == */
	fprintf(f, "%d calls to SMT\n", SMT_total_calls);
	fprintf(f, "%d SMT for flush\n", SMT_in_flush);

	int milliseconds = (end_time.tv_sec - start_time.tv_sec) * 1000 + 
				(((int)end_time.tv_usec - (int)start_time.tv_usec)/1000);

	fprintf(f, "%d time\n", milliseconds);

	/* == distinct cubes caching == */
	fprintf(f, "%d distinct cubes:\n", g_hash_table_size(used_cubes_choose));
	GList *keys = g_hash_table_get_keys(used_cubes_choose);
	char *cube;
	while (keys != NULL) {
		cube = (char*)(keys->data);
		fprintf(f, "%s\n", cube);
		free(cube);
		keys = keys->next;
	}


	fclose(f);
}

void genRenamedPreds(int wmm, char *original_preds_file_name, char *renamed_preds_file_name) {
	FILE 	*renamed_preds_file, *original_preds_file;
	char  	line[MAX_LINE];

	renamed_preds_file = fopen(renamed_preds_file_name, "w");
	if (!renamed_preds_file) {
		printf("could not create file %s\n", renamed_preds_file_name);
		exit(EXIT_FAILURE);
	}

	original_preds_file = fopen(original_preds_file_name, "r");
	if (!original_preds_file) {
		printf("did not find pred file %s\n", original_preds_file_name);
		exit(EXIT_FAILURE);
	}

	fprintf(renamed_preds_file, "(overflow = 0)\n");

	if (wmm == TSO) {
		genTsoCounterPreds(renamed_preds_file);
	} else {
		genPsoCounterPreds(renamed_preds_file);	
	}

	while(fgets(line, sizeof(line), original_preds_file)) {
		if (strlen(line) < 2) {
			fprintf(renamed_preds_file, "%s", line);
			assert(extrapolate_flag);
			/* == simple preds are separated by a newline from composed preds == */
			while(fgets(line, sizeof(line), original_preds_file)) {
				fprintf(renamed_preds_file, "%s", line);
				if (wmm == TSO) {
					genLessTsoPreds(renamed_preds_file, line);
				} else {
					genLessPsoPreds(renamed_preds_file, line);
					//genPsoPreds(renamed_preds_file, line, 0);
				}
				//printf("composed pred: %d\n", composed_pred_count);
			}
		} else {
			fprintf(renamed_preds_file, "%s", line);
			if (wmm == TSO) {
				genLessTsoPreds(renamed_preds_file, line);
			} else {
				genLessPsoPreds(renamed_preds_file, line);
				//genPsoPreds(renamed_preds_file, line, 0);
			}
			//printf("simple pred: %d\n", pred_count);
		}
	}
	fclose(renamed_preds_file);
	fclose(original_preds_file);
}


int main(int argc, char *argv[]) {

	char  	line[MAX_LINE], temp_file_name[500];
	int   	i, j, SMT_before, start_stmt, end_stmt, in_atomic, atomic_nested_ifs;
	int		SMT_in_flush;
	FILE  	*fp, *tfp;
	char 	*pred, *var;
	char	*disjunct;
	char	file_name[500];
	int		wmm;
	int		gen_bool_prog;

	gen_bool_prog = 0;
	in_atomic = 0;
	atomic_nested_ifs = 0;
	SMT_in_flush = 0;

	/* incorrect call to c2bp */
	if ((argc < 4) || (argc > 8)) {
		printf("usage: %s <SLAM_BOOL_PROG|X_BOOL_PROG> <max_cube_size> <code_file> <predicates_file>\n", argv[0]);
		printf("usage: %s ENCODE_BUFFERS <PSO|TSO> <code_file> <predicates_file> <# buffered vars> <buffer size> <YF|NF>\n", argv[0]);
		printf("YF - with flushes | NF - no flushes\n");
		return -1;
	}

	if (strcmp(argv[1], "SLAM_BOOL_PROG") == 0) {
		extrapolate_flag = 0;
		gen_bool_prog = 1;
	} else if (strcmp(argv[1], "X_BOOL_PROG") == 0) {
		extrapolate_flag = 1;
		gen_bool_prog = 1;
	} else if (strcmp(argv[1], "ENCODE_BUFFERS") == 0) {
		extrapolate_flag = 1;
		gen_bool_prog = 0;
	}

	initState();

	if (gen_bool_prog) {
		max_cubes = atoi(argv[2]);

		/* parse code file */
		parseCodeFile(argv[3]);

		/* parse predicates file */
		parsePredFile(argv[4]);

		/* == compute invalid combinations of predicates == */
		allocateCubesMemory(getStmt(stmt_count), 1, MAX_DISJUNCTS, MAX_TOTAL_PRED_COUNT);
		myComputeAssumption(getStmt(stmt_count), 0, FALSE_PREDICATE, max_cubes);

		/* generate boolean program */
		sprintf(file_name, "%s.bl", argv[3]);
		genBoolProg(file_name);

		exit(EXIT_SUCCESS);
	}

	assert(strcmp(argv[1], "ENCODE_BUFFERS") == 0);

	/* set weak memory model */
	if (strcmp(argv[2], "PSO") == 0) {
		wmm = PSO;
	} else {
		assert(strcmp(argv[2], "TSO") == 0);
		wmm = TSO;
	}

	/* parse code file */
	parseCodeFile(argv[3]);


	/* read rest of arguments */
	nr_weak_global_vars = atoi(argv[5]);
	k_buffer_size = atoi(argv[6]);
	add_random_flushes = (strcmp(argv[7], "YF") == 0);

	printf("encoding buffers for %s, %d vars relaxed, buffer size %d...\n",
			argv[1], nr_weak_global_vars, k_buffer_size);
	assert (add_random_flushes || (strcmp(argv[7], "NF") == 0));
	sprintf(file_name, "%s.%s.b%d.%dv.%s.prog", argv[3], argv[2], k_buffer_size, nr_weak_global_vars, argv[7]);

	/* generate code with encoded buffers */
	if (wmm == PSO) {
		printPsoProgram(file_name);
	} else {
		assert(wmm == TSO);
		printTsoProgram(file_name);
	}

	/* generate renamed predicates */
	sprintf(file_name, "%s.%s.b%d.%dv.%s.pred", argv[3], argv[2], k_buffer_size, nr_weak_global_vars, argv[7]);
	genRenamedPreds(wmm, argv[4], file_name);


	return EXIT_SUCCESS;
}
