#ifndef __CSTDIO_INCLUDED__
#define __CSTDIO_INCLUDED__
#include <cstdio>
#endif

#ifndef __IOSTREAM_INCLUDED__
#define __IOSTREAM_INCLUDED__
#include <iostream>
#endif

#ifndef __STRING_INCLUDED__
#define __STRING_INCLUDED__
#include <string>
#endif

#ifndef __REGEX_INCLUDED__
#define __REGEX_INCLUDED__
#include <regex>
#endif

#ifndef __PARSER_H_INCLUDED__
#define __PARSER_H_INCLUDED__
#include "parser.h"
#endif

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

	ifstream ruleFile(CONFIG_FILE_PATH);
	string line;
	smatch stringMatch;
	ssub_match subMatch;
	string property;
	string value;
	regex propertyValueRegex(CONFIG_REGEX);
	int commentMarkerIndex;

	while (getline(ruleFile, line))
	{
		commentMarkerIndex = line.find(CONFIG_COMMENT);

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
				
				if (property == LEXER_RULE_FILE_PROPERTY)
				{
					lexerPath = value;
				}
				else if (property == PROGRAM_PARSER_RULE_FILE_PROPERTY)
				{
					programParserPath = value;
				}
				else if (property == PREDICATE_PARSER_RULE_FILE_PROPERTY)
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

	cout << "lexerPath = " << lexerPath << "\n"
		<< "programParserPath = " << programParserPath << "\n"
		<< "predicateParserPath = " << predicateParserPath << "\n"
		<< "programInputPath = " << programInputPath << "\n"
		<< "programOutputPath = " << programOutputPath << "\n"
		<< "predicateInputPath = " << predicateInputPath << "\n"
		<< "predicateOutputPath = " << predicateOutputPath << "\n";

	parse(lexerPath, programParserPath, predicateParserPath, programInputPath, predicateInputPath);

	ofstream programOut(programOutputPath);
	programOut << programInputContent;
	programOut.close();

	ofstream predicateOut(predicateOutputPath);
	predicateOut << predicateInputContent;
	predicateOut.close();

	cout << "\nParsed program:\n" << programInputContent << "\n\nParsed predicates:\n" << predicateInputContent << "\n";

	return 0;
}