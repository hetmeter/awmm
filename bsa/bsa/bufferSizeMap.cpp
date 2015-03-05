#include "bufferSizeMap.h"
#include "config.h"

using namespace std;

bool bufferSizeMapContains(bufferSizeMap* container, string key)
{
	return !(container->find(key) == container->end());
}

int bufferSizeValueCompare(int first, int second)
{
	if (first == second)
	{
		return 0;
	}
	else if (first == config::BOTTOM_VALUE || second == config::TOP_VALUE)
	{
		return -1;
	}
	else if (second == config::BOTTOM_VALUE || first == config::TOP_VALUE)
	{
		return 1;
	}
	else
	{
		if (first < second)
		{
			return -1;
		}
		else
		{
			return 1;
		}
	}
}

void copyBufferSizes(bufferSizeMap* source, bufferSizeMap* destination)
{
	for (bufferSizeMapIterator iterator = source->begin(); iterator != source->end(); iterator++)
	{
		(*destination)[iterator->first] = iterator->second;
	}
}

void incrementCost(string varName, int increment, bufferSizeMap* target)
{
	if (!bufferSizeMapContains(target, varName) || (*target)[varName] == config::BOTTOM_VALUE)
	{
		(*target)[varName] = increment;
	}
	else if ((*target)[varName] != config::TOP_VALUE)
	{
		(*target)[varName] += increment;
	}
}

void incrementCostIfExists(string varName, int increment, bufferSizeMap* target)
{
	if (bufferSizeMapContains(target, varName))
	{
		incrementCost(varName, increment, target);
	}
}

void additiveMergeBufferSizes(bufferSizeMap* source, bufferSizeMap* destination)
{
	for (bufferSizeMapIterator iterator = source->begin(); iterator != source->end(); iterator++)
	{
		incrementCost(iterator->first, iterator->second, destination);
	}
}

void conditionalAdditiveMergeBufferSizes(bufferSizeMap* source, bufferSizeMap* destination)
{
	for (bufferSizeMapIterator iterator = source->begin(); iterator != source->end(); iterator++)
	{
		incrementCostIfExists(iterator->first, iterator->second, destination);
	}
}

void setTopIfIncremented(bufferSizeMap* source, bufferSizeMap* destination)
{
	for (bufferSizeMapIterator iterator = source->begin(); iterator != source->end(); iterator++)
	{
		if (bufferSizeMapContains(destination, iterator->first) && bufferSizeValueCompare((*destination)[iterator->first], iterator->second) == -1)
		{
			(*destination)[iterator->first] = config::TOP_VALUE;
		}
	}
}

void copyAndSetNonTopToZero(bufferSizeMap* source, bufferSizeMap* destination)
{
	copyBufferSizes(source, destination);

	for (bufferSizeMapIterator iterator = destination->begin(); iterator != destination->end(); iterator++)
	{
		if ((*destination)[iterator->first] != config::TOP_VALUE)
		{
			(*destination)[iterator->first] = 0;
		}
	}
}

string toString(bufferSizeMap* source)
{
	string result = "";
	string currentValue;

	for (bufferSizeMapIterator iterator = source->begin(); iterator != source->end(); iterator++)
	{
		if (iterator->second == config::TOP_VALUE)
		{
			currentValue = config::TOP_STRING;
		}
		else if (iterator->second == config::BOTTOM_VALUE)
		{
			currentValue = config::BOTTOM_STRING;
		}
		else
		{
			currentValue = to_string(iterator->second);
		}

		result += iterator->first + ": " + currentValue + ", ";
	}

	if (((int)result.length()) > 0)
	{
		result = result.substr(0, result.length() - 2);
	}

	return "(" + result + ")";
}