#include <string>
#include <iostream>
#include <fstream>
#include <regex>
#include <map>

#include "config.h"
#include "Ast.h"

using namespace std;

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

	regex programInputRegex(config::ACCEPTING_STATE_REGEX);
	smatch stringMatch;
	string parsedProgramString;
	Ast rootAst;

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

	Ast* currentAst = &rootAst;
	char currentChar;
	string currentName = "";
	int parsedProgramStringLength = parsedProgramString.length();

	for (int ctr = 0; ctr < parsedProgramStringLength; ctr++)
	{
		currentChar = parsedProgramString.at(ctr);

		if (currentChar == config::LEFT_PARENTHESIS)
		{
			currentAst->name = currentName;
			currentName = "";
			currentAst->addChild(new Ast);
			currentAst = currentAst->children.at(0);
		}
		else if (currentChar == config::COMMA)
		{
			if (!currentName.empty())
			{
				currentAst->name = currentName;
				currentName = "";
			}

			currentAst = currentAst->parent;
			currentAst->addChild(new Ast);
			currentAst = currentAst->children.at(currentAst->children.size() - 1);
		}
		else if (currentChar == config::RIGHT_PARENTHESIS)
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