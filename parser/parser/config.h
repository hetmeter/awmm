#pragma once

#include <string>
#include <vector>
#include <regex>
#include <iostream>

struct token;

namespace config
{
	const extern char DESCRIPTION_RULE_SEPARATOR;
	const extern std::string WHITESPACES_WITHOUT_SPACE;
	const extern std::string IDENTIFIER_RULE;
	const extern std::string EOF_TAG;
	const extern std::string ACCEPTING_STATE_TAG;

	const extern char LEFT_TOKEN_DELIMITER;
	const extern char RIGHT_TOKEN_DELIMITER;
	const extern char TAG_PARAMETER_SEPARATOR;
	const extern char CONFIG_COMMENT;
	const extern std::string CONFIG_FILE_PATH;

	const extern std::string RMA_EXTENSION;
	const extern std::string TSO_EXTENSION;
	const extern std::string PSO_EXTENSION;

	const extern std::string LEXER_RULE_FILE_PROPERTY;
	const extern std::string PROGRAM_PARSER_RULE_FILE_PROPERTY;
	const extern std::string PREDICATE_PARSER_RULE_FILE_PROPERTY;

	const extern std::string CONFIG_REGEX;
	const extern std::string IDENTIFIER_REGEX;
	const extern std::string TAG_REGEX;

	extern std::string normalize(std::string s);
	extern std::vector<std::string> find_all_matches(std::regex const& r, std::string input);
	extern std::string toIdentifier(std::string s);
	extern std::vector<std::string> separate(std::string input, char separator);
	extern void throwError(std::string message);
	extern int tagCount(std::string s);
	extern int errorLine(std::string parsedProgram, std::string originalProgram, std::vector<token>* lexerTokens, std::vector<token>* parserTokens);
}