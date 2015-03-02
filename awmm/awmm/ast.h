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

#ifndef __MAP_INCLUDED__
#define __MAP_INCLUDED__
#include <map>
#endif

using namespace std;

const int TOP_VALUE = -1;
const int BOTTOM_VALUE = -2;
const string TOP_STRING = "T";
const string BOTTOM_STRING = "_";

const string ID_TOKEN_NAME = "ID";
const string PROGRAM_DECLARATION_TOKEN_NAME = "programDeclaration";
const string PROCESS_DECLARATION_TOKEN_NAME = "processDeclaration";
const string INITIALIZATION_BLOCK_TOKEN_NAME = "initializationBlock";
const string STATEMENTS_TOKEN_NAME = "statements";
const string STORE_TOKEN_NAME = "store";
const string FENCE_TOKEN_NAME = "fence";
const string IF_ELSE_TOKEN_NAME = "ifElse";
const string WHILE_TOKEN_NAME = "while";
const string NONE_TAG_NAME = "none";

typedef map<string, int> bufferSizeMap;
typedef bufferSizeMap::iterator bufferSizeMapIterator;

struct ast
{
	string name;
	ast* parent = NULL;
	vector<ast*> children;

	bufferSizeMap writeBufferSizeMap;
	bufferSizeMap readBufferSizeMap;

	void addChild(ast* child)
	{
		child->parent = this;
		children.push_back(child);
	}

	void initializeVariableBufferSize(string variableName, bufferSizeMap* target)
	{
		if (target->find(variableName) == target->end())
		{
			(*target)[variableName] = 0;
		}
	}

	void resetVariableBufferSize(string variableName, bufferSizeMap* target)
	{
		(*target)[variableName] = 0;
	}

	void incrementBufferSize(string variableName, bufferSizeMap* target)
	{
		if (target->find(variableName) == target->end())
		{
			(*target)[variableName] = 1;
		}
		else
		{
			(*target)[variableName]++;
		}

		if (name != PROGRAM_DECLARATION_TOKEN_NAME)
		{
			bufferSizeMap* parentTarget = (*target) == writeBufferSizeMap ? &(parent->writeBufferSizeMap) : &(parent->readBufferSizeMap);
			parent->incrementBufferSize(variableName, parentTarget);
		}
	}

	void incrementBufferSizeIfExists(string variableName, bufferSizeMap* target)
	{
		if (target->find(variableName) != target->end())
		{
			(*target)[variableName]++;

			if (name != PROGRAM_DECLARATION_TOKEN_NAME)
			{
				bufferSizeMap* parentTarget = (*target) == writeBufferSizeMap ? &(parent->writeBufferSizeMap) : &(parent->readBufferSizeMap);
				parent->incrementBufferSizeIfExists(variableName, parentTarget);
			}
		}
	}

	void setBufferSizeToTop(string variableName, bufferSizeMap* target)
	{
		(*target)[variableName] = TOP_VALUE;
	}

	void setBufferSizeToBottom(string variableName, bufferSizeMap* target)
	{
		(*target)[variableName] = BOTTOM_VALUE;
	}

	void topDownCascadingBufferSizeMapUnion(bufferSizeMap source)
	{
		for (bufferSizeMapIterator iterator = source.begin(); iterator != source.end(); iterator++)
		{
			initializeVariableBufferSize(iterator->first, &readBufferSizeMap);
			initializeVariableBufferSize(iterator->first, &writeBufferSizeMap);
		}

		for (ast* child : children)
		{
			child->topDownCascadingBufferSizeMapUnion(source);
		}
	}

	void topDownBufferSizeInitialization()
	{
		if (children.size() == 0)
		{
			bottomUpBufferSizeInitialization();
		}
		else
		{
			for (ast* child : children)
			{
				child->topDownBufferSizeInitialization();
			}
		}
	}

	void topDownBufferSizeEvaluation()
	{
		if (children.size() == 0)
		{
			bottomUpBufferSizeEvaluation();
		}
		else
		{
			for (ast* child : children)
			{
				//cout << "checkpoint 1 " << name << "\n";
				child->topDownBufferSizeEvaluation();
			}
		}
	}

	void bottomUpBufferSizeInitialization()
	{
		if (parent->name == ID_TOKEN_NAME)
		{
			if (parent->parent->name == STORE_TOKEN_NAME && parent->parent->children.at(0) == parent)
			{
				incrementBufferSize(name, &(parent->writeBufferSizeMap));
			}
			else
			{
				incrementBufferSize(name, &(parent->readBufferSizeMap));
			}
		}
	}

