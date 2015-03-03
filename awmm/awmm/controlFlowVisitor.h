#ifndef __AST_H_INCLUDED__
#define __AST_H_INCLUDED__
#include "ast.h"
#endif

using namespace std;

struct controlFlowVisitor
{
	vector<int> visitedLabels;

	bufferSizeMap writeBufferSizeMap;
	bufferSizeMap readBufferSizeMap;

	controlFlowVisitor* clone()
	{
		controlFlowVisitor* result = new controlFlowVisitor;

		for (int visitedLabel : visitedLabels)
		{
			result->visitedLabels.push_back(visitedLabel);
		}

		result->copyBufferSizes(&writeBufferSizeMap, &readBufferSizeMap);

		return result;
	}

	void copyBufferSizes(bufferSizeMap* writeSource, bufferSizeMap* readSource)
	{
		for (bufferSizeMapIterator iterator = writeSource->begin(); iterator != writeSource->end(); iterator++)
		{
			writeBufferSizeMap[iterator->first] = iterator->second;
		}

		for (bufferSizeMapIterator iterator = readSource->begin(); iterator != readSource->end(); iterator++)
		{
			readBufferSizeMap[iterator->first] = iterator->second;
		}
	}

	void copyBufferSizes(ast* source)
	{
		copyBufferSizes(&(source->writeBufferSizeMap), &(source->readBufferSizeMap));
	}

	void resetBufferSizes()
	{
		for (bufferSizeMapIterator iterator = writeBufferSizeMap.begin(); iterator != writeBufferSizeMap.end(); iterator++)
		{
			writeBufferSizeMap[iterator->first] = 0;
		}

		for (bufferSizeMapIterator iterator = readBufferSizeMap.begin(); iterator != readBufferSizeMap.end(); iterator++)
		{
			readBufferSizeMap[iterator->first] = 0;
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

	void additiveMergeBufferSizes(ast* node)
	{
		cout << "\t\t" + bufferSizeMapString() + "\n";

		if (node->name != FENCE_TOKEN_NAME)
		{
			for (bufferSizeMapIterator iterator = node->writeBufferSizeMap.begin(); iterator != node->writeBufferSizeMap.end(); iterator++)
			{
				incrementBufferSizeByN(&writeBufferSizeMap, iterator->first, iterator->second);
			}

			for (bufferSizeMapIterator iterator = node->readBufferSizeMap.begin(); iterator != node->readBufferSizeMap.end(); iterator++)
			{
				incrementBufferSizeByN(&readBufferSizeMap, iterator->first, iterator->second);
			}
		}
		else
		{
			resetBufferSizes();
		}

		cout << "\t\t" + bufferSizeMapString() + "\n";
	}

	bool contains(vector<int> container, int item)
	{
		int count = container.size();

		for (int ctr = 0; ctr < count; ctr++)
		{
			if (item == container.at(ctr))
			{
				return true;
			}
		}

		return false;
	}

	void traverseControlFlowGraph(ast* startNode)
	{
		cout << "\t\tTraversing " << startNode->name << "\n";

		if (writeBufferSizeMap.empty() || readBufferSizeMap.empty())
		{
			copyBufferSizes(startNode);
		}

		if (startNode->name == LABEL_TOKEN_NAME)
		{
			int labelValue = stoi(startNode->children.at(0)->name);

			if (contains(visitedLabels, labelValue))
			{
				for (bufferSizeMapIterator iterator = writeBufferSizeMap.begin(); iterator != writeBufferSizeMap.end(); iterator++)
				{
					if (iterator->second == TOP_VALUE || startNode->writeBufferSizeMap[iterator->first] < iterator->second)
					{
						startNode->writeBufferSizeMap[iterator->first] = TOP_VALUE;
					}
				}

				for (bufferSizeMapIterator iterator = readBufferSizeMap.begin(); iterator != readBufferSizeMap.end(); iterator++)
				{
					if (iterator->second == TOP_VALUE || startNode->readBufferSizeMap[iterator->first] < iterator->second)
					{
						startNode->readBufferSizeMap[iterator->first] = TOP_VALUE;
					}
				}

				startNode->propagateTopValues();
			}
			else
			{
				visitedLabels.push_back(labelValue);
				traverseControlFlowGraph(startNode->outgoingEdges.at(0)->end);
			}
		}
		else
		{
			additiveMergeBufferSizes(startNode);

			int outgoingEdgeCount = startNode->outgoingEdges.size();

			if (outgoingEdgeCount > 0)
			{
				traverseControlFlowGraph(startNode->outgoingEdges.at(0)->end);

				if (outgoingEdgeCount > 1)
				{
					controlFlowVisitor* newSpawn;

					for (int ctr = 1; ctr < outgoingEdgeCount; ctr++)
					{
						newSpawn = clone();
						newSpawn->traverseControlFlowGraph(startNode->outgoingEdges.at(ctr)->end);
					}
				}
			}
		}
	}
};
typedef struct controlFlowVisitor controlFlowVisitor;