#ifndef __PARSER_H_INCLUDED__
#define __PARSER_H_INCLUDED__

#include <fstream>
#include <sstream>
#include <string>
#include <list>

using namespace std;

string parse(string path)
{
	string line;
	ifstream file(path);
	list<string> allLines;
	list<string> result;

	while (getline(file, line))
	{
		allLines.push_front(line);
	}


}

#endif