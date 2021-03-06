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
#include <ctime>
#include <fstream>
#include <regex>
#include <map>

#include "config.h"
#include "literalCode.h"
#include "Ast.h"

using namespace std;

/* Local constants */
const string ACCEPTING_STATE_REGEX = "\\{ACCEPTING_STATE,(\\S+)\\}";
const string EXTENSION_REGEX = "(.*)\\.(\\S\\S\\S)\\.out";
const string PSO_EXTENSION = "pso";
const string TSO_EXTENSION = "tso";
const string RMA_EXTENSION = "rma";
const string BSA_EXTENSION = "bsa";
const string OUT_EXTENSION = "out";
const string PREDICATE_EXTENSION = "predicate";
const string BOOLEAN_EXTENSION = "bl";

int main(int argc, char** argv)
{
	time_t startedAt = time(0);

	string parsedProgramPath;
	string outputPath;
	//string endAssertion;

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

	config::generateAuxiliaryPredicates = !(argc > 4 && literalCode::OMIT_AUXILIARY_PREDICATES_PARAMETER.compare(argv[4]) == 0);

	// Parse input file (no line breaks are expected)
	ifstream parsedProgramFile(parsedProgramPath);
	string parsedProgramLine;

	if (!getline(parsedProgramFile, parsedProgramLine))
	{
		config::throwCriticalError("Empty parsed program file");
	}

	parsedProgramFile.close();

	// Extract AST representation from the input file
	regex programInputRegex(ACCEPTING_STATE_REGEX);
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
	programInputRegex = regex(EXTENSION_REGEX);
	regex_search(parsedProgramPath, stringMatch, programInputRegex);

	if (stringMatch.size() == 3)
	{
		fileNameStub = stringMatch[1].str();
		extension = stringMatch[2].str();

		if (extension == PSO_EXTENSION)
		{
			config::currentLanguage = config::language::PSO;
		}
		else if (extension == TSO_EXTENSION)
		{
			config::currentLanguage = config::language::TSO;
		}
		else if (extension == RMA_EXTENSION)
		{
			config::currentLanguage = config::language::RMA;
		}
	}
	else
	{
		config::throwCriticalError("Invalid parsed program extension");
	}

	cout << "Parsing program...\n";
	Ast* rootAstRef = Ast::newAstFromParsedProgram(parsedProgramString);

	cout << "Performing buffer size analysis...\n";
	/*cout << "\n---\nParsed program:\n\n";
	cout << rootAstRef->emitCode();*/

	rootAstRef->cascadingUnifyVariableNames();		// Send a cascading command to the root node that results in all variable identifiers registering their variable names in a global vector
	rootAstRef->getCostsFromChildren();				// Send a cascading command to the root node that results in all program points storing the buffer size increases they cause
	rootAstRef->initializePersistentCosts();		// Prompt the AST to gather all globally initialized variables and have all program point nodes keep track of their buffer sizes
	rootAstRef->topDownCascadingRegisterLabels();	// Send a cascading command to the root node that results in all label AST nodes registering themselves in a global map
	rootAstRef->cascadingGenerateOutgoingEdges();	// Send a cascading command to the root node that results in all program points estabilishing outgoing program flow edges to their possible successor nodes in the control flow graph
	rootAstRef->visitAllProgramPoints();			// Generate one control flow visitor in the first program point nodes of each process declaration and prompt them to start traversing the AST
	rootAstRef->cascadingUnifyVariableNames();
	rootAstRef->cascadingInitializeAuxiliaryVariables();
	rootAstRef->carryOutReplacements();

	/*cout << "\n---\nCarried out buffer size analysis:\n\n";
	cout << rootAstRef->emitCode();*/

	// Generate the output
	outputPath = fileNameStub + "." + BSA_EXTENSION + "." + extension;
	ofstream programOut(outputPath);
	programOut << rootAstRef->emitCode();
	programOut.close();

	config::lazyReplacements.clear();

	ifstream parsedPredicateFile(fileNameStub + "." + PREDICATE_EXTENSION + "." + extension + "." + OUT_EXTENSION);
	string parsedPredicateLine, parsedPredicateString;
	Ast* predicateAstRef;

	if (!getline(parsedPredicateFile, parsedPredicateLine))
	{
		config::throwError("Empty or invaldid parsed predicate file");
	}
	else
	{
		// Check if the predicates are in an accepting state, prompt an error otherwise
		programInputRegex = regex(ACCEPTING_STATE_REGEX);
		regex_search(parsedPredicateLine, stringMatch, programInputRegex);

		if (stringMatch.size() == 2)
		{
			predicateAstRef = Ast::newAstFromParsedProgram(stringMatch[1].str());

			for (Ast* child : predicateAstRef->children)
			{
				config::globalPredicates.push_back(child);
				config::globalPredicatesCount++;
			}

			if (config::generateAuxiliaryPredicates)
			{
				config::processes = rootAstRef->getAllProcessDeclarations();
				config::initializeAuxiliaryPredicates();
				config::globalPredicatesCount = config::globalPredicates.size();
			}

			config::initializeAuxiliaryVariables();

			cout << "\nUsing global predicates:\n";

			for (Ast* globalPredicate : config::globalPredicates)
			{
				cout << globalPredicate->getCode() << "\n";
			}

			cout << "\nPerforming predicate abstraction...\n";

			/*config::initializeImplicativeCubes();
			config::getAllFalseImplyingCubes();*/

			rootAstRef->cascadingUnfoldIfElses();
			rootAstRef->cascadingPerformPredicateAbstraction();
			rootAstRef->setVariableInitializations();
			config::carryOutLazyReplacements();
			config::lazyReplacements.clear();

			/*cout << "\n---\nCarried out predicate abstraction:\n\n";
			cout << rootAstRef->emitCode();*/
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
	string booleanOutputPath = fileNameStub + "." + BOOLEAN_EXTENSION;
	ofstream booleanProgramOut(booleanOutputPath);
	booleanProgramOut << rootAstRef->emitCode() << "\n";
	//booleanProgramOut << rootAstRef->emitCode() << "\n\n" << endAssertion;
	booleanProgramOut.close();

	// Output elapsed time
	time_t finishedAt = time(0);
	double elapsedSeconds = difftime(finishedAt, startedAt);

	int elapsedHours = elapsedSeconds / (60 * 60);
	elapsedSeconds -= elapsedHours * 60 * 60;
	int elapsedMinutes = elapsedSeconds / 60;
	elapsedSeconds -= elapsedMinutes * 60;

	cout << "\nElapsed time: " << elapsedHours << "h " << elapsedMinutes << "min " << elapsedSeconds << "s\n";

	return 0;
}