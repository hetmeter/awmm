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
	string programInputPath;
	string programOutputPath;
	string predicateInputPath;
	string predicateOutputPath;

	// Get the persistent configuration settings from a configuration file
	ifstream ruleFile(config::CONFIG_FILE_PATH);
	string line;
	smatch stringMatch;
	string property;
	string value;
	regex propertyValueRegex(config::CONFIG_REGEX);
	int commentMarkerIndex;

	while (getline(ruleFile, line))	// Iterate through every line
	{
		commentMarkerIndex = line.find(config::CONFIG_COMMENT);	// Check if the line has a comment

		if (commentMarkerIndex != string::npos)
		{
			line = line.substr(0, commentMarkerIndex);	// If the line has a comment marker, remove the comment
		}

		if (regex_match(line, propertyValueRegex))	// If the line matches the format of a property-value assignment, parse them
		{
			regex_search(line, stringMatch, propertyValueRegex);

			if (stringMatch.size() >= 3)
			{
				property = stringMatch[1].str();
				value = stringMatch[2].str();

				if (property == config::LEXER_RULE_FILE_PROPERTY)
				{
					lexerPath = value;
				}
				else if (property == config::PROGRAM_PARSER_RULE_FILE_PROPERTY)
				{
					programParserPath = value;
				}
				else if (property == config::PREDICATE_PARSER_RULE_FILE_PROPERTY)
				{
					predicateParserPath = value;
				}
			}
		}
	}

	// Get the program arguments
	if (argc > 1)
	{
		programInputPath = argv[1];
	}
	else
	{
		cout << "No program input path specified\n";
		return 1;
	}

	if (argc > 2)
	{
		programOutputPath = argv[2];
	}
	else
	{
		cout << "No program output path specified\n";
		return 1;
	}

	if (argc > 3)
	{
		predicateInputPath = argv[3];
	}
	else
	{
		cout << "No predicate input path specified\n";
		return 1;
	}

	if (argc > 4)
	{
		predicateOutputPath = argv[4];
	}
	else
	{
		cout << "No predicate output path specified\n";
		return 1;
	}

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