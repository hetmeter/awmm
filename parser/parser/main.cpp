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

	ifstream ruleFile(config::CONFIG_FILE_PATH);
	string line;
	smatch stringMatch;
	ssub_match subMatch;
	string property;
	string value;
	regex propertyValueRegex(config::CONFIG_REGEX);
	int commentMarkerIndex;

	while (getline(ruleFile, line))
	{
		commentMarkerIndex = line.find(config::CONFIG_COMMENT);

		if (commentMarkerIndex != string::npos)
		{
			line = line.substr(0, commentMarkerIndex);
		}

		if (regex_match(line, propertyValueRegex))
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

	parse(lexerPath, programParserPath, predicateParserPath, programInputPath, predicateInputPath);

	ofstream programOut(programOutputPath);
	programOut << programInputContent;
	programOut.close();

	ofstream predicateOut(predicateOutputPath);
	predicateOut << predicateInputContent;
	predicateOut.close();

	return 0;
}