#pragma once

#include <string>
#include <vector>

namespace literalCode
{
/* Collections */

	const extern std::vector<std::string> UNARY_OPERATORS;
	const extern std::vector<std::string> BINARY_OPERATORS;
	const extern std::vector<std::string> RMA_PROGRAM_POINT_TOKENS;
	const extern std::vector<std::string> PSO_TSO_PROGRAM_POINT_TOKENS;

/* Symbols */

	/* Parentheses and brackets */

		const extern char LEFT_PARENTHESIS;
		const extern char RIGHT_PARENTHESIS;

		const extern char LEFT_CURLY_BRACKET;
		const extern char RIGHT_CURLY_BRACKET;

	/* Punctuation */

		const extern char COLON;
		const extern char COMMA;
		const extern char QUOTATION;
		const extern char SEMICOLON;
		const extern char SPACE;

	/* One-character operators */

		const extern char ASSIGN_OPERATOR;
		const extern char GREATER_THAN;
		const extern char PLUS;

	/* Operator strings */

		const extern std::string AND;
		const extern std::string DOUBLE_AND;

		const extern std::string OR;
		const extern std::string DOUBLE_OR;

		const extern std::string EQUALS;
		const extern std::string NOT_EQUALS;

		const extern std::string GREATER_EQUALS;
		const extern std::string LESS_EQUALS;
		const extern std::string LESS_THAN;

		const extern std::string NOT;

	/* Variable name components */

		const extern char AUXILIARY_VARIABLE_SEPARATOR;
		const extern char BOOLEAN_VARIABLE_PREFIX;
		const extern char TEMPORARY_VARIABLE_PREFIX;

/* Token names */

	/* Boolean program language */

		const extern std::string BL_ALWAYS_TOKEN_NAME;
		const extern std::string BL_IF_TOKEN_NAME;
		const extern std::string BL_INITIALIZATION_BLOCK_TOKEN_NAME;
		const extern std::string BL_LOCAL_VARIABLES_BLOCK_TOKEN_NAME;
		const extern std::string BL_PROCESS_DECLARATION_TOKEN_NAME;
		const extern std::string BL_PROGRAM_DECLARATION_TOKEN_NAME;
		const extern std::string BL_SHARED_VARIABLES_BLOCK_TOKEN_NAME;

	/* PSO and TSO */

		const extern std::string PSO_TSO_LOAD_TOKEN_NAME;
		const extern std::string PSO_TSO_STORE_TOKEN_NAME;

	/* RMA */

		const extern std::string RMA_GET_TOKEN_NAME;
		const extern std::string RMA_PUT_TOKEN_NAME;
		const extern std::string RMA_PROCESS_INITIALIZATION_TOKEN_NAME;

	/* Shared */

		const extern std::string BEGIN_ATOMIC_TOKEN_NAME;
		const extern std::string END_ATOMIC_TOKEN_NAME;

		const extern std::string ABORT_TOKEN_NAME;
		const extern std::string ASSERT_TOKEN_NAME;
		const extern std::string ASSUME_TOKEN_NAME;
		const extern std::string ASTERISK_TOKEN_NAME;
		const extern std::string BOOL_TOKEN_NAME;
		const extern std::string CHOOSE_TOKEN_NAME;
		const extern std::string FENCE_TOKEN_NAME;
		const extern std::string FLUSH_TOKEN_NAME;
		const extern std::string GOTO_TOKEN_NAME;
		const extern std::string ID_TOKEN_NAME;
		const extern std::string IF_ELSE_TOKEN_NAME;
		const extern std::string INITIALIZATION_BLOCK_TOKEN_NAME;
		const extern std::string INT_TOKEN_NAME;
		const extern std::string LABEL_TOKEN_NAME;
		const extern std::string LOCAL_ASSIGN_TOKEN_NAME;
		const extern std::string NONE_TOKEN_NAME;
		const extern std::string NOP_TOKEN_NAME;
		const extern std::string PC_TOKEN_NAME;
		const extern std::string PROCESS_DECLARATION_TOKEN_NAME;
		const extern std::string PROCESS_HEADER_TOKEN_NAME;
		const extern std::string PROGRAM_DECLARATION_TOKEN_NAME;
		const extern std::string STATEMENTS_TOKEN_NAME;

/* Tag names*/

	/* Language tags */
		
		const extern std::string BEGINIT_TAG_NAME;
		const extern std::string ENDINIT_TAG_NAME;
		const extern std::string INIT_TAG_NAME;

		const extern std::string PROCESS_TAG_NAME;

		const extern std::string IF_TAG_NAME;
		const extern std::string ELSE_TAG_NAME;
		const extern std::string ENDIF_TAG_NAME;

		const extern std::string TRUE_TAG_NAME;
		const extern std::string FALSE_TAG_NAME;

	/* Auxiliary variable tags */

		const extern std::string AUXILIARY_COUNTER_TAG;
		const extern std::string AUXILIARY_FIRST_POINTER_TAG;

/* Comment delimiters */

	const extern std::string MULTILINE_COMMENT_PREFIX;
	const extern std::string MULTILINE_COMMENT_SUFFIX;

/* Comments */

	const extern std::string REPLACING_CAPTION;
	const extern std::string FINISHED_REPLACING_CAPTION;
	const extern std::string ASSUMPTION_MOOT_CAPTION;
	const extern std::string ASSUMPTION_EXCESSIVE_CAPTION;
	const extern std::string PREDICATE_ABSTRACTION_COMMENT_PREFIX;

/* Parameters */

	const extern std::string EVALUATION_MODE_PARAMETER;
	const extern std::string VERBOSE_MODE_PARAMETER;
}