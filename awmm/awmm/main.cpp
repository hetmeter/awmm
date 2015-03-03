#ifndef __STRING_INCLUDED__
#define __STRING_INCLUDED__
#include <string>
#endif

#ifndef __IOSTREAM_INCLUDED__
#define __IOSTREAM_INCLUDED__
#include <iostream>
#endif

#ifndef __REGEX_INCLUDED__
#define __REGEX_INCLUDED__
#include <regex>
#endif

#ifndef __FSTREAM_INCLUDED__
#define __FSTREAM_INCLUDED__
#include <fstream>
#endif

#ifndef __AST_H_INCLUDED__
#define __AST_H_INCLUDED__
#include "ast.h"
#endif

#ifndef __CONTROLFLOWVISITOR_H_INCLUDED__
#define __CONTROLFLOWVISITOR_H_INCLUDED__
#include "controlFlowVisitor.h"
#endif

using namespace std;

const char LEFT_PARENTHESIS = '(';
const char RIGHT_PARENTHESIS = ')';
const char COMMA = ',';

const string ACCEPTING_STATE_REGEX = "\\{ACCEPTING_STATE,(\\S+)\\}";

int main(int argc, char** argv)
{
	string parsedProgramPath;

	if (argc > 1)
	{
		parsedProgramPath = argv[1];
	}
	else
	{
		cout << "No program input path specified\n";
		return 1;
	}

	ifstream parsedProgramFile(parsedProgramPath);
	string parsedProgramLine;

	if (!getline(parsedProgramFile, parsedProgramLine))
	{
		cout << "Error: Empty parsed program file.\n";
		return 1;
	}

	regex programInputRegex(ACCEPTING_STATE_REGEX);
	smatch stringMatch;
	string parsedProgramString;
	ast rootAst;

	regex_search(parsedProgramLine, stringMatch, programInputRegex);

	if (stringMatch.size() >= 2)
	{
		parsedProgramString = stringMatch[1].str();
	}
	else
	{
		cout << "Error: Invalid parsed program format or program in a non-accepting state.\n";
		return 1;
	}

	ast* currentAst = &rootAst;
	char currentChar;
	string currentName = "";
	int parsedProgramStringLength = parsedProgramString.length();

	for (int ctr = 0; ctr < parsedProgramStringLength; ctr++)
	{
		currentChar = parsedProgramString.at(ctr);

		if (currentChar == LEFT_PARENTHESIS)
		{
			currentAst->name = currentName;
			currentName = "";
			currentAst->addChild(new ast);
			currentAst = currentAst->children.at(0);
		}
		else if (currentChar == COMMA)
		{
			if (!currentName.empty())
			{
				currentAst->name = currentName;
				currentName = "";
			}

			currentAst = currentAst->parent;
			currentAst->addChild(new ast);
			currentAst = currentAst->children.at(currentAst->children.size() - 1);
		}
		else if (currentChar == RIGHT_PARENTHESIS)
		{
			if (!currentName.empty())
			{
				currentAst->name = currentName;
				currentName = "";
			}

			currentAst = currentAst->parent;
		}
		else
		{
			currentName += currentChar;
		}
	}

	rootAst.initializeGlobalVariables();
	rootAst.inceptControlFlowGraph();

	int processCount = rootAst.children.size();
	controlFlowVisitor* crawler;

	for (int ctr = 1; ctr < processCount; ctr++)
	{
		crawler = new controlFlowVisitor;
		crawler->traverseControlFlowGraph(rootAst.children.at(ctr)->children.at(1)->children.at(0));
	}

	cout << rootAst.toString() << "\n\n";

	return 0;
}