/*The MIT License(MIT)

Copyright(c) 2015 Attila Zoltán Printz

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "literalCode.h"

namespace literalCode
{
/* Collections */

	const std::vector<std::string> UNARY_OPERATORS = { "!" };
	const std::vector<std::string> BINARY_OPERATORS = { "+", "-", "*", "/", "&", "|", "<", ">", "<=", ">=", "=", "==", "!=", "&&", "||" };
	const std::vector<std::string> RMA_PROGRAM_POINT_TOKENS = {
		LABEL_TOKEN_NAME, GOTO_TOKEN_NAME, RMA_GET_TOKEN_NAME, PSO_TSO_LOAD_TOKEN_NAME, IF_ELSE_TOKEN_NAME, FENCE_TOKEN_NAME,
		RMA_PUT_TOKEN_NAME, LOCAL_ASSIGN_TOKEN_NAME, NOP_TOKEN_NAME, FLUSH_TOKEN_NAME
	};
	const std::vector<std::string> PSO_TSO_PROGRAM_POINT_TOKENS = {
		LABEL_TOKEN_NAME, GOTO_TOKEN_NAME, PSO_TSO_STORE_TOKEN_NAME, PSO_TSO_LOAD_TOKEN_NAME, IF_ELSE_TOKEN_NAME, FENCE_TOKEN_NAME,
		NOP_TOKEN_NAME, FLUSH_TOKEN_NAME, LOCAL_ASSIGN_TOKEN_NAME
	};

/* Symbols */

	/* Parentheses and brackets */

		const char LEFT_PARENTHESIS = '(';
		const char RIGHT_PARENTHESIS = ')';

		const char LEFT_CURLY_BRACKET = '{';
		const char RIGHT_CURLY_BRACKET = '}';

	/* Punctuation */

		const char COLON = ':';
		const char COMMA = ',';
		const char QUOTATION = '\"';
		const char SEMICOLON = ';';
		const char SPACE = ' ';

	/* One-character operators */

		const char ASSIGN_OPERATOR = '=';
		const char GREATER_THAN = '>';
		const char PLUS = '+';

	/* Operator strings */

		const std::string AND = "&";
		const std::string DOUBLE_AND = "&&";

		const std::string OR = "|";
		const std::string DOUBLE_OR = "||";

		const std::string EQUALS = "==";
		const std::string NOT_EQUALS = "!=";

		const std::string GREATER_EQUALS = ">=";
		const std::string LESS_EQUALS = "<=";
		const std::string LESS_THAN = "<";

		const std::string NOT = "!";

	/* Variable name components */

		const char AUXILIARY_VARIABLE_SEPARATOR = '_';
		const char BOOLEAN_VARIABLE_PREFIX = 'B';
		const char TEMPORARY_VARIABLE_PREFIX = 'T';

		const std::string ABORT_VARIABLE_NAME = "B_ABORT";

/* Token names */

	/* Boolean program language */

		const std::string BL_ALWAYS_TOKEN_NAME = "always";
		const std::string BL_IF_TOKEN_NAME = "booleanIf";
		const std::string BL_INITIALIZATION_BLOCK_TOKEN_NAME = "booleanInitialization";
		const std::string BL_LOCAL_VARIABLES_BLOCK_TOKEN_NAME = "local";
		const std::string BL_PROCESS_DECLARATION_TOKEN_NAME = "booleanProcess";
		const std::string BL_PROGRAM_DECLARATION_TOKEN_NAME = "booleanProgramDeclaration";
		const std::string BL_SHARED_VARIABLES_BLOCK_TOKEN_NAME = "shared";

	/* PSO and TSO */

		const std::string PSO_TSO_LOAD_TOKEN_NAME = "load";
		const std::string PSO_TSO_STORE_TOKEN_NAME = "store";

	/* RMA */

		const std::string RMA_GET_TOKEN_NAME = "get";
		const std::string RMA_PUT_TOKEN_NAME = "put";
		const std::string RMA_PROCESS_INITIALIZATION_TOKEN_NAME = "processInitialization";

	/* Shared */

		const std::string BEGIN_ATOMIC_TOKEN_NAME = "begin_atomic";
		const std::string END_ATOMIC_TOKEN_NAME = "end_atomic";

		const std::string ABORT_TOKEN_NAME = "abort";
		const std::string ASSERT_TOKEN_NAME = "assert";
		const std::string ASSUME_TOKEN_NAME = "assume";
		const std::string ASTERISK_TOKEN_NAME = "*";
		const std::string BOOL_TOKEN_NAME = "BOOL";
		const std::string CHOOSE_TOKEN_NAME = "choose";
		const std::string FENCE_TOKEN_NAME = "fence";
		const std::string FLUSH_TOKEN_NAME = "flush";
		const std::string GOTO_TOKEN_NAME = "goto";
		const std::string ID_TOKEN_NAME = "ID";
		const std::string IF_ELSE_TOKEN_NAME = "ifElse";
		const std::string INITIALIZATION_BLOCK_TOKEN_NAME = "initializationBlock";
		const std::string INT_TOKEN_NAME = "INT";
		const std::string LABEL_TOKEN_NAME = "label";
		const std::string LOCAL_ASSIGN_TOKEN_NAME = "localAssign";
		const std::string NONE_TOKEN_NAME = "none";
		const std::string NOP_TOKEN_NAME = "nop";
		const std::string PC_TOKEN_NAME = "pc";
		const std::string PROCESS_DECLARATION_TOKEN_NAME = "processDeclaration";
		const std::string PROCESS_HEADER_TOKEN_NAME = "processHeader";
		const std::string PROGRAM_DECLARATION_TOKEN_NAME = "programDeclaration";
		const std::string STATEMENTS_TOKEN_NAME = "statements";

/* Tag names*/

	/* Language tags */

		const std::string BEGINIT_TAG_NAME = "beginit";
		const std::string ENDINIT_TAG_NAME = "endinit";
		const std::string INIT_TAG_NAME = "init";

		const std::string PROCESS_TAG_NAME = "process";

		const std::string IF_TAG_NAME = "if";
		const std::string ELSE_TAG_NAME = "else";
		const std::string ENDIF_TAG_NAME = "endif";

		const std::string TRUE_TAG_NAME = "true";
		const std::string FALSE_TAG_NAME = "false";

	/* Auxiliary variable tags */

		const std::string AUXILIARY_COUNTER_TAG = "cnt";
		const std::string AUXILIARY_FIRST_POINTER_TAG = "fst";

/* Comment delimiters */

		const std::string MULTILINE_COMMENT_PREFIX = "/*";
		const std::string MULTILINE_COMMENT_SUFFIX = "*/";

/* Comments */

		const std::string REPLACING_CAPTION = "Replacing";
		const std::string FINISHED_REPLACING_CAPTION = "Finished replacing";
		const std::string ASSUMPTION_MOOT_CAPTION = "No cubes found that imply false";
		const std::string ASSUMPTION_EXCESSIVE_CAPTION = "Number of cubes that imply false exceeds the set limit";
		const std::string PREDICATE_ABSTRACTION_COMMENT_PREFIX = "Predicate abstraction: ";

/* Parameters */

		const std::string MEMORY_ORDER_PSO = "-pso";
		const std::string MEMORY_ORDER_TSO = "-tso";
		const std::string EVALUATION_MODE_PARAMETER = "-e";
		const std::string VERBOSE_MODE_PARAMETER = "-v";
}