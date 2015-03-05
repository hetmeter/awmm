#pragma once

#include <vector>
#include <iostream>

#include "bufferSizeMap.h"
class Ast;

class ControlFlowVisitor
{
public:

	ControlFlowVisitor();
	~ControlFlowVisitor();

	void traverseControlFlowGraph(Ast* startNode);

private:

	std::vector<int> visitedLabels;
	bufferSizeMap writeBufferSizeMap;
	bufferSizeMap readBufferSizeMap;
	ControlFlowVisitor* clone();
	bool intVectorContains(std::vector<int> container, int item);
	bool isVisitingAlreadyVisitedLabel(Ast* currentNode);
};

