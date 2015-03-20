#pragma once

#include <map>
#include <string>
#include <vector>


typedef std::map<std::string, int> bufferSizeMap;
typedef bufferSizeMap::iterator bufferSizeMapIterator;

bool bufferSizeMapContains(bufferSizeMap* container, std::string key);
int bufferSizeValueCompare(int first, int second);
bool bufferSizeMapCompare(bufferSizeMap* first, bufferSizeMap* second);
std::vector<std::string> getAllKeys(bufferSizeMap* source);
void copyBufferSizes(bufferSizeMap* source, bufferSizeMap* destination);
void incrementCost(std::string varName, int increment, bufferSizeMap* target);
void incrementCostIfExists(std::string varName, int increment, bufferSizeMap* target);
void additiveMergeBufferSizes(bufferSizeMap* source, bufferSizeMap* destination);
void conditionalAdditiveMergeBufferSizes(bufferSizeMap* source, bufferSizeMap* destination);
void setTopIfIncremented(bufferSizeMap* source, bufferSizeMap* destination);
void copyAndSetNonTopToZero(bufferSizeMap* source, bufferSizeMap* destination);
std::string toString(bufferSizeMap* source);