#include "bufferSize.h"

using namespace std;

const int TOP_VALUE = -1;
const int BOTTOM_VALUE = -2;
const string TOP_STRING = "T";
const string BOTTOM_STRING = "_";

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
	else if (first == BOTTOM_VALUE || second == TOP_VALUE)
	{
		return -1;
	}
	else if (second == BOTTOM_VALUE || first == TOP_VALUE)
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
	if (!bufferSizeMapContains(target, varName) || (*target)[varName] == BOTTOM_VALUE)
	{
		(*target)[varName] = increment;
	}
	else if ((*target)[varName] != TOP_VALUE)
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
			(*destination)[iterator->first] = TOP_VALUE;
		}
	}
}

void copyAndSetNonTopToZero(bufferSizeMap* source, bufferSizeMap* destination)
{
	copyBufferSizes(source, destination);

	for (bufferSizeMapIterator iterator = destination->begin(); iterator != destination->end(); iterator++)
	{
		if ((*destination)[iterator->first] != TOP_VALUE)
		{
			(*destination)[iterator->first] = 0;
		}
	}
}