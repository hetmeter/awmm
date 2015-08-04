#pragma once

#include <map>
#include <set>
#include <string>

class MergeableSetContainer
{
public:

/* Public fields */
	const std::set<int> getSet(std::set<std::string> names);

/* Public methods */
	void insert(const std::string &name, int element);

private:

/* Locals */
	int _indexCtr = 0;
	std::map<std::string, int> _nameIndexMap;
	std::map<int, std::set<int>> _indexSetMap;
	std::set<int> _allElements;
};