/*
Buffer Size Analysis:
	Takes the path of a parsed program and the path of the would-be output file as input. The parsed program must be of the format:
		{ACCEPTING_STATE, [content] }
	, where [content] stands for the description of the program's abstract semantic tree in the following format:
		rootNode(childNode1,childNode2(...))

	Outputs the program's AST, with the corresponding read and write buffer sizes at every control flow point.
*/

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
	string outputPath;

	// Read arguments
	if (argc > 1)
	{
		parsedProgramPath = argv[1];
	}
	else
	{
		config::throwCriticalError("No program input path specified");
	}

	if (argc > 2)
	{
		outputPath = argv[2];
	}
	else
	{
		config::throwCriticalError("No output path specified");
	}

	// Parse input file (no line breaks are expected)
	ifstream parsedProgramFile(parsedProgramPath);
	string parsedProgramLine;

	if (!getline(parsedProgramFile, parsedProgramLine))
	{
		config::throwCriticalError("Empty parsed program file");
	}

	// Extract AST representation from the input file
	regex programInputRegex(config::ACCEPTING_STATE_REGEX);
	smatch stringMatch;
	string parsedProgramString;
	Ast rootAst;

	// Check if the program is in an accepting state, prompt an error otherwise
	regex_search(parsedProgramLine, stringMatch, programInputRegex);

	if (stringMatch.size() == 2)
	{
		parsedProgramString = stringMatch[1].str();
	}
	else
	{
		config::throwCriticalError("Invalid parsed program format or program in a non-accepting state");
	}

	// Get the input extension and set the global language variable accordingly. Prompt an error otherwise.
	string extension;
	programInputRegex = regex(config::EXTENSION_REGEX);
	regex_search(parsedProgramPath, stringMatch, programInputRegex);

	if (stringMatch.size() == 2)
	{
		extension = stringMatch[1].str();

		if (extension == config::PSO_EXTENSION)
		{
			config::currentLanguage = config::language::PSO;
		}
		else if (extension == config::TSO_EXTENSION)
		{
			config::currentLanguage = config::language::TSO;
		}
		else if (extension == config::RMA_EXTENSION)
		{
			config::currentLanguage = config::language::RMA;
		}
	}
	else
	{
		config::throwCriticalError("Invalid parsed program extension");
	}

	// Parse through the AST representation string character by character
	Ast* currentAst = &rootAst;
	char currentChar;
	string currentName = "";
	int parsedProgramStringLength = parsedProgramString.length();

	for (int ctr = 0; ctr < parsedProgramStringLength; ctr++)
	{
		currentChar = parsedProgramString.at(ctr);

		if (currentChar == config::LEFT_PARENTHESIS)	// Create a new node with the word parsed so far as its name.
		{												// Subsequently found nodes are to be added as this one's children
			currentAst->name = currentName;				// until the corresponding ')' is found.
			currentName = "";
			currentAst->addChild(new Ast);
			currentAst = currentAst->children.at(0);
		}
		else if (currentChar == config::COMMA)			// Add the currently parsed node as the previous one's sibling
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
		else if (currentChar == config::RIGHT_PARENTHESIS)	// Move up one level
		{
			if (!currentName.empty())
			{
				currentAst->name = currentName;
				currentName = "";
			}

			currentAst = currentAst->parent;
		}
		else // Continue parsing the name of the next node
		{
			currentName += currentChar;
		}
	}

	Ast* rootAstRef = &rootAst;

	rootAstRef->getCostsFromChildren();				// Send a cascading command to the root node that results in all program points storing the buffer size increases they cause
	rootAstRef->initializePersistentCosts();		// Prompt the AST to gather all globally initialized variables and have all program point nodes keep track of their buffer sizes
	rootAstRef->topDownCascadingRegisterLabels();	// Send a cascading command to the root node that results in all label AST nodes registering themselves in a global map
	rootAstRef->cascadingGenerateOutgoingEdges();	// Send a cascading command to the root node that results in all program points estabilishing outgoing program flow edges to their possible successor nodes in the control flow graph
	rootAstRef->visitAllProgramPoints();			// Generate one control flow visitor in the first program point nodes of each process declaration and prompt them to start traversing the AST

	cout << rootAst.astToString();

	// Generate the output
	ofstream programOut(outputPath);
	programOut << rootAst.astToString();
	programOut.close();

	return 0;
}