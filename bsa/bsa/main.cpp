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
		config::K = stoi(argv[2]);
	}
	else
	{
		config::throwCriticalError("No K specified");
	}

	if (argc > 3)
	{
		config::globalCubeSizeLimit = stoi(argv[3]);
	}
	else
	{
		config::throwCriticalError("No cube size limit specified");
	}

	// Parse input file (no line breaks are expected)
	ifstream parsedProgramFile(parsedProgramPath);
	string parsedProgramLine;

	if (!getline(parsedProgramFile, parsedProgramLine))
	{
		config::throwCriticalError("Empty parsed program file");
	}

	parsedProgramFile.close();

	// Extract AST representation from the input file
	regex programInputRegex(config::ACCEPTING_STATE_REGEX);
	smatch stringMatch;
	string parsedProgramString;

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
	string extension, fileNameStub;
	programInputRegex = regex(config::EXTENSION_REGEX);
	regex_search(parsedProgramPath, stringMatch, programInputRegex);

	if (stringMatch.size() == 3)
	{
		fileNameStub = stringMatch[1].str();
		extension = stringMatch[2].str();

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
	Ast* rootAstRef = config::stringToAst(parsedProgramString);

	cout << "\n---\nParsed program:\n\n";
	cout << rootAstRef->emitCode();

	rootAstRef->cascadingUnifyVariableNames();		// Send a cascading command to the root node that results in all variable identifiers registering their variable names in a global vector
	rootAstRef->getCostsFromChildren();				// Send a cascading command to the root node that results in all program points storing the buffer size increases they cause
	rootAstRef->initializePersistentCosts();		// Prompt the AST to gather all globally initialized variables and have all program point nodes keep track of their buffer sizes
	rootAstRef->topDownCascadingRegisterLabels();	// Send a cascading command to the root node that results in all label AST nodes registering themselves in a global map
	rootAstRef->cascadingGenerateOutgoingEdges();	// Send a cascading command to the root node that results in all program points estabilishing outgoing program flow edges to their possible successor nodes in the control flow graph
	rootAstRef->visitAllProgramPoints();			// Generate one control flow visitor in the first program point nodes of each process declaration and prompt them to start traversing the AST
	rootAstRef->cascadingUnifyVariableNames();
	rootAstRef->cascadingInitializeAuxiliaryVariables();
	rootAstRef->carryOutReplacements();

	cout << "\n---\nCarried out buffer size analysis:\n\n";
	cout << rootAstRef->emitCode();

	// Generate the output
	outputPath = fileNameStub + "." + config::BSA_EXTENSION + "." + extension;
	ofstream programOut(outputPath);
	programOut << rootAstRef->emitCode();
	programOut.close();

	config::lazyReplacements.clear();

	ifstream parsedPredicateFile(fileNameStub + "." + config::PREDICATE_EXTENSION + "." + extension + "." + config::OUT_EXTENSION);
	string parsedPredicateLine, parsedPredicateString;
	Ast* predicateAstRef;

	if (!getline(parsedPredicateFile, parsedPredicateLine))
	{
		config::throwError("Empty or invaldid parsed predicate file");
	}
	else
	{
		// Check if the predicates are in an accepting state, prompt an error otherwise
		programInputRegex = regex(config::ACCEPTING_STATE_REGEX);
		regex_search(parsedPredicateLine, stringMatch, programInputRegex);

		if (stringMatch.size() == 2)
		{
			predicateAstRef = config::stringToAst(stringMatch[1].str());

			for (Ast* child : predicateAstRef->children)
			{
				config::globalPredicates.push_back(child);
			}

			config::initializeAuxiliaryVariables();

			cout << "Performing predicate abstraction...\n";

			rootAstRef->cascadingPerformPredicateAbstraction();
			rootAstRef->setVariableInitializations();

			cout << "\n---\nCarried out predicate abstraction:\n\n";
			cout << rootAstRef->emitCode();
		}
		else
		{
			config::throwError("Invalid parsed predicate format or predicates in a non-accepting state");
		}

		config::carryOutLazyReplacements();
	}

	parsedPredicateFile.close();

	/*cout << "\n---\n";
	cout << rootAstRef->astToString();*/
	cout << "\n---\nFinal state:\n\n";
	cout << rootAstRef->emitCode();

	// Generate the output
	string booleanOutputPath = fileNameStub + "." + config::BOOLEAN_EXTENSION;
	ofstream booleanProgramOut(booleanOutputPath);
	booleanProgramOut << rootAstRef->emitCode();
	booleanProgramOut.close();

	return 0;
}