#ifndef __STRING_INCLUDED__
#define __STRING_INCLUDED__
#include <string>
#endif

#ifndef __REGEX_INCLUDED__
#define __REGEX_INCLUDED__
#include <regex>
#endif

#ifndef __PARSE_STATICS_INCLUDED__
#define __PARSE_STATICS_INCLUDED__
#include "parserStatics.h"
#endif

using namespace std;

struct token
{
	string tag;
	vector<string> rules;

	void initialize(string inputTag, string inputRule)
	{
		if (tag.empty())
		{
			tag = inputTag;
		}
		else if (tag != inputTag)
		{
			throwError("initialize(" + inputTag + ", " + inputRule + ") - Token already initialized with \"" + tag + "\".");
		}

		rules.push_back(normalize(inputRule));
	}

	void initialize(string inputLine)
	{
		vector<string> separatedLine = separate(inputLine, DESCRIPTION_RULE_SEPARATOR);

		if (separatedLine.size() == 2)
		{
			initialize(separatedLine.at(0), separatedLine.at(1));
		}
		else
		{
			throwError("Invalid token initialization input \"" + inputLine + "\"");
		}
	}

	bool isInitialized()
	{
		return tag.length() > 0 && rules.size() > 0;
	}

	string toString()
	{
		string rule = "";
		int rulesCount = rules.size();

		for (int ctr = 0; ctr < rulesCount; ctr++)
		{
			rule += "\"" + rules.at(ctr) + "\"";

			if (ctr < rulesCount - 1)
			{
				rule += ", ";
			}
		}

		return "token " + tag + " { \"" + rule + "\" }";
	}

	string applyToString(string s)
	{
		regex ruleRegex;
		int rulesCount = rules.size();
		string currentRule;
		string result = s;
		string oldResult = "";

		while (result != oldResult)
		{
			oldResult = result;
			smatch stringMatch;

			for (int ctr = 0; ctr < rulesCount; ctr++)
			{
				//cout << ctr << " " << toString() << "\n";
				currentRule = rules.at(ctr);

				if (currentRule != IDENTIFIER_RULE)
				{
					ruleRegex = regex(currentRule);

					/*regex_search(result, stringMatch, ruleRegex);
					cout << "\t\t\t" << stringMatch[0] << "\n";*/

					result = regex_replace(result, ruleRegex, tag);
				}
				else
				{
					//cout << result << "\n\n";

					string identifier;
					string extendedIdentifier;
					string currentWord;
					ruleRegex = regex("(\\S+)");
					vector<string> matches = find_all_matches(ruleRegex, result);
					int matchesCount = matches.size();
					string tagPrefix = tag.substr(0, tag.length() - 2);

					for (int ctr = 0; ctr < matchesCount; ctr++)
					{
						currentWord = matches.at(ctr);

						if (!(identifier = toIdentifier(currentWord)).empty())
						{
							extendedIdentifier = " " + identifier + " ";
							ruleRegex = regex(extendedIdentifier);
							result = regex_replace(result, ruleRegex, " " + tagPrefix + TAG_PARAMETER_SEPARATOR + identifier + RIGHT_TOKEN_DELIMITER + " ");
						}
					}
				}
			}
		}

		return result;
	}
};
typedef struct token token;