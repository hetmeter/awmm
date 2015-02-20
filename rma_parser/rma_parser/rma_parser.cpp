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

#ifndef __LEXER_H_INCLUDED__
#define __LEXER_H_INCLUDED__
#include "lexer.h"
#endif

using namespace std;

int main(int argc, char** argv)
{
	string lexerPath;
	string outputPath;

	if (argc > 1)
	{
		lexerPath = argv[1];
	}
	else
	{
		cout << "No lexer path specified\n";
		return 1;
	}

	if (argc > 2)
	{
		outputPath = argv[2];
	}
	else
	{
		cout << "No output path specified\n";
		return 1;
	}

	cout << lexerPath << '\n';

	string parsedLexer = parseLexer(lexerPath);

	ofstream out(outputPath);
	out << parsedLexer;
	out.close();

	return 0;
}