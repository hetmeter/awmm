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

wordList _inputContent;

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

void initializeInputFromFile(string path)
{
	ifstream inputFile(path);
	stringstream buffer;
	buffer << inputFile.rdbuf();
	string inputString = buffer.str();
	inputFile.close();

	_inputContent.parseString(inputString);
}

string parse(string lexerPath, string parserPath, string inputPath)
{
	vector<token> lexerTokens = initializeTokensFromFile(lexerPath);
	vector<token> parserTokens = initializeTokensFromFile(parserPath);

	initializeInputFromFile(inputPath);

	string result = "";

	int numberOfLexerTokens = lexerTokens.size();
	cout << "Number of lexer tokens: " << numberOfLexerTokens << "\n\n";
	cout << "Lexer tokens:\n";
	for (int ctr = 0; ctr < numberOfLexerTokens; ctr++)
	{
		cout << "Lexer token " << ctr << ": " << lexerTokens[ctr].toString() << "\n";
	}
	cout << "\n";

	int numberOfParserTokens = parserTokens.size();
	cout << "Number of parser tokens: " << numberOfParserTokens << "\n\n";
	cout << "Parser tokens:\n";
	for (int ctr = 0; ctr < numberOfParserTokens; ctr++)
	{
		cout << "Parser token " << ctr << ": " << parserTokens[ctr].toString() << "\n";
	}
	cout << "\n";

	cout << "Input content:\n" << _inputContent.toString() << "\n\n";

	for (int ctr = 0; ctr < numberOfLexerTokens; ctr++)
	{
		lexerTokens[ctr].applyToWordList(&_inputContent);
	}

	result = _inputContent.toString();

	return result;
}