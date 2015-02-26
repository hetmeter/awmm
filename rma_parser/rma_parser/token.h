#ifndef __STRING_INCLUDED__
#define __STRING_INCLUDED__
#include <string>
#endif

#ifndef __WORDLIST_H_INCLUDED__
#define __WORDLIST_H_INCLUDED__
#include "wordList.h"
#endif

#ifndef __REGEX_INCLUDED__
#define __REGEX_INCLUDED__
#include <regex>
#endif

using namespace std;

//struct token
//{
//	string tag;
//	vector<wordList> rule;
//
//	void initialize(string inputTag, string inputRule)
//	{
//		tag = inputTag;
//
//		//cout << "Initializing token with inputTag = \"" << inputTag << "\", inputRule = \"" << inputRule << "\"...\n";
//
//		string normalizedInputRule = normalize(inputRule);
//
//		//cout << "\tNormalized inputrule as \"" << normalizedInputRule << "\"\n";
//
//		int orIndex = normalizedInputRule.find(OR_OPERATOR);
//		wordList* currentWordList;
//
//		if (orIndex == string::npos)
//		{
//			currentWordList = new wordList;
//			currentWordList->parseString(normalizedInputRule);
//			currentWordList->collapse();
//			rule.push_back(*currentWordList);
//		}
//		else
//		{
//			while (orIndex != string::npos)
//			{
//				currentWordList = new wordList;
//				currentWordList->parseString(normalizedInputRule.substr(0, orIndex));
//				currentWordList->collapse();
//				rule.push_back(*currentWordList);
//
//				normalizedInputRule = ((int)normalizedInputRule.length()) > orIndex + 1 ? normalizedInputRule.substr(orIndex + 1, string::npos) : "";
//				orIndex = normalizedInputRule.find(OR_OPERATOR);
//
//				if (orIndex == string::npos)
//				{
//					currentWordList = new wordList;
//					currentWordList->parseString(normalizedInputRule);
//					currentWordList->collapse();
//					rule.push_back(*currentWordList);
//				}
//			}
//		}
//
//		if (orIndex != string::npos && orIndex == inputRule.rfind(OR_OPERATOR) && ((int)inputRule.length()) > orIndex + 1)
//		{
//			initialize(inputRule.substr(0, orIndex), inputRule.substr(orIndex + 1, string::npos));
//		}
//
//		//rule.parseString(inputRule);
//		//rule.prune();
//	}
//
//	void initialize(string inputLine)
//	{
//		int separatorIndex = inputLine.find(DESCRIPTION_RULE_SEPARATOR);
//
//		if (separatorIndex != string::npos && separatorIndex == inputLine.rfind(DESCRIPTION_RULE_SEPARATOR) && ((int)inputLine.length()) > separatorIndex + 1)
//		{
//			initialize(inputLine.substr(0, separatorIndex), inputLine.substr(separatorIndex + 1, string::npos));
//		}
//	}
//
//	bool isInitialized()
//	{
//		return tag.length() > 0 && rule.size() > 0u;
//	}
//
//	string toString()
//	{
//		string result = "token " + tag + " {\n";
//		int numberOfRules = rule.size();
//
//		for (int ctr = 0; ctr < numberOfRules; ctr++)
//		{
//			result += "\t" + rule[ctr].toString() + "\n";
//		}
//
//		result += "}";
//
//		return result;
//	}
//
//	void applyToWordList(wordList* wl)
//	{
//		wl->applyAllReplacements(tag, rule);
//	}
//};
//typedef struct token token;

struct token
{
	string tag;
	string rule;

	void initialize(string inputTag, string inputRule)
	{
		tag = normalize(inputTag);
		rule = normalize(inputRule);
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
		return tag.length() > 0u && rule.length() > 0u;
	}

	string toString()
	{
		return "token " + tag + " { \"" + rule + "\" }";
	}

	void applyToString(string s)
	{
		cout << "---\nApplying \"" << rule << "\" -> \"" << tag << "\" to \"" << s << "\":\n---\n";

		regex ruleRegex(rule);
		s = regex_replace(s, ruleRegex, tag);

		cout << "\tResult: \"" << s << "\":\n";
	}
};
typedef struct token token;