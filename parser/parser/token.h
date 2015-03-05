#pragma once

#include <string>
#include <vector>
#include <regex>

struct token
{
	std::string tag;
	std::vector<std::string> rules;

	void initialize(std::string inputTag, std::string inputRule);
	void initialize(std::string inputLine);
	bool isInitialized();
	std::string toString();
	std::string applyToString(std::string s);
};