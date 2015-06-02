#include "literalCode.h"

namespace literalCode
{
/* Collections */
	const std::vector<std::string> UNARY_OPERATORS = { "!" };
	const std::vector<std::string> BINARY_OPERATORS = { "+", "-", "*", "/", "&", "|", "<", ">", "<=", ">=", "=", "==", "!=", "&&", "||" };

/* Symbols */
	const char LEFT_PARENTHESIS = '(';
	const char RIGHT_PARENTHESIS = ')';
	const char LEFT_CURLY_BRACKET = '{';
	const char RIGHT_CURLY_BRACKET = '}';
	const char COMMA = ',';
	const char SEMICOLON = ';';
	const char COLON = ':';
	const char SPACE = ' ';
	const char QUOTATION = '\"';
	const char ASSIGN_OPERATOR = '=';
	const char PLUS = '+';
	const char GREATER_THAN = '>';
	const char LABEL_SEPARATOR = '.';
	const char AUXILIARY_VARIABLE_SEPARATOR = '_';

	const std::string EQUALS = "==";
	const std::string NOT_EQUALS = "!=";
	const std::string LESS_THAN = "<";
	const std::string LESS_EQUALS = "<=";
	const std::string GREATER_EQUALS = ">=";
	const std::string DOUBLE_OR = "||";
	const std::string DOUBLE_AND = "&&";
	const std::string OR = "|";
	const std::string AND = "&";
	const std::string NOT = "!";

/* Token names */
	const std::string BL_PROGRAM_DECLARATION_TOKEN_NAME = "booleanProgramDeclaration";
	const std::string BL_INITIALIZATION_BLOCK_TOKEN_NAME = "booleanInitialization";
	const std::string BL_LOCAL_VARIABLES_BLOCK_TOKEN_NAME = "local";
	const std::string BL_SHARED_VARIABLES_BLOCK_TOKEN_NAME = "shared";
	const std::string BL_PROCESS_DECLARATION_TOKEN_NAME = "booleanProcess";
	const std::string BL_IF_TOKEN_NAME = "booleanIf";
	const std::string BL_ALWAYS_TOKEN_NAME = "always";
	
	const std::string PSO_TSO_STORE_TOKEN_NAME = "store";
	const std::string PSO_TSO_LOAD_TOKEN_NAME = "load";
	
	const std::string RMA_PROCESS_INITIALIZATION_TOKEN_NAME = "processInitialization";
	const std::string RMA_PUT_TOKEN_NAME = "put";
	const std::string RMA_GET_TOKEN_NAME = "get";
	
	const std::string ID_TOKEN_NAME = "ID";
	const std::string INT_TOKEN_NAME = "INT";
	const std::string BOOL_TOKEN_NAME = "BOOL";
	const std::string PROGRAM_DECLARATION_TOKEN_NAME = "programDeclaration";
	const std::string PROCESS_DECLARATION_TOKEN_NAME = "processDeclaration";
	const std::string PROCESS_HEADER_TOKEN_NAME = "processHeader";
	const std::string INITIALIZATION_BLOCK_TOKEN_NAME = "initializationBlock";
	const std::string STATEMENTS_TOKEN_NAME = "statements";
	const std::string LABEL_TOKEN_NAME = "label";
	const std::string IF_ELSE_TOKEN_NAME = "ifElse";
	const std::string LOCAL_ASSIGN_TOKEN_NAME = "localAssign";
	const std::string NONE_TOKEN_NAME = "none";
	const std::string ASTERISK_TOKEN_NAME = "*";
	const std::string ABORT_TOKEN_NAME = "abort";
	const std::string NOP_TOKEN_NAME = "nop";
	const std::string FLUSH_TOKEN_NAME = "flush";
	const std::string ASSUME_TOKEN_NAME = "assume";
	const std::string FENCE_TOKEN_NAME = "fence";
	const std::string GOTO_TOKEN_NAME = "goto";
	const std::string BEGIN_ATOMIC_TOKEN_NAME = "begin_atomic";
	const std::string END_ATOMIC_TOKEN_NAME = "end_atomic";
	const std::string CHOOSE_TOKEN_NAME = "choose";
	const std::string ASSERT_TOKEN_NAME = "assert";
	const std::string PC_TOKEN_NAME = "pc";

/* Tag names */
	const std::string BEGINIT_TAG_NAME = "beginit";
	const std::string ENDINIT_TAG_NAME = "endinit";
	const std::string INIT_TAG_NAME = "init";
	const std::string PROCESS_TAG_NAME = "process";
	const std::string IF_TAG_NAME = "if";
	const std::string ELSE_TAG_NAME = "else";
	const std::string ENDIF_TAG_NAME = "endif";
	const std::string TRUE_TAG_NAME = "true";
	const std::string FALSE_TAG_NAME = "false";
	
	const std::string AUXILIARY_COUNTER_TAG = "cnt";
	const std::string AUXILIARY_FIRST_POINTER_TAG = "fst";

/* Comments */
	const std::string MULTILINE_COMMENT_PREFIX = "/*";
	const std::string MULTILINE_COMMENT_SUFFIX = "*/";

	const std::string REPLACING_CAPTION = "Replacing";
	const std::string FINISHED_REPLACING_CAPTION = "Finished replacing";
	const std::string PREDICATE_ABSTRACTION_COMMENT_PREFIX = "Predicate abstraction: ";
}

//	const char TAB = '\t';
//
//	const std::string COMMENT_PREFIX = "//";
//	const std::string WHILE_TOKEN_NAME = "while";