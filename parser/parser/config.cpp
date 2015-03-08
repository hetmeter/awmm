/*
Global variables, constants, and methods
*/

#include "config.h"

#include "parser.h"

namespace config
{
	const char DESCRIPTION_RULE_SEPARATOR = '\t';
	const std::string WHITESPACES_WITHOUT_SPACE = "\t\r\n";
	const std::string IDENTIFIER_RULE = "[identifier]";
	const std::string EOF_TAG = "{EOF}";
	const std::string ACCEPTING_STATE_TAG = "{ACCEPTING_STATE}";

	const char LEFT_TOKEN_DELIMITER = '{';
	const char RIGHT_TOKEN_DELIMITER = '}';
	const char TAG_PARAMETER_SEPARATOR = ',';
	const char CONFIG_COMMENT = '#';
	const std::string CONFIG_FILE_PATH = "config.txt";

	const std::string RMA_EXTENSION = "RMA";
	const std::string TSO_EXTENSION = "TSO";
	const std::string PSO_EXTENSION = "PSO";

	const std::string LEXER_RULE_FILE_PROPERTY = "lexer rule file";
	const std::string PROGRAM_PARSER_RULE_FILE_PROPERTY = "program parser rule file";
	const std::string PREDICATE_PARSER_RULE_FILE_PROPERTY = "predicate parser rule file";

	const std::string CONFIG_REGEX = "^\\s*(.*\\S)\\s*=\\s*(.*\\S)\\s*$";
	const std::string IDENTIFIER_REGEX = "([a-zA-Z_][a-zA-Z0-9_]*)";
	const std::string TAG_REGEX = "\\{.*?\\}";
	const std::string ACCEPTING_STATE_REGEX = "\\{ACCEPTING_STATE,(\\S+)\\}";

	// Returns a copy of the parameter string without whitespace characters
	std::string removeWhitespace(std::string s)
	{
		std::string result = s;
		std::string emptyString = "";
		std::regex ruleRegex("\\s");
		result = std::regex_replace(result, ruleRegex, emptyString);
		return result;
	}

	// Returns a vector of all matches to the regex in the input string
	std::vector<std::string> find_all_matches(std::regex const& r, std::string input)
	{
		std::smatch match;
		std::vector<std::string> results;

		while (regex_search(input, match, r))
		{
			results.push_back(match[1]);
			input = match.suffix().str();
		}

		return results;
	}

	// Returns the pure identifier from the string, providing it's not a token. Otherwise, returns an empty string.
	std::string toIdentifier(std::string s)
	{
		if (s.empty())	// Check if string is empty
		{
			return "";
		}
		else if (s.at(0) == LEFT_TOKEN_DELIMITER || s.at(s.length() - 1) == RIGHT_TOKEN_DELIMITER)	// Check if string is a token (judging by its first and last character)
		{
			return "";
		}
		else // Extracts the identifier, which might be cluttered by symbols not belonging to the name
		{
			std::regex identifierRegex(IDENTIFIER_REGEX);
			std::smatch stringMatch;

			if (regex_match(s, identifierRegex))
			{
				regex_search(s, stringMatch, identifierRegex);

				if (stringMatch.size() >= 2)
				{
					return stringMatch[1].str();
				}
			}
		}

		return "";
	}

	// Returns a vector of strings that are separated by the separator character in the input string
	std::vector<std::string> separate(std::string input, char separator)
	{
		std::vector<std::string> result;
		std::string source = input;
		int separatorIndex;

		while ((separatorIndex = source.find(separator)) != std::string::npos)
		{
			result.push_back(source.substr(0, separatorIndex));

			if (((int)source.length()) > separatorIndex + 1)
			{
				source = source.substr(separatorIndex + 1, std::string::npos);
			}
			else
			{
				source = "";
			}
		}

		result.push_back(source);

		return result;
	}

	// Handles an error with an attached message
	void throwError(std::string message)
	{
		std::cout << "\n\tERROR: " << message << "\n\n";
	}

	// Counts the number of tags in the parameter string
	int tagCount(std::string s)
	{
		std::string sCopy = s;
		std::smatch match;;
		std::regex tagRegex(TAG_REGEX);
		int matchCtr = 0;

		while (regex_search(sCopy, match, tagRegex))
		{
			matchCtr++;
			sCopy = match.suffix().str();
		}

		return matchCtr;
	}

	// Checks if the parsed program is in a non-accepting state and if so, returns the line number of the first parsing error in the original program. Otherwise, returns -1
	int errorLine(std::string parsedProgram, std::string originalProgram, std::vector<token>* lexerTokens, std::vector<token>* parserTokens)
	{
		int programTagCount = tagCount(parsedProgram);

		if (programTagCount > 0)	// Check if there's at least one tag in the parsed program
		{
			std::regex programInputRegex(config::ACCEPTING_STATE_REGEX);
			std::smatch match;
			std::regex_search(parsedProgram, match, programInputRegex);

			if (match.size() != 2)	// Check if there's anything else but exactly one match for an accepting state tag
			{
				std::string originalProgramCopy = originalProgram;
				std::string currentParsedLine;
				std::regex lineRegex("\\n");
				int lineCtr = 1;

				while (regex_search(originalProgramCopy, match, lineRegex))	// Iterate through every line in the unprocessed program
				{
					currentParsedLine = processContent(match.prefix().str(), lexerTokens, parserTokens);

					// Check if the current line is non-empty and parses to other than exactly one tag
					if (currentParsedLine != "" || tagCount(currentParsedLine) != 1)
					{
						return lineCtr;
					}

					originalProgramCopy = match.suffix().str();
					lineCtr++;
				}

				// If no error line has been returned so far, return 0 as a representation of an error line that's unfindable with
				// the current search method
				return 0;
			}
		}
		else if (programTagCount == 0)	// If no tags can be parsed in the entire program, return the first line as the first error line
		{
			return 1;
		}

		return -1;	// Return that no parsing error has been found
	}
}