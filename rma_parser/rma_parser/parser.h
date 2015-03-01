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

using namespace std;

string programInputContent;
string predicateInputContent;

vector<token> initializeTokensFromFile(string path)
{
	vector<token> result;
	vector<string> separatedLine;

	ifstream ruleFile(path);
	string line;
	string description;
	string rule;
	token* currentToken;

	while (getline(ruleFile, line))
	{
		separatedLine = separate(line, DESCRIPTION_RULE_SEPARATOR);

		if (separatedLine.size() == 2)
		{
			description = separatedLine.at(0);
			rule = separatedLine.at(1);

			if (result.size() > 0 && result.at(result.size() - 1).tag == description)
			{
				result.at(result.size() - 1).initialize(description, rule);
			}
			else
			{
				currentToken = new token;
				currentToken->initialize(description, rule);

				if (currentToken->isInitialized())
				{
					result.push_back(*currentToken);
				}
			}
		}
		else
		{
			throwError("Invalid token initialization input \"" + line + "\"");
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

	return inputString + " " + EOF_TAG + " ";
}

string processContent(string content, vector<token>* lexerTokens, vector<token>* parserTokens)
{
	int numberOfLexerTokens = lexerTokens->size();
	int numberOfParserTokens = parserTokens->size();
	string result = content;
	string temp;

	cout << "\tApplying lexer tokens...\n";

	for (int ctr = 0; ctr < numberOfLexerTokens; ctr++)
	{
		result = lexerTokens->at(ctr).applyToString(result);
		/*cout << "Lexer tokens applied:\n" << result << "\n\n";
		cin >> temp;
		if (temp == "C")
		{
			break;
		}*/
	}

	//cout << "Lexer tokens applied:\n" << result << "\n\n";

	result = normalize(result);

	cout << "Lexer tokens applied and normalized:\n" << result << "\n\n";

	string oldInputContent = "";
	string secondaryOldInputContent = "";

	cout << "\tApplying parser tokens...\n";

	while (result.compare(oldInputContent) != 0)
	{
		oldInputContent = result;

		for (int ctr = 0; ctr < numberOfParserTokens; ctr++)
		{
			if (parserTokens->at(ctr).tag != ACCEPTING_STATE_TAG)
			{
				result = parserTokens->at(ctr).applyToString(result);
			}

			/*cout << "Parser token \"" + parserTokens->at(ctr).tag + "\" applied:\n" << result << "\n\n";
			cin >> temp;
			if (temp == "C")
			{
				break;
			}*/
		}
	}

	result = normalize(result);

	//cout << "Non-accepting parser tokens applied and normalized:\n" << result << "\n\n";

	for (int ctr = 0; ctr < numberOfParserTokens; ctr++)
	{
		if (parserTokens->at(ctr).tag == ACCEPTING_STATE_TAG)
		{
			result = parserTokens->at(ctr).applyToString(result);
		}
	}

	//cout << "Parser tokens applied:\n" << result << "\n\n";

	return result;
}

void parse(string lexerPath, string programParserPath, string predicateParserPath, string programInputPath, string predicateInputPath)
{
	vector<token> lexerTokens = initializeTokensFromFile(lexerPath);
	vector<token> programParserTokens = initializeTokensFromFile(programParserPath);
	vector<token> predicateParserTokens = initializeTokensFromFile(predicateParserPath);

	programInputContent = initializeInputFromFile(programInputPath);
	predicateInputContent = initializeInputFromFile(predicateInputPath);

	cout << "Processing program...\n";
	programInputContent = processContent(programInputContent, &lexerTokens, &programParserTokens);

	cout << "Processing predicates...\n";
	predicateInputContent = processContent(predicateInputContent, &lexerTokens, &predicateParserTokens);
}