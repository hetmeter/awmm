/*
Contains methods and variables used directly for parsing programs and predicates, and retrieving the results
*/

#include "parser.h"

#include "config.h"
#include "token.h"

using namespace std;

string programInputContent;
string predicateInputContent;

// Returns a vector filled with tokens parsed from the file at the path passed by the parameter
vector<token> initializeTokensFromFile(string path)
{
	vector<token> result;
	vector<string> separatedLine;	// Will contain 2 strings, the second being the regex search pattern, the first being the regex replacement string

	ifstream ruleFile(path);
	string line;
	string description;	// Will contain the regex replacement string
	string rule;		// Will contain the regex search pattern
	token* currentToken;

	while (getline(ruleFile, line))	// Read the file line by line
	{
		separatedLine = config::separate(line, config::DESCRIPTION_RULE_SEPARATOR);	// Separates the line into its two tab-separated components

		if (separatedLine.size() == 2)	// Check if the line was formatted correctly and resulted in 2 parts
		{
			description = separatedLine.at(0);
			rule = separatedLine.at(1);

			// If the last token has the same description as the recently parsed one, add the new rule to the same token
			if (result.size() > 0 && result.at(result.size() - 1).tag == description)
			{
				result.at(result.size() - 1).initialize(description, rule);
			}
			else // If the newly parsed description doesn't match the last token's description, create and push a new token
			{
				currentToken = new token;
				currentToken->initialize(description, rule);

				if (currentToken->isInitialized())
				{
					result.push_back(*currentToken);
				}
			}
		}
		else // Throw an error if the line doesn't parse to a description-rule-pair
		{
			config::throwError("Invalid token initialization input \"" + line + "\"");
		}
	}
	
	ruleFile.close();

	return result;
}

// Reads the input file into the string, adds an end-of-file token and pads it with spaces
string initializeInputFromFile(string path)
{
	ifstream inputFile(path);
	stringstream buffer;
	buffer << inputFile.rdbuf();
	string inputString = buffer.str();
	inputFile.close();

	return inputString + " " + config::EOF_TAG + " ";
}

// Applies the lexer tokens and the parser tokens to the content in that order
string processContent(string content, vector<token>* lexerTokens, vector<token>* parserTokens)
{
	int numberOfLexerTokens = lexerTokens->size();
	int numberOfParserTokens = parserTokens->size();
	string result = content;
	string temp;

	cout << "\tApplying lexer tokens...\n";

	for (int ctr = 0; ctr < numberOfLexerTokens; ctr++)	// Apply each lexer token once
	{
		result = lexerTokens->at(ctr).applyToString(result);
	}

	cout << "\tApplying parser tokens...\n";

	string oldInputContent = "";

	while (result.compare(oldInputContent) != 0)	// Apply all non-accepting parser tokens until the content ceases to be changed by the application
	{
		oldInputContent = result;

		for (int ctr = 0; ctr < numberOfParserTokens; ctr++)	// Iterate through all parser tokens
		{
			//cout << parserTokens->at(ctr).toString() << "\n\n";

			if (parserTokens->at(ctr).tag != config::ACCEPTING_STATE_TAG)	// Consider only non-accepting tokens
			{
				result = parserTokens->at(ctr).applyToString(result);
			}

			//cout << result << "\n\n";
		}
	}

	for (int ctr = 0; ctr < numberOfParserTokens; ctr++)	// Apply each accepting parser token once
	{
		if (parserTokens->at(ctr).tag == config::ACCEPTING_STATE_TAG)
		{
			result = parserTokens->at(ctr).applyToString(result);
		}
	}

	return config::removeWhitespace(result);
}

// Parses the program and predicate input files using the rules defined in the respective rule files
void parse(string lexerPath, string programParserPath, string predicateParserPath, string programInputPath, string predicateInputPath)
{
	vector<token> lexerTokens;
	vector<token> programParserTokens;
	vector<token> predicateParserTokens;

	// Initialize tokens from the input files
	lexerTokens = initializeTokensFromFile(lexerPath);
	programParserTokens = initializeTokensFromFile(programParserPath);
	predicateParserTokens = initializeTokensFromFile(predicateParserPath);

	// Initializes the input contents from the input files and copies them for use in case of a failed parse
	programInputContent = initializeInputFromFile(programInputPath);
	string programInputContentCopy = programInputContent;
	predicateInputContent = initializeInputFromFile(predicateInputPath);
	string predicateInputContentCopy = predicateInputContent;

	// Processes program
	cout << "Processing program...\n";
	programInputContent = processContent(programInputContent, &lexerTokens, &programParserTokens);

	// Checks if the program has an error and outputs the error line
	int programErrorLine = config::errorLine(programInputContent, programInputContentCopy, &lexerTokens, &programParserTokens);
	if (programErrorLine == 0)
	{
		config::throwError("Error parsing the program on an unknown line.");
	}
	else if (programErrorLine > 0)
	{
		config::throwError("Error parsing the program on line " + to_string(programErrorLine));
	}

	// Processes predicates
	cout << "Processing predicates...\n";
	predicateInputContent = processContent(predicateInputContent, &lexerTokens, &predicateParserTokens);

	// Checks if the predicates has an error and outputs the error line
	int predicateErrorLine = config::errorLine(predicateInputContent, predicateInputContentCopy, &lexerTokens, &predicateParserTokens);
	if (predicateErrorLine == 0)
	{
		config::throwError("Error parsing the predicates on an unknown line.");
	}
	else if (predicateErrorLine > 0)
	{
		config::throwError("Error parsing the predicates on line " + to_string(predicateErrorLine));
	}
}