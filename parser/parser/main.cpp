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
	else if (stringMatch[2] == config::TSO_EXTENSION)
	{
		lexerPath = configVariables[config::TSO_LEXER_RULE_FILE_PROPERTY];
		programParserPath = configVariables[config::TSO_PROGRAM_PARSER_RULE_FILE_PROPERTY];
		predicateParserPath = configVariables[config::TSO_PREDICATE_PARSER_RULE_FILE_PROPERTY];
	}
	else if (stringMatch[2] == config::PSO_EXTENSION)
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