/*
A node of the abstract semantic tree. Its type is contained in the name variable, unless it's an identifier's name or an integer value.
It can hold buffer size maps containing the buffer size increases caused by the program point it's representing, and the buffer size
the program would at most need at this point. May also contain directed edges to other AST nodes, representing control flow.

Cascading operations may go top to bottom or bottom to top in the tree, or down along the control flow graph, which might be cyclic.
*/
#include "Ast.h"
#include "GlobalVariable.h"
#include "ControlFlowEdge.h"

using namespace std;

Ast::Ast()
{
	// The index by which this node can be referred to from its parent's children vector. The root always has an index of -1
	indexAsChild = -1;
}

// Initializes an ID node
Ast::Ast(string variableName)
{
	name = config::ID_TOKEN_NAME;
	addChild(new Ast());
	children.at(0)->name = variableName;
}

// Initializes an ID node
Ast::Ast(int value)
{
	name = config::INT_TOKEN_NAME;
	addChild(new Ast());
	children.at(0)->name = to_string(value);
}

// Initializes a localAssign(ID(variableName), INT(initialValue)) node
Ast::Ast(string variableName, int initialValue)
{
	name = config::LOCAL_ASSIGN_TOKEN_NAME;

	addChild(new Ast(variableName));

	addChild(new Ast());
	children.at(1)->name = config::INT_TOKEN_NAME;
	children.at(1)->addChild(new Ast());
	children.at(1)->children.at(0)->name = to_string(initialValue);
}

// Initializes a localAssign(ID(variableName), node) node
Ast::Ast(string variableName, Ast* assignmentNode)
{
	name = config::LOCAL_ASSIGN_TOKEN_NAME;

	addChild(new Ast(variableName));

	addChild(assignmentNode);
}

// Initializes an ifElse(ifConditional, statements) node
Ast::Ast(Ast* ifConditionalNode, vector<Ast*> statements)
{
	name = config::IF_ELSE_TOKEN_NAME;

	addChild(ifConditionalNode);

	addChild(new Ast());
	children.at(1)->name = config::STATEMENTS_TOKEN_NAME;
	children.at(1)->addChildren(statements);

	addChild(new Ast());
	children.at(2)->name = config::NONE_TOKEN_NAME;
}

// Initializes an ifElse(ifConditional, ifStatements, elseStatements) node
Ast::Ast(Ast* ifConditionalNode, vector<Ast*> ifStatements, vector<Ast*> elseStatements)
{
	name = config::IF_ELSE_TOKEN_NAME;

	addChild(ifConditionalNode);

	addChild(new Ast());
	children.at(1)->name = config::STATEMENTS_TOKEN_NAME;
	children.at(1)->addChildren(ifStatements);

	addChild(new Ast());
	children.at(2)->name = config::STATEMENTS_TOKEN_NAME;
	children.at(2)->addChildren(elseStatements);
}

// Initializes an assume(conditional) node
Ast::Ast(Ast* assumeConditionalNode)
{
	name = config::ASSUME_TOKEN_NAME;

	addChild(assumeConditionalNode);
}

// Initializes a binary operation node
Ast::Ast(Ast* leftOperand, Ast* rightOperand, string operation)
{
	name = operation;

	addChild(leftOperand);
	addChild(rightOperand);
}

// Initializes a unary operation node
Ast::Ast(Ast* operand, string operation)
{
	name = operation;

	addChild(operand);
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
	if (config::currentLanguage == config::language::RMA)
	{
		return name == config::LABEL_TOKEN_NAME || name == config::GOTO_TOKEN_NAME || name == config::RMA_GET_TOKEN_NAME
			|| name == config::PSO_TSO_LOAD_TOKEN_NAME || name == config::IF_ELSE_TOKEN_NAME || name == config::FENCE_TOKEN_NAME
			|| name == config::RMA_PUT_TOKEN_NAME || name == config::LOCAL_ASSIGN_TOKEN_NAME || name == config::NOP_TOKEN_NAME
			|| name == config::FLUSH_TOKEN_NAME;
	}
	else
	{
		return name == config::LABEL_TOKEN_NAME || name == config::GOTO_TOKEN_NAME || name == config::PSO_TSO_STORE_TOKEN_NAME
			|| name == config::PSO_TSO_LOAD_TOKEN_NAME || name == config::IF_ELSE_TOKEN_NAME || name == config::FENCE_TOKEN_NAME
			|| name == config::NOP_TOKEN_NAME || name == config::FLUSH_TOKEN_NAME || name == config::LOCAL_ASSIGN_TOKEN_NAME;
	}
}

