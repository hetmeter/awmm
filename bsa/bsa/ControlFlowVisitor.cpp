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

	for (string visitedLabel : visitedLabels)
	{
		result->visitedLabels.push_back(visitedLabel);
	}

	copyBufferSizes(&writeBufferSizeMap, &(result->writeBufferSizeMap));
	copyBufferSizes(&readBufferSizeMap, &(result->readBufferSizeMap));

	return result;
}

bool ControlFlowVisitor::stringVectorContains(vector<string> container, string item)
{
	for (string member : container)
	{
		if (member == item)
		{
			return true;
		}
	}

	/*int count = container.size();

	for (int ctr = 0; ctr < count; ctr++)
	{
		if (item == container.at(ctr))
		{
			return true;
		}
	}*/

	return false;
}

bool ControlFlowVisitor::isVisitingAlreadyVisitedLabel(Ast* currentNode)
{
	if (currentNode->name == config::LABEL_TOKEN_NAME)
	{
		if (stringVectorContains(visitedLabels, currentNode->getLabelCode()))
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
	//cout << "Traversing " << startNode->name << " " << to_string((int)startNode) << " " << s1 << " " << s2 << "\n\twriteBufferSizeMap: " << toString(&writeBufferSizeMap) << " readBufferSizeMap: " << toString(&readBufferSizeMap) << "\n\tvisitedLabelCount = " << visitedLabels.size() << "\n";

	if (isVisitingAlreadyVisitedLabel(startNode))
	{
		//cout << "\t\tstartNode->persistentWriteCost: " << toString(&(startNode->persistentWriteCost)) << ", startNode->persistentReadCost: " << toString(&(startNode->persistentReadCost)) << "\n";
		setTopIfIncremented(&writeBufferSizeMap, &(startNode->persistentWriteCost));
		//cout << "\t\tstartNode->persistentWriteCost: " << toString(&(startNode->persistentWriteCost)) << ", startNode->persistentReadCost: " << toString(&(startNode->persistentReadCost)) << "\n";
		setTopIfIncremented(&readBufferSizeMap, &(startNode->persistentReadCost));
		//cout << "\t\tstartNode->persistentWriteCost: " << toString(&(startNode->persistentWriteCost)) << ", startNode->persistentReadCost: " << toString(&(startNode->persistentReadCost)) << "\n";
		startNode->controlFlowDirectionCascadingPropagateTops();
		//cout << "\t\tstartNode->persistentWriteCost: " << toString(&(startNode->persistentWriteCost)) << ", startNode->persistentReadCost: " << toString(&(startNode->persistentReadCost)) << "\n";
	}
	else
	{
		if (startNode->name == config::LABEL_TOKEN_NAME)
		{
			visitedLabels.push_back(startNode->getLabelCode());
		}
		else if (startNode->name == config::FENCE_TOKEN_NAME)
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

		//cout << "\t\tstartNode->persistentWriteCost: " << toString(&(startNode->persistentWriteCost)) << ", startNode->persistentReadCost: " << toString(&(startNode->persistentReadCost)) << "\n";

		additiveMergeBufferSizes(&writeBufferSizeMap, &(startNode->persistentWriteCost));
		additiveMergeBufferSizes(&readBufferSizeMap, &(startNode->persistentReadCost));

		//cout << "\t\tstartNode->persistentWriteCost: " << toString(&(startNode->persistentWriteCost)) << ", startNode->persistentReadCost: " << toString(&(startNode->persistentReadCost)) << "\n";

		copyBufferSizes(&(startNode->persistentWriteCost), &writeBufferSizeMap);
		copyBufferSizes(&(startNode->persistentReadCost), &readBufferSizeMap);

		//cout << "\t\tstartNode->persistentWriteCost: " << toString(&(startNode->persistentWriteCost)) << ", startNode->persistentReadCost: " << toString(&(startNode->persistentReadCost)) << "\n";

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