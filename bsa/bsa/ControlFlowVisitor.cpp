#include "ControlFlowVisitor.h"

#include "ControlFlowEdge.h"
#include "Ast.h"
#include "config.h"

using namespace std;

ControlFlowVisitor::ControlFlowVisitor()
{
}


ControlFlowVisitor::~ControlFlowVisitor()
{
}

// Generates a copy of the current node, with its visited label vector and buffer size maps having the same content
ControlFlowVisitor* ControlFlowVisitor::clone()
{
	ControlFlowVisitor * result = new ControlFlowVisitor;

	// Copy the visited label vector content
	for (string visitedLabel : visitedLabels)
	{
		result->visitedLabels.push_back(visitedLabel);
	}

	// Copy the contents of both buffer size maps
	copyBufferSizes(&writeBufferSizeMap, &(result->writeBufferSizeMap));
	copyBufferSizes(&readBufferSizeMap, &(result->readBufferSizeMap));

	return result;
}

// Returns whether the currently visited node has already been visited by this visitor or one of its ancestors
bool ControlFlowVisitor::isVisitingAlreadyVisitedLabel(Ast* currentNode)
{
	if (currentNode->name == config::LABEL_TOKEN_NAME)	// Since the only possible closing points of cycles are labels, disregard any other node type
	{
		if (config::stringVectorContains(visitedLabels, currentNode->getLabelCode()))
		{
			return true;
		}
	}

	return false;
}

// Performs one visit to the target node. Updates the node's persistent buffer size map, continues along the control flow path,
// or spawns copies of itself, according to the state of the visit
void ControlFlowVisitor::traverseControlFlowGraph(Ast* startNode)
{
	if (isVisitingAlreadyVisitedLabel(startNode))	// If re-visiting this node, set the local persistent buffer sizes to TOP, if they'd been increased
	{												// on the traversed path. Cascade these changes along all successive control flow paths.

		// Add the visitor's accumulated buffer sizes to the persistent buffer sizes of the node
		additiveMergeBufferSizes(&(startNode->causedWriteCost), &writeBufferSizeMap);
		additiveMergeBufferSizes(&(startNode->causedReadCost), &readBufferSizeMap);

		setTopIfIncremented(&writeBufferSizeMap, &(startNode->persistentWriteCost));
		setTopIfIncremented(&readBufferSizeMap, &(startNode->persistentReadCost));
		startNode->controlFlowDirectionCascadingPropagateTops();
	}
	else
	{
		if (startNode->name == config::LABEL_TOKEN_NAME)	// If visiting a label for the first time, add it to the list of visited labels
		{
			visitedLabels.push_back(startNode->getLabelCode());
		}

		// Add the visitor's accumulated buffer sizes to the persistent buffer sizes of the node
		additiveMergeBufferSizes(&writeBufferSizeMap, &(startNode->persistentWriteCost));
		additiveMergeBufferSizes(&readBufferSizeMap, &(startNode->persistentReadCost));

		// Copy the node's updated buffer sizes to the visitor's maps
		copyBufferSizes(&(startNode->persistentWriteCost), &writeBufferSizeMap);
		copyBufferSizes(&(startNode->persistentReadCost), &readBufferSizeMap);

		if (startNode->name == config::FENCE_TOKEN_NAME)	// If visiting a fence, reset all buffer sizes to 0
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