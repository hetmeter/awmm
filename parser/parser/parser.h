#pragma once

struct token;

#include <string>
#include <vector>
#include <fstream>
#include <sstream>

extern std::string programInputContent;
extern std::string predicateInputContent;

std::vector<token> initializeTokensFromFile(std::string path);
std::string initializeInputFromFile(std::string path);
std::string processContent(std::string content, std::vector<token>* lexerTokens, std::vector<token>* parserTokens);
void parse(std::string lexerPath, std::string programParserPath, std::string predicateParserPath, std::string programInputPath, std::string predicateInputPath);