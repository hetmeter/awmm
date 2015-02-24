#ifndef __STRING_INCLUDED__
#define __STRING_INCLUDED__
#include <string>
#endif

#ifndef __WORDLIST_H_INCLUDED__
#define __WORDLIST_H_INCLUDED__
#include "wordList.h"
#endif

using namespace std;

//struct parserToken
//{
//	string tag;
//	string rule;
//
//	string toString()
//	{
//		return "Parser token " + tag + " implements " + rule;
//	}
//};
//typedef struct parserToken parserToken;

struct parserToken
{
	string tag;
	vector<wordList> rule;

	void initialize(string inputTag, string inputRule)
	{
		tag = inputTag;

		int orIndex = inputRule.find(DESCRIPTION_RULE_SEPARATOR);

		if (orIndex != string::npos && orIndex == inputRule.rfind(DESCRIPTION_RULE_SEPARATOR) && ((int)inputRule.length()) > orIndex + 1)
		{
			initialize(inputRule.substr(0, orIndex), inputRule.substr(orIndex + 1, string::npos));
		}

		rule.parseString(inputRule);
		rule.prune();
	}

	void initialize(string inputLine)
	{
		int separatorIndex = inputLine.find(DESCRIPTION_RULE_SEPARATOR);

		if (separatorIndex != string::npos && separatorIndex == inputLine.rfind(DESCRIPTION_RULE_SEPARATOR) && ((int)inputLine.length()) > separatorIndex + 1)
		{
			initialize(inputLine.substr(0, separatorIndex), inputLine.substr(separatorIndex + 1, string::npos));
		}
	}

	bool isInitialized()
	{
		return tag.length() > 0 && rule.wordCount > 0;
	}

	string toString()
	{
		return "\tParser token\n\t" + tag + "\n\timplements\n" + rule.toString();
	}
};
typedef struct parserToken parserToken;