#ifndef __STRING_INCLUDED__
#define __STRING_INCLUDED__
#include <string>
#endif

using namespace std;

const char DESCRIPTION_RULE_SEPARATOR = '\t';
const char OR_OPERATOR = '|';
const string WHITESPACES = " \t\r\n";
const string WHITESPACES_WITHOUT_SPACE = "\t\r\n";

bool isWhitespace(char c)
{
	int ctr = 0;
	char currentChar = WHITESPACES[ctr];

	while (currentChar != '\0')
	{
		if (currentChar == c)
		{
			return true;
		}

		ctr++;
		currentChar = WHITESPACES[ctr];
	}

	return false;
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