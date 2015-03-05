#include "ast.h"
#include <string>
#include <vector>
#include <regex>
#include <map>
#include "controlFlowEdge.h"
#include "controlFlowVisitor.h"
#include "bufferSize.h"

using namespace std;

const char LABEL_SEPARATOR = '.';

//const string ID_TOKEN_NAME = "ID";
//const string PROGRAM_DECLARATION_TOKEN_NAME = "programDeclaration";
//const string PROCESS_DECLARATION_TOKEN_NAME = "processDeclaration";
//const string INITIALIZATION_BLOCK_TOKEN_NAME = "initializationBlock";
//const string STATEMENTS_TOKEN_NAME = "statements";
//const string STORE_TOKEN_NAME = "store";
//const string LOAD_TOKEN_NAME = "load";
//const string FENCE_TOKEN_NAME = "fence";
////const string LABEL_TOKEN_NAME = "label";
//const string GOTO_TOKEN_NAME = "goto";
//const string IF_ELSE_TOKEN_NAME = "ifElse";
//const string WHILE_TOKEN_NAME = "while";
//const string NONE_TAG_NAME = "none";

typedef struct ast ast;

map<int, ast*> labelLookupMap;

void ast::getCostsFromChildren()
{
	if (name == STORE_TOKEN_NAME || name == LOAD_TOKEN_NAME)
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
		for (ast* child : children)
		{
			child->getCostsFromChildren();
		}
	}
}

struct ast
{
private:

	vector<bufferSizeMap*> _allBufferSizeContainers;
	vector<bufferSizeMap*> allBufferSizeContainers()
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

	/*
	Section: Private general methods
	*/

	string bufferSizeMapString(bufferSizeMap* source)
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

	bool isProgramPoint()
	{
		return name == LABEL_TOKEN_NAME || name == GOTO_TOKEN_NAME || name == STORE_TOKEN_NAME || name == LOAD_TOKEN_NAME || name == IF_ELSE_TOKEN_NAME || name == FENCE_TOKEN_NAME;
	}

	/*
	End section: Private general methods
	*/

public:

	/*
	Section: Cost map access
	*/

