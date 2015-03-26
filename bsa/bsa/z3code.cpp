/*
Methods for generating boolean program code for Z3
*/

#include "z3code.h"

namespace z3code
{
	const std::string PUSH_PREFIX = "(push ";
	const std::string GENERAL_SUFFIX = ")";

	std::string push(int number)
	{
		return PUSH_PREFIX + std::to_string(number) + GENERAL_SUFFIX;
	}
}