	void bottomUpBufferSizeEvaluation()
	{
		if (parent->name == ID_TOKEN_NAME)
		{
			if (parent->parent->name == STORE_TOKEN_NAME && parent->parent->children.at(0) == parent)
			{
				incrementBufferSizeIfExists(name, &(parent->writeBufferSizeMap));
			}
			else
			{
				incrementBufferSizeIfExists(name, &(parent->readBufferSizeMap));
			}
		}
	}

	void incrementBufferSizeByN(bufferSizeMap* targetMap, string variableName, int n)
	{
		if (targetMap->find(variableName) != targetMap->end())
		{
			if ((*targetMap)[variableName] == TOP_VALUE || n == TOP_VALUE)
			{
				(*targetMap)[variableName] = TOP_VALUE;
			}
			else if ((*targetMap)[variableName] == BOTTOM_VALUE)
			{
				(*targetMap)[variableName] = n;
			}
			else if (n == BOTTOM_VALUE)
			{
				// do nothing
			}
			else
			{
				(*targetMap)[variableName] += n;
			}
		}
	}

	void additiveMergeBufferSizes(ast* precedingNode)
	{
		if (name != FENCE_TOKEN_NAME)
		{
			for (bufferSizeMapIterator iterator = precedingNode->writeBufferSizeMap.begin(); iterator != precedingNode->writeBufferSizeMap.end(); iterator++)
			{
				incrementBufferSizeByN(&writeBufferSizeMap, iterator->first, iterator->second);
			}

			for (bufferSizeMapIterator iterator = precedingNode->readBufferSizeMap.begin(); iterator != precedingNode->readBufferSizeMap.end(); iterator++)
			{
				incrementBufferSizeByN(&readBufferSizeMap, iterator->first, iterator->second);
			}
		}
	}

	void setBufferSizeToMaximum(bufferSizeMap* targetMap, string variableName, int n)
	{
		if (targetMap->find(variableName) != targetMap->end())
		{
			if ((*targetMap)[variableName] == TOP_VALUE || n == TOP_VALUE)
			{
				(*targetMap)[variableName] = TOP_VALUE;
			}
			else if ((*targetMap)[variableName] == BOTTOM_VALUE)
			{
				(*targetMap)[variableName] = n;
			}
			else if (n == BOTTOM_VALUE)
			{
				// do nothing
			}
			else
			{
				(*targetMap)[variableName] = n > (*targetMap)[variableName] ? n : (*targetMap)[variableName];
			}
		}
	}

	void maximalMergeBufferSizes(ast* precedingNode)
	{
		if (name != FENCE_TOKEN_NAME)
		{
			for (bufferSizeMapIterator iterator = precedingNode->writeBufferSizeMap.begin(); iterator != precedingNode->writeBufferSizeMap.end(); iterator++)
			{
				setBufferSizeToMaximum(&writeBufferSizeMap, iterator->first, iterator->second);
			}

			for (bufferSizeMapIterator iterator = precedingNode->readBufferSizeMap.begin(); iterator != precedingNode->readBufferSizeMap.end(); iterator++)
			{
				setBufferSizeToMaximum(&readBufferSizeMap, iterator->first, iterator->second);
			}
		}
	}

	void setNonZeroToTop()
	{
		for (bufferSizeMapIterator iterator = writeBufferSizeMap.begin(); iterator != writeBufferSizeMap.end(); iterator++)
		{
			if (iterator->second > 0)
			{
				writeBufferSizeMap[iterator->first] = TOP_VALUE;
			}
		}

		for (bufferSizeMapIterator iterator = readBufferSizeMap.begin(); iterator != readBufferSizeMap.end(); iterator++)
		{
			if (iterator->second > 0)
			{
				readBufferSizeMap[iterator->first] = TOP_VALUE;
			}
		}
	}

	void copyBufferSizes(ast* source)
	{
		for (bufferSizeMapIterator iterator = source->writeBufferSizeMap.begin(); iterator != source->writeBufferSizeMap.end(); iterator++)
		{
			if (writeBufferSizeMap.find(iterator->first) != writeBufferSizeMap.end())
			{
				writeBufferSizeMap[iterator->first] = iterator->second;
			}
		}

		for (bufferSizeMapIterator iterator = source->readBufferSizeMap.begin(); iterator != source->readBufferSizeMap.end(); iterator++)
		{
			if (readBufferSizeMap.find(iterator->first) != readBufferSizeMap.end())
			{
				readBufferSizeMap[iterator->first] = iterator->second;
			}
		}
	}

