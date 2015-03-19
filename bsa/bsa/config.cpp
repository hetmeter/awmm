/*
Global variables, constants, and methods
*/

#include "config.h"

namespace config
{
	const char LEFT_PARENTHESIS = '(';
	const char RIGHT_PARENTHESIS = ')';
	const char QUOTATION = '\"';
	const char COMMA = ',';
	const char SEMICOLON = ';';
	const char COLON = ':';
	const char EQUALS = '=';
	const char SPACE = ' ';
	const char LABEL_SEPARATOR = '.';
	const char AUXILIARY_VARIABLE_SEPARATOR = '_';

	const std::vector<std::string> UNARY_OPERATORS = { "!" };
	const std::vector<std::string> BINARY_OPERATORS = { "+", "-", "*", "/", "&", "|", "<", ">", "<=", ">=", "=",
		"==", "!=" };

	const std::string ID_TOKEN_NAME = "ID";
	const std::string INT_TOKEN_NAME = "INT";
	const std::string PROGRAM_DECLARATION_TOKEN_NAME = "programDeclaration";
	const std::string PROCESS_DECLARATION_TOKEN_NAME = "processDeclaration";
	const std::string STATEMENTS_TOKEN_NAME = "statements";
	const std::string FENCE_TOKEN_NAME = "fence";
	const std::string LABEL_TOKEN_NAME = "label";
	const std::string GOTO_TOKEN_NAME = "goto";
	const std::string IF_ELSE_TOKEN_NAME = "ifElse";
	const std::string WHILE_TOKEN_NAME = "while";
	const std::string NONE_TOKEN_NAME = "none";
	const std::string NOP_TOKEN_NAME = "nop";
	const std::string ABORT_TOKEN_NAME = "abort";
	const std::string FLUSH_TOKEN_NAME = "flush";
	const std::string PROCESS_HEADER_TOKEN_NAME = "processHeader";
	const std::string ASTERISK_TOKEN_NAME = "*";
	const std::string INITIALIZATION_BLOCK_TOKEN_NAME = "initializationBlock";

	const std::string BEGINIT_TAG_NAME = "beginit";
	const std::string ENDINIT_TAG_NAME = "endinit";
	const std::string PROCESS_TAG_NAME = "process";
	const std::string IF_TAG_NAME = "if";
	const std::string ELSE_TAG_NAME = "else";
	const std::string ENDIF_TAG_NAME = "endif";

	const std::string AUXILIARY_COUNTER_TAG = "cnt";
	const std::string AUXILIARY_FIRST_POINTER_TAG = "fst";
	
	const std::string PSO_TSO_STORE_TOKEN_NAME = "store";
	const std::string PSO_TSO_LOAD_TOKEN_NAME = "load";
	
	const std::string RMA_PROCESS_INITIALIZATION_TOKEN_NAME = "processInitialization";
	const std::string RMA_LOCAL_ASSIGN_TOKEN_NAME = "localAssign";
	const std::string RMA_PUT_TOKEN_NAME = "put";
	const std::string RMA_GET_TOKEN_NAME = "get";

	const std::string ACCEPTING_STATE_REGEX = "\\{ACCEPTING_STATE,(\\S+)\\}";
	const std::string EXTENSION_REGEX = "(.*)\\.(\\S\\S\\S)\\.out";

	const int TOP_VALUE = -1;
	const int BOTTOM_VALUE = -2;
	const int UNDEFINED_VALUE = -3;
	const std::string TOP_STRING = "T";
	const std::string BOTTOM_STRING = "_";

	const std::string PSO_EXTENSION = "pso";
	const std::string TSO_EXTENSION = "tso";
	const std::string RMA_EXTENSION = "rma";

	language currentLanguage;
	std::map<std::string, Ast*> labelLookupMap;
	std::vector<std::string> variableNames;
	int K;

	void throwError(std::string msg)
	{
		std::cout << "Error: " << msg << "\n";
	}

	void throwCriticalError(std::string msg)
	{
		std::cout << "Error: " << msg << "\n";
		std::exit(EXIT_FAILURE);
	}

	std::string replicateString(std::string s, int n)
	{
		std::string result = "";

		for (int ctr = 0; ctr < n; ctr++)
		{
			result += s;
		}

		return result;
	}

	std::string addTabs(std::string s, int numberOfTabs)
	{
		std::regex newLineRegex("\\n");
		std::smatch stringMatch;
		std::string tabs = replicateString("\t", numberOfTabs);
		std::string result = tabs + s;

		result = std::regex_replace(result, newLineRegex, "\n" + tabs);

		return result;
	}

	bool stringVectorContains(std::vector<std::string> container, std::string element)
	{
		return find(container.begin(), container.end(), element) != container.end();
	}
}