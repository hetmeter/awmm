/*The MIT License(MIT)

Copyright(c) 2015 Attila Zolt�n Printz

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/*
General purpose methods to use on bufferSizeMap objects
*/

#include "bufferSizeMap.h"
#include "literalCode.h"
//#include "config.h"

namespace bsm
{
/* Constants */

	const int TOP_VALUE = -1;
	const int BOTTOM_VALUE = -2;
	const int UNDEFINED_VALUE = -3;

	const std::string TOP_STRING = "T";
	const std::string BOTTOM_STRING = "_";

/* Methods */

	// Adds a set amount to the value of the given key, creating the entry with the increment as its value if it doesn't already exist
	void incrementCost(const std::string &varName, int increment, bufferSizeMap* target)
	{
		if (!bufferSizeMapContains(target, varName) || (*target)[varName] == BOTTOM_VALUE)
		{
			(*target)[varName] = increment;
		}
		else if ((*target)[varName] != TOP_VALUE)
		{
			if (increment == TOP_VALUE)
			{
				(*target)[varName] = TOP_VALUE;
			}
			else if (increment != BOTTOM_VALUE)
			{
				(*target)[varName] += increment;
			}
		}
	}
	
	// Adds a set amount to the value of the given key, but only if it already exists
	void incrementCostIfExists(const std::string &varName, int increment, bufferSizeMap* target)
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

	// Compares every entry of the source to its counterpart in the destination. If it exists and is lower than the source value, it gets set to TOP
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

	// Copies all members of the source map to the destination map, creating them if they don't exist already, and overwriting them if they do
	void copyBufferSizes(bufferSizeMap* source, bufferSizeMap* destination)
	{
		for (bufferSizeMapIterator iterator = source->begin(); iterator != source->end(); iterator++)
		{
			if (destination->find(iterator->first) != destination->end())
			{
				destination->at(iterator->first) = iterator->second;
			}
			else
			{
				destination->insert(std::pair<std::string, int>(iterator->first, iterator->second));
			}

			//(*destination)[iterator->first] = iterator->second;
		}
	}

	std::vector<std::string> getAllKeys(bufferSizeMap* source)
	{
		std::vector<std::string> result;

		for (bufferSizeMapIterator iterator = source->begin(); iterator != source->end(); iterator++)
		{
			result.push_back(iterator->first);
		}

		return result;
	}
	
	// Increments the values of all entries of the destination by all corresponding values of the source, but only if the entries exist
	void conditionalAdditiveMergeBufferSizes(bufferSizeMap* source, bufferSizeMap* destination)
	{
		for (bufferSizeMapIterator iterator = source->begin(); iterator != source->end(); iterator++)
		{
			incrementCostIfExists(iterator->first, iterator->second, destination);
		}
	}
	
	// Copies the entries of the source into the destination map, with all values set to 0 unless they're TOP. Creates the entries if they don't exist
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
	
	// Returns a string representation of the map. Its format is: "(key1: value1, key2: value2, ...)"
	std::string toString(bufferSizeMap* source)
	{
		std::string result = "";
		std::string currentValue;
	
		for (bufferSizeMapIterator iterator = source->begin(); iterator != source->end(); iterator++)
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
				currentValue = std::to_string(iterator->second);
			}
	
			result += iterator->first + ": " + currentValue + ", ";
		}
	
		if (((int)result.length()) > 0)
		{
			result = result.substr(0, result.length() - 2);
		}
	
		return "(" + result + ")";
	}

/* Map methods */

	// Return whether the map contains a key
	bool bufferSizeMapContains(bufferSizeMap* container, const std::string &key)
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
	
	// Compare two buffer size maps. If they both hold the same entries with the same values, return true
	bool bufferSizeMapCompare(bufferSizeMap* first, bufferSizeMap* second)
	{
		for (bufferSizeMapIterator iterator = first->begin(); iterator != first->end(); iterator++)
		{
			if (!bufferSizeMapContains(second, iterator->first))
			{
				return false;
			}
			else if (bufferSizeValueCompare(iterator->second, (*(second))[iterator->first]) != 0)
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
			else if (bufferSizeValueCompare(iterator->second, (*(first))[iterator->first]) != 0)
			{
				return false;
			}
		}
		
		return true;
	}
}