/*
Struct representing a token containing a tag and at least one rule used to process a raw string input by successively performing regex replacements
with each rule being a regex search pattern used to replace any corresponding matches with the regex expression stored in the rule
*/

#include "token.h"

#include "config.h"

using namespace std;

// Initializes the tag-rule-pair. If the token has an empty tag, it adopts the tag parameter, if it has a differring tag from the parameter, it
// throws an error. If no error is thrown, the rule parameter is added to the current set of rules.
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

	rules.push_back(inputRule);
}

// Initializes the token by parsing the raw input line and calling the initialize method taking the separated tag-rule-pair. If such a separation
// fails, an error is thrown.
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

// Returns whether the token has been initialized successfully
bool token::isInitialized()
{
	return tag.length() > 0 && rules.size() > 0;
}

// Returns a string representation of the token
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

// Applies the token to an input string until this token can no longer effect any change
string token::applyToString(string s)
{
	regex ruleRegex;
	int rulesCount = rules.size();
	string currentRule;
	string result = s;
	string oldResult = "";

	while (result != oldResult)	// Apply the token until no further change is registered
	{
		oldResult = result;
		smatch stringMatch;

		for (int ctr = 0; ctr < rulesCount; ctr++)	// Iterate through all rules
		{
			currentRule = rules.at(ctr);

			// Check if the rule is an identifier rule. If not, apply a regex replace with the rule as the search-,
			// and the tag as the replace pattern
			if (currentRule != config::IDENTIFIER_RULE)
			{
				ruleRegex = regex(currentRule);

				result = regex_replace(result, ruleRegex, tag);
			}
			else // If the rule is an identifier rule, apply a regex replace with the extracted identifier as the search-, and the tag as the replace pattern
			{
				string identifier;
				string extendedIdentifier;
				string currentWord;
				ruleRegex = regex("(\\S+)");
				vector<string> matches = config::findAllMatches(ruleRegex, result);	// Collect every non-whitespace character sequence
				int matchesCount = matches.size();
				string tagPrefix = tag.substr(0, tag.length() - 2);	// This is a hardcoded assumption that the ID tag ends with "} "

				for (int ctr = 0; ctr < matchesCount; ctr++)	// Iterate through every non-whitespace character sequence found
				{
					currentWord = matches.at(ctr);

					if (!(identifier = config::toIdentifier(currentWord)).empty())
					{
						extendedIdentifier = " " + identifier + " ";	// This is a hardcoded assumption that by this point, all words are space-padded
						ruleRegex = regex(extendedIdentifier);

						// Replace all instances of the current identifier with the ID tag containing the identifier
						result = regex_replace(result, ruleRegex, " " + tagPrefix + config::TAG_PARAMETER_SEPARATOR + identifier + config::RIGHT_TOKEN_DELIMITER + " ");
					}
				}
			}
		}
	}

	return result;
}