	void linearlyIterateThroughControlFlow()
	{
		if (name == PROGRAM_DECLARATION_TOKEN_NAME)
		{
			int childrenCount = children.size();

			// Initialize global variables
			children.at(0)->topDownBufferSizeInitialization();

			for (int ctr = 1; ctr < childrenCount; ctr++)
			{
				children.at(ctr)->topDownCascadingBufferSizeMapUnion(children.at(0)->writeBufferSizeMap);
				children.at(ctr)->topDownBufferSizeEvaluation();
				children.at(ctr)->linearlyIterateThroughControlFlow();
			}
		}
		else if (name == PROCESS_DECLARATION_TOKEN_NAME)
		{
			children.at(1)->linearlyIterateThroughControlFlow();
			copyBufferSizes(children.at(1));
		}
		else if (name == STATEMENTS_TOKEN_NAME)
		{
			int childrenCount = children.size();
			ast* precursor;

			for (int ctr = 1; ctr < childrenCount; ctr++)
			{
				precursor = children.at(ctr - 1);

				if (children.at(ctr)->name == IF_ELSE_TOKEN_NAME)
				{
					evaluateIfElseBufferSizes(children.at(ctr));
				}
				else if (children.at(ctr)->name == WHILE_TOKEN_NAME)
				{
					evaluateWhileBufferSizes(children.at(ctr));
				}

				children.at(ctr)->additiveMergeBufferSizes(precursor);
			}

			copyBufferSizes(children.at(childrenCount - 1));
		}
	}

	void evaluateIfElseBufferSizes(ast* ifElseNode)
	{
		if (ifElseNode->name == IF_ELSE_TOKEN_NAME)
		{
			ast* conditionalNode = ifElseNode->children.at(0);
			ifElseNode->children.at(1)->linearlyIterateThroughControlFlow();
			ifElseNode->children.at(1)->additiveMergeBufferSizes(conditionalNode);
			ifElseNode->copyBufferSizes(ifElseNode->children.at(1));

			if (ifElseNode->children.at(2)->name != NONE_TAG_NAME)
			{
				ifElseNode->children.at(2)->linearlyIterateThroughControlFlow();
				ifElseNode->children.at(2)->additiveMergeBufferSizes(conditionalNode);
				ifElseNode->maximalMergeBufferSizes(ifElseNode->children.at(2));
			}
		}
	}

	void evaluateWhileBufferSizes(ast* whileNode)
	{
		if (whileNode->name == WHILE_TOKEN_NAME)
		{
			whileNode->children.at(1)->linearlyIterateThroughControlFlow();
			whileNode->children.at(1)->additiveMergeBufferSizes(whileNode->children.at(0));
			whileNode->copyBufferSizes(whileNode->children.at(1));
			whileNode->setNonZeroToTop();
		}
	}

	string bufferSizeMapString()
	{
		string result = "read: (";
		string currentValue;
		int bufferSizeMapCount = readBufferSizeMap.size();
		
		for (bufferSizeMapIterator iterator = readBufferSizeMap.begin(); iterator != readBufferSizeMap.end(); iterator++)
		{
			if (iterator->second == TOP_VALUE)
			{
				currentValue = TOP_STRING;
			}
			else if (iterator->second == BOTTOM_VALUE)
			{
				currentValue = BOTTOM_STRING;
			}
			else
			{
				currentValue = to_string(iterator->second);
			}

			result += iterator->first + ": " + currentValue + ", ";
		}

		if (result.length() > 2)
		{
			result = result.substr(0, result.length() - 2) + "), write: (";
		}

		for (bufferSizeMapIterator iterator = writeBufferSizeMap.begin(); iterator != writeBufferSizeMap.end(); iterator++)
		{
			if (iterator->second == TOP_VALUE)
			{
				currentValue = TOP_STRING;
			}
			else if (iterator->second == BOTTOM_VALUE)
			{
				currentValue = BOTTOM_STRING;
			}
			else
			{
				currentValue = to_string(iterator->second);
			}

			result += iterator->first + ": " + currentValue + ", ";
		}

		if (result.length() > 2)
		{
			result = result.substr(0, result.length() - 2) + ")";
		}

		return result;
	}

	string toString()
	{
		regex indentationRegex("\n");
		string result = name;

		//if (name != PROGRAM_DECLARATION_TOKEN_NAME && (name == STATEMENTS_TOKEN_NAME || name == INITIALIZATION_BLOCK_TOKEN_NAME || parent->name == IF_ELSE_TOKEN_NAME || parent->name == WHILE_TOKEN_NAME || parent->name == STATEMENTS_TOKEN_NAME))
		if (name != PROGRAM_DECLARATION_TOKEN_NAME && (parent->name == STATEMENTS_TOKEN_NAME || parent->name == IF_ELSE_TOKEN_NAME || parent->name == WHILE_TOKEN_NAME))
		{
			result += "\t\t" + bufferSizeMapString();
		}
		
		int childrenCount = children.size();

		for (int ctr = 0; ctr < childrenCount; ctr++)
		{
			result += "\n" + children.at(ctr)->toString();
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