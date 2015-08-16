/*The MIT License(MIT)

Copyright(c) 2015 Attila Zoltán Printz

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

#include "MergeableSetContainer.h"

using namespace std;

/* Public fields */

	const set<int> MergeableSetContainer::getSet(set<string> names)
	{
		set<int> indices;
		set<int> result;

		for (string name : names)
		{
			if (_nameIndexMap.find(name) != _nameIndexMap.end())
			{
				indices.insert(_nameIndexMap[name]);
			}
		}

		for (int index : indices)
		{
			if (_indexSetMap.find(index) != _indexSetMap.end())
			{
				result.insert(_indexSetMap[index].begin(), _indexSetMap[index].end());
			}
		}

		return result;
	}

/* Public methods */

	void MergeableSetContainer::insert(const string &name, int element)
	{
		int index;

		if (_allElements.find(element) == _allElements.end())
		{
			// Element is not in any other set, proceed with inserting it

			if (_nameIndexMap.find(name) == _nameIndexMap.end())
			{
				_nameIndexMap[name] = _indexCtr++;
			}

			index = _nameIndexMap[name];

			if (_indexSetMap.find(index) == _indexSetMap.end())
			{
				_indexSetMap[index] = set<int>();
			}

			_indexSetMap[index].insert(element);
			_allElements.insert(element);
		}
		else
		{
			// Element is already in a set. If it has a different name, merge them.

			index = -1;

			if (_nameIndexMap.find(name) != _nameIndexMap.end())
			{
				index = _nameIndexMap[name];
			}

			if (index == -1 || _indexSetMap[index].find(element) == _indexSetMap[index].end())
			{
				for (map<int, set<int>>::iterator it = _indexSetMap.begin(); it != _indexSetMap.end(); ++it)
				{
					if (it->second.find(element) != it->second.end())
					{
						if (index != -1)
						{
							it->second.insert(_indexSetMap[index].begin(), _indexSetMap[index].end());
							_indexSetMap[index].clear();
							_indexSetMap.erase(index);
							_nameIndexMap[name] = it->first;
						}
					}
				}
			}
		}
	}