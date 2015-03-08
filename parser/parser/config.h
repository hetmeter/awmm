#pragma once

#include <string>
#include <vector>
#include <regex>
#include <iostream>
#include <map>

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
	const extern char EXTENSION_SEPARATOR;
	const extern char CONFIG_COMMENT;
	const extern std::string CONFIG_FILE_PATH;

	const extern std::string PREDICATE_EXTENSION;
	const extern std::string OUT_EXTENSION;
	const extern std::string RMA_EXTENSION;
	const extern std::string TSO_EXTENSION;
	const extern std::string PSO_EXTENSION;

	const extern std::string RMA_LEXER_RULE_FILE_PROPERTY;
	const extern std::string RMA_PROGRAM_PARSER_RULE_FILE_PROPERTY;
	const extern std::string RMA_PREDICATE_PARSER_RULE_FILE_PROPERTY;
	const extern std::string TSO_LEXER_RULE_FILE_PROPERTY;
	const extern std::string TSO_PROGRAM_PARSER_RULE_FILE_PROPERTY;
	const extern std::string TSO_PREDICATE_PARSER_RULE_FILE_PROPERTY;
	const extern std::string PSO_LEXER_RULE_FILE_PROPERTY;
	const extern std::string PSO_PROGRAM_PARSER_RULE_FILE_PROPERTY;
	const extern std::string PSO_PREDICATE_PARSER_RULE_FILE_PROPERTY;

	const extern std::string CONFIG_REGEX;
	const extern std::string EXTENSION_REGEX;
	const extern std::string IDENTIFIER_REGEX;
	const extern std::string TAG_REGEX;
	const extern std::string ACCEPTING_STATE_REGEX;

	extern std::string removeWhitespace(std::string s);
	extern std::vector<std::string> findAllMatches(std::regex const& r, std::string input);
	extern std::string toIdentifier(std::string s);
	extern std::vector<std::string> separate(std::string input, char separator);
	extern std::map<std::string, std::string> parseConfiguration(std::string path);
	extern void throwError(std::string message);
	extern int tagCount(std::string s);
	extern int errorLine(std::string parsedProgram, std::string originalProgram, std::vector<token>* lexerTokens, std::vector<token>* parserTokens);
}