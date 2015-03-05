#include "config.h"

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
	const std::string LEXER_RULE_FILE_PROPERTY = "lexer rule file";
	const std::string PROGRAM_PARSER_RULE_FILE_PROPERTY = "program parser rule file";
	const std::string PREDICATE_PARSER_RULE_FILE_PROPERTY = "predicate parser rule file";
	const std::string CONFIG_REGEX = "^\\s*(.*\\S)\\s*=\\s*(.*\\S)\\s*$";
	const std::string IDENTIFIER_REGEX = "([a-zA-Z_][a-zA-Z0-9_]*)";

	std::string normalize(std::string s)
	{
		std::string result = std::string(s);

		int ctr = 0;
		char currentWhitespace = WHITESPACES_WITHOUT_SPACE[ctr];

		while (currentWhitespace != '\0')
		{
			replace(result.begin(), result.end(), currentWhitespace, ' ');

			ctr++;
			currentWhitespace = WHITESPACES_WITHOUT_SPACE[ctr];
		}

		int doubleSpaceIndex;

		while ((doubleSpaceIndex = result.find("  ")) != std::string::npos)
		{
			result.erase(doubleSpaceIndex, 1);
		}

		while (result.length() > 0 && result[0] == ' ')
		{
			result.erase(0, 1);
		}

		int resultLength = result.length();

		while (resultLength > 0 && result[resultLength - 1] == ' ')
		{
			result.erase(resultLength - 1, 1);
		}

		return result;
	}

	std::vector<std::string> find_all_matches(std::regex const& r, std::string input)
	{
		std::smatch match;
		std::vector<std::string> results;

		while (regex_search(input, match, r)) {
			results.push_back(match[1]);
			input = match.suffix().str();
		}

		return results;
	}

	std::string toIdentifier(std::string s)
	{
		if (s.empty())
		{
			return "";
		}
		else if (s.at(0) == LEFT_TOKEN_DELIMITER || s.at(s.length() - 1) == RIGHT_TOKEN_DELIMITER)
		{
			return "";
		}
		else
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

	void throwError(std::string message)
	{
		std::cout << "\n\tERROR: " << message << "\n\n";
	}
}