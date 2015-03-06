/*
A node of the abstract semantic tree. Its type is contained in the name variable, unless it's an identifier's name or an integer value.
It can hold buffer size maps containing the buffer size increases caused by the program point it's representing, and the buffer size
the program would at most need at this point. May also contain directed edges to other AST nodes, representing control flow.

Cascading operations may go top to bottom or bottom to top in the tree, or down along the control flow graph, which might be cyclic.
*/
#include "Ast.h"

#include "ControlFlowEdge.h"

using namespace std;

Ast::Ast()
{
	// The index by which this node can be referred to from its parent's children vector. The root always has an index of -1
	indexAsChild = -1;
}


Ast::~Ast()
{
}

// Contains all buffer size maps
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

// Returns whether the node represents one of the program point types
bool Ast::isProgramPoint()
{
	return name == config::LABEL_TOKEN_NAME || name == config::GOTO_TOKEN_NAME || name == config::STORE_TOKEN_NAME
		|| name == config::LOAD_TOKEN_NAME || name == config::IF_ELSE_TOKEN_NAME || name == config::FENCE_TOKEN_NAME
		|| name == config::NOP_TAG_NAME;
}

// Cascades from top to bottom. If a store or a load is encountered, the cascade stops and the IDs read and written by its children are gathered
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

// Sets all buffer size map entry values to 0
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

// Copies the contents of the persistent buffer size maps from another node
void Ast::copyPersistentBufferSizes(Ast* source)
{
	copyBufferSizes(&(source->persistentWriteCost), &persistentWriteCost);
	copyBufferSizes(&(source->persistentReadCost), &persistentReadCost);
}

// Compares the persistent buffer size maps of those of the direct control flow successors, and if they don't match, it sets the locally TOP values
// remotely to TOP. If no changes have been made, returns false.
bool Ast::propagateTops()
{
	bool result = false;
	bufferSizeMap topContainer;

	// Creates a map containing 0s for each non-TOP, and TOP for each TOP value in the persistent write cost map, and adds it to the successor's map if necessary
	copyAndSetNonTopToZero(&persistentWriteCost, &topContainer);

	for (ControlFlowEdge* edge : outgoingEdges)
	{
		if (bufferSizeMapCompare(&persistentWriteCost, &(edge->end->persistentWriteCost)))
		{
			result = true;
			additiveMergeBufferSizes(&topContainer, &(edge->end->persistentWriteCost));
		}
	}

	// Creates a map containing 0s for each non-TOP, and TOP for each TOP value in the persistent read cost map, and adds it to the successor's map if necessary
	copyAndSetNonTopToZero(&persistentReadCost, &topContainer);

	for (ControlFlowEdge* edge : outgoingEdges)
	{
		if (bufferSizeMapCompare(&persistentReadCost, &(edge->end->persistentReadCost)))
		{
			result = true;
			additiveMergeBufferSizes(&topContainer, &(edge->end->persistentReadCost));
		}
	}

	return result;
}

