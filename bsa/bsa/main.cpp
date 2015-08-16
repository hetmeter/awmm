/*The MIT License(MIT)

Copyright(c) 2015 Attila Zoltán Printz

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

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
#include "PredicateData.h"
#include "MergeableSetContainer.h"
#include "VariableEntry.h"

using namespace std;

/* Local constants */
const string ACCEPTING_STATE_REGEX = "\\{ACCEPTING_STATE,(\\S+)\\}";
const string EXTENSION_REGEX = "(.*)\\.(\\S\\S\\S)\\.out";
const string SALPL_EXTENSION = "sal";
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

	if (argc > 4)
	{
		if (literalCode::MEMORY_ORDER_PSO.compare(argv[4]) == 0)
		{
			config::memoryOrdering = config::PSO;
		}
		else if (literalCode::MEMORY_ORDER_TSO.compare(argv[4]) == 0)
		{
			config::memoryOrdering = config::TSO;
		}
		else
		{
			config::throwCriticalError("Invalid memory ordering scheme");
		}
	}
	else
	{
		config::throwCriticalError("No memory ordering scheme specified");
	}

	config::evaluationMode = argc > 5 && literalCode::EVALUATION_MODE_PARAMETER.compare(argv[5]) == 0;
	config::verboseMode = argc > 5 && literalCode::VERBOSE_MODE_PARAMETER.compare(argv[5]) == 0;

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

		if (extension == SALPL_EXTENSION)
		{
			config::currentLanguage = config::language::SALPL;
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

	if (!config::evaluationMode)
	{
		cout << "Parsing program...\n";
	}

	Ast* rootAstRef = Ast::newAstFromParsedProgram(parsedProgramString);

	if (!config::evaluationMode)
	{
		cout << "Performing buffer size analysis...\n";
	}
	/*cout << "\n---\nParsed program:\n\n";
	cout << rootAstRef->getCode();*/

	// Register all global variables
	rootAstRef->getChild(0)->registerIDsAsGlobal();

	// Register all local variables and processes
	for (int ctr = 1; ctr < rootAstRef->getChildrenCount(); ctr++)
	{
		if (rootAstRef->getChild(ctr)->getName().compare(literalCode::PROCESS_DECLARATION_TOKEN_NAME) == 0)
		{
			config::tryRegisterProcess(stoi(rootAstRef->getChild(ctr)->getChild(0)->getChild(0)->getName()));
			rootAstRef->getChild(ctr)->registerIDsAsLocal();
		}
	}

	// Register all auxiliary variables
	config::generateAllAuxiliarySymbols();

	rootAstRef->getCostsFromChildren();						// Send a cascading command to the root node that results in all program points storing the buffer size increases they cause
	rootAstRef->initializePersistentCosts();				// Prompt the AST to gather all globally initialized variables and have all program point nodes keep track of their buffer sizes
	rootAstRef->topDownCascadingRegisterLabels();			// Send a cascading command to the root node that results in all label AST nodes registering themselves in a global map
	rootAstRef->topDownCascadingGenerateOutgoingEdges();	// Send a cascading command to the root node that results in all program points estabilishing outgoing program flow edges to their possible successor nodes in the control flow graph
	rootAstRef->visitAllProgramPoints();					// Generate one control flow visitor in the first program point nodes of each process declaration and prompt them to start traversing the AST
	rootAstRef->topDownCascadingReportBufferSizes();
	rootAstRef->performBufferSizeAnalysisReplacements();
	
	config::carryOutLazyReplacements();
	config::lazyReplacements.clear();

	// Generate the output
	outputPath = fileNameStub + "." + BSA_EXTENSION + "." + extension;
	ofstream programOut(outputPath);
	programOut << rootAstRef->getCode();
	programOut.close();

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
			int predicateAstRefChildCount = predicateAstRef->getChildrenCount();
			Ast* currentPredicateAst;

			for (int ctr = 0; ctr < predicateAstRefChildCount; ctr++)
			{
				currentPredicateAst = predicateAstRef->getChild(ctr);
				config::predicates[currentPredicateAst->getCode()] = new PredicateData(currentPredicateAst);
				config::originalPredicateCodes.push_back(currentPredicateAst->getCode());
				++config::originalPredicatesCount;
				++config::totalPredicatesCount;
			}

			config::variableTransitiveClosures = new MergeableSetContainer();
			set<string> currentIDs;

			for (int ctr = 0; ctr < config::originalPredicatesCount; ++ctr)
			{
				currentIDs = config::predicates[config::originalPredicateCodes[ctr]]->getPredicateIDs();

				for (set<string>::iterator it = currentIDs.begin(); it != currentIDs.end(); ++it)
				{
					config::variableTransitiveClosures->insert(*it, ctr);
				}
			}

			if (config::generateAuxiliaryPredicates)
			{ 
				config::initializeAuxiliaryPredicates();
				config::totalPredicatesCount = config::predicates.size();
			}

			/*cout << "\nSymbols:\n";

			for (map<string, VariableEntry*>::iterator it = config::symbolMap.begin(); it != config::symbolMap.end(); it++)
			{
				cout << "\n" << config::addTabs(it->second->toString(), 1) << "\n";
			}

			cout << "\nUsing global predicates:\n";

			for (map<string, PredicateData*>::iterator it = config::predicates.begin(); it != config::predicates.end(); it++)
			{
				cout << "\t" << it->first << "\n" << config::addTabs(it->second->toString(), 2) << "\n";
			}*/

			if (!config::evaluationMode)
			{
				cout << "Performing predicate abstraction...\n";
			}

			rootAstRef->topDownCascadingUnfoldIfElses();
			rootAstRef->topDownCascadingPerformPredicateAbstraction();
			rootAstRef->generateBooleanVariableInitializations();
			config::carryOutLazyReplacements();
			config::lazyReplacements.clear();
		}
		else
		{
			config::throwError("Invalid parsed predicate format or predicates in a non-accepting state");
		}

		config::carryOutLazyReplacements();
	}

	parsedPredicateFile.close();

	rootAstRef->topDownCascadingLabelAllStatements();

	/*cout << "\n---\nFinal state:\n\n";
	cout << rootAstRef->getCode();*/

	// Generate the output
	string booleanOutputPath = fileNameStub + "." + BOOLEAN_EXTENSION;
	ofstream booleanProgramOut(booleanOutputPath);
	booleanProgramOut << rootAstRef->getCode() << "\n";
	booleanProgramOut.close();

	// Output elapsed time
	time_t finishedAt = time(0);
	double totalElapsedSeconds = difftime(finishedAt, startedAt);

	int elapsedHours = totalElapsedSeconds / (60 * 60);
	int elapsedSeconds = totalElapsedSeconds - elapsedHours * 60 * 60;
	int elapsedMinutes = elapsedSeconds / 60;
	elapsedSeconds -= elapsedMinutes * 60;

	if (config::evaluationMode)
	{
		cout << fileNameStub << "." << PREDICATE_EXTENSION << "." << extension << "\t" << elapsedHours << "h " << elapsedMinutes << "min "
			<< elapsedSeconds << "s\t" << totalElapsedSeconds << "\n";
	}
	else
	{
		cout << "\nElapsed time: " << elapsedHours << "h " << elapsedMinutes << "min " << elapsedSeconds << "s\n";
	}

	cout << fileNameStub << "\tcubes: " << config::implicativeCubes.size() << "\tpredicates: " << config::predicates.size() << "\n";

	return 0;
}