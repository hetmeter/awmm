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

struct wordList
{
	vector<string> words;
	int wordCount = 0;

	void appendWord(string word)
	{
		//cout << "\t\t\t" << word << "\n";
		words.push_back(word);
		wordCount++;
	}

	void parseString(string input)
	{
		if (input != "")
		{
			string currentWord = "";
			int ctr = 0;
			char currentChar = input[ctr];
			//bool parsingWhitespace = isWhitespace(currentChar);
			//char currentWhitespace = parsingWhitespace ? currentChar : '\0';
			bool parsingWhitespace = false;
			bool currentCharIsWhitespace = isWhitespace(currentChar);
			char currentWhitespace = '\0';

			while (currentChar != '\0')
			{
				//cout << "Parsing " << currentChar << "\n";

				if (parsingWhitespace)
				{
					if (currentCharIsWhitespace && currentChar == currentWhitespace)
					{
						//cout << "\tAdding " << currentChar << " to current word\n";
						currentWord += currentChar;
						//cout << "\t\t" << currentWord << "\n";
					}
					else if (currentCharIsWhitespace && currentChar != currentWhitespace)
					{
						//cout << "\t\tAdding " << currentWord << " to list\n";
						appendWord(currentWord);
						//cout << "\tAdding " << currentChar << " to current word\n";
						currentWord = string(1, currentChar);
					}
					else
					{
						//cout << "\t\tAdding " << currentWord << " to list\n";
						appendWord(currentWord);
						//cout << "\tAdding " << currentChar << " to current word\n";
						currentWord = string(1, currentChar);
						parsingWhitespace = false;
					}
				}
				else
				{
					if (currentCharIsWhitespace)
					{
						//cout << "\t\tAdding " << currentWord << " to list\n";
						appendWord(currentWord);
						//cout << "\tAdding " << currentChar << " to current word\n";
						currentWord = string(1, currentChar);
						parsingWhitespace = true;
					}
					else
					{
						//cout << "\tAdding " << currentChar << " to current word\n";
						currentWord += currentChar;
						//cout << "\t\t" << currentWord << "\n";
					}
				}

				ctr++;
				currentChar = input[ctr];
				currentCharIsWhitespace = isWhitespace(currentChar);
			}
		}
	}

	string toString()
	{
		string result = "";

		for (int ctr = 0; ctr < wordCount; ctr++)
		{
			result += words[ctr] + "\n";
		}

		return result;
	}

	void applyRule(parserToken token)
	{
		// left off here
	}
};
typedef struct wordList wordList;

string parse(string lexerPath, string inputPath)
{
	ifstream lexerFile(lexerPath);
	stringstream lexerBuffer;
	lexerBuffer << lexerFile.rdbuf();
	string lexerString = lexerBuffer.str();
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
		//cout << "Parsing " << currentChar << "\n";

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
				currentToken->tag = currentDescription;
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

	ifstream inputFile(inputPath);
	stringstream inputBuffer;
	inputBuffer << inputFile.rdbuf();
	string result = inputBuffer.str();
	//string result = "";
	inputFile.close();

	//int numberOfTokens = tokens.size();

	//while (!tokens.empty())
	//{
	//	//cout << "tokens.size() = " << tokens.size() << "\n";

	//	result.append(tokens.front().ToString() + "\n");
	//	tokens.pop_front();
	//}

	wordList inputWordList;
	inputWordList.parseString(result);
	result = inputWordList.toString();

	return result;
}