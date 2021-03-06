/*The MIT License(MIT)

Copyright(c) 2015 Attila Zolt�n Printz

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

#include <iostream>
#include <fstream>
#include <string>
#include <regex>

#include "config.h"
#include "parser.h"

using namespace std;

typedef match_results<const char*> regexResultString;

int main(int argc, char** argv)
{
	string lexerPath;
	string programParserPath;
	string predicateParserPath;
	string inputArgument;
	string memoryOrder;
	string fileName;

	// Get the persistent configuration settings from a configuration file
	map<string, string> configVariables = config::parseConfiguration(config::CONFIG_FILE_PATH);

	// Get the program arguments
	if (argc > 1)
	{
		inputArgument = argv[1];
	}
	else
	{
		config::throwError("No program input specified");
		return 1;
	}

	if (argc > 2)
	{
		memoryOrder = argv[2];
	}
	else
	{
		config::throwError("No memory order specified");
		return 1;
	}

	config::evaluationMode = argc > 3 && config::EVALUATION_MODE_PARAMETER.compare(argv[3]) == 0;

	// Derive the input file paths from the input argument
	regex fileNameRegex(config::EXTENSION_REGEX);
	std::smatch stringMatch;

	regex_search(inputArgument, stringMatch, fileNameRegex);
	if (stringMatch[2] == config::RMA_EXTENSION)
	{
		lexerPath = configVariables[config::RMA_LEXER_RULE_FILE_PROPERTY];
		programParserPath = configVariables[config::RMA_PROGRAM_PARSER_RULE_FILE_PROPERTY];
		predicateParserPath = configVariables[config::RMA_PREDICATE_PARSER_RULE_FILE_PROPERTY];
	}
	else if (stringMatch[2] == config::SALPL_EXTENSION && memoryOrder == config::TSO_PARAMETER)
	{
		lexerPath = configVariables[config::TSO_LEXER_RULE_FILE_PROPERTY];
		programParserPath = configVariables[config::TSO_PROGRAM_PARSER_RULE_FILE_PROPERTY];
		predicateParserPath = configVariables[config::TSO_PREDICATE_PARSER_RULE_FILE_PROPERTY];
	}
	else if (stringMatch[2] == config::SALPL_EXTENSION && memoryOrder == config::PSO_PARAMETER)
	{
		lexerPath = configVariables[config::PSO_LEXER_RULE_FILE_PROPERTY];
		programParserPath = configVariables[config::PSO_PROGRAM_PARSER_RULE_FILE_PROPERTY];
		predicateParserPath = configVariables[config::PSO_PREDICATE_PARSER_RULE_FILE_PROPERTY];
	}
	else
	{
		config::throwError("Invalid input file extension");
	}

	string programInputPath = inputArgument;
	string programOutputPath = programInputPath + config::EXTENSION_SEPARATOR + config::OUT_EXTENSION;
	string predicateInputPath = stringMatch[1].str() + config::EXTENSION_SEPARATOR + config::PREDICATE_EXTENSION + config::EXTENSION_SEPARATOR + stringMatch[2].str();
	string predicateOutputPath = predicateInputPath + config::EXTENSION_SEPARATOR + config::OUT_EXTENSION;

	// Parse the input program and predicate files
	parse(lexerPath, programParserPath, predicateParserPath, programInputPath, predicateInputPath);

	// Output the parsed program and predicate each to its own file
	ofstream programOut(programOutputPath);
	programOut << programInputContent;
	programOut.close();

	ofstream predicateOut(predicateOutputPath);
	predicateOut << predicateInputContent;
	predicateOut.close();

	return 0;
}