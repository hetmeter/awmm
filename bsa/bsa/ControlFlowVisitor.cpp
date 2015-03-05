#include "ControlFlowVisitor.h"

#include "ControlFlowEdge.h"
#include "Ast.h"

using namespace std;

ControlFlowVisitor::ControlFlowVisitor()
{
}


ControlFlowVisitor::~ControlFlowVisitor()
{
}

ControlFlowVisitor* ControlFlowVisitor::clone()
{
	ControlFlowVisitor * result = new ControlFlowVisitor;

	for (int visitedLabel : visitedLabels)
	{
		result->visitedLabels.push_back(visitedLabel);
	}

	copyBufferSizes(&writeBufferSizeMap, &(result->writeBufferSizeMap));
	copyBufferSizes(&readBufferSizeMap, &(result->readBufferSizeMap));

	return result;
}

bool ControlFlowVisitor::intVectorContains(std::vector<int> container, int item)
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

bool ControlFlowVisitor::isVisitingAlreadyVisitedLabel(Ast* currentNode)
{
	if (currentNode->name == config::LABEL_TOKEN_NAME)
	{
		if (intVectorContains(visitedLabels, currentNode->getLabelCode()))
		{
			return true;
		}
	}

	return false;
}

void ControlFlowVisitor::traverseControlFlowGraph(Ast* startNode)
{
	string s1 = startNode->children.size() > 0 ? startNode->children.at(0)->name : "";
	string s2 = startNode->children.size() > 0 && startNode->children.at(0)->children.size() > 0 ? startNode->children.at(0)->children.at(0)->name : "";
	cout << "\t\tTraversing " << startNode->name << " " << s1 << " " << s2 << "\n\t\t\twriteBufferSizeMap: " << toString(&writeBufferSizeMap) << " readBufferSizeMap: " << toString(&readBufferSizeMap) << "\n";

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

			ControlFlowVisitor* newControlFlowVisitor;

			for (int ctr = 1; ctr < outgoingEdgeCount; ctr++)
			{
				newControlFlowVisitor = clone();
				newControlFlowVisitor->traverseControlFlowGraph(startNode->outgoingEdges.at(ctr)->end);
			}
		}
	}
}