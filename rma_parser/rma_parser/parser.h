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

#ifndef __VECTOR_INCLUDED__
#define __VECTOR_INCLUDED__
#include <vector>
#endif

#ifndef __PARSERTOKEN_H_INCLUDED__
#define __PARSERTOKEN_H_INCLUDED__
#include "parserToken.h"
#endif

using namespace std;

const char DESCRIPTION_RULE_SEPARATOR = '\t';
const string WHITESPACES = " \t\r\n";

struct wordList
{
	vector<string> words;

	void parseString(string input)
	{
		if (input != "")
		{
			char currentWhitespace;
			bool parsingWhitespace;
			string currentWord = "";

			// left off here
		}
	}
};
typedef struct wordList wordList;

string parse(string lexerPath)
{
	//load the text file and put it into a single string:
	ifstream lexerFile(lexerPath);
	stringstream buffer;
	buffer << lexerFile.rdbuf();
	string lexerString = buffer.str();

	lexerFile.close();

	list<parserToken> tokens;
	int lexerStringCharCounter = 0;
	string currentDescription = "";
	string currentRule = "";
	bool parsingDescription = true;
	char currentChar = lexerString[lexerStringCharCounter];
	parserToken* currentToken;

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
				currentToken = new parserToken;
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
	}

	return false;
}