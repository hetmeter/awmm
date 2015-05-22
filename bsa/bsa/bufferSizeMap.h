#pragma once

#include <map>
//#include <string>
#include <vector>


typedef std::map<std::string, int> bufferSizeMap;
typedef bufferSizeMap::iterator bufferSizeMapIterator;


namespace bsm
{
/* Constants */
	const extern int TOP_VALUE;
	const extern int BOTTOM_VALUE;
	const extern int UNDEFINED_VALUE;

/* General methods */
	extern void incrementCost(const std::string &varName, int increment, bufferSizeMap* target);
	void incrementCostIfExists(const std::string &varName, int increment, bufferSizeMap* target);
	extern void additiveMergeBufferSizes(bufferSizeMap* source, bufferSizeMap* destination);
	extern void setTopIfIncremented(bufferSizeMap* source, bufferSizeMap* destination);
	extern void copyBufferSizes(bufferSizeMap* source, bufferSizeMap* destination);
	extern std::vector<std::string> getAllKeys(bufferSizeMap* source);
	void conditionalAdditiveMergeBufferSizes(bufferSizeMap* source, bufferSizeMap* destination);
	void copyAndSetNonTopToZero(bufferSizeMap* source, bufferSizeMap* destination);
	std::string toString(bufferSizeMap* source);

/* Map methods */
	bool bufferSizeMapContains(bufferSizeMap* container, const std::string &key);
	int bufferSizeValueCompare(int first, int second);
	bool bufferSizeMapCompare(bufferSizeMap* first, bufferSizeMap* second);
}