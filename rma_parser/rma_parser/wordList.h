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
		if (input != "")
		{
			string currentWord = "";
			int ctr = 0;
			char currentChar = input[ctr];
			//bool parsingWhitespace = isWhitespace(currentChar);
			//char currentWhitespace = parsingWhitespace ? currentChar : '\0';
			bool parsingWhitespace = false;
			bool currentCharIsWhitespace = isWhitespace(currentChar);
			char currentWhitespace = '\0';

			while (currentChar != '\0')
			{
				//cout << "Parsing " << currentChar << "\n";

				if (parsingWhitespace)
				{
					if (currentCharIsWhitespace && currentChar == currentWhitespace)
					{
						//cout << "\tAdding " << currentChar << " to current word\n";
						currentWord += currentChar;
						//cout << "\t\t" << currentWord << "\n";
					}
					else if (currentCharIsWhitespace && currentChar != currentWhitespace)
					{
						//cout << "\t\tAdding " << currentWord << " to list\n";
						appendWord(currentWord);
						//cout << "\tAdding " << currentChar << " to current word\n";
						currentWord = string(1, currentChar);
					}
					else
					{
						//cout << "\t\tAdding " << currentWord << " to list\n";
						appendWord(currentWord);
						//cout << "\tAdding " << currentChar << " to current word\n";
						currentWord = string(1, currentChar);
						parsingWhitespace = false;
					}
				}
				else
				{
					if (currentCharIsWhitespace)
					{
						//cout << "\t\tAdding " << currentWord << " to list\n";
						appendWord(currentWord);
						//cout << "\tAdding " << currentChar << " to current word\n";
						currentWord = string(1, currentChar);
						parsingWhitespace = true;
					}
					else
					{
						//cout << "\tAdding " << currentChar << " to current word\n";
						currentWord += currentChar;
						//cout << "\t\t" << currentWord << "\n";
					}
				}

				ctr++;
				currentChar = input[ctr];
				currentCharIsWhitespace = isWhitespace(currentChar);
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

	void applyRule(parserToken token)
	{
		// left off here
	}
};
typedef struct wordList wordList;