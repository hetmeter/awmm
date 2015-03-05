#pragma once
#include <map>
#include <string>


typedef std::map<std::string, int> bufferSizeMap;
typedef bufferSizeMap::iterator bufferSizeMapIterator;
//
//int TOP_VALUE;
//int BOTTOM_VALUE;
//std::string TOP_STRING;
//std::string BOTTOM_STRING;

bool bufferSizeMapContains(bufferSizeMap* container, std::string key);
int bufferSizeValueCompare(int first, int second);
void copyBufferSizes(bufferSizeMap* source, bufferSizeMap* destination);
void incrementCost(std::string varName, int increment, bufferSizeMap* target);
void incrementCostIfExists(std::string varName, int increment, bufferSizeMap* target);
void additiveMergeBufferSizes(bufferSizeMap* source, bufferSizeMap* destination);
void conditionalAdditiveMergeBufferSizes(bufferSizeMap* source, bufferSizeMap* destination);
void setTopIfIncremented(bufferSizeMap* source, bufferSizeMap* destination);
void copyAndSetNonTopToZero(bufferSizeMap* source, bufferSizeMap* destination);