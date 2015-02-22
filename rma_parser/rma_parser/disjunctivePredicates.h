#ifndef __VECTOR_INCLUDED__
#define __VECTOR_INCLUDED__
#include <vector>
#endif

#ifndef __WORDLIST_H_INCLUDED__
#define __WORDLIST_H_INCLUDED__
#include "wordList.h"
#endif

using namespace std;

struct disjunctivePredicates
{
	vector<wordList> predicates;

	void parsePredicates(string rule)
	{
		//cout << "Parsing " << rule << ":\n";

		int orPosition;
		string normalizedRule = normalize(rule);

		if ((orPosition = normalizedRule.find(OR_OPERATOR)) != string::npos)
		{
			parsePredicates(normalizedRule.substr(0, orPosition));

			if (normalizedRule[orPosition + 1] != '\0')
			{
				parsePredicates(normalizedRule.substr(orPosition + 1, string::npos));
			}
		}
		else
		{
			wordList currentPredicate;
			currentPredicate.parseString(normalizedRule);
			currentPredicate.prune();
			
			if (currentPredicate.wordCount > 0)
			{
				predicates.push_back(currentPredicate);
			}
			//cout << "Parsed " << currentPredicate.toString() << "\n";
		}
	}

	string toString()
	{
		string result = "";
		int numberOfPredicates = predicates.size();

		for (int ctr = 0; ctr < numberOfPredicates; ctr++)
		{
			result += "PREDICATE:\n" + predicates[ctr].toString() + "\n";
		}

		return result;
	}
};
typedef struct disjunctivePredicates disjunctivePredicates;