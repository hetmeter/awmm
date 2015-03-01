//#ifndef __STRING_INCLUDED__
//#define __STRING_INCLUDED__
//#include <string>
//#endif
//
//#ifndef __REGEX_INCLUDED__
//#define __REGEX_INCLUDED__
//#include <regex>
//#endif
//
//#ifndef __IOSTREAM_INCLUDED__
//#define __IOSTREAM_INCLUDED__
//#include <iostream>
//#endif
//
//#ifndef __VECTOR_INCLUDED__
//#define __VECTOR_INCLUDED__
//#include <vector>
//#endif

using namespace std;

const char DESCRIPTION_RULE_SEPARATOR = '\t';
const string WHITESPACES_WITHOUT_SPACE = "\t\r\n";
const string IDENTIFIER_RULE = "[identifier]";
const string EOF_TAG = "{EOF}";
const string ACCEPTING_STATE_TAG = "{ACCEPTING_STATE}";

const char LEFT_TOKEN_DELIMITER = '{';
const char RIGHT_TOKEN_DELIMITER = '}';
const char TAG_PARAMETER_SEPARATOR = ',';
const char CONFIG_COMMENT = '#';
const string CONFIG_FILE_PATH = "config.txt";
const string LEXER_RULE_FILE_PROPERTY = "lexer rule file";
const string PROGRAM_PARSER_RULE_FILE_PROPERTY = "program parser rule file";
const string PREDICATE_PARSER_RULE_FILE_PROPERTY = "predicate parser rule file";
const string CONFIG_REGEX = "^\\s*(.*\\S)\\s*=\\s*(.*\\S)\\s*$";
const string IDENTIFIER_REGEX = "([a-zA-Z_][a-zA-Z0-9_]*)";

string normalize(string s)
{
	string result = string(s);

	int ctr = 0;
	char currentWhitespace = WHITESPACES_WITHOUT_SPACE[ctr];

	while (currentWhitespace != '\0')
	{
		replace(result.begin(), result.end(), currentWhitespace, ' ');

		ctr++;
		currentWhitespace = WHITESPACES_WITHOUT_SPACE[ctr];
	}

	int doubleSpaceIndex;

	while ((doubleSpaceIndex = result.find("  ")) != string::npos)
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

vector<string> find_all_matches(regex const& r, string input) {
	smatch match;
	vector<string> results;

	while (regex_search(input, match, r)) {
		results.push_back(match[1]);
		input = match.suffix().str();
	}

	return results;
}

string toIdentifier(string s)
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
		regex identifierRegex(IDENTIFIER_REGEX);
		smatch stringMatch;

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

vector<string> separate(string input, char separator)
{
	vector<string> result;
	string source = input;
	int separatorIndex;

	while ((separatorIndex = source.find(separator)) != string::npos)
	{
		result.push_back(source.substr(0, separatorIndex));

		if (((int)source.length()) > separatorIndex + 1)
		{
			source = source.substr(separatorIndex + 1, string::npos);
		}
		else
		{
			source = "";
		}
	}
	
	result.push_back(source);

	return result;
}

void throwError(string message)
{
	cout << "\n\tERROR: " << message << "\n\n";
}