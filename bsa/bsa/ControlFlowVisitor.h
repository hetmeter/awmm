#pragma once

#include <string>
#include <vector>
//#include <iostream>

#include "bufferSizeMap.h"

class Ast;

class ControlFlowVisitor
{
public:

	ControlFlowVisitor();
	~ControlFlowVisitor();

	void traverseControlFlowGraph(Ast* startNode);

private:

	bool isVisitingAlreadyVisitedLabel(Ast* currentNode);
	bufferSizeMap writeBufferSizeMap;
	bufferSizeMap readBufferSizeMap;
	std::vector<std::string> visitedLabels;
	ControlFlowVisitor* clone();
};