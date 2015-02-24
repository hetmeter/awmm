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

#ifndef __PARSER_H_INCLUDED__
#define __PARSER_H_INCLUDED__
#include "parser.h"
#endif

using namespace std;

int main(int argc, char** argv)
{
	string lexerPath;
	string programParserPath;
	string predicateParserPath;
	string programInputPath;
	string programOutputPath;
	string predicateInputPath;
	string predicateOutputPath;

	if (argc > 1)
	{
		lexerPath = argv[1];
	}
	else
	{
		cout << "No lexer rules file path specified\n";
		return 1;
	}

	if (argc > 2)
	{
		programParserPath = argv[2];
	}
	else
	{
		cout << "No program parser rules file path specified\n";
		return 1;
	}

	if (argc > 3)
	{
		predicateParserPath = argv[3];
	}
	else
	{
		cout << "No predicate parser rules file path specified\n";
		return 1;
	}

	if (argc > 4)
	{
		programInputPath = argv[4];
	}
	else
	{
		cout << "No program input path specified\n";
		return 1;
	}

	if (argc > 5)
	{
		programOutputPath = argv[5];
	}
	else
	{
		cout << "No program output path specified\n";
		return 1;
	}

	if (argc > 6)
	{
		predicateInputPath = argv[6];
	}
	else
	{
		cout << "No predicate input path specified\n";
		return 1;
	}

	if (argc > 7)
	{
		predicateOutputPath = argv[7];
	}
	else
	{
		cout << "No predicate output path specified\n";
		return 1;
	}

	parse(lexerPath, programParserPath, predicateParserPath, programInputPath, predicateInputPath);

	ofstream programOut(programOutputPath);
	programOut << programInputContent.toString();
	programOut.close();

	ofstream predicateOut(predicateOutputPath);
	predicateOut << predicateInputContent.toString();
	predicateOut.close();

	return 0;
}