// Cascades from top to bottom and back. Returns a string vector containing all identifiers along the downward path, including the current node's own.
vector<string> Ast::getIDs()
{
	vector<string> results;

	if (name == config::ID_TOKEN_NAME)	// If this is an identifier, push it on the vector to return it upwards
	{
		results.push_back(children.at(0)->name);
	}
	else // If this isn't an identifier, gather all identifiers from the progeny
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

// Cascades from top to bottom. Copies all persistent buffer size maps to the successors.
void Ast::topDownCascadingCopyPersistentBufferSizes(Ast* source)
{
	copyPersistentBufferSizes(source);

	for (Ast* child : children)
	{
		child->topDownCascadingCopyPersistentBufferSizes(source);
	}
}

// Cascades from top to bottom. Causes those nodes that have their own caused costs to add them to their persistent maps.
void Ast::topDownCascadingAddInitializedCausedCostsToPersistentCosts()
{
	conditionalAdditiveMergeBufferSizes(&causedReadCost, &persistentReadCost);
	conditionalAdditiveMergeBufferSizes(&causedWriteCost, &persistentWriteCost);

	for (Ast* child : children)
	{
		child->topDownCascadingAddInitializedCausedCostsToPersistentCosts();
	}
}

// Cascades from top to bottom. Causes label nodes to register themselves with the global register.
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

// Cascades down the control flow graph. If any successor node has a different persistent map, causes this to propagate its TOP values down the line.
void Ast::controlFlowDirectionCascadingPropagateTops()
{
	if (propagateTops())
	{
		for (ControlFlowEdge* edge : outgoingEdges)
		{
			edge->end->controlFlowDirectionCascadingPropagateTops();
		}
	}
}

// Cascades from top to bottom. Causes label nodes to generate outgoing directional control flow edges, if applicable.
void Ast::cascadingGenerateOutgoingEdges()
{
	generateOutgoingEdges();
	
	for (Ast* child : children)
	{
		child->cascadingGenerateOutgoingEdges();
	}
}

// Returns whether the current node is an ifElse node with an Else statement block.
bool Ast::hasElse()
{
	if (name == config::IF_ELSE_TOKEN_NAME)
	{
		return children.at(2)->name == config::NONE_TAG_NAME;
	}

	return false;
}

// Checks whether the node is a program point and has any control flow successors, and if so, creates control flow edges between this node and its successors.
bool Ast::generateOutgoingEdges()
{
	if (isProgramPoint())	// Checks whether the node is a program point
	{
		ControlFlowEdge* newEdge;
		Ast* nextStatement = NULL;

		if (name == config::GOTO_TOKEN_NAME)	// Goto nodes only lead to their corresponding label
		{
			newEdge = new ControlFlowEdge(this, config::labelLookupMap[getGotoCode()]);
			outgoingEdges.push_back(newEdge);
		}
		else if (name == config::LABEL_TOKEN_NAME)	// Label nodes only lead to their corresponding statement
		{
			newEdge = new ControlFlowEdge(this, children.at(1));
			outgoingEdges.push_back(newEdge);
		}
		else if (name == config::IF_ELSE_TOKEN_NAME)	// IfElse nodes lead to their conditionals, which lead to both statement blocks' first statements,
		{												// of which the last statements lead to the statement following the IfElse block, unless they're goto statements
			Ast* lastChild = NULL;
			nextStatement = tryGetNextStatement();
			bool nextStatementExists = nextStatement != NULL;

			newEdge = new ControlFlowEdge(this, children.at(0));	// Create an edge to the conditional
			outgoingEdges.push_back(newEdge);

			newEdge = new ControlFlowEdge(children.at(0), children.at(1)->children.at(0));	// Create an edge from the conditional to the first statement of the If block
			children.at(0)->outgoingEdges.push_back(newEdge);

			lastChild = children.at(1)->tryGetLastStatement();		// If there is a statement after the IfElse block, connect to it from the last statement of the If block, if it's not a goto statement
			if (nextStatementExists && lastChild != NULL && lastChild->name != config::GOTO_TOKEN_NAME)
			{
				newEdge = new ControlFlowEdge(lastChild, nextStatement);
				lastChild->outgoingEdges.push_back(newEdge);
			}

			if (hasElse())	// If there is a non-empty Else block, connect to its first statement from the conditional
			{
				newEdge = new ControlFlowEdge(children.at(0), children.at(2));
				children.at(0)->outgoingEdges.push_back(newEdge);

				lastChild = children.at(2)->tryGetLastStatement();		// If there is a statement after the IfElse block, connect to it from the last statement of the Else block, if it's not a goto statement
				if (nextStatementExists && lastChild != NULL && lastChild->name != config::GOTO_TOKEN_NAME)
				{
					newEdge = new ControlFlowEdge(lastChild, nextStatement);
					lastChild->outgoingEdges.push_back(newEdge);
				}
			}
			else if (nextStatementExists)	// If there is no Else block, but there is a statement following the IfElse block, connect to it from the conditional
			{
				newEdge = new ControlFlowEdge(children.at(0), nextStatement);
				children.at(0)->outgoingEdges.push_back(newEdge);
			}
		}
		else if ((nextStatement = tryGetNextStatement()) != NULL)	// For other program points, follow the statement block if it continues
		{
			newEdge = new ControlFlowEdge(this, nextStatement);
			outgoingEdges.push_back(newEdge);
		}

		return true;
	}

	return false;
}

// Gets the qualified name representing the label to which this goto node links
string Ast::getGotoCode()
{
	if (name == config::GOTO_TOKEN_NAME)
	{
		string parentProcessNumber;
		string label = children.at(0)->name;

		if (tryGetParentProcessNumber(&parentProcessNumber))
		{
			return parentProcessNumber + config::LABEL_SEPARATOR + label;
		}
	}

	return "";
}

// Gets the qualified name representing this label
string Ast::getLabelCode()
{
	if (name == config::LABEL_TOKEN_NAME)
	{
		string parentProcessNumber;
		string label = children.at(0)->name;

		if (tryGetParentProcessNumber(&parentProcessNumber))
		{
			return parentProcessNumber + config::LABEL_SEPARATOR + label;
		}
	}

	return "";
}

// Returns the qualified name described by the process- and label numbers
int Ast::toLabelCode(string processNumber, string labelNumber)
{
	hash<string> labelHash;
	return labelHash(processNumber + config::LABEL_SEPARATOR + labelNumber);
}

// Registers this label node with the global label container
void Ast::registerLabel()
{
	if (name == config::LABEL_TOKEN_NAME)
	{
		config::labelLookupMap[getLabelCode()] = this;
	}
}

// Cascades from top to bottom. Causes nodes to initialize the global variables initialized in the program's first block into their persistent maps
void Ast::initializePersistentCosts()
{
	if (name == config::PROGRAM_DECLARATION_TOKEN_NAME)
	{
		// Collect all buffer costs caused by the variable initialization statements
		vector<Ast*> initializationBlockStatements = children.at(0)->children;

		for (Ast* initializationStatement : initializationBlockStatements)
		{
			additiveMergeBufferSizes(&(initializationStatement->causedWriteCost), &(persistentWriteCost));
			additiveMergeBufferSizes(&(initializationStatement->causedWriteCost), &(persistentReadCost));
		}

		// Sets the values of the recently initialized entries to 0
		resetBufferSizes();

		// Propagates the 0-initialized global variable map to the progeny of the process declarations
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

bool Ast::tryGetParentProcessNumber(string* out)
{
	bool result = false;

	if (!isRoot())
	{
		if (parent->name == config::PROCESS_DECLARATION_TOKEN_NAME)
		{
			(*out) = parent->children.at(0)->children.at(0)->name;
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

Ast* Ast::tryGetLastStatement()
{
	Ast* lastChild = tryGetLastChild();

	if (lastChild->name == config::LABEL_TOKEN_NAME)
	{
		lastChild = lastChild->children.at(1);
	}

	return lastChild;
}

string Ast::astToString()
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

		result += "\tpersistentReadCost = (" + toString(&persistentReadCost) + ")";
		result += "\tpersistentWriteCost = (" + toString(&persistentWriteCost) + ")";

		/*result += "\t";
		string startName, endName;
		for (ControlFlowEdge* edge : outgoingEdges)
		{
			startName = edge->start->name == config::LABEL_TOKEN_NAME ? edge->start->name + " " + edge->start->children.at(0)->name : edge->start->name;
			endName = edge->end->name == config::LABEL_TOKEN_NAME ? edge->end->name + " " + edge->end->children.at(0)->name : edge->end->name;
			result += "(" + startName + ", " + endName + ") ";
		}*/
	}

	int childrenCount = children.size();

	for (int ctr = 0; ctr < childrenCount; ctr++)
	{
		result += "\n" + children.at(ctr)->astToString();
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