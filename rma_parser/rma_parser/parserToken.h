#ifndef __STRING_INCLUDED__
#define __STRING_INCLUDED__
#include <string>
#endif

using namespace std;

struct parserToken
{
	string description;
	string rule;

	string ToString()
	{
		return "Parser token " + description + " implements " + rule;
	}
};
typedef struct parserToken parserToken;