// Cascades from top to bottom. If for a read- or write-statement is encountered, the cascade stops and the IDs read and written by its children are gathered
void Ast::getCostsFromChildren()
{
	if (config::currentLanguage == config::language::RMA)
	{
		if (name == config::LOCAL_ASSIGN_TOKEN_NAME)
		{
			vector<string> writtenVariables = children.at(0)->getIDs();
			vector<string> readVariables = children.at(2)->getIDs();

			for (string member : writtenVariables)
			{
				incrementCost(member, 1, &causedWriteCost);
			}

			for (string member : readVariables)
			{
				incrementCost(member, 1, &causedReadCost);
			}
		}
		else if (name == config::RMA_PUT_TOKEN_NAME)
		{
			incrementCost(children.at(2)->name, 1, &causedWriteCost);
			incrementCost(children.at(4)->name, 1, &causedReadCost);
		}
		else if (name == config::RMA_GET_TOKEN_NAME)
		{
			incrementCost(children.at(0)->name, 1, &causedWriteCost);
			incrementCost(children.at(3)->name, 1, &causedReadCost);
		}
		else
		{
			for (Ast* child : children)
			{
				child->getCostsFromChildren();
			}
		}
	}
	else	// TSO/PSO: critical statements are store and load
	{
		if (name == config::PSO_TSO_STORE_TOKEN_NAME || name == config::PSO_TSO_LOAD_TOKEN_NAME)
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
		if (!bufferSizeMapCompare(&persistentWriteCost, &(edge->end->persistentWriteCost)))
		{
			result = true;
			additiveMergeBufferSizes(&topContainer, &(edge->end->persistentWriteCost));
		}
	}

	// Creates a map containing 0s for each non-TOP, and TOP for each TOP value in the persistent read cost map, and adds it to the successor's map if necessary
	copyAndSetNonTopToZero(&persistentReadCost, &topContainer);

	for (ControlFlowEdge* edge : outgoingEdges)
	{
		if (!bufferSizeMapCompare(&persistentReadCost, &(edge->end->persistentReadCost)))
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

void Ast::cascadingUnifyVariableNames()
{
	if (!unifyVariableNames())
	{
		for (Ast* child : children)
		{
			child->cascadingUnifyVariableNames();
		}
	}
}

void Ast::cascadingInitializeAuxiliaryVariables()
{
	initializeAuxiliaryVariables();

	for (Ast* child : children)
	{
		child->cascadingInitializeAuxiliaryVariables();
	}
}

int Ast::numberOfVariablesInPersistentWriteBuffer()
{
	int result = 0;

	for (bufferSizeMapIterator iterator = persistentWriteCost.begin(); iterator != persistentWriteCost.end(); iterator++)
	{
		if (iterator->second > 0 || iterator->second == config::TOP_VALUE)
		{
			result++;
		}
	}

	return result;
}

void Ast::carryOutReplacements()
{
	if (config::currentLanguage == config::language::RMA)
	{
		// TODO
		config::throwError("Auxiliary variable generation not implemented for RMA");
	}
	else
	{
		if (name == config::PROGRAM_DECLARATION_TOKEN_NAME)
		{
			for (Ast* child : children)
			{
				if (child->name == config::PROCESS_DECLARATION_TOKEN_NAME)
				{
					child->carryOutReplacements();
				}
			}
		}
		else if (name == config::PROCESS_DECLARATION_TOKEN_NAME)
		{
			vector<Ast*> toBeReplaced = children.at(1)->reportBack();

			for (Ast* statement : toBeReplaced)
			{
				additiveMergeBufferSizes(&(statement->persistentWriteCost), &persistentWriteCost);
			}
			
			int process = stoi(children.at(0)->children.at(0)->name);
			int currentMaximum;
			Ast* currentNode;
			vector<string> globalVariableNames = getAllKeys(&persistentWriteCost);

			for (string globalVariableName : globalVariableNames)
			{
				if (persistentWriteCost[globalVariableName] == config::TOP_VALUE || persistentWriteCost[globalVariableName] > config::K)
				{
					currentMaximum = config::K;
				}
				else
				{
					currentMaximum = persistentWriteCost[globalVariableName];
				}

				for (int ctr = currentMaximum; ctr > 0; ctr--)
				{
					currentNode = new Ast(config::globalVariables[globalVariableName]->auxiliaryWriteBufferVariableNames[pair<int, int>(process, ctr)], 0);
					currentNode->parent = children.at(1);
					children.at(1)->children.insert(children.at(1)->children.begin(), currentNode);
				}

				currentNode = new Ast(config::globalVariables[globalVariableName]->auxiliaryCounterVariableNames[process], 0);
				currentNode->parent = children.at(1);
				children.at(1)->children.insert(children.at(1)->children.begin(), currentNode);

				currentNode = new Ast(config::globalVariables[globalVariableName]->auxiliaryFirstPointerVariableNames[process], 0);
				currentNode->parent = children.at(1);
				children.at(1)->children.insert(children.at(1)->children.begin(), currentNode);
			}

			children.at(1)->refreshChildIndices();

			for (Ast* statement : toBeReplaced)
			{
				statement->carryOutReplacements();
			}
		}
		else if (name == config::PSO_TSO_STORE_TOKEN_NAME || name == config::PSO_TSO_LOAD_TOKEN_NAME ||
			name == config::FENCE_TOKEN_NAME || name == config::FLUSH_TOKEN_NAME)
		{
			vector<Ast*> replacement;

			if (name == config::PSO_TSO_STORE_TOKEN_NAME)
			{
				string globalVariableName = children.at(0)->children.at(0)->name;
				string processId;
				tryGetParentProcessNumber(&processId);
				int process = stoi(processId);
				string x_fst_t = config::globalVariables[globalVariableName]->auxiliaryFirstPointerVariableNames[process];
				string x_cnt_t = config::globalVariables[globalVariableName]->auxiliaryCounterVariableNames[process];

				vector<Ast*> statements;

				int s;
				if (persistentWriteCost[globalVariableName] == config::TOP_VALUE || persistentWriteCost[globalVariableName] > config::K)
				{
					s = config::K;

					Ast* abortAst = new Ast();
					abortAst->name = config::ABORT_TOKEN_NAME;
					abortAst->addChild(new Ast());
					abortAst->children.at(0)->name = config::OVERFLOW_MESSAGE;

					statements.push_back(abortAst);

					replacement.push_back(new Ast(
							new Ast(
									new Ast(x_cnt_t),
									new Ast(config::K),
									config::EQUALS
								),
							statements
						));

					statements.clear();
				}
				else
				{
					s = persistentWriteCost[globalVariableName];
				}

				if (persistentWriteCost[globalVariableName] == 1)
				{
					replacement.push_back(new Ast(x_fst_t, 1));
					replacement.push_back(new Ast(config::globalVariables[globalVariableName]->auxiliaryWriteBufferVariableNames[pair<int, int>(process, 1)], children.at(1)));
				}
				else
				{
					statements.push_back(new Ast(x_fst_t, 1));
					statements.push_back(new Ast(config::globalVariables[globalVariableName]->auxiliaryWriteBufferVariableNames[pair<int, int>(process, 1)], children.at(1)));

					replacement.push_back(new Ast(
							new Ast(
									new Ast(x_fst_t),
									new Ast(0),
									config::EQUALS
								),
							statements
						));
				}

				replacement.push_back(new Ast(
						x_cnt_t,
						new Ast(
							new Ast(x_cnt_t),
							new Ast(1),
							string(1, config::PLUS)
						)
					));

				for (int ctr = 2; ctr <= s; ctr++)
				{
					statements.clear();
					statements.push_back(new Ast(config::globalVariables[globalVariableName]->auxiliaryWriteBufferVariableNames[pair<int, int>(process, ctr)], children.at(1)));

					replacement.push_back(new Ast(
							new Ast(
								new Ast(x_cnt_t),
								new Ast(ctr),
								config::EQUALS
							),
							statements
						));
				}
			}
			else if (name == config::PSO_TSO_LOAD_TOKEN_NAME)
			{
				string globalVariableName = children.at(1)->children.at(0)->name;
				string processId;
				tryGetParentProcessNumber(&processId);
				int process = stoi(processId);
				string x_cnt_t = config::globalVariables[globalVariableName]->auxiliaryCounterVariableNames[process];

				int s;
				if (persistentWriteCost[globalVariableName] == config::TOP_VALUE || persistentWriteCost[globalVariableName] > config::K)
				{
					s = config::K;
				}
				else
				{
					s = persistentWriteCost[globalVariableName];
				}

				vector<Ast*> statements;
				statements.push_back(new Ast(children.at(0), children.at(1), config::PSO_TSO_LOAD_TOKEN_NAME));

				replacement.push_back(new Ast(
						new Ast(
							new Ast(x_cnt_t),
							new Ast(0),
							config::EQUALS
						),
						statements
					));

				for (int ctr = 1; ctr <= s; ctr++)
				{
					statements.clear();
					statements.push_back(new Ast(
						children.at(0),
						new Ast(config::globalVariables[globalVariableName]->auxiliaryWriteBufferVariableNames[pair<int, int>(process, ctr)]),
						config::LOCAL_ASSIGN_TOKEN_NAME
						));

					replacement.push_back(new Ast(
							new Ast(
								new Ast(x_cnt_t),
								new Ast(ctr),
								config::EQUALS
							),
							statements
						));
				}
			}
			else if (name == config::FENCE_TOKEN_NAME)
			{
				string processId;
				tryGetParentProcessNumber(&processId);
				int process = stoi(processId);
				vector<string> globalVariableNames = getAllKeys(&persistentWriteCost);
				string x_cnt_t;
				string x_fst_t;

				for (string globalVariableName : globalVariableNames)
				{
					x_fst_t = config::globalVariables[globalVariableName]->auxiliaryFirstPointerVariableNames[process];
					x_cnt_t = config::globalVariables[globalVariableName]->auxiliaryCounterVariableNames[process];

					replacement.push_back(new Ast(
							new Ast(
								new Ast(x_cnt_t),
								new Ast(0),
								config::EQUALS
							)
						));

					replacement.push_back(new Ast(
							new Ast(
								new Ast(x_fst_t),
								new Ast(0),
								config::EQUALS
							)
						));
				}
			}
			else if (name == config::FLUSH_TOKEN_NAME && numberOfVariablesInPersistentWriteBuffer() > 0)
			{
				string processId;
				tryGetParentProcessNumber(&processId);
				int process = stoi(processId);
				vector<string> globalVariableNames = getAllKeys(&persistentWriteCost);
				string x_cnt_t;
				string x_fst_t;
				srand(time(NULL));
				int uniqueLabel = rand();
				int s;
				Ast* currentIfElse;
				Ast* asteriskConditional;

				vector<Ast*> currentIfStatements;
				vector<Ast*> oldIfStatements;
				vector<Ast*> currentElseStatements;

				vector<Ast*> flushStatements;
				for (string globalVariableName : globalVariableNames)
				{
					if (persistentWriteCost[globalVariableName] == config::TOP_VALUE || persistentWriteCost[globalVariableName] > config::K)
					{
						s = config::K;
					}
					else
					{
						s = persistentWriteCost[globalVariableName];
					}

					x_fst_t = config::globalVariables[globalVariableName]->auxiliaryFirstPointerVariableNames[process];
					x_cnt_t = config::globalVariables[globalVariableName]->auxiliaryCounterVariableNames[process];

					for (int ctr = s - 1; ctr >= 1; ctr--)
					{
						if (currentIfStatements.empty())
						{
							currentIfStatements.push_back(new Ast(
									new Ast(globalVariableName),
									new Ast(config::globalVariables[globalVariableName]->auxiliaryWriteBufferVariableNames[pair<int, int>(process, ctr + 1)]),
									config::PSO_TSO_STORE_TOKEN_NAME
								));
						}
						else
						{
							currentIfStatements.clear();
							currentIfStatements.push_back(currentIfElse);
						}

						currentElseStatements.clear();

						currentElseStatements.push_back(new Ast(
								new Ast(globalVariableName),
								new Ast(config::globalVariables[globalVariableName]->auxiliaryWriteBufferVariableNames[pair<int, int>(process, ctr)]),
								config::PSO_TSO_STORE_TOKEN_NAME
							));

						currentIfElse = new Ast(
								new Ast(
									new Ast(x_fst_t),
									new Ast(ctr),
									string(1, config::GREATER_THAN)
								),
								currentIfStatements,
								currentElseStatements
							);
					}

					if (s >= 1)
					{
						asteriskConditional = new Ast();
						asteriskConditional->name = config::ASTERISK_TOKEN_NAME;

						currentIfStatements.clear();

						if (s > 1)
						{
							currentIfStatements.push_back(currentIfElse);
						}

						currentIfStatements.push_back(new Ast(
								x_fst_t,
								new Ast(
									new Ast(x_fst_t),
									new Ast(1),
									string(1, config::PLUS)
								)
							));

						if (numberOfVariablesInPersistentWriteBuffer() > 1)
						{
							currentIfElse = new Ast(
									asteriskConditional,
									currentIfStatements
								);

							currentIfStatements.clear();
							currentIfStatements.push_back(currentIfElse);
						}

						currentIfElse = new Ast(
								new Ast(
									new Ast(x_cnt_t),
									new Ast(0),
									string(1, config::GREATER_THAN)
								),
								currentIfStatements
							);

						flushStatements.push_back(currentIfElse);
						currentIfStatements.clear();
					}
				}

				if (!flushStatements.empty())
				{
					Ast* gotoNode = new Ast();
					gotoNode->addChild(new Ast());
					gotoNode->name = config::GOTO_TOKEN_NAME;
					gotoNode->children.at(0)->name = to_string(uniqueLabel);

					flushStatements.push_back(gotoNode);

					asteriskConditional = new Ast();
					asteriskConditional->name = config::ASTERISK_TOKEN_NAME;

					Ast* envelopingIfNode = new Ast(
							asteriskConditional,
							flushStatements
						);

					Ast* labelAst = new Ast();
					labelAst->name = config::LABEL_TOKEN_NAME;
					labelAst->addChild(new Ast());
					labelAst->children.at(0)->name = to_string(uniqueLabel);
					labelAst->addChild(envelopingIfNode);

					replacement.push_back(labelAst);
				}
			}

			if (!replacement.empty())
			{
				if (this->parent->name == config::LABEL_TOKEN_NAME)
				{
					this->parent->startComment = config::REPLACING_CAPTION + config::SPACE + config::QUOTATION + emitCode() + config::QUOTATION;
				}
				else
				{
					replacement.at(0)->startComment = config::REPLACING_CAPTION + config::SPACE + config::QUOTATION + emitCode() + config::QUOTATION;
				}
				replacement.at(replacement.size() - 1)->endComment = config::FINISHED_REPLACING_CAPTION + config::SPACE + config::QUOTATION + emitCode() + config::QUOTATION;
			}

			replaceNode(replacement, this);
		}
	}
}

// Returns whether the current node is an ifElse node with an Else statement block.
bool Ast::hasElse()
{
	if (name == config::IF_ELSE_TOKEN_NAME)
	{
		return children.at(2)->name != config::NONE_TOKEN_NAME;
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

			if (this->hasElse())	// If there is a non-empty Else block, connect to its first statement from the conditional
			{
				newEdge = new ControlFlowEdge(children.at(0), children.at(2)->children.at(0));
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

bool Ast::unifyVariableNames()
{
	if (name == config::ID_TOKEN_NAME)
	{
		string variableName = children.at(0)->name;

		if (find(config::variableNames.begin(), config::variableNames.end(), children.at(0)->name) == config::variableNames.end())
		{
			config::variableNames.push_back(variableName);
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
		if (config::currentLanguage == config::language::RMA)
		{
			for (Ast* processInitialization : children.at(0)->children)
			{
				if (processInitialization->name != config::RMA_PROCESS_INITIALIZATION_TOKEN_NAME)
				{
					break;
				}

				for (Ast* initializationStatement : processInitialization->children.at(1)->children)
				{
					additiveMergeBufferSizes(&(initializationStatement->causedWriteCost), &(persistentWriteCost));
					additiveMergeBufferSizes(&(initializationStatement->causedWriteCost), &(persistentReadCost));
				}
			}
		}
		else
		{
			for (Ast* initializationStatement : children.at(0)->children)
			{
				additiveMergeBufferSizes(&(initializationStatement->causedWriteCost), &(persistentWriteCost));
				additiveMergeBufferSizes(&(initializationStatement->causedWriteCost), &(persistentReadCost));
			}
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

// Spawns a control flow visitor on the first statement of every process declaration statement block
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

// Adds a child node and sets its variables connecting it to this node
void Ast::addChild(Ast* child)
{
	child->parent = this;
	child->indexAsChild = children.size();
	children.push_back(child);
}

void Ast::addChildren(vector<Ast*> newChildren)
{
	for (Ast* child : newChildren)
	{
		child->parent = this;
		child->indexAsChild = children.size();
		children.push_back(child);
	}
}

void Ast::refreshChildIndices()
{
	int childCount = children.size();

	for (int ctr = 0; ctr < childCount; ctr++)
	{
		children.at(ctr)->indexAsChild = ctr;
	}
}

// Returns the next child of the node's parent if it exists, otherwise returns NULL
Ast* Ast::tryGetNextSibling()
{
	if (!isRoot() && ((int)parent->children.size()) > indexAsChild + 1)
	{
		return parent->children.at(indexAsChild + 1);
	}

	return NULL;
}

// Returns the next statement node of the current statements block if applicable, otherwise returns NULL
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

// Returns whether the node has no parent
bool Ast::isRoot()
{
	return indexAsChild == -1;
}

// Outputs the number string of the superior process in the value of the out pointer and returns true if applicable, otherwise returns false
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

// Returns the last child node if it exists, otherwise returns NULL
Ast* Ast::tryGetLastChild()
{
	int childrenCount = children.size();

	if (childrenCount > 0)
	{
		return children.at(childrenCount - 1);
	}

	return NULL;
}

// Returns the last statement node of the current statements block if applicable, otherwise returns NULL
Ast* Ast::tryGetLastStatement()
{
	if (name == config::STATEMENTS_TOKEN_NAME)
	{
		Ast* lastChild = tryGetLastChild();

		if (lastChild->name == config::LABEL_TOKEN_NAME)
		{
			lastChild = lastChild->children.at(1);
		}

		return lastChild;
	}

	return NULL;
}

// Returns a string representation of the node and its children
string Ast::astToString()
{
	regex indentationRegex("\n");
	string result = name;

	if (isProgramPoint()) //|| (!isRoot() && parent->name == config::IF_ELSE_TOKEN_NAME))
	{
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

	return result;
}

string Ast::emitCode()
{
	string result = "";

	if (!startComment.empty())
	{
		result += config::COMMENT_PREFIX + config::SPACE + startComment + "\n";
	}

	if (config::currentLanguage == config::language::RMA)
	{
		if (name == config::PROGRAM_DECLARATION_TOKEN_NAME)
		{
			for (Ast* child : children)
			{
				result += child->emitCode() + "\n\n";
			}
		}
		else if (name == config::INITIALIZATION_BLOCK_TOKEN_NAME)
		{
			result += config::BEGINIT_TAG_NAME + "\n\n";

			for (Ast* child : children)
			{
				result += config::addTabs(child->emitCode(), 1) + "\n";
			}

			result += "\n" + config::ENDINIT_TAG_NAME;
		}
		else if (name == config::RMA_PROCESS_INITIALIZATION_TOKEN_NAME)
		{
			result += children.at(0)->emitCode() + "\n" + config::addTabs(children.at(1)->emitCode(), 1);
		}
		else if (name == config::PROCESS_DECLARATION_TOKEN_NAME)
		{
			result += children.at(0)->emitCode() + "\n\n" + config::addTabs(children.at(1)->emitCode(), 1);
		}
		else if (name == config::PROCESS_HEADER_TOKEN_NAME)
		{
			result += config::PROCESS_TAG_NAME + config::SPACE + children.at(0)->name + config::COLON;
		}
		else if (name == config::STATEMENTS_TOKEN_NAME)
		{
			for (Ast* child : children)
			{
				result += child->emitCode() + "\n";
			}

			if (!result.empty())
			{
				result = result.substr(0, result.length() - 1);
			}
		}
		else if (name == config::LABEL_TOKEN_NAME)
		{
			if (children.at(0)->name == config::IF_ELSE_TOKEN_NAME)
			{
				result.insert(result.begin(), '\n');
			}

			result += children.at(0)->name + config::COLON + config::SPACE + children.at(1)->emitCode();
		}
		else if (name == config::LOCAL_ASSIGN_TOKEN_NAME)
		{
			result += children.at(0)->emitCode() + config::SPACE + config::ASSIGN_OPERATOR + config::LEFT_PARENTHESIS + 
				children.at(1)->name + config::RIGHT_PARENTHESIS + config::SPACE + 
				children.at(2)->emitCode() + config::SEMICOLON;
		}
		else if (name == config::ID_TOKEN_NAME)
		{
			result += children.at(0)->name;
		}
		else if (name == config::INT_TOKEN_NAME)
		{
			result += children.at(0)->name;
		}
		else if (name == config::RMA_PUT_TOKEN_NAME)
		{
			result += config::RMA_PUT_TOKEN_NAME + config::LEFT_PARENTHESIS + children.at(0)->name +
				config::COMMA + config::SPACE + children.at(1)->name + config::RIGHT_PARENTHESIS +
				config::SPACE + config::LEFT_PARENTHESIS + children.at(2)->name + config::COMMA +
				config::SPACE + children.at(3)->name + config::COMMA + config::SPACE + children.at(4)->name +
				config::RIGHT_PARENTHESIS + config::SEMICOLON;
		}
		else if (name == config::RMA_GET_TOKEN_NAME)
		{
			result += children.at(0)->name + config::SPACE + config::EQUALS + config::SPACE +
				config::RMA_GET_TOKEN_NAME + config::LEFT_PARENTHESIS + children.at(1)->name +
				config::COMMA + config::SPACE + children.at(2)->name + config::RIGHT_PARENTHESIS +
				config::SPACE + config::LEFT_PARENTHESIS + children.at(3)->name + config::COMMA +
				config::SPACE + children.at(4)->name + config::RIGHT_PARENTHESIS + config::SEMICOLON;
		}
		else if (name == config::IF_ELSE_TOKEN_NAME)
		{
			if (parent->name != config::LABEL_TOKEN_NAME)
			{
				result.insert(result.begin(), '\n');
			}

			result += config::IF_TAG_NAME + config::SPACE + config::LEFT_PARENTHESIS +
				children.at(0)->emitCode() + config::RIGHT_PARENTHESIS + "\n" +
				config::addTabs(children.at(1)->emitCode(), 1) + "\n";

			if (children.at(2)->name != config::NONE_TOKEN_NAME)
			{
				result += config::ELSE_TAG_NAME + "\n" + config::addTabs(children.at(2)->emitCode(), 1) + "\n";
			}

			result += config::ENDIF_TAG_NAME + config::SEMICOLON;
		}
		else if (find(config::UNARY_OPERATORS.begin(), config::UNARY_OPERATORS.end(), name) != config::UNARY_OPERATORS.end())
		{
			result += name + config::LEFT_PARENTHESIS + children.at(0)->emitCode() + config::RIGHT_PARENTHESIS;
		}
		else if (find(config::BINARY_OPERATORS.begin(), config::BINARY_OPERATORS.end(), name) != config::BINARY_OPERATORS.end())
		{
			if (name == config::ASTERISK_TOKEN_NAME && children.size() == 0)
			{
				result += name;
			}
			else
			{
				if (children.at(0)->name == config::ID_TOKEN_NAME || children.at(0)->name == config::INT_TOKEN_NAME)
				{
					result += children.at(0)->children.at(0)->name;
				}
				else
				{
					result += config::LEFT_PARENTHESIS + children.at(0)->emitCode() + config::RIGHT_PARENTHESIS;
				}

				result += config::SPACE + name + config::SPACE;

				if (children.at(1)->name == config::ID_TOKEN_NAME || children.at(1)->name == config::INT_TOKEN_NAME)
				{
					result += children.at(1)->children.at(0)->name;
				}
				else
				{
					result += config::LEFT_PARENTHESIS + children.at(1)->emitCode() + config::RIGHT_PARENTHESIS;
				}
			}
		}
		else if (name == config::ABORT_TOKEN_NAME)
		{
			result += config::ABORT_TOKEN_NAME + config::LEFT_PARENTHESIS + config::QUOTATION +
				children.at(0)->name + config::QUOTATION + config::RIGHT_PARENTHESIS + config::SEMICOLON;
		}
		else if (name == config::FLUSH_TOKEN_NAME)
		{
			result += config::FLUSH_TOKEN_NAME + config::SEMICOLON;
		}
		else if (name == config::FENCE_TOKEN_NAME)
		{
			result += config::FENCE_TOKEN_NAME + config::SEMICOLON;
		}
		else if (name == config::GOTO_TOKEN_NAME)
		{
			result += config::GOTO_TOKEN_NAME + config::SPACE + children.at(0)->name + config::SEMICOLON;
		}
		else if (name == config::NOP_TOKEN_NAME)
		{
			result += config::NOP_TOKEN_NAME + config::SEMICOLON;
		}
		else if (name == config::ASSUME_TOKEN_NAME)
		{
			result += config::ASSUME_TOKEN_NAME + config::LEFT_PARENTHESIS + children.at(0)->emitCode() +
				config::RIGHT_PARENTHESIS + config::SEMICOLON;
		}
		else
		{
			config::throwError("Can't emit node: " + name);
		}
	}
	else
	{
		if (name == config::PROGRAM_DECLARATION_TOKEN_NAME)
		{
			for (Ast* child : children)
			{
				result += child->emitCode() + "\n\n";
			}
		}
		else if (name == config::INITIALIZATION_BLOCK_TOKEN_NAME)
		{
			result += config::BEGINIT_TAG_NAME + "\n\n";

			for (Ast* child : children)
			{
				result += "\t" + child->emitCode() + "\n";
			}

			result += "\n" + config::ENDINIT_TAG_NAME;
		}
		else if (name == config::PSO_TSO_STORE_TOKEN_NAME)
		{
			result += config::PSO_TSO_STORE_TOKEN_NAME + config::SPACE +
				children.at(0)->emitCode() + config::SPACE + config::ASSIGN_OPERATOR +
				config::SPACE + children.at(1)->emitCode() + config::SEMICOLON;
		}
		else if (name == config::PSO_TSO_LOAD_TOKEN_NAME)
		{
			result += config::PSO_TSO_LOAD_TOKEN_NAME + config::SPACE +
				children.at(0)->emitCode() + config::SPACE + config::ASSIGN_OPERATOR +
				config::SPACE + children.at(1)->emitCode() + config::SEMICOLON;
		}
		else if (name == config::LOCAL_ASSIGN_TOKEN_NAME)
		{
			result += children.at(0)->emitCode() + config::SPACE + config::ASSIGN_OPERATOR +
				config::SPACE + children.at(1)->emitCode() + config::SEMICOLON;
		}
		else if (name == config::ID_TOKEN_NAME)
		{
			result += children.at(0)->name;
		}
		else if (name == config::INT_TOKEN_NAME)
		{
			result += children.at(0)->name;
		}
		else if (name == config::PROCESS_DECLARATION_TOKEN_NAME)
		{
			result += children.at(0)->emitCode() + "\n\n" + config::addTabs(children.at(1)->emitCode(), 1);
		}
		else if (name == config::PROCESS_HEADER_TOKEN_NAME)
		{
			result += config::PROCESS_TAG_NAME + config::SPACE + children.at(0)->name + config::COLON;
		}
		else if (name == config::STATEMENTS_TOKEN_NAME)
		{
			for (Ast* child : children)
			{
				result += child->emitCode() + "\n";
			}

			if (!result.empty())
			{
				result = result.substr(0, result.length() - 1);
			}
		}
		else if (name == config::LABEL_TOKEN_NAME)
		{
			if (children.at(0)->name == config::IF_ELSE_TOKEN_NAME)
			{
				result.insert(result.begin(), '\n');
			}

			result += children.at(0)->name + config::COLON + config::SPACE + children.at(1)->emitCode();
		}
		else if (name == config::IF_ELSE_TOKEN_NAME)
		{
			if (parent->name != config::LABEL_TOKEN_NAME)
			{
				result.insert(result.begin(), '\n');
			}

			result += config::IF_TAG_NAME + config::SPACE + config::LEFT_PARENTHESIS +
				children.at(0)->emitCode() + config::RIGHT_PARENTHESIS + "\n" +
				config::addTabs(children.at(1)->emitCode(), 1) + "\n";

			if (children.at(2)->name != config::NONE_TOKEN_NAME)
			{
				result += config::ELSE_TAG_NAME + "\n" + config::addTabs(children.at(2)->emitCode(), 1) + "\n";
			}

			result += config::ENDIF_TAG_NAME + config::SEMICOLON;
		}
		else if (find(config::UNARY_OPERATORS.begin(), config::UNARY_OPERATORS.end(), name) != config::UNARY_OPERATORS.end())
		{
			result += name + config::LEFT_PARENTHESIS + children.at(0)->emitCode() + config::RIGHT_PARENTHESIS;
		}
		else if (find(config::BINARY_OPERATORS.begin(), config::BINARY_OPERATORS.end(), name) != config::BINARY_OPERATORS.end())
		{
			if (name == config::ASTERISK_TOKEN_NAME && children.size() == 0)
			{
				result += name;
			}
			else
			{
				if (children.at(0)->name == config::ID_TOKEN_NAME || children.at(0)->name == config::INT_TOKEN_NAME)
				{
					result += children.at(0)->children.at(0)->name;
				}
				else
				{
					result += config::LEFT_PARENTHESIS + children.at(0)->emitCode() + config::RIGHT_PARENTHESIS;
				}

				result += config::SPACE + name + config::SPACE;

				if (children.at(1)->name == config::ID_TOKEN_NAME || children.at(1)->name == config::INT_TOKEN_NAME)
				{
					result += children.at(1)->children.at(0)->name;
				}
				else
				{
					result += config::LEFT_PARENTHESIS + children.at(1)->emitCode() + config::RIGHT_PARENTHESIS;
				}
			}
		}
		else if (name == config::ABORT_TOKEN_NAME)
		{
			result += config::ABORT_TOKEN_NAME + config::LEFT_PARENTHESIS + config::QUOTATION +
				children.at(0)->name + config::QUOTATION + config::RIGHT_PARENTHESIS + config::SEMICOLON;
		}
		else if (name == config::FLUSH_TOKEN_NAME)
		{
			result += config::FLUSH_TOKEN_NAME + config::SEMICOLON;
		}
		else if (name == config::FENCE_TOKEN_NAME)
		{
			result += config::FENCE_TOKEN_NAME + config::SEMICOLON;
		}
		else if (name == config::GOTO_TOKEN_NAME)
		{
			result += config::GOTO_TOKEN_NAME + config::SPACE + children.at(0)->name + config::SEMICOLON;
		}
		else if (name == config::NOP_TOKEN_NAME)
		{
			result += config::NOP_TOKEN_NAME + config::SEMICOLON;
		}
		else if (name == config::ASSUME_TOKEN_NAME)
		{
			result += config::ASSUME_TOKEN_NAME + config::LEFT_PARENTHESIS + children.at(0)->emitCode() +
				config::RIGHT_PARENTHESIS + config::SEMICOLON;
		}
		else
		{
			config::throwError("Can't emit node: " + name);
		}
	}

	if (!endComment.empty())
	{
		result += "\n" + config::COMMENT_PREFIX + config::SPACE + endComment;
	}

	if (name == config::IF_ELSE_TOKEN_NAME)
	{
		result += "\n";
	}

	return result;
}

string Ast::emitBooleanCode()
{
	string result = "";

	if (config::currentLanguage == config::language::RMA)
	{
		if (name == config::PROGRAM_DECLARATION_TOKEN_NAME)
		{
			for (Ast* child : children)
			{
				result += child->emitCode() + "\n\n";
			}
		}
		else if (name == config::INITIALIZATION_BLOCK_TOKEN_NAME)
		{
			result += config::BEGINIT_TAG_NAME + "\n\n";

			for (Ast* child : children)
			{
				result += config::addTabs(child->emitCode(), 1) + "\n";
			}

			result += "\n" + config::ENDINIT_TAG_NAME;
		}
		else if (name == config::RMA_PROCESS_INITIALIZATION_TOKEN_NAME)
		{
			result += children.at(0)->emitCode() + "\n" + config::addTabs(children.at(1)->emitCode(), 1);
		}
		else if (name == config::PROCESS_DECLARATION_TOKEN_NAME)
		{
			result += children.at(0)->emitCode() + "\n\n" + config::addTabs(children.at(1)->emitCode(), 1);
		}
		else if (name == config::PROCESS_HEADER_TOKEN_NAME)
		{
			result += config::PROCESS_TAG_NAME + config::SPACE + children.at(0)->name + config::COLON;
		}
		else if (name == config::STATEMENTS_TOKEN_NAME)
		{
			for (Ast* child : children)
			{
				result += child->emitCode() + "\n";
			}

			if (!result.empty())
			{
				result = result.substr(0, result.length() - 1);
			}
		}
		else if (name == config::LABEL_TOKEN_NAME)
		{
			if (children.at(0)->name == config::IF_ELSE_TOKEN_NAME)
			{
				result.insert(result.begin(), '\n');
			}

			result += children.at(0)->name + config::COLON + config::SPACE + children.at(1)->emitCode();
		}
		else if (name == config::LOCAL_ASSIGN_TOKEN_NAME)
		{
			result += children.at(0)->emitCode() + config::SPACE + config::ASSIGN_OPERATOR + config::LEFT_PARENTHESIS +
				children.at(1)->name + config::RIGHT_PARENTHESIS + config::SPACE +
				children.at(2)->emitCode() + config::SEMICOLON;
		}
		else if (name == config::ID_TOKEN_NAME)
		{
			result += children.at(0)->name;
		}
		else if (name == config::INT_TOKEN_NAME)
		{
			result += children.at(0)->name;
		}
		else if (name == config::RMA_PUT_TOKEN_NAME)
		{
			result += config::RMA_PUT_TOKEN_NAME + config::LEFT_PARENTHESIS + children.at(0)->name +
				config::COMMA + config::SPACE + children.at(1)->name + config::RIGHT_PARENTHESIS +
				config::SPACE + config::LEFT_PARENTHESIS + children.at(2)->name + config::COMMA +
				config::SPACE + children.at(3)->name + config::COMMA + config::SPACE + children.at(4)->name +
				config::RIGHT_PARENTHESIS + config::SEMICOLON;
		}
		else if (name == config::RMA_GET_TOKEN_NAME)
		{
			result += children.at(0)->name + config::SPACE + config::EQUALS + config::SPACE +
				config::RMA_GET_TOKEN_NAME + config::LEFT_PARENTHESIS + children.at(1)->name +
				config::COMMA + config::SPACE + children.at(2)->name + config::RIGHT_PARENTHESIS +
				config::SPACE + config::LEFT_PARENTHESIS + children.at(3)->name + config::COMMA +
				config::SPACE + children.at(4)->name + config::RIGHT_PARENTHESIS + config::SEMICOLON;
		}
		else if (name == config::IF_ELSE_TOKEN_NAME)
		{
			if (parent->name != config::LABEL_TOKEN_NAME)
			{
				result.insert(result.begin(), '\n');
			}

			result += config::IF_TAG_NAME + config::SPACE + config::LEFT_PARENTHESIS +
				children.at(0)->emitCode() + config::RIGHT_PARENTHESIS + "\n" +
				config::addTabs(children.at(1)->emitCode(), 1) + "\n";

			if (children.at(2)->name != config::NONE_TOKEN_NAME)
			{
				result += config::ELSE_TAG_NAME + "\n" + config::addTabs(children.at(2)->emitCode(), 1) + "\n";
			}

			result += config::ENDIF_TAG_NAME + config::SEMICOLON;
		}
		else if (find(config::UNARY_OPERATORS.begin(), config::UNARY_OPERATORS.end(), name) != config::UNARY_OPERATORS.end())
		{
			result += name + config::LEFT_PARENTHESIS + children.at(0)->emitCode() + config::RIGHT_PARENTHESIS;
		}
		else if (find(config::BINARY_OPERATORS.begin(), config::BINARY_OPERATORS.end(), name) != config::BINARY_OPERATORS.end())
		{
			if (name == config::ASTERISK_TOKEN_NAME && children.size() == 0)
			{
				result += name;
			}
			else
			{
				if (children.at(0)->name == config::ID_TOKEN_NAME || children.at(0)->name == config::INT_TOKEN_NAME)
				{
					result += children.at(0)->children.at(0)->name;
				}
				else
				{
					result += config::LEFT_PARENTHESIS + children.at(0)->emitCode() + config::RIGHT_PARENTHESIS;
				}

				result += config::SPACE + name + config::SPACE;

				if (children.at(1)->name == config::ID_TOKEN_NAME || children.at(1)->name == config::INT_TOKEN_NAME)
				{
					result += children.at(1)->children.at(0)->name;
				}
				else
				{
					result += config::LEFT_PARENTHESIS + children.at(1)->emitCode() + config::RIGHT_PARENTHESIS;
				}
			}
		}
		else if (name == config::ABORT_TOKEN_NAME)
		{
			result += config::ABORT_TOKEN_NAME + config::LEFT_PARENTHESIS + config::QUOTATION +
				children.at(0)->name + config::QUOTATION + config::RIGHT_PARENTHESIS + config::SEMICOLON;
		}
		else if (name == config::FLUSH_TOKEN_NAME)
		{
			result += config::FLUSH_TOKEN_NAME + config::SEMICOLON;
		}
		else if (name == config::FENCE_TOKEN_NAME)
		{
			result += config::FENCE_TOKEN_NAME + config::SEMICOLON;
		}
		else if (name == config::GOTO_TOKEN_NAME)
		{
			result += config::GOTO_TOKEN_NAME + config::SPACE + children.at(0)->name + config::SEMICOLON;
		}
		else if (name == config::NOP_TOKEN_NAME)
		{
			result += config::NOP_TOKEN_NAME + config::SEMICOLON;
		}
		else if (name == config::ASSUME_TOKEN_NAME)
		{
			result += config::ASSUME_TOKEN_NAME + config::LEFT_PARENTHESIS + children.at(0)->emitCode() +
				config::RIGHT_PARENTHESIS + config::SEMICOLON;
		}
		else
		{
			config::throwError("Can't emit node: " + name);
		}
	}
	else
	{
		if (name == config::PROGRAM_DECLARATION_TOKEN_NAME)
		{
			for (Ast* child : children)
			{
				if (child->name == config::PROCESS_DECLARATION_TOKEN_NAME)
				{
					if (!result.empty())
					{
						result += "\n";
					}

					result += z3code::push(stoi(child->children.at(0)->name));
				}
			}
		}
		else if (name == config::INITIALIZATION_BLOCK_TOKEN_NAME)
		{
			//
		}
		// Output (ite (weakestPrecondition(left = right, predicate)) (true) ((ite (weakestPrecondition(left = right, !predicate)) (false) (Unknown))))
		else if (name == config::PSO_TSO_STORE_TOKEN_NAME)
		{
			for (z3code::Predicate predicate : config::predicates)
			{

			}

			result += z3code::ite(
					z3code::weakestPrecondition(
						new z3code::Predicate(
							z3code::booleanOperator.EQUALS,
							new z3code::Term(children.at(0)),
							new z3code::Term(children.at(1))
						)
					),
					(true),
					((ite(weakestPrecondition(left = right, !predicate)) (false) (Unknown)))
				);
		}
		else if (name == config::PSO_TSO_LOAD_TOKEN_NAME)
		{
			result += config::PSO_TSO_LOAD_TOKEN_NAME + config::SPACE +
				children.at(0)->emitCode() + config::SPACE + config::ASSIGN_OPERATOR +
				config::SPACE + children.at(1)->emitCode() + config::SEMICOLON;
		}
		else if (name == config::LOCAL_ASSIGN_TOKEN_NAME)
		{
			result += children.at(0)->emitCode() + config::SPACE + config::ASSIGN_OPERATOR +
				config::SPACE + children.at(1)->emitCode() + config::SEMICOLON;
		}
		else if (name == config::ID_TOKEN_NAME)
		{
			result += children.at(0)->name;
		}
		else if (name == config::INT_TOKEN_NAME)
		{
			result += children.at(0)->name;
		}
		else if (name == config::PROCESS_DECLARATION_TOKEN_NAME)
		{
			result += children.at(0)->emitCode() + "\n\n" + config::addTabs(children.at(1)->emitCode(), 1);
		}
		else if (name == config::PROCESS_HEADER_TOKEN_NAME)
		{
			result += config::PROCESS_TAG_NAME + config::SPACE + children.at(0)->name + config::COLON;
		}
		else if (name == config::STATEMENTS_TOKEN_NAME)
		{
			for (Ast* child : children)
			{
				result += child->emitCode() + "\n";
			}

			if (!result.empty())
			{
				result = result.substr(0, result.length() - 1);
			}
		}
		else if (name == config::LABEL_TOKEN_NAME)
		{
			if (children.at(0)->name == config::IF_ELSE_TOKEN_NAME)
			{
				result.insert(result.begin(), '\n');
			}

			result += children.at(0)->name + config::COLON + config::SPACE + children.at(1)->emitCode();
		}
		else if (name == config::IF_ELSE_TOKEN_NAME)
		{
			if (parent->name != config::LABEL_TOKEN_NAME)
			{
				result.insert(result.begin(), '\n');
			}

			result += config::IF_TAG_NAME + config::SPACE + config::LEFT_PARENTHESIS +
				children.at(0)->emitCode() + config::RIGHT_PARENTHESIS + "\n" +
				config::addTabs(children.at(1)->emitCode(), 1) + "\n";

			if (children.at(2)->name != config::NONE_TOKEN_NAME)
			{
				result += config::ELSE_TAG_NAME + "\n" + config::addTabs(children.at(2)->emitCode(), 1) + "\n";
			}

			result += config::ENDIF_TAG_NAME + config::SEMICOLON;
		}
		else if (find(config::UNARY_OPERATORS.begin(), config::UNARY_OPERATORS.end(), name) != config::UNARY_OPERATORS.end())
		{
			result += name + config::LEFT_PARENTHESIS + children.at(0)->emitCode() + config::RIGHT_PARENTHESIS;
		}
		else if (find(config::BINARY_OPERATORS.begin(), config::BINARY_OPERATORS.end(), name) != config::BINARY_OPERATORS.end())
		{
			if (name == config::ASTERISK_TOKEN_NAME && children.size() == 0)
			{
				result += name;
			}
			else
			{
				if (children.at(0)->name == config::ID_TOKEN_NAME || children.at(0)->name == config::INT_TOKEN_NAME)
				{
					result += children.at(0)->children.at(0)->name;
				}
				else
				{
					result += config::LEFT_PARENTHESIS + children.at(0)->emitCode() + config::RIGHT_PARENTHESIS;
				}

				result += config::SPACE + name + config::SPACE;

				if (children.at(1)->name == config::ID_TOKEN_NAME || children.at(1)->name == config::INT_TOKEN_NAME)
				{
					result += children.at(1)->children.at(0)->name;
				}
				else
				{
					result += config::LEFT_PARENTHESIS + children.at(1)->emitCode() + config::RIGHT_PARENTHESIS;
				}
			}
		}
		else if (name == config::ABORT_TOKEN_NAME)
		{
			result += config::ABORT_TOKEN_NAME + config::LEFT_PARENTHESIS + config::QUOTATION +
				children.at(0)->name + config::QUOTATION + config::RIGHT_PARENTHESIS + config::SEMICOLON;
		}
		else if (name == config::FLUSH_TOKEN_NAME)
		{
			result += config::FLUSH_TOKEN_NAME + config::SEMICOLON;
		}
		else if (name == config::FENCE_TOKEN_NAME)
		{
			result += config::FENCE_TOKEN_NAME + config::SEMICOLON;
		}
		else if (name == config::GOTO_TOKEN_NAME)
		{
			result += config::GOTO_TOKEN_NAME + config::SPACE + children.at(0)->name + config::SEMICOLON;
		}
		else if (name == config::NOP_TOKEN_NAME)
		{
			result += config::NOP_TOKEN_NAME + config::SEMICOLON;
		}
		else if (name == config::ASSUME_TOKEN_NAME)
		{
			result += config::ASSUME_TOKEN_NAME + config::LEFT_PARENTHESIS + children.at(0)->emitCode() +
				config::RIGHT_PARENTHESIS + config::SEMICOLON;
		}
		else
		{
			config::throwError("Can't emit node: " + name);
		}
	}

	return result;
}

int Ast::effectiveMaxWriteBufferSize(string variableName)
{
	if (bufferSizeMapContains(&persistentWriteCost, variableName))
	{
		int maxBufferSize = persistentWriteCost[variableName];

		if (maxBufferSize == config::TOP_VALUE || maxBufferSize > config::K)
		{
			return config::K;
		}
		else
		{
			return maxBufferSize;
		}
	}
	else
	{
		return config::UNDEFINED_VALUE;
	}
}

void Ast::replaceNode(vector<Ast*> nodes, Ast* oldNode)
{
	Ast* newParent = oldNode->parent;
	int newIndex = oldNode->indexAsChild;
	int nodesCount = nodes.size();
	int parentChildrenCount = newParent->children.size();

	newParent->children.erase(newParent->children.begin() + newIndex);

	for (int ctr = newIndex + nodesCount - 1; ctr >= newIndex; ctr--)
	{
		nodes[ctr - newIndex]->parent = newParent;
		newParent->children.insert(newParent->children.begin() + newIndex, nodes[ctr - newIndex]);
	}

	newParent->refreshChildIndices();
}

void Ast::initializeAuxiliaryVariables()
{
	if (config::currentLanguage == config::language::RMA)
	{
		// TODO
		config::throwError("Auxiliary variable generation not implemented for RMA");
	}
	else
	{
		if (name == config::PROGRAM_DECLARATION_TOKEN_NAME)
		{
			vector<string> globalVariableNames = getAllKeys(&persistentWriteCost);
			vector<int> processes;

			for (Ast* child : children)
			{
				if (child->name == config::PROCESS_DECLARATION_TOKEN_NAME)
				{
					processes.push_back(stoi(child->children.at(0)->children.at(0)->name));
				}
			}

			for (string globalVariableName : globalVariableNames)
			{
				config::globalVariables[globalVariableName] = new GlobalVariable(globalVariableName, processes);
			}
		}
	}
}

vector<Ast*> Ast::reportBack()
{
	vector<Ast*> result;
	vector<Ast*> subResult;

	if (name == config::PSO_TSO_STORE_TOKEN_NAME || name == config::PSO_TSO_LOAD_TOKEN_NAME ||
		name == config::FENCE_TOKEN_NAME || name == config::FLUSH_TOKEN_NAME)
	{
		result.push_back(this);
	}
	else if (children.size() > 0)
	{
		for (Ast* child : children)
		{
			subResult = child->reportBack();

			for (Ast* reported : subResult)
			{
				result.push_back(reported);
			}
		}
	}

	return result;
}