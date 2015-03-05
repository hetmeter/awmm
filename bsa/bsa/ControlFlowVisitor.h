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

	std::vector<std::string> visitedLabels;
	bufferSizeMap writeBufferSizeMap;
	bufferSizeMap readBufferSizeMap;
	ControlFlowVisitor* clone();
	bool stringVectorContains(std::vector<std::string> container, std::string item);
	bool isVisitingAlreadyVisitedLabel(Ast* currentNode);
};

