#ifndef STRING_INCLUDED
#define STRING_INCLUDED
#include <string>
#endif

#ifndef IOSTREAM_INCLUDED
#define IOSTREAM_INCLUDED
#include <iostream>
#endif

#ifndef REGEX_INCLUDED
#define REGEX_INCLUDED
#include <regex>
#endif

#ifndef FSTREAM_INCLUDED
#define FSTREAM_INCLUDED
#include <fstream>
#endif

#ifndef AST_H_INCLUDED
#define AST_H_INCLUDED
#include "ast.h"
#endif

#ifndef CONTROLFLOWVISITOR_H_INCLUDED
#define CONTROLFLOWVISITOR_H_INCLUDED
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

	rootAst.getCostsFromChildren();
	rootAst.initializePersistentCosts();
	rootAst.topDownCascadingRegisterLabels();
	rootAst.cascadingGenerateOutgoingEdges();
	rootAst.visitAllProgramPoints();

	cout << rootAst.toString() << "\n\n";

	ofstream programOut("output.txt");
	programOut << rootAst.toString();
	programOut.close();

	return 0;
}