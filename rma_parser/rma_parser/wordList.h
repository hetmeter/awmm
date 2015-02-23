#ifndef __VECTOR_INCLUDED__
#define __VECTOR_INCLUDED__
#include <vector>
#endif

#ifndef __STRING_INCLUDED__
#define __STRING_INCLUDED__
#include <string>
#endif

#ifndef __TOKEN_H_INCLUDED__
#define __TOKEN_H_INCLUDED__
#include "token.h"
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

	void insertWord(string word, int position)
	{
		words.insert(words.begin() + position, word);
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
		int inputLength = input.length();
		string temp = "";
		char currentChar;
		CharacterType currentCharType;
		CharacterType lastCharType;

		if (inputLength > 0)
		{
			lastCharType = getType(input.at(0));
		}

		for (int ctr = 0; ctr < inputLength; ctr++)
		{
			currentChar = input.at(ctr);
			currentCharType = getType(currentChar);

			if (lastCharType == currentCharType && currentCharType != OTHER)
			{
				temp += currentChar;
			}
			else
			{
				appendWord(temp);
				temp = string(1, currentChar);
			}

			lastCharType = getType(currentChar);
		}

		appendWord(temp);
	}

	string toString()
	{
		string result = "wordList { ";

		for (int ctr = 0; ctr < wordCount; ctr++)
		{
			result += "\"" + words[ctr] + "\"";

			if (ctr < wordCount - 1)
			{
				result += ",";
			}

			result += " ";
		}

		result += "}";

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

	void applyReplacement(string replacement, string replacee)
	{
		for (int ctr = 0; ctr < wordCount; ctr++)
		{
			if (words.at(ctr) == replacee)
			{
				removeWord(ctr);
				insertWord(replacement, ctr);
			}
			else if (replacee == IDENTIFIER_RULE)	// special case: identifier
			{
				if (isIdentifier(words.at(ctr)))
				{
					removeWord(ctr);
					insertWord(replacement, ctr);
				}
			}
			else if (replacee == NUMBER_RULE)	// special case: number
			{
				if (isNumber(words.at(ctr)))
				{
					removeWord(ctr);
					insertWord(replacement, ctr);
				}
			}
		}
	}

	void applyAllReplacements(string replacement, vector<wordList> rule)
	{
		int ruleCount = rule.size();

		for (int ctr = 0; ctr < ruleCount; ctr++)
		{
			for (int subctr = 0; subctr < rule.at(ctr).wordCount; subctr++)
			{
				applyReplacement(replacement, rule.at(ctr).words.at(subctr));
			}
		}
	}
};
typedef struct wordList wordList;