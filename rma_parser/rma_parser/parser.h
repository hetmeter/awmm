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

#ifndef __ALGORITHM_INCLUDED__
#define __ALGORITHM_INCLUDED__
#include <algorithm>
#endif

#ifndef __PARSERTOKEN_H_INCLUDED__
#define __PARSERTOKEN_H_INCLUDED__
#include "parserToken.h"
#endif

#ifndef __WORDLIST_H_INCLUDED__
#define __WORDLIST_H_INCLUDED__
#include "wordList.h"
#endif

#ifndef __DISJUNCTIVEPREDICATES_H_INCLUDED__
#define __DISJUNCTIVEPREDICATES_H_INCLUDED__
#include "disjunctivePredicates.h"
#endif

using namespace std;

string parse(string lexerPath, string inputPath)
{
	ifstream lexerFile(lexerPath);
	stringstream lexerBuffer;
	lexerBuffer << lexerFile.rdbuf();
	string lexerString = lexerBuffer.str();
	lexerFile.close();

	vector<parserToken> tokens;
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

	//ifstream inputFile(inputPath);
	//stringstream inputBuffer;
	//inputBuffer << inputFile.rdbuf();
	//string result = inputBuffer.str();
	////string result = "";
	//inputFile.close();

	//int numberOfTokens = tokens.size();

	//while (!tokens.empty())
	//{
	//	//cout << "tokens.size() = " << tokens.size() << "\n";

	//	result.append(tokens.front().ToString() + "\n");
	//	tokens.pop_front();
	//}

	//wordList inputWordList;
	//inputWordList.parseString(result);
	//result = inputWordList.toString();

	disjunctivePredicates rules;
	int numberOfTokens = tokens.size();
	for (int ctr = 0; ctr < numberOfTokens; ctr++)
	{
		rules.parsePredicates(tokens[ctr].rule);
	}
	string result = rules.toString();

	return result;
}