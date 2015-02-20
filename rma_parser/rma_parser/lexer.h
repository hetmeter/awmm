#ifndef __IOSTREAM_INCLUDED__
#define __IOSTREAM_INCLUDED__
#include <iostream>
#endif

#ifndef __STRING_INCLUDED__
#define __STRING_INCLUDED__
#include <string>
#endif

#ifndef __FSTREAM_INCLUDED__
#define __FSTREAM_INCLUDED__
#include <fstream>
#endif

#ifndef __SSTREAM_INCLUDED__
#define __SSTREAM_INCLUDED__
#include <sstream>
#endif

#ifndef __LIST_INCLUDED__
#define __LIST_INCLUDED__
#include <list>
#endif

using namespace std;

struct lexerToken
{
	string description;
	string rule;

	string ToString()
	{
		return "token " + description + " implements " + rule;
	}
};
typedef struct lexerToken lexerToken;

const char DESCRIPTION_RULE_SEPARATOR = '\t';

string parseLexer(string lexerPath)
{
	//load the text file and put it into a single string:
	ifstream lexerFile(lexerPath);
	stringstream buffer;
	buffer << lexerFile.rdbuf();
	string lexerString = buffer.str();

	lexerFile.close();

	list<lexerToken> tokens;
	int lexerStringCharCounter = 0;
	string currentDescription = "";
	string currentRule = "";
	bool parsingDescription = true;
	char currentChar = lexerString[lexerStringCharCounter];
	lexerToken* currentToken;

	while (true)
	{
		cout << "Parsing " << currentChar << "\n";

		if (parsingDescription)
		{
			if (currentChar != DESCRIPTION_RULE_SEPARATOR)
			{
				currentDescription += currentChar;
			}
			else
			{
				parsingDescription = false;
			}
		}
		else
		{
			if (currentChar == '\r')
			{
				// ignore \r
			}
			else if (currentChar == '\n' || currentChar == '\0')
			{
				currentToken = new lexerToken;
				currentToken->description = currentDescription;
				currentToken->rule = currentRule;

				currentDescription = "";
				currentRule = "";

				tokens.push_back(*currentToken);

				if (currentChar == '\0')
				{
					break;
				}

				parsingDescription = true;
			}
			else
			{
				currentRule += currentChar;
			}
		}

		lexerStringCharCounter++;
		currentChar = lexerString[lexerStringCharCounter];
	}

	string result = "";
	int numberOfTokens = tokens.size();

	while (!tokens.empty())
	{
		cout << "tokens.size() = " << tokens.size() << "\n";

		result.append(tokens.front().ToString() + "\n");
		tokens.pop_front();
	}

	return result;
}