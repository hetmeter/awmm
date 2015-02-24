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
private: int _hashCode = -1;

public:
	vector<string> words;
	int wordCount = 0;

	void appendWord(string word)
	{
		words.push_back(word);
		wordCount++;
		_hashCode = -1;
	}

	void insertWord(string word, int position)
	{
		words.insert(words.begin() + position, word);
		wordCount++;
		_hashCode = -1;
	}

	void removeWord(int position)
	{
		if (position < wordCount && position >= 0)
		{
			words.erase(words.begin() + position);
			wordCount--;
			_hashCode = -1;
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

	void collapse()
	{
		for (int ctr = 0; ctr < wordCount; ctr++)
		{
			if (words.at(ctr).find_first_not_of(WHITESPACES) == string::npos)
			{
				removeWord(ctr);
				ctr--;
			}
		}
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
			else if (replacee == WHITESPACE_RULE)	// special case: whitespace
			{
				if (isWhitespace(words.at(ctr)))
				{
					removeWord(ctr);
					insertWord(replacement, ctr);
				}
			}
		}
	}

	void applyReplacement(string replacement, wordList replacee)
	{
		int wordsInSequence = replacee.wordCount;
		int matchesSoFar = 0;

		for (int ctr = 0; ctr < wordCount; ctr++)
		{
			if (isMatch(words.at(ctr), replacee.words.at(matchesSoFar)))
			{
				matchesSoFar++;
			}
			else
			{
				matchesSoFar = 0;

				if (isMatch(words.at(ctr), replacee.words.at(matchesSoFar)))
				{
					matchesSoFar++;
				}
			}

			if (matchesSoFar == wordsInSequence)
			{
				ctr = ctr - (wordsInSequence - 1);

				for (int subctr = 0; subctr < wordsInSequence; subctr++)
				{
					removeWord(ctr);
				}

				if (replacement != IGNORE_TAG)
				{
					insertWord(replacement, ctr);
				}

				matchesSoFar = 0;
			}
		}
	}

	void applyAllReplacements(string replacement, vector<wordList> rule)
	{
		int ruleCount = rule.size();

		for (int ctr = 0; ctr < ruleCount; ctr++)
		{
			applyReplacement(replacement, rule.at(ctr));
		}
	}

	int hashCode()
	{
		if (_hashCode == -1)
		{
			hash<string> stringHash;

			for (int ctr = 0; ctr < wordCount; ctr++)
			{
				_hashCode = _hashCode ^ stringHash(words.at(ctr));
			}
		}

		return _hashCode;

		/*hash<string> stringHash;
		_hashCode = -1;

		for (int ctr = 0; ctr < wordCount; ctr++)
		{
			_hashCode = _hashCode ^ stringHash(words.at(ctr));
		}

		return _hashCode;*/
	}
};
typedef struct wordList wordList;