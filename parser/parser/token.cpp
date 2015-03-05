#include "token.h"

#include "config.h"

using namespace std;

void token::initialize(string inputTag, string inputRule)
{
	if (tag.empty())
	{
		tag = inputTag;
	}
	else if (tag != inputTag)
	{
		config::throwError("initialize(" + inputTag + ", " + inputRule + ") - Token already initialized with \"" + tag + "\".");
	}

	rules.push_back(config::normalize(inputRule));
}

void token::initialize(string inputLine)
{
	vector<string> separatedLine = config::separate(inputLine, config::DESCRIPTION_RULE_SEPARATOR);

	if (separatedLine.size() == 2)
	{
		initialize(separatedLine.at(0), separatedLine.at(1));
	}
	else
	{
		config::throwError("Invalid token initialization input \"" + inputLine + "\"");
	}
}

bool token::isInitialized()
{
	return tag.length() > 0 && rules.size() > 0;
}

string token::toString()
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

string token::applyToString(string s)
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

			if (currentRule != config::IDENTIFIER_RULE)
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
				vector<string> matches = config::find_all_matches(ruleRegex, result);
				int matchesCount = matches.size();
				string tagPrefix = tag.substr(0, tag.length() - 2);

				for (int ctr = 0; ctr < matchesCount; ctr++)
				{
					currentWord = matches.at(ctr);

					if (!(identifier = config::toIdentifier(currentWord)).empty())
					{
						extendedIdentifier = " " + identifier + " ";
						ruleRegex = regex(extendedIdentifier);
						result = regex_replace(result, ruleRegex, " " + tagPrefix + config::TAG_PARAMETER_SEPARATOR + identifier + config::RIGHT_TOKEN_DELIMITER + " ");
					}
				}
			}
		}
	}

	return result;
}