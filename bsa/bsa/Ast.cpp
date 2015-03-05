#include "Ast.h"

#include "ControlFlowEdge.h"

using namespace std;

Ast::Ast()
{
	indexAsChild = -1;
}


Ast::~Ast()
{
}


vector<bufferSizeMap*> Ast::allBufferSizeContainers()
{
	if (_allBufferSizeContainers.empty())
	{
		_allBufferSizeContainers.push_back(&causedWriteCost);
		_allBufferSizeContainers.push_back(&causedReadCost);
		_allBufferSizeContainers.push_back(&persistentWriteCost);
		_allBufferSizeContainers.push_back(&persistentReadCost);
	}

	return _allBufferSizeContainers;
}

string Ast::bufferSizeMapString(bufferSizeMap* source)
{
	string result = "";

	for (bufferSizeMapIterator iterator = source->begin(); iterator != source->end(); iterator++)
	{
		result += iterator->first + ": " + to_string((*source)[iterator->first]) + ", ";
	}

	if (result.length() > 2)
	{
		result = result.substr(0, result.length() - 2);
	}

	return result;
}

bool Ast::isProgramPoint()
{
	return name == config::LABEL_TOKEN_NAME || name == config::GOTO_TOKEN_NAME || name == config::STORE_TOKEN_NAME || name == config::LOAD_TOKEN_NAME || name == config::IF_ELSE_TOKEN_NAME || name == config::FENCE_TOKEN_NAME;
}

void Ast::getCostsFromChildren()
{
	if (name == config::STORE_TOKEN_NAME || name == config::LOAD_TOKEN_NAME)
	{
		vector<string> writtenVariables = children.at(0)->getIDs();
		vector<string> readVariables = children.at(1)->getIDs();

		for (string member : writtenVariables)
		{
			incrementCost(member, 1, &causedWriteCost);
		}

		for (string member : readVariables)
		{
			incrementCost(member, 1, &causedReadCost);
		}
	}
	else
	{
		for (Ast* child : children)
		{
			child->getCostsFromChildren();
		}
	}
}

void Ast::resetBufferSizes()
{
	vector<bufferSizeMap*> containers = allBufferSizeContainers();

	for (bufferSizeMap* container : containers)
	{
		for (bufferSizeMapIterator iterator = container->begin(); iterator != container->end(); iterator++)
		{
			(*container)[iterator->first] = 0;
		}
	}
}

void Ast::copyPersistentBufferSizes(Ast* source)
{
	copyBufferSizes(&(source->persistentWriteCost), &persistentWriteCost);
	copyBufferSizes(&(source->persistentReadCost), &persistentReadCost);
}

void Ast::propagateTops()
{
	bufferSizeMap topContainer;

	copyAndSetNonTopToZero(&persistentWriteCost, &topContainer);

	for (Ast* child : children)
	{
		additiveMergeBufferSizes(&topContainer, &(child->persistentWriteCost));
	}

	copyAndSetNonTopToZero(&persistentReadCost, &topContainer);

	for (Ast* child : children)
	{
		additiveMergeBufferSizes(&topContainer, &(child->persistentReadCost));
	}
}

vector<string> Ast::getIDs()
{
	vector<string> results;

	if (name == config::ID_TOKEN_NAME)
	{
		results.push_back(children.at(0)->name);
	}
	else
	{
		vector<string> subResults;

		for (Ast* child : children)
		{
			subResults = child->getIDs();

			for (string subResult : subResults)
			{
				results.push_back(subResult);
			}
		}
	}

	return results;
}

void Ast::topDownCascadingCopyPersistentBufferSizes(Ast* source)
{
	copyPersistentBufferSizes(source);

	for (Ast* child : children)
	{
		child->topDownCascadingCopyPersistentBufferSizes(source);
	}
}

void Ast::topDownCascadingAddInitializedCausedCostsToPersistentCosts()
{
	conditionalAdditiveMergeBufferSizes(&causedReadCost, &persistentReadCost);
	conditionalAdditiveMergeBufferSizes(&causedWriteCost, &persistentWriteCost);

	for (Ast* child : children)
	{
		child->topDownCascadingAddInitializedCausedCostsToPersistentCosts();
	}
}

void Ast::topDownCascadingRegisterLabels()
{
	if (name == config::LABEL_TOKEN_NAME)
	{
		registerLabel();
	}

	for (Ast* child : children)
	{
		child->topDownCascadingRegisterLabels();
	}
}

void Ast::controlFlowDirectionCascadingPropagateTops()
{
	propagateTops();

	for (ControlFlowEdge* edge : outgoingEdges)
	{
		edge->end->controlFlowDirectionCascadingPropagateTops();
	}
}

