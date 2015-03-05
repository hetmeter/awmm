#pragma once

#include <string>
#include <map>

class Ast;

namespace config
{
	const extern char LEFT_PARENTHESIS;
	const extern char RIGHT_PARENTHESIS;
	const extern char COMMA;
	const extern char LABEL_SEPARATOR;

	const extern std::string ID_TOKEN_NAME;
	const extern std::string PROGRAM_DECLARATION_TOKEN_NAME;
	const extern std::string PROCESS_DECLARATION_TOKEN_NAME;
	const extern std::string INITIALIZATION_BLOCK_TOKEN_NAME;
	const extern std::string STATEMENTS_TOKEN_NAME;
	const extern std::string STORE_TOKEN_NAME;
	const extern std::string LOAD_TOKEN_NAME;
	const extern std::string FENCE_TOKEN_NAME;
	const extern std::string LABEL_TOKEN_NAME;
	const extern std::string GOTO_TOKEN_NAME;
	const extern std::string IF_ELSE_TOKEN_NAME;
	const extern std::string WHILE_TOKEN_NAME;
	const extern std::string NONE_TAG_NAME;
	const extern std::string NOP_TAG_NAME;
	const extern std::string ACCEPTING_STATE_REGEX;
	
	const extern int TOP_VALUE;
	const extern int BOTTOM_VALUE;
	const extern std::string TOP_STRING;
	const extern std::string BOTTOM_STRING;

	extern std::map<std::string, Ast*> labelLookupMap;
}