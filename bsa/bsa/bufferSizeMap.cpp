/*
General purpose methods to use on bufferSizeMap objects
*/

#include "bufferSizeMap.h"
#include "config.h"

using namespace std;

// Return whether the map contains a key
bool bufferSizeMapContains(bufferSizeMap* container, string key)
{
	return !(container->find(key) == container->end());
}

// Compare two integers that may hold values representing TOP (maximum) or BOTTOM (minimum). Return -1 for <, 0 for ==, and 1 for >
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

// Compare two buffer size maps. If they both hold the same entries with the same values, return true
bool bufferSizeMapCompare(bufferSizeMap* first, bufferSizeMap* second)
{
	for (bufferSizeMapIterator iterator = first->begin(); iterator != first->end(); iterator++)
	{
		if (!bufferSizeMapContains(second, iterator->first))
		{
			return false;
		}
		else if (bufferSizeValueCompare(iterator->second, *(second)[iterator->first]) != 0)
		{
			return false;
		}
	}

	for (bufferSizeMapIterator iterator = second->begin(); iterator != second->end(); iterator++)
	{
		if (!bufferSizeMapContains(first, iterator->first))
		{
			return false;
		}
		else if (bufferSizeValueCompare(iterator->second, *(first)[iterator->first]) != 0)
		{
			return false;
		}
	}

	return true;
}

// Copies all members of the source map to the destination map, creating them if they don't exist already, and overwriting them if they do
void copyBufferSizes(bufferSizeMap* source, bufferSizeMap* destination)
{
	for (bufferSizeMapIterator iterator = source->begin(); iterator != source->end(); iterator++)
	{
		(*destination)[iterator->first] = iterator->second;
	}
}

// Adds a set amount to the value of the given key, creating the entry with the increment as its value if it doesn't already exist
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

// Adds a set amount to the value of the given key, but only if it already exists
void incrementCostIfExists(string varName, int increment, bufferSizeMap* target)
{
	if (bufferSizeMapContains(target, varName))
	{
		incrementCost(varName, increment, target);
	}
}

// Increments the values of all entries of the destination by all corresponding values of the source, creating them if they don't exist
void additiveMergeBufferSizes(bufferSizeMap* source, bufferSizeMap* destination)
{
	for (bufferSizeMapIterator iterator = source->begin(); iterator != source->end(); iterator++)
	{
		incrementCost(iterator->first, iterator->second, destination);
	}
}

// Increments the values of all entries of the destination by all corresponding values of the source, but only if the entries exist
void conditionalAdditiveMergeBufferSizes(bufferSizeMap* source, bufferSizeMap* destination)
{
	for (bufferSizeMapIterator iterator = source->begin(); iterator != source->end(); iterator++)
	{
		incrementCostIfExists(iterator->first, iterator->second, destination);
	}
}

// Compares every entry of the source to its counterpart in the destination. If it exists and is lower than the source value, it gets set to TOP
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

// Copies the entries of the source into the destination map, with all values set to 0 unless they're TOP. Creates the entries if they don't exist
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

// Returns a string representation of the map. Its format is: "(key1: value1, key2: value2, ...)"
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