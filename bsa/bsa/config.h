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
	const extern std::string ACCEPTING_STATE_REGEX;
	
	const extern int TOP_VALUE;
	const extern int BOTTOM_VALUE;
	const extern std::string TOP_STRING;
	const extern std::string BOTTOM_STRING;

	static std::map<int, Ast*> labelLookupMap;

	/*const char LEFT_PARENTHESIS = '(';
	const char RIGHT_PARENTHESIS = ')';
	const char COMMA = ',';
	const char LABEL_SEPARATOR = '.';

	const std::string ID_TOKEN_NAME = "ID";
	const std::string PROGRAM_DECLARATION_TOKEN_NAME = "programDeclaration";
	const std::string PROCESS_DECLARATION_TOKEN_NAME = "processDeclaration";
	const std::string INITIALIZATION_BLOCK_TOKEN_NAME = "initializationBlock";
	const std::string STATEMENTS_TOKEN_NAME = "statements";
	const std::string STORE_TOKEN_NAME = "store";
	const std::string LOAD_TOKEN_NAME = "load";
	const std::string FENCE_TOKEN_NAME = "fence";
	const std::string LABEL_TOKEN_NAME = "label";
	const std::string GOTO_TOKEN_NAME = "goto";
	const std::string IF_ELSE_TOKEN_NAME = "ifElse";
	const std::string WHILE_TOKEN_NAME = "while";
	const std::string NONE_TAG_NAME = "none";
	const std::string ACCEPTING_STATE_REGEX = "\\{ACCEPTING_STATE,(\\S+)\\}";

	const int TOP_VALUE = -1;
	const int BOTTOM_VALUE = -2;
	const std::string TOP_STRING = "T";
	const std::string BOTTOM_STRING = "_";*/
}