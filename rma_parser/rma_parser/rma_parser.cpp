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
	string rulesPath;
	string inputPath;
	string outputPath;

	if (argc > 1)
	{
		rulesPath = argv[1];
	}
	else
	{
		cout << "No rule file path specified\n";
		return 1;
	}

	if (argc > 2)
	{
		inputPath = argv[2];
	}
	else
	{
		cout << "No input path specified\n";
		return 1;
	}

	if (argc > 3)
	{
		outputPath = argv[3];
	}
	else
	{
		cout << "No output path specified\n";
		return 1;
	}

	cout << rulesPath << '\n';

	string parsedLexer = parse(rulesPath, inputPath);

	ofstream out(outputPath);
	out << parsedLexer;
	out.close();

	return 0;
}