#pragma once

#include <string>
#include <iostream>
#include <map>

class Ast;

namespace config
{
	const extern char LEFT_PARENTHESIS;
	const extern char RIGHT_PARENTHESIS;
	const extern char COMMA;
	const extern char SEMICOLON;
	const extern char EQUALS;
	const extern char SPACE;
	const extern char LABEL_SEPARATOR;

	const extern std::string ID_TOKEN_NAME;
	const extern std::string INT_TOKEN_NAME;
	const extern std::string PROGRAM_DECLARATION_TOKEN_NAME;
	const extern std::string PROCESS_DECLARATION_TOKEN_NAME;
	const extern std::string STATEMENTS_TOKEN_NAME;
	const extern std::string FENCE_TOKEN_NAME;
	const extern std::string LABEL_TOKEN_NAME;
	const extern std::string GOTO_TOKEN_NAME;
	const extern std::string IF_ELSE_TOKEN_NAME;
	const extern std::string WHILE_TOKEN_NAME;
	const extern std::string NONE_TOKEN_NAME;
	const extern std::string NOP_TOKEN_NAME;
	const extern std::string FLUSH_TOKEN_NAME;

	const extern std::string BEGINIT_TAG_NAME;
	const extern std::string ENDINIT_TAG_NAME;

	const extern std::string PSO_TSO_INITIALIZATION_BLOCK_TOKEN_NAME;
	const extern std::string PSO_TSO_STORE_TOKEN_NAME;
	const extern std::string PSO_TSO_LOAD_TOKEN_NAME;

	const extern std::string RMA_PROCESS_INITIALIZATION_TOKEN_NAME;
	const extern std::string RMA_LOCAL_ASSIGN_TOKEN_NAME;
	const extern std::string RMA_PUT_TOKEN_NAME;
	const extern std::string RMA_GET_TOKEN_NAME;

	const extern std::string ACCEPTING_STATE_REGEX;
	const extern std::string EXTENSION_REGEX;
	
	const extern int TOP_VALUE;
	const extern int BOTTOM_VALUE;
	const extern std::string TOP_STRING;
	const extern std::string BOTTOM_STRING;

	const extern std::string PSO_EXTENSION;
	const extern std::string TSO_EXTENSION;
	const extern std::string RMA_EXTENSION;
	enum language { PSO, TSO, RMA };

	extern language currentLanguage;
	extern std::map<std::string, Ast*> labelLookupMap;

	extern void throwError(std::string msg);
	extern void throwCriticalError(std::string msg);
}