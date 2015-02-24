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

wordList programInputContent;
wordList predicateInputContent;

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

void initializeInputFromFile(string path, wordList* content)
{
	ifstream inputFile(path);
	stringstream buffer;
	buffer << inputFile.rdbuf();
	string inputString = buffer.str();
	inputFile.close();

	content->parseString(inputString);
}

void processContent(wordList* content, vector<token>* lexerTokens, vector<token>* parserTokens)
{
	int numberOfLexerTokens = lexerTokens->size();
	int numberOfParserTokens = parserTokens->size();

	for (int ctr = 0; ctr < numberOfLexerTokens; ctr++)
	{
		lexerTokens->at(ctr).applyToWordList(content);
	}

	content->collapse();

	string oldInputContent = "";
	string secondaryOldInputContent = "";
	while (content->toString().compare(oldInputContent) != 0)
	{
		oldInputContent = content->toString();

		for (int ctr = 0; ctr < numberOfParserTokens; ctr++)
		{
			if (parserTokens->at(ctr).tag != ACCEPTING_STATE_TAG)
			{
				parserTokens->at(ctr).applyToWordList(content);
			}
		}
	}

	for (int ctr = 0; ctr < numberOfParserTokens; ctr++)
	{
		if (parserTokens->at(ctr).tag == ACCEPTING_STATE_TAG)
		{
			parserTokens->at(ctr).applyToWordList(content);
		}
	}
}

void parse(string lexerPath, string programParserPath, string predicateParserPath, string programInputPath, string predicateInputPath)
{
	vector<token> lexerTokens = initializeTokensFromFile(lexerPath);
	vector<token> programParserTokens = initializeTokensFromFile(programParserPath);
	vector<token> predicateParserTokens = initializeTokensFromFile(predicateParserPath);

	initializeInputFromFile(programInputPath, &programInputContent);
	initializeInputFromFile(predicateInputPath, &predicateInputContent);

	/*int numberOfLexerTokens = lexerTokens.size();
	cout << "Number of lexer tokens: " << numberOfLexerTokens << "\n\n";
	cout << "Lexer tokens:\n";
	for (int ctr = 0; ctr < numberOfLexerTokens; ctr++)
	{
		cout << "Lexer token " << ctr << ": " << lexerTokens.at(ctr).toString() << "\n";
	}
	cout << "\n";

	int numberOfParserTokens = programParserTokens.size();
	cout << "Number of parser tokens: " << numberOfParserTokens << "\n\n";
	cout << "Parser tokens:\n";
	for (int ctr = 0; ctr < numberOfParserTokens; ctr++)
	{
		cout << "Parser token " << ctr << ": " << programParserTokens.at(ctr).toString() << "\n";
	}
	cout << "\n";

	cout << "Input content:\n" << programInputContent.toString() << "\n\n";*/

	processContent(&programInputContent, &lexerTokens, &programParserTokens);
	processContent(&predicateInputContent, &lexerTokens, &predicateParserTokens);
}