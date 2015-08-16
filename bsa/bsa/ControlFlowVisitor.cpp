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

#include "ControlFlowVisitor.h"

#include "ControlFlowEdge.h"
#include "Ast.h"
#include "literalCode.h"
#include "config.h"

using namespace std;

ControlFlowVisitor::ControlFlowVisitor()
{
}


ControlFlowVisitor::~ControlFlowVisitor()
{
}

// Performs one visit to the target node. Updates the node's persistent buffer size map, continues along the control flow path,
// or spawns copies of itself, according to the state of the visit
void ControlFlowVisitor::traverseControlFlowGraph(Ast* startNode)
{
	if (isVisitingAlreadyVisitedLabel(startNode))	// If re-visiting this node, set the local persistent buffer sizes to TOP, if they'd been increased
	{												// on the traversed path. Cascade these changes along all successive control flow paths.

		// Add the visitor's accumulated buffer sizes to the persistent buffer sizes of the node
		bsm::additiveMergeBufferSizes(&(startNode->causedWriteCost), &writeBufferSizeMap);
		bsm::additiveMergeBufferSizes(&(startNode->causedReadCost), &readBufferSizeMap);

		bsm::setTopIfIncremented(&writeBufferSizeMap, &(startNode->persistentWriteCost));
		bsm::setTopIfIncremented(&readBufferSizeMap, &(startNode->persistentReadCost));
		startNode->controlFlowDirectionCascadingPropagateTops();
	}
	else
	{
		if (startNode->getName() == literalCode::LABEL_TOKEN_NAME)	// If visiting a label for the first time, add it to the list of visited labels
		{
			visitedLabels.insert(stoi(startNode->getChild(0)->getName()));
		}

		// Add the visitor's accumulated buffer sizes to the persistent buffer sizes of the node
		bsm::additiveMergeBufferSizes(&writeBufferSizeMap, &(startNode->persistentWriteCost));
		bsm::additiveMergeBufferSizes(&readBufferSizeMap, &(startNode->persistentReadCost));

		// Copy the node's updated buffer sizes to the visitor's maps
		bsm::copyBufferSizes(&(startNode->persistentWriteCost), &writeBufferSizeMap);
		bsm::copyBufferSizes(&(startNode->persistentReadCost), &readBufferSizeMap);

		if (startNode->getName() == literalCode::FENCE_TOKEN_NAME)	// If visiting a fence, reset all buffer sizes to 0
		{
			for (bufferSizeMapIterator iterator = writeBufferSizeMap.begin(); iterator != writeBufferSizeMap.end(); iterator++)
			{
				writeBufferSizeMap[iterator->first] = 0;
			}

			for (bufferSizeMapIterator iterator = readBufferSizeMap.begin(); iterator != readBufferSizeMap.end(); iterator++)
			{
				readBufferSizeMap[iterator->first] = 0;
			}
		}

		// Continue visiting the nodes on all outgoing control flow paths, if they exist
		int outgoingEdgeCount = startNode->outgoingEdges.size();

		if (outgoingEdgeCount > 0)
		{
			traverseControlFlowGraph(startNode->outgoingEdges.at(0)->end);	// The current visitor takes the first path

			ControlFlowVisitor* newControlFlowVisitor;

			for (int ctr = 1; ctr < outgoingEdgeCount; ctr++)	// If there are multiple paths available, spawn a new visitor for each further one
			{
				newControlFlowVisitor = clone();
				newControlFlowVisitor->traverseControlFlowGraph(startNode->outgoingEdges.at(ctr)->end);
			}
		}
	}
}

// Returns whether the currently visited node has already been visited by this visitor or one of its ancestors
bool ControlFlowVisitor::isVisitingAlreadyVisitedLabel(Ast* currentNode)
{
	if (currentNode->getName() == literalCode::LABEL_TOKEN_NAME)	// Since the only possible closing points of cycles are labels, disregard any other node type
	{
		if (visitedLabels.find(stoi(currentNode->getChild(0)->getName())) != visitedLabels.end())
		{
			return true;
		}
	}

	return false;
}

// Generates a copy of the current node, with its visited label vector and buffer size maps having the same content
ControlFlowVisitor* ControlFlowVisitor::clone()
{
	ControlFlowVisitor * result = new ControlFlowVisitor;

	// Copy the visited label vector content
	for (int visitedLabel : visitedLabels)
	{
		result->visitedLabels.insert(visitedLabel);
	}

	// Copy the contents of both buffer size maps
	bsm::copyBufferSizes(&writeBufferSizeMap, &(result->writeBufferSizeMap));
	bsm::copyBufferSizes(&readBufferSizeMap, &(result->readBufferSizeMap));

	return result;
}