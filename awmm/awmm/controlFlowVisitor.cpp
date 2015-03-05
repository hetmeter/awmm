#include "controlFlowVisitor.h"
#include "controlFlowEdge.h"
#include <iostream>
#include "ast.h"
#include "bufferSize.h"

//#ifndef CONTROLFLOWEDGE_H_INCLUDED
//#define CONTROLFLOWEDGE_H_INCLUDED
//#include "controlFlowEdge.h"
//#endif
//
//#ifndef IOSTREAM_INCLUDED
//#define IOSTREAM_INCLUDED
//#include <iostream>
//#endif
//
//#ifndef AST_H_INCLUDED__
//#define AST_H_INCLUDED__
//#include "ast.h"
//#endif
//
//#ifndef BUFFERSIZE_H_INCLUDED__
//#define BUFFERSIZE_H_INCLUDED__
//#include "bufferSize.h"
//#endif

using namespace std;

struct controlFlowVisitor
{
	vector<int> visitedLabels;

	bufferSizeMap writeBufferSizeMap;
	bufferSizeMap readBufferSizeMap;

	controlFlowVisitor* clone()
	{
		controlFlowVisitor* result = new controlFlowVisitor;

		for (int visitedLabel : visitedLabels)
		{
			result->visitedLabels.push_back(visitedLabel);
		}

		copyBufferSizes(&writeBufferSizeMap, &(result->writeBufferSizeMap));
		copyBufferSizes(&readBufferSizeMap, &(result->readBufferSizeMap));

		return result;
	}

	bool intVectorContains(vector<int> container, int item)
	{
		int count = container.size();

		for (int ctr = 0; ctr < count; ctr++)
		{
			if (item == container.at(ctr))
			{
				return true;
			}
		}

		return false;
	}

	bool isVisitingAlreadyVisitedLabel(ast* currentNode)
	{
		if (currentNode->name == LABEL_TOKEN_NAME)
		{
			if (intVectorContains(visitedLabels, currentNode->getLabelCode()))
			{
				return true;
			}
		}

		return false;
	}

	void traverseControlFlowGraph(ast* startNode)
	{
		cout << "\t\tTraversing " << startNode->name << "\n";

		if (isVisitingAlreadyVisitedLabel(startNode))
		{
			setTopIfIncremented(&writeBufferSizeMap, &(startNode->persistentWriteCost));
			setTopIfIncremented(&readBufferSizeMap, &(startNode->persistentReadCost));
			startNode->controlFlowDirectionCascadingPropagateTops();
		}
		else
		{
			additiveMergeBufferSizes(&writeBufferSizeMap, &(startNode->persistentWriteCost));
			additiveMergeBufferSizes(&readBufferSizeMap, &(startNode->persistentReadCost));

			copyBufferSizes(&(startNode->persistentWriteCost), &writeBufferSizeMap);
			copyBufferSizes(&(startNode->persistentReadCost), &readBufferSizeMap);

			int outgoingEdgeCount = startNode->outgoingEdges.size();

			if (outgoingEdgeCount > 0)
			{
				traverseControlFlowGraph(startNode->outgoingEdges.at(0)->end);

				controlFlowVisitor* newControlFlowVisitor;

				for (int ctr = 1; ctr < outgoingEdgeCount; ctr++)
				{
					newControlFlowVisitor = clone();
					newControlFlowVisitor->traverseControlFlowGraph(startNode->outgoingEdges.at(ctr)->end);
				}
			}
		}
	}
};
typedef struct controlFlowVisitor controlFlowVisitor;