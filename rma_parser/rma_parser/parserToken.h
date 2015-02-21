#ifndef __STRING_INCLUDED__
#define __STRING_INCLUDED__
#include <string>
#endif

using namespace std;

struct parserToken
{
	string tag;
	string rule;

	string toString()
	{
		return "Parser token " + tag + " implements " + rule;
	}
};
typedef struct parserToken parserToken;