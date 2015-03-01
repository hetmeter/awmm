#ifndef __STRING_INCLUDED__
#define __STRING_INCLUDED__
#include <string>
#endif

#ifndef __VECTOR_INCLUDED__
#define __VECTOR_INCLUDED__
#include <vector>
#endif

#ifndef __REGEX_INCLUDED__
#define __REGEX_INCLUDED__
#include <regex>
#endif

using namespace std;

struct ast
{
	string name;
	ast* parent;
	vector<ast> children;

	void addChild(ast* child)
	{
		(*child).parent = this;
		children.push_back(*child);
	}

	string toString()
	{
		regex indentationRegex("\n");
		string result = name;
		int childrenCount = children.size();

		for (int ctr = 0; ctr < childrenCount; ctr++)
		{
			result += "\n" + children.at(ctr).toString();
		}

		result = regex_replace(result, indentationRegex, "\n|");

		/*string result = name + ", children:";
		int childrenCount = children.size();

		for (int ctr = 0; ctr < childrenCount; ctr++)
		{
			result += " " + children.at(ctr).name;
		}

		result += " {\n";

		for (int ctr = 0; ctr < childrenCount; ctr++)
		{
			result += "\n" + children.at(ctr).toString();
		}

		result += "}";*/

		return result;
	}
};
typedef struct ast ast;