	void getCostsFromChildren()
	{
		if (name == STORE_TOKEN_NAME || name == LOAD_TOKEN_NAME)
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
			for (ast* child : children)
			{
				child->getCostsFromChildren();
			}
		}
	}

	void resetBufferSizes()
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

	void copyPersistentBufferSizes(ast* source)
	{
		copyBufferSizes(&(source->persistentWriteCost), &persistentWriteCost);
		copyBufferSizes(&(source->persistentReadCost), &persistentReadCost);
	}

	void propagateTops()
	{
		bufferSizeMap topContainer;

		copyAndSetNonTopToZero(&persistentWriteCost, &topContainer);

		for (ast* child : children)
		{
			additiveMergeBufferSizes(&topContainer, &(child->persistentWriteCost));
		}

		copyAndSetNonTopToZero(&persistentReadCost, &topContainer);

		for (ast* child : children)
		{
			additiveMergeBufferSizes(&topContainer, &(child->persistentReadCost));
		}
	}

	/*
	End section: Cost map access
	*/

	/*
	Section: Cascading methods
	*/

	vector<string> getIDs()
	{
		vector<string> results;

		if (name == ID_TOKEN_NAME)
		{
			results.push_back(children.at(0)->name);
		}
		else
		{
			vector<string> subResults;

			for (ast* child : children)
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

	void topDownCascadingCopyPersistentBufferSizes(ast* source)
	{
		copyPersistentBufferSizes(source);

		for (ast* child : children)
		{
			child->topDownCascadingCopyPersistentBufferSizes(source);
		}
	}

	void topDownCascadingAddInitializedCausedCostsToPersistentCosts()
	{
		conditionalAdditiveMergeBufferSizes(&causedReadCost, &persistentReadCost);
		conditionalAdditiveMergeBufferSizes(&causedWriteCost, &persistentWriteCost);

		for (ast* child : children)
		{
			child->topDownCascadingAddInitializedCausedCostsToPersistentCosts();
		}
	}

	void topDownCascadingRegisterLabels()
	{
		if (name == LABEL_TOKEN_NAME)
		{
			registerLabel();
		}

		for (ast* child : children)
		{
			child->topDownCascadingRegisterLabels();
		}
	}

	void controlFlowDirectionCascadingPropagateTops()
	{
		propagateTops();

		for (controlFlowEdge* edge : outgoingEdges)
		{
			edge->end->controlFlowDirectionCascadingPropagateTops();
		}
	}

	void cascadingGenerateOutgoingEdges()	// If the current node is a program point, the method cascades along the control flow. Otherwise, it cascades down the tree.
	{
		if (generateOutgoingEdges())
		{
			for (controlFlowEdge* edge : outgoingEdges)
			{
				edge->end->cascadingGenerateOutgoingEdges();
			}
		}
		else
		{
			for (ast* child : children)
			{
				child->cascadingGenerateOutgoingEdges();
			}
		}
	}

	/*
	End section: Cascading methods
	*/

	/*
	Section: Methods specific to ifElse point nodes
	*/

	bool hasElse()
	{
		if (name == IF_ELSE_TOKEN_NAME)
		{
			return children.at(2)->name == NONE_TAG_NAME;
		}

		return false;
	}

	/*
	End section: Methods specific to ifElse point nodes
	*/

	/*
	Section: Methods specific to program point nodes
	*/

	bool generateOutgoingEdges()
	{
		if (isProgramPoint()) // STORE_TOKEN_NAME || name == IF_ELSE_TOKEN_NAME || name == FENCE_TOKEN_NAME;
		{
			controlFlowEdge* newEdge;
			ast* nextStatement;

			if (name == GOTO_TOKEN_NAME)
			{
				newEdge = new controlFlowEdge(this, labelLookupMap[getGotoCode()]);
				outgoingEdges.push_back(newEdge);
			}
			else if (name == LABEL_TOKEN_NAME)
			{
				newEdge = new controlFlowEdge(this, children.at(1));
				outgoingEdges.push_back(newEdge);
			}
			else if (name == IF_ELSE_TOKEN_NAME)
			{
				ast* lastChild;
				bool nextStatementExists = tryGetNextSibling(nextStatement);

				newEdge = new controlFlowEdge(this, children.at(0));
				outgoingEdges.push_back(newEdge);

				newEdge = new controlFlowEdge(children.at(0), children.at(1));
				children.at(0)->outgoingEdges.push_back(newEdge);

				if (nextStatementExists && children.at(1)->tryGetLastChild(lastChild))
				{
					newEdge = new controlFlowEdge(lastChild, nextStatement);
					lastChild->outgoingEdges.push_back(newEdge);
				}

				if (hasElse())
				{
					newEdge = new controlFlowEdge(children.at(0), children.at(2));
					children.at(0)->outgoingEdges.push_back(newEdge);

					if (nextStatementExists && children.at(2)->tryGetLastChild(lastChild))
					{
						newEdge = new controlFlowEdge(lastChild, nextStatement);
						lastChild->outgoingEdges.push_back(newEdge);
					}
				}
				else if (nextStatementExists)
				{
					newEdge = new controlFlowEdge(children.at(0), nextStatement);
					children.at(0)->outgoingEdges.push_back(newEdge);
				}
			}
			else if (tryGetNextSibling(nextStatement))
			{
				newEdge = new controlFlowEdge(this, nextStatement);
				outgoingEdges.push_back(newEdge);
			}

			return true;
		}

		return false;
	}

	/*
	End section: Methods specific to program point nodes
	*/

	/*
	Section: Methods specific to goto nodes
	*/

	int getGotoCode()
	{
		if (name == GOTO_TOKEN_NAME)
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

	/*
	End section: Methods specific to goto nodes
	*/

	/*
	Section: Methods specific to label nodes
	*/

	int getLabelCode()
	{
		if (name == LABEL_TOKEN_NAME)
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

	int toLabelCode(string processNumber, string labelNumber)
	{
		hash<string> labelHash;
		return labelHash(processNumber + LABEL_SEPARATOR + labelNumber);
	}

	void registerLabel()
	{
		if (name == LABEL_TOKEN_NAME)
		{
			labelLookupMap[getLabelCode()] = this;
		}
	}

	/*
	End section: Methods specific to label nodes
	*/

	/*
	Section: Methods specific to programDeclaration nodes
	*/

	void initializePersistentCosts()
	{
		if (name == PROGRAM_DECLARATION_TOKEN_NAME)
		{
			vector<ast*> initializationBlockStatements = children.at(0)->children;

			for (ast* initializationStatement : initializationBlockStatements)
			{
				additiveMergeBufferSizes(&(initializationStatement->causedWriteCost), &(persistentWriteCost));
				additiveMergeBufferSizes(&(initializationStatement->causedWriteCost), &(persistentReadCost));
			}

			resetBufferSizes();

			for (ast* child : children)
			{
				if (child->name == PROCESS_DECLARATION_TOKEN_NAME)
				{
					child->topDownCascadingCopyPersistentBufferSizes(this);
					child->topDownCascadingAddInitializedCausedCostsToPersistentCosts();
				}
			}
		}
	}

	void visitAllProgramPoints()
	{
		if (name == PROGRAM_DECLARATION_TOKEN_NAME)
		{
			int childrenCount = children.size();
			ast* currentProcessDeclarationStatements;
			controlFlowVisitor* newControlFlowVisitor;

			for (int ctr = 1; ctr < childrenCount; ctr++)
			{
				currentProcessDeclarationStatements = children.at(ctr)->children.at(1);

				if (((int)currentProcessDeclarationStatements->children.size()) > 0)
				{
					newControlFlowVisitor = new controlFlowVisitor;
					newControlFlowVisitor->traverseControlFlowGraph(currentProcessDeclarationStatements->children.at(0));
				}
			}
		}
	}

	/*
	End section: Methods specific to programDeclaration nodes
	*/

	/*
	Section: General methods
	*/

	void addChild(ast* child)
	{
		child->parent = this;
		child->indexAsChild = children.size();
		children.push_back(child);
	}

	bool tryGetNextSibling(ast* out)
	{
		bool result = false;

		if (!isRoot() && ((int)parent->children.size()) > indexAsChild + 1)
		{
			out = parent->children.at(indexAsChild + 1);
			result = true;
		}

		return result;
	}

	bool isRoot()
	{
		return indexAsChild == -1;
	}

	bool tryGetParentProcessNumber(int* out)
	{
		bool result = false;

		if (!isRoot())
		{
			if (parent->name == PROCESS_DECLARATION_TOKEN_NAME)
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

	bool tryGetLastChild(ast* out)
	{
		int childrenCount = children.size();

		if (childrenCount > 0)
		{
			out = children.at(childrenCount - 1);
			return true;
		}

		return false;
	}

	string toString()
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

	/*
	End section: General methods
	*/
};
typedef struct ast ast;