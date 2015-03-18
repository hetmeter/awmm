/*
Global variables, constants, and methods
*/

#include "config.h"

namespace config
{
	const char LEFT_PARENTHESIS = '(';
	const char RIGHT_PARENTHESIS = ')';
	const char COMMA = ',';
	const char SEMICOLON = ';';
	const char EQUALS = '=';
	const char SPACE = ' ';
	const char LABEL_SEPARATOR = '.';

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
	const std::string FLUSH_TOKEN_NAME = "flush";

	const std::string BEGINIT_TAG_NAME = "beginit";
	const std::string ENDINIT_TAG_NAME = "endinit";
	
	const std::string PSO_TSO_INITIALIZATION_BLOCK_TOKEN_NAME = "initializationBlock";
	const std::string PSO_TSO_STORE_TOKEN_NAME = "store";
	const std::string PSO_TSO_LOAD_TOKEN_NAME = "load";
	
	const std::string RMA_PROCESS_INITIALIZATION_TOKEN_NAME = "processInitialization";
	const std::string RMA_LOCAL_ASSIGN_TOKEN_NAME = "localAssign";
	const std::string RMA_PUT_TOKEN_NAME = "put";
	const std::string RMA_GET_TOKEN_NAME = "get";

	const std::string ACCEPTING_STATE_REGEX = "\\{ACCEPTING_STATE,(\\S+)\\}";
	const std::string EXTENSION_REGEX = ".*\\.(\\S\\S\\S)\\.out";

	const int TOP_VALUE = -1;
	const int BOTTOM_VALUE = -2;
	const std::string TOP_STRING = "T";
	const std::string BOTTOM_STRING = "_";

	const std::string PSO_EXTENSION = "pso";
	const std::string TSO_EXTENSION = "tso";
	const std::string RMA_EXTENSION = "rma";

	language currentLanguage;
	std::map<std::string, Ast*> labelLookupMap;

	void throwError(std::string msg)
	{
		std::cout << "Error: " << msg << "\n";
	}

	void throwCriticalError(std::string msg)
	{
		std::cout << "Error: " << msg << "\n";
		std::exit(EXIT_FAILURE);
	}
}