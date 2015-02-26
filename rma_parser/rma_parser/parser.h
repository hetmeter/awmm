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

#ifndef __TOKEN_H_INCLUDED__
#define __TOKEN_H_INCLUDED__
#include "token.h"
#endif

#ifndef __WORDLIST_H_INCLUDED__
#define __WORDLIST_H_INCLUDED__
#include "wordList.h"
#endif

using namespace std;

//wordList programInputContent;
//wordList predicateInputContent;

string programInputContent;
string predicateInputContent;

vector<token> initializeTokensFromFile(string path)
{
	vector<token> result;

	ifstream ruleFile(path);
	string line;
	token* currentToken;

	while (getline(ruleFile, line))
	{
		currentToken = new token;
		currentToken->initialize(line);

		if (currentToken->isInitialized())
		{
			result.push_back(*currentToken);
		}
	}

	return result;
}

string initializeInputFromFile(string path)
{
	ifstream inputFile(path);
	stringstream buffer;
	buffer << inputFile.rdbuf();
	string inputString = buffer.str();
	inputFile.close();

	return inputString;
}

//void initializeInputFromFile(string path, wordList* content)
//{
//	ifstream inputFile(path);
//	stringstream buffer;
//	buffer << inputFile.rdbuf();
//	string inputString = buffer.str();
//	inputFile.close();
//
//	content->parseString(inputString);
//}

void processContent(string content, vector<token>* lexerTokens, vector<token>* parserTokens)
{
	int numberOfLexerTokens = lexerTokens->size();
	int numberOfParserTokens = parserTokens->size();

	for (int ctr = 0; ctr < numberOfLexerTokens; ctr++)
	{
		lexerTokens->at(ctr).applyToString(content);
	}

	content = normalize(content);

	string oldInputContent = "";
	string secondaryOldInputContent = "";
	while (content.compare(oldInputContent) != 0)
	{
		oldInputContent = content;

		for (int ctr = 0; ctr < numberOfParserTokens; ctr++)
		{
			if (parserTokens->at(ctr).tag != ACCEPTING_STATE_TAG)
			{
				parserTokens->at(ctr).applyToString(content);
			}
		}
	}

	for (int ctr = 0; ctr < numberOfParserTokens; ctr++)
	{
		if (parserTokens->at(ctr).tag == ACCEPTING_STATE_TAG)
		{
			parserTokens->at(ctr).applyToString(content);
		}
	}
}

//void processContent(wordList* content, vector<token>* lexerTokens, vector<token>* parserTokens)
//{
//	int numberOfLexerTokens = lexerTokens->size();
//	int numberOfParserTokens = parserTokens->size();
//
//	for (int ctr = 0; ctr < numberOfLexerTokens; ctr++)
//	{
//		lexerTokens->at(ctr).applyToWordList(content);
//	}
//
//	content->collapse();
//
//	string oldInputContent = "";
//	string secondaryOldInputContent = "";
//	while (content->toString().compare(oldInputContent) != 0)
//	{
//		oldInputContent = content->toString();
//
//		for (int ctr = 0; ctr < numberOfParserTokens; ctr++)
//		{
//			if (parserTokens->at(ctr).tag != ACCEPTING_STATE_TAG)
//			{
//				parserTokens->at(ctr).applyToWordList(content);
//			}
//		}
//	}
//
//	for (int ctr = 0; ctr < numberOfParserTokens; ctr++)
//	{
//		if (parserTokens->at(ctr).tag == ACCEPTING_STATE_TAG)
//		{
//			parserTokens->at(ctr).applyToWordList(content);
//		}
//	}
//}

void parse(string lexerPath, string programParserPath, string predicateParserPath, string programInputPath, string predicateInputPath)
{
	vector<token> lexerTokens = initializeTokensFromFile(lexerPath);
	vector<token> programParserTokens = initializeTokensFromFile(programParserPath);
	vector<token> predicateParserTokens = initializeTokensFromFile(predicateParserPath);

	programInputContent = initializeInputFromFile(programInputPath);
	predicateInputContent = initializeInputFromFile(predicateInputPath);

	processContent(programInputContent, &lexerTokens, &programParserTokens);
	processContent(predicateInputContent, &lexerTokens, &predicateParserTokens);

	/*initializeInputFromFile(programInputPath, &programInputContent);
	initializeInputFromFile(predicateInputPath, &predicateInputContent);

	processContent(&programInputContent, &lexerTokens, &programParserTokens);
	processContent(&predicateInputContent, &lexerTokens, &predicateParserTokens);*/
}