//void Ast::cascadingGenerateOutgoingEdges()	// If the current node is a program point, the method cascades along the control flow. Otherwise, it cascades down the tree.
void Ast::cascadingGenerateOutgoingEdges()
{
	/*if (generateOutgoingEdges())
	{
		for (ControlFlowEdge* edge : outgoingEdges)
		{
			edge->end->cascadingGenerateOutgoingEdges();
		}
	}
	else
	{
		for (Ast* child : children)
		{
			child->cascadingGenerateOutgoingEdges();
		}
	}*/

	generateOutgoingEdges();
	
	for (Ast* child : children)
	{
		child->cascadingGenerateOutgoingEdges();
	}
}

bool Ast::hasElse()
{
	if (name == config::IF_ELSE_TOKEN_NAME)
	{
		return children.at(2)->name == config::NONE_TAG_NAME;
	}

	return false;
}

bool Ast::generateOutgoingEdges()
{
	if (isProgramPoint())
	{
		//cout << "\tGenerating outgoing edges for \"" << name << "\"\n";

		ControlFlowEdge* newEdge;
		Ast* nextStatement = NULL;

		if (name == config::GOTO_TOKEN_NAME)
		{
			newEdge = new ControlFlowEdge(this, config::labelLookupMap[getGotoCode()]);
			outgoingEdges.push_back(newEdge);
		}
		else if (name == config::LABEL_TOKEN_NAME)
		{
			newEdge = new ControlFlowEdge(this, children.at(1));
			outgoingEdges.push_back(newEdge);
		}
		else if (name == config::IF_ELSE_TOKEN_NAME)
		{
			Ast* lastChild = NULL;
			nextStatement = tryGetNextStatement();
			bool nextStatementExists = nextStatement != NULL;

			newEdge = new ControlFlowEdge(this, children.at(0));
			outgoingEdges.push_back(newEdge);

			newEdge = new ControlFlowEdge(children.at(0), children.at(1));
			children.at(0)->outgoingEdges.push_back(newEdge);

			lastChild = children.at(1)->tryGetLastChild();
			if (nextStatementExists && lastChild != NULL)
			{
				newEdge = new ControlFlowEdge(lastChild, nextStatement);
				lastChild->outgoingEdges.push_back(newEdge);
			}

			if (hasElse())
			{
				newEdge = new ControlFlowEdge(children.at(0), children.at(2));
				children.at(0)->outgoingEdges.push_back(newEdge);

				lastChild = children.at(2)->tryGetLastChild();
				if (nextStatementExists && lastChild != NULL)
				{
					newEdge = new ControlFlowEdge(lastChild, nextStatement);
					lastChild->outgoingEdges.push_back(newEdge);
				}
			}
			else if (nextStatementExists)
			{
				newEdge = new ControlFlowEdge(children.at(0), nextStatement);
				children.at(0)->outgoingEdges.push_back(newEdge);
			}
		}
		else if ((nextStatement = tryGetNextStatement()) != NULL)
		{
			newEdge = new ControlFlowEdge(this, nextStatement);
			outgoingEdges.push_back(newEdge);
		}

		return true;
	}

	return false;
}

int Ast::getGotoCode()
{
	if (name == config::GOTO_TOKEN_NAME)
	{
		int parentProcessNumber;
		string label = children.at(0)->name;

		if (tryGetParentProcessNumber(&parentProcessNumber))
		{
			return toLabelCode(to_string(parentProcessNumber), label);
		}
	}

	return -1;
}

int Ast::getLabelCode()
{
	if (name == config::LABEL_TOKEN_NAME)
	{
		int parentProcessNumber;
		string label = children.at(0)->name;

		if (tryGetParentProcessNumber(&parentProcessNumber))
		{
			return toLabelCode(to_string(parentProcessNumber), label);
		}
	}

	return -1;
}

int Ast::toLabelCode(string processNumber, string labelNumber)
{
	hash<string> labelHash;
	return labelHash(processNumber + config::LABEL_SEPARATOR + labelNumber);
}

void Ast::registerLabel()
{
	if (name == config::LABEL_TOKEN_NAME)
	{
		config::labelLookupMap[getLabelCode()] = this;
	}
}

void Ast::initializePersistentCosts()
{
	if (name == config::PROGRAM_DECLARATION_TOKEN_NAME)
	{
		vector<Ast*> initializationBlockStatements = children.at(0)->children;

		for (Ast* initializationStatement : initializationBlockStatements)
		{
			additiveMergeBufferSizes(&(initializationStatement->causedWriteCost), &(persistentWriteCost));
			additiveMergeBufferSizes(&(initializationStatement->causedWriteCost), &(persistentReadCost));
		}

		resetBufferSizes();

		for (Ast* child : children)
		{
			if (child->name == config::PROCESS_DECLARATION_TOKEN_NAME)
			{
				child->topDownCascadingCopyPersistentBufferSizes(this);
				child->topDownCascadingAddInitializedCausedCostsToPersistentCosts();
			}
		}
	}
}

void Ast::visitAllProgramPoints()
{
	if (name == config::PROGRAM_DECLARATION_TOKEN_NAME)
	{
		int childrenCount = children.size();
		Ast* currentProcessDeclarationStatements;
		ControlFlowVisitor* newControlFlowVisitor;

		for (int ctr = 1; ctr < childrenCount; ctr++)
		{
			currentProcessDeclarationStatements = children.at(ctr)->children.at(1);

			if (((int)currentProcessDeclarationStatements->children.size()) > 0)
			{
				newControlFlowVisitor = new ControlFlowVisitor;
				newControlFlowVisitor->traverseControlFlowGraph(currentProcessDeclarationStatements->children.at(0));
			}
		}
	}
}

void Ast::addChild(Ast* child)
{
	child->parent = this;
	child->indexAsChild = children.size();
	children.push_back(child);
}

Ast* Ast::tryGetNextSibling()
{
	if (!isRoot() && ((int)parent->children.size()) > indexAsChild + 1)
	{
		return parent->children.at(indexAsChild + 1);
	}

	return NULL;
}

Ast* Ast::tryGetNextStatement()
{
	if (parent->name == config::LABEL_TOKEN_NAME)
	{
		return parent->tryGetNextSibling();
	}
	else if (parent->name == config::STATEMENTS_TOKEN_NAME)
	{
		return tryGetNextSibling();
	}

	return NULL;
}

bool Ast::isRoot()
{
	return indexAsChild == -1;
}

bool Ast::tryGetParentProcessNumber(int* out)
{
	bool result = false;

	if (!isRoot())
	{
		if (parent->name == config::PROCESS_DECLARATION_TOKEN_NAME)
		{
			(*out) = stoi(parent->children.at(0)->children.at(0)->name);
			result = true;
		}
		else
		{
			result = parent->tryGetParentProcessNumber(out);
		}
	}

	return result;
}

Ast* Ast::tryGetLastChild()
{
	int childrenCount = children.size();

	if (childrenCount > 0)
	{
		return children.at(childrenCount - 1);
	}

	return NULL;
}

string Ast::toString()
{
	regex indentationRegex("\n");
	string result = name;

	//if (name != PROGRAM_DECLARATION_TOKEN_NAME && (name == STATEMENTS_TOKEN_NAME || name == INITIALIZATION_BLOCK_TOKEN_NAME || parent->name == IF_ELSE_TOKEN_NAME || parent->name == WHILE_TOKEN_NAME || parent->name == STATEMENTS_TOKEN_NAME))
	/*if (name != PROGRAM_DECLARATION_TOKEN_NAME && (parent->name == STATEMENTS_TOKEN_NAME || parent->name == IF_ELSE_TOKEN_NAME || parent->name == WHILE_TOKEN_NAME || name == IF_ELSE_TOKEN_NAME || name == STORE_TOKEN_NAME || name == LOAD_TOKEN_NAME))
	{
	result += "\t\t" + bufferSizeMapString();
	}*/

	if (isProgramPoint())
	{
		//result += "\tcausedReadCost = (" + bufferSizeMapString(&causedReadCost) + ")";
		//result += "\tcausedWriteCost = (" + bufferSizeMapString(&causedWriteCost) + ")";

		result += "\tpersistentReadCost = (" + bufferSizeMapString(&persistentReadCost) + ")";
		result += "\tpersistentWriteCost = (" + bufferSizeMapString(&persistentWriteCost) + ")";

		//result += "\t";
		//for (ControlFlowEdge* edge : outgoingEdges)
		//{
		//	result += "(" + edge->start->name + ", " + edge->end->name + ") ";
		//}
	}

	int childrenCount = children.size();

	for (int ctr = 0; ctr < childrenCount; ctr++)
	{
		result += "\n" + children.at(ctr)->toString();
	}

	result = regex_replace(result, indentationRegex, "\n|");

	/*string result = name + ", children:";
	int childrenCount = children.size();

	for (int ctr = 0; ctr < childrenCount; ctr++)
	{
	result += " " + children.at(ctr).name;
	}

	result += " {\n";

	for (int ctr = 0; ctr < childrenCount; ctr++)
	{
	result += "\n" + children.at(ctr).toString();
	}

	result += "}";*/

	return result;
}