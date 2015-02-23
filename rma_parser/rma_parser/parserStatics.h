#ifndef __STRING_INCLUDED__
#define __STRING_INCLUDED__
#include <string>
#endif

using namespace std;

const char DESCRIPTION_RULE_SEPARATOR = '\t';
const char OR_OPERATOR = '`';
const string WHITESPACES = " \t\r\n";
const string WHITESPACES_WITHOUT_SPACE = "\t\r\n";
const string IDENTIFIER_RULE = "[identifier]";
const string NUMBER_RULE = "[number]";
const string WHITESPACE_RULE = "[whitespace]";
const string IGNORE_TAG = "{IGNORE}";
const string WILDCARD_TAG = "{WILDCARD}";
const string ACCEPTING_STATE_TAG = "{ACCEPTING_STATE}";

const enum CharacterType { WHITESPACE, ALPHANUMERIC_OR_BRACKET, OTHER };

bool isWhitespace(char c)
{
	int ctr = 0;
	char currentChar = WHITESPACES[ctr];

	while (currentChar != '\0')
	{
		if (currentChar == c)
		{
			//cout << "\t'" << c << "' is a whitespace character\n";

			return true;
		}

		ctr++;
		currentChar = WHITESPACES[ctr];
	}

	return false;
}

bool isWhitespace(string s)
{
	int sLength = s.length();

	for (int ctr = 0; ctr < sLength; ctr++)
	{
		if (!isWhitespace(s.at(ctr)))
		{
			return false;
		}
	}

	return sLength > 0;
}

bool isAlphanumericOrBracket(char c)
{
	int cValue = (int)c;
	return (cValue >= 48 && cValue <= 57) || (cValue >= 65 && cValue <= 91) || (cValue == 93) || (cValue == 95) || (cValue >= 97 && cValue <= 123 || (cValue == 125));
}

bool isNumber(char c)
{
	return (int)c >= 48 && (int)c <= 57;
}

bool isNumber(string s)
{
	int sLength = s.length();

	for (int ctr = 0; ctr < sLength; ctr++)
	{
		if (!isNumber(s.at(ctr)))
		{
			return false;
		}
	}

	return sLength > 0;
}

bool isStartOfIdentifier(char c)
{
	int cValue = (int)c;
	return (cValue == '_') || (cValue >= 65 && cValue <= 90) || (cValue >= 97 && cValue <= 122);
}

CharacterType getType(char c)
{
	if (isWhitespace(c))
	{
		return WHITESPACE;
	}
	else if (isAlphanumericOrBracket(c))
	{
		return ALPHANUMERIC_OR_BRACKET;
	}
	else
	{
		return OTHER;
	}
}

string normalize(string s)
{
	string result = string(s);

	//cout << "[" << result << "]\n";
	//cout << result << "\n";

	int ctr = 0;
	char currentWhitespace = WHITESPACES_WITHOUT_SPACE[ctr];

	while (currentWhitespace != '\0')
	{
		replace(result.begin(), result.end(), currentWhitespace, ' ');

		ctr++;
		currentWhitespace = WHITESPACES_WITHOUT_SPACE[ctr];
	}

	//cout << "\t[" << result << "]\n";

	int doubleSpaceIndex;

	while ((doubleSpaceIndex = result.find("  ")) != string::npos)
	{
		result.erase(doubleSpaceIndex, 1);
	}

	//cout << "\t\t[" << result << "]\n";

	while (result.length() > 0 && result[0] == ' ')
	{
		result.erase(0, 1);
	}

	//cout << "\t\t\t[" << result << "]\n";

	int resultLength = result.length();

	while (resultLength > 0 && result[resultLength - 1] == ' ')
	{
		result.erase(resultLength - 1, 1);
	}

	//cout << "\t\t\t\t[" << result << "]\n";

	return result;
}

bool isIdentifier(string s)
{
	int sLength = s.length();
	if (sLength > 0)
	{
		if (isStartOfIdentifier(s.at(0)))
		{
			for (int ctr = 0; ctr < sLength; ctr++)
			{
				if (!isAlphanumericOrBracket(s.at(ctr)) || s.at(ctr) == '[' || s.at(ctr) == ']' || s.at(ctr) == '{' || s.at(ctr) == '}')
				{
					return false;
				}
			}

			return true;
		}
	}

	return false;
}

bool isMatch(string s, string rule)
{
	return (s == rule) ||
		(rule == IDENTIFIER_RULE && isIdentifier(s)) ||
		(rule == NUMBER_RULE && isNumber(s)) ||
		(rule == WHITESPACE_RULE && isWhitespace(s) ||
		(rule == WILDCARD_TAG));
}