#ifndef __VECTOR_INCLUDED__
#define __VECTOR_INCLUDED__
#include <vector>
#endif

#ifndef __STRING_INCLUDED__
#define __STRING_INCLUDED__
#include <string>
#endif

#ifndef __PARSERTOKEN_H_INCLUDED__
#define __PARSERTOKEN_H_INCLUDED__
#include "parserToken.h"
#endif

#ifndef __PARSERSTATICS_H_INCLUDED__
#define __PARSERSTATICS_H_INCLUDED__
#include "parserStatics.h"
#endif

using namespace std;

struct wordList
{
	vector<string> words;
	int wordCount = 0;

	void appendWord(string word)
	{
		//cout << "\t\t\t" << word << "\n";
		words.push_back(word);
		wordCount++;
	}

	void removeWord(int position)
	{
		if (position < wordCount && position >= 0)
		{
			//cout << "\t\t---\n\t\tremoveWord(" << position << "):\n" << toString() << "\n";

			words.erase(words.begin() + position);
			wordCount--;

			//cout << "\t\tremoveWord(" << position << ") done.\n" << toString() << "\n\t\t---\n";
		}
	}

	void parseString(string input)
	{
		cout << "Parsing \"" << input << "\" for a wordList...\n";

		string temp = "";
		int ctr = 0;
		char currentChar = input[ctr];
		bool charIsWhitespace = isWhitespace(currentChar);
		bool lastCharWasWhitespace = isWhitespace(currentChar);

		while (currentChar != '\0')
		{
			if (lastCharWasWhitespace == charIsWhitespace)	// Non-whitespace continuing
			{
				temp += currentChar;
			}
			else
			{
				cout << "\tAppending \"" << temp << "\"\n";

				appendWord(temp);
				temp = string(1, currentChar);
			}

			lastCharWasWhitespace = isWhitespace(currentChar);
			ctr++;
			currentChar = input[ctr];
			charIsWhitespace = isWhitespace(currentChar);

			if (currentChar == '\0')
			{
				cout << "\tAppending \"" << temp << "\"\n";

				appendWord(temp);
			}
		}
	}

	string toString()
	{
		string result = "";

		for (int ctr = 0; ctr < wordCount; ctr++)
		{
			result += words[ctr] + "\n";
		}

		return result;
	}

	void prune()
	{
		int ctr = 0;

		//cout << "Pruning:\n" << toString() << "---\n";

		while (ctr < wordCount)
		{
			if (words[ctr].find_first_not_of(WHITESPACES) == string::npos)
			{
				//cout << "Preparing to remove: \"" << words[ctr] << "\"\n";

				removeWord(ctr);
				ctr--;
			}

			ctr++;
		}

		//cout << "Finished pruning\n";
	}
};
typedef struct wordList wordList;