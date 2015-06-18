/*
A node of the abstract semantic tree. Its type is contained in the name variable, unless it's an identifier's name or an integer value.
It can hold buffer size maps containing the buffer size increases caused by the program point it's representing, and the buffer size
the program would at most need at this point. May also contain directed edges to other AST nodes, representing control flow.

Cascading operations may go top to bottom or bottom to top in the tree, or down along the control flow graph, which might be cyclic.
*/

#include "Ast.h"
#include "literalCode.h"
#include "config.h"
#include "ControlFlowVisitor.h"
#include "GlobalVariable.h"
#include "VariableEntry.h"
#include "ControlFlowEdge.h"
#include "PredicateData.h"
//#include "Cube.h"
#include "CubeTreeNode.h"

using namespace std;

/* Private fields */
	int Ast::getParentProcessNumber()
	{
		Ast* currentNode = this;

		while (currentNode != nullptr)
		{
			if (currentNode->getName() == literalCode::PROCESS_DECLARATION_TOKEN_NAME ||
				currentNode->getName() == literalCode::BL_PROCESS_DECLARATION_TOKEN_NAME)
			{
				return stoi(currentNode->getChild(0)->getChild(0)->getName());
			}
			else
			{
				currentNode = currentNode->getParent();
			}
		}

		return -1;
	}

/* Local methods */
	// Sets all buffer size map entry values to 0
	void Ast::resetBufferSizes()
	{
		bufferSizeMap* containers[] = { &causedWriteCost, &causedReadCost, &persistentWriteCost, &persistentReadCost };

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
		bsm::copyBufferSizes(&(source->persistentWriteCost), &persistentWriteCost);
		bsm::copyBufferSizes(&(source->persistentReadCost), &persistentReadCost);
	}

	// Checks whether the node is a program point and has any control flow successors, and if so, creates control flow edges between this node and its successors.
	bool Ast::generateOutgoingEdges()
	{
		if (isProgramPoint())	// Checks whether the node is a program point
		{
			ControlFlowEdge* newEdge;
			Ast* nextStatement = nullptr;

			if (_name == literalCode::GOTO_TOKEN_NAME)	// Goto nodes only lead to their corresponding label
			{
				newEdge = new ControlFlowEdge(this, config::labelLookupMap[_children[0]->getName()]);
				outgoingEdges.push_back(newEdge);
			}
			else if (_name == literalCode::LABEL_TOKEN_NAME)	// Label nodes only lead to their corresponding statement
			{
				newEdge = new ControlFlowEdge(this, _children[1]);
				outgoingEdges.push_back(newEdge);
			}
			else if (_name == literalCode::IF_ELSE_TOKEN_NAME)	// IfElse nodes lead to their conditionals, which lead to both statement blocks' first statements,
			{												// of which the last statements lead to the statement following the IfElse block, unless they're goto statements
				Ast* lastChild = nullptr;
				nextStatement = tryGetNextStatement();
				bool nextStatementExists = nextStatement != nullptr;

				newEdge = new ControlFlowEdge(this, _children[0]);	// Create an edge to the conditional
				outgoingEdges.push_back(newEdge);

				newEdge = new ControlFlowEdge(_children[0], _children[1]->getChild(0));	// Create an edge from the conditional to the first statement of the If block
				_children[0]->outgoingEdges.push_back(newEdge);

				lastChild = _children[1]->tryGetLastStatement();		// If there is a statement after the IfElse block, connect to it from the last statement of the If block, if it's not a goto statement
				if (nextStatementExists && lastChild != nullptr && lastChild->getName() != literalCode::GOTO_TOKEN_NAME)
				{
					newEdge = new ControlFlowEdge(lastChild, nextStatement);
					lastChild->outgoingEdges.push_back(newEdge);
				}

				if (_children[2]->getName().compare(literalCode::NONE_TOKEN_NAME) != 0)		// If there is a non-empty Else block, connect to its first statement from the conditional
				{
					newEdge = new ControlFlowEdge(_children[0], _children[2]->getChild(0));
					_children[0]->outgoingEdges.push_back(newEdge);

					lastChild = _children[2]->tryGetLastStatement();		// If there is a statement after the IfElse block, connect to it from the last statement of the Else block, if it's not a goto statement
					if (nextStatementExists && lastChild != nullptr && lastChild->getName() != literalCode::GOTO_TOKEN_NAME)
					{
						newEdge = new ControlFlowEdge(lastChild, nextStatement);
						lastChild->outgoingEdges.push_back(newEdge);
					}
				}
				else if (nextStatementExists)	// If there is no Else block, but there is a statement following the IfElse block, connect to it from the conditional
				{
					newEdge = new ControlFlowEdge(_children[0], nextStatement);
					_children[0]->outgoingEdges.push_back(newEdge);
				}
			}
			else if ((nextStatement = tryGetNextStatement()) != nullptr)	// For other program points, follow the statement block if it continues
			{
				newEdge = new ControlFlowEdge(this, nextStatement);
				outgoingEdges.push_back(newEdge);
			}

			return true;
		}

		return false;
	}

	// Returns whether the node represents one of the program point types
	bool Ast::isProgramPoint()
	{
		if (config::currentLanguage == config::language::RMA)
		{
			return config::stringVectorContains(literalCode::RMA_PROGRAM_POINT_TOKENS, _name);
		}
		else
		{
			return config::stringVectorContains(literalCode::PSO_TSO_PROGRAM_POINT_TOKENS, _name);
		}
	}

	// Returns the next statement node of the current statements block if applicable, otherwise returns nullptr
	Ast* Ast::tryGetNextStatement()
	{
		if (_parent->getName() == literalCode::LABEL_TOKEN_NAME)
		{
			return _parent->tryGetNextSibling();
		}
		else if (_parent->getName() == literalCode::STATEMENTS_TOKEN_NAME)
		{
			return tryGetNextSibling();
		}

		return nullptr;
	}

	// Returns the last statement node of the current statements block if applicable, otherwise returns nullptr
	Ast* Ast::tryGetLastStatement()
	{
		if (_name == literalCode::STATEMENTS_TOKEN_NAME)
		{
			Ast* lastChild = tryGetLastChild();

			if (lastChild->getName() == literalCode::LABEL_TOKEN_NAME)
			{
				lastChild = lastChild->getChild(1);
			}

			return lastChild;
		}

		return nullptr;
	}

	// Returns the next child of the node's parent if it exists, otherwise returns nullptr
	Ast* Ast::tryGetNextSibling()
	{
		if (_parent != nullptr && (_parent->getChildrenCount()) > getIndexAsChild() + 1)
		{
			return _parent->getChild(getIndexAsChild() + 1);
		}

		return nullptr;
	}

	// Returns the last child node if it exists, otherwise returns nullptr
	Ast* Ast::tryGetLastChild()
	{
		if (_childrenCount > 0)
		{
			return _children[_childrenCount - 1];
		}

		return nullptr;
	}

	bool Ast::performPredicateAbstraction()
	{
		bool result = false;
		int firstLabel = config::getCurrentAuxiliaryLabel();

		if (_parent != nullptr)
		{
			if (_parent->getName() == literalCode::LABEL_TOKEN_NAME)
			{
				firstLabel = stoi(_parent->getChild(0)->getName());
			}
		}

		if (config::currentLanguage == config::language::RMA)
		{
			// TODO
			config::throwError("Predicate abstraction operations not implemented for RMA");
		}
		else
		{
			if (_name == literalCode::PROGRAM_DECLARATION_TOKEN_NAME)
			{
				_name = literalCode::BL_PROGRAM_DECLARATION_TOKEN_NAME;

				vector<string> auxiliaryBooleanVariableNames;
				vector<string> auxiliaryTemporaryVariableNames;

				for (PredicateData* globalPredicate : config::globalPredicates)
				{
					auxiliaryBooleanVariableNames.push_back(globalPredicate->getSingleBooleanVariableName());
					auxiliaryTemporaryVariableNames.push_back(globalPredicate->getSingleTemporaryVariableName());
				}

				Ast* sharedVars = newSharedVariables(auxiliaryBooleanVariableNames);
				Ast* localVars = newLocalVariables(auxiliaryTemporaryVariableNames);

				insertChild(localVars, 0);
				insertChild(sharedVars, 0);
			}
			else if (_name == literalCode::INITIALIZATION_BLOCK_TOKEN_NAME)
			{
				_name = literalCode::BL_INITIALIZATION_BLOCK_TOKEN_NAME;
			}
			else if (_name == literalCode::PROCESS_DECLARATION_TOKEN_NAME)
			{
				_name = literalCode::BL_PROCESS_DECLARATION_TOKEN_NAME;
			}
			else if (_name == literalCode::PSO_TSO_STORE_TOKEN_NAME || _name == literalCode::PSO_TSO_LOAD_TOKEN_NAME
				|| _name == literalCode::LOCAL_ASSIGN_TOKEN_NAME)
			{
				cout << "\tPerforming predicate abstraction on: " << getCode() << "\t\t\t \n\n";

				vector<int> relevantPredicateIndices = config::getRelevantAuxiliaryBooleanVariableIndices(_children[0]->
					getChild(0)->getName());
				vector<Ast*> replacementStatements;

				replacementStatements.push_back(newLabel(firstLabel, newNop()));

				if (relevantPredicateIndices.size() > 0)
				{
					int numberOfPredicates = config::globalPredicates.size();
					Ast* positiveWeakestLiberalPrecondition;
					Ast* negativeWeakestLiberalPrecondition;
					vector<Ast*> parallelAssignments;
					vector<string> usedVariables;
					vector<string> currentIDs;

					for (int ctr : relevantPredicateIndices)
					{
						positiveWeakestLiberalPrecondition = config::getWeakestLiberalPrecondition(this,
							config::globalPredicates[ctr]->getPredicateAst());
						negativeWeakestLiberalPrecondition = config::getWeakestLiberalPrecondition(this,
							config::globalPredicates[ctr]->getPredicateAst()->negate());

						parallelAssignments.push_back(
							newStore(
									config::globalPredicates[ctr]->getSingleBooleanVariableName(),
									newAbstractAssignmentFragment(this, config::globalPredicates[ctr]->getPredicateAst())
								)
							);
					}

					relevantPredicateIndices = config::getRelevantAuxiliaryTemporaryVariableIndices(parallelAssignments);

					replacementStatements.push_back(newBeginAtomic());

					for (int ctr : relevantPredicateIndices)
					{
						replacementStatements.push_back(
								newLoad(
									config::globalPredicates[ctr]->getSingleTemporaryVariableName(),
									newID(config::globalPredicates[ctr]->getSingleBooleanVariableName())
								)
							);
					}

					replacementStatements.insert(replacementStatements.end(), parallelAssignments.begin(),
						parallelAssignments.end());

					for (int ctr : relevantPredicateIndices)
					{
						replacementStatements.push_back(
								newLocalAssign(config::globalPredicates[ctr]->getSingleTemporaryVariableName(), newINT(0))
							);
					}

					replacementStatements.push_back(
						config::getAssumptionOfNegatedLargestFalseImplicativeDisjunctionOfCubes());
					replacementStatements.push_back(Ast::newEndAtomic());
				}

				replacementStatements[0]->setStartComment(literalCode::PREDICATE_ABSTRACTION_COMMENT_PREFIX + getCode());

				if (_parent->getName() == literalCode::LABEL_TOKEN_NAME)
				{
					config::prepareNodeForLazyReplacement(replacementStatements, _parent);
				}
				else
				{
					config::prepareNodeForLazyReplacement(replacementStatements, this);
				}

				result = true;
			}
			else if (_name == literalCode::ASSERT_TOKEN_NAME)
			{
				vector<Ast*> nonPcAssertions;
				vector<Ast*> currentReplacement;
				Ast* leftSide;
				Ast* rightSide;
				Ast* currentAssertion;
				nonPcAssertions.push_back(_children[0]);

				for (int ctr = 0; ctr < nonPcAssertions.size(); ctr++)
				{
					currentAssertion = nonPcAssertions.at(0);
					leftSide = currentAssertion->getChild(0);
					rightSide = currentAssertion->getChild(1);

					if (currentAssertion->getName() == literalCode::AND ||
						currentAssertion->getName() == literalCode::OR ||
						currentAssertion->getName() == literalCode::DOUBLE_AND ||
						currentAssertion->getName() == literalCode::DOUBLE_OR)
					{
						if (currentAssertion->getName() == literalCode::AND)
						{
							currentAssertion->setName(literalCode::DOUBLE_AND);
						}
						else if (currentAssertion->getName() == literalCode::OR)
						{
							currentAssertion->setName(literalCode::DOUBLE_OR);
						}

						nonPcAssertions.push_back(leftSide);
						nonPcAssertions.push_back(rightSide);
						nonPcAssertions.erase(nonPcAssertions.begin() + ctr);
						ctr--;
					}
					else if (config::stringVectorContains(currentAssertion->getIDs(), literalCode::PC_TOKEN_NAME))
					{
						nonPcAssertions.erase(nonPcAssertions.begin() + ctr);
						ctr--;
					}
				}

				for (Ast* nonPcAssertion : nonPcAssertions)
				{
					currentReplacement.clear();
					currentReplacement.push_back(newLargestImplicativeDisjunctionOfCubes(
						config::globalCubeSizeLimit, nonPcAssertion, false));
					config::prepareNodeForLazyReplacement(currentReplacement, nonPcAssertion);
				}

				result = true;
			}
		}

		return result;
	}

/* Constructors and destructor */
	Ast::Ast(const string &name)
	{
		_name = name;
		_code = "";
		_codeIsValid = false;
		_parent = nullptr;
		_childrenCount = 0;
	}

	Ast::~Ast()
	{
		for (Ast* child : _children)
		{
			delete child;
		}

		for (ControlFlowEdge* outgoingEdge : outgoingEdges)
		{
			delete outgoingEdge;
		}
	}

/* Public fields */
	const string Ast::getName()
	{
		return _name;
	}

	void Ast::setName(const string value)
	{
		_name = value;
		invalidateCode();
	}

	const string Ast::getCode()
	{
		if (!_codeIsValid)
		{
			string result = "";
			bool isBooleanProgramNode = false;
			int childrenCount;

			if (_name == literalCode::BL_PROGRAM_DECLARATION_TOKEN_NAME)
			{
				for (Ast* child : _children)
				{
					result += child->getCode() + "\n\n";
				}

				isBooleanProgramNode = true;
			}
			else if (_name == literalCode::BL_INITIALIZATION_BLOCK_TOKEN_NAME)
			{
				string subResult = "";

				for (Ast* child : _children)
				{
					subResult += child->getCode() + "\n";
				}

				if (!subResult.empty())
				{
					subResult = subResult.substr(0, subResult.length() - 1);
				}

				result += literalCode::INIT_TAG_NAME + "\n\n" + config::addTabs(subResult, 1) + "\n\n";

				isBooleanProgramNode = true;
			}
			else if (_name == literalCode::BL_SHARED_VARIABLES_BLOCK_TOKEN_NAME)
			{
				result += literalCode::BL_SHARED_VARIABLES_BLOCK_TOKEN_NAME;

				for (Ast* child : _children)
				{
					result += " " + child->getCode() + ",";
				}

				if (result.size() > literalCode::BL_SHARED_VARIABLES_BLOCK_TOKEN_NAME.size())
				{
					result[result.size() - 1] = literalCode::SEMICOLON;
				}
				else
				{
					result += literalCode::SEMICOLON;
				}

				isBooleanProgramNode = true;
			}
			else if (_name == literalCode::BL_LOCAL_VARIABLES_BLOCK_TOKEN_NAME)
			{
				result += literalCode::BL_LOCAL_VARIABLES_BLOCK_TOKEN_NAME;

				for (Ast* child : _children)
				{
					result += " " + child->getCode() + ",";
				}

				if (result.size() > literalCode::BL_LOCAL_VARIABLES_BLOCK_TOKEN_NAME.size())
				{
					result[result.size() - 1] = literalCode::SEMICOLON;
				}
				else
				{
					result += literalCode::SEMICOLON;
				}

				isBooleanProgramNode = true;
			}
			else if (_name == literalCode::BL_PROCESS_DECLARATION_TOKEN_NAME)
			{
				result += literalCode::PROCESS_TAG_NAME + " " + _children.at(0)->getChild(0)->getName() + "\n\n" +
					config::addTabs(_children.at(1)->getCode(), 1);

				isBooleanProgramNode = true;
			}
			else if (_name == literalCode::BL_IF_TOKEN_NAME)
			{
				result += literalCode::IF_TAG_NAME + literalCode::SPACE + literalCode::LEFT_PARENTHESIS +
					_children.at(0)->getCode() + literalCode::RIGHT_PARENTHESIS + literalCode::SPACE + _children.at(1)->getCode();

				isBooleanProgramNode = true;
			}
			else if (_name == literalCode::ASSERT_TOKEN_NAME)
			{
				result += literalCode::ASSERT_TOKEN_NAME + literalCode::SPACE + literalCode::BL_ALWAYS_TOKEN_NAME +
					literalCode::LEFT_PARENTHESIS + _children.at(0)->getCode() + literalCode::RIGHT_PARENTHESIS +
					literalCode::SEMICOLON;

				isBooleanProgramNode = true;
			}

			if (!_startComment.empty())
			{
				result += "\n" + literalCode::MULTILINE_COMMENT_PREFIX + literalCode::SPACE + _startComment + literalCode::SPACE +
					literalCode::MULTILINE_COMMENT_SUFFIX + "\n";
			}

			if (config::currentLanguage == config::language::RMA)
			{
				if (_name == literalCode::PROGRAM_DECLARATION_TOKEN_NAME)
				{
					for (Ast* child : _children)
					{
						result += child->getCode() + "\n\n";
					}
				}
				else if (_name == literalCode::INITIALIZATION_BLOCK_TOKEN_NAME)
				{
					result += literalCode::BEGINIT_TAG_NAME + "\n\n";

					for (Ast* child : _children)
					{
						result += config::addTabs(child->getCode(), 1) + "\n";
					}

					result += "\n" + literalCode::ENDINIT_TAG_NAME;
				}
				else if (_name == literalCode::RMA_PROCESS_INITIALIZATION_TOKEN_NAME)
				{
					result += _children.at(0)->getCode() + "\n" + config::addTabs(_children.at(1)->getCode(), 1);
				}
				else if (_name == literalCode::PROCESS_DECLARATION_TOKEN_NAME)
				{
					result += _children.at(0)->getCode() + "\n\n" + config::addTabs(_children.at(1)->getCode(), 1);
				}
				else if (_name == literalCode::PROCESS_HEADER_TOKEN_NAME)
				{
					result += literalCode::PROCESS_TAG_NAME + literalCode::SPACE + _children.at(0)->getName() + literalCode::COLON;
				}
				else if (_name == literalCode::STATEMENTS_TOKEN_NAME)
				{
					for (Ast* child : _children)
					{
						result += child->getCode() + "\n";
					}

					if (!result.empty())
					{
						result = result.substr(0, result.length() - 1);
					}
				}
				else if (_name == literalCode::LABEL_TOKEN_NAME)
				{
					if (_children.at(0)->getName() == literalCode::IF_ELSE_TOKEN_NAME)
					{
						result.insert(result.begin(), '\n');
					}

					result += _children.at(0)->getName() + literalCode::COLON + literalCode::SPACE + _children.at(1)->getCode();
				}
				else if (_name == literalCode::LOCAL_ASSIGN_TOKEN_NAME)
				{
					result += _children.at(0)->getCode() + literalCode::SPACE + literalCode::ASSIGN_OPERATOR + literalCode::LEFT_PARENTHESIS +
						_children.at(1)->getName() + literalCode::RIGHT_PARENTHESIS + literalCode::SPACE +
						_children.at(2)->getCode() + literalCode::SEMICOLON;
				}
				else if (_name == literalCode::ID_TOKEN_NAME)
				{
					if (_children.at(0)->getName() == literalCode::PC_TOKEN_NAME)
					{
						result += literalCode::PC_TOKEN_NAME + literalCode::LEFT_CURLY_BRACKET + _children.at(0)->getChild(0)->getName() +
							literalCode::RIGHT_CURLY_BRACKET;
					}
					else
					{
						result += _children.at(0)->getName();
					}
				}
				else if (_name == literalCode::INT_TOKEN_NAME)
				{
					result += _children.at(0)->getName();
				}
				else if (_name == literalCode::BOOL_TOKEN_NAME)
				{
					result += _children.at(0)->getName();
				}
				else if (_name == literalCode::RMA_PUT_TOKEN_NAME)
				{
					result += literalCode::RMA_PUT_TOKEN_NAME + literalCode::LEFT_PARENTHESIS + _children.at(0)->getName() +
						literalCode::COMMA + literalCode::SPACE + _children.at(1)->getName() + literalCode::RIGHT_PARENTHESIS +
						literalCode::SPACE + literalCode::LEFT_PARENTHESIS + _children.at(2)->getName() + literalCode::COMMA +
						literalCode::SPACE + _children.at(3)->getName() + literalCode::COMMA + literalCode::SPACE + _children.at(4)->getName() +
						literalCode::RIGHT_PARENTHESIS + literalCode::SEMICOLON;
				}
				else if (_name == literalCode::RMA_GET_TOKEN_NAME)
				{
					result += _children.at(0)->getName() + literalCode::SPACE + literalCode::EQUALS + literalCode::SPACE +
						literalCode::RMA_GET_TOKEN_NAME + literalCode::LEFT_PARENTHESIS + _children.at(1)->getName() +
						literalCode::COMMA + literalCode::SPACE + _children.at(2)->getName() + literalCode::RIGHT_PARENTHESIS +
						literalCode::SPACE + literalCode::LEFT_PARENTHESIS + _children.at(3)->getName() + literalCode::COMMA +
						literalCode::SPACE + _children.at(4)->getName() + literalCode::RIGHT_PARENTHESIS + literalCode::SEMICOLON;
				}
				else if (_name == literalCode::IF_ELSE_TOKEN_NAME)
				{
					if (_parent != nullptr)
					{
						if (_parent->getName() != literalCode::LABEL_TOKEN_NAME)
						{
							result.insert(result.begin(), '\n');
						}
					}

					result += literalCode::IF_TAG_NAME + literalCode::SPACE + literalCode::LEFT_PARENTHESIS +
						_children.at(0)->getCode() + literalCode::RIGHT_PARENTHESIS + "\n" +
						config::addTabs(_children.at(1)->getCode(), 1) + "\n";

					if (_children.at(2)->getName() != literalCode::NONE_TOKEN_NAME)
					{
						result += literalCode::ELSE_TAG_NAME + "\n" + config::addTabs(_children.at(2)->getCode(), 1) + "\n";
					}

					result += literalCode::ENDIF_TAG_NAME + literalCode::SEMICOLON;
				}
				else if (find(literalCode::UNARY_OPERATORS.begin(), literalCode::UNARY_OPERATORS.end(), _name) != literalCode::UNARY_OPERATORS.end())
				{
					result += _name + literalCode::LEFT_PARENTHESIS + _children.at(0)->getCode() + literalCode::RIGHT_PARENTHESIS;
				}
				else if (find(literalCode::BINARY_OPERATORS.begin(), literalCode::BINARY_OPERATORS.end(), _name) != literalCode::BINARY_OPERATORS.end())
				{
					if (_name == literalCode::ASTERISK_TOKEN_NAME && _children.size() == 0)
					{
						result += _name;
					}
					else
					{
						childrenCount = _children.size();

						for (int ctr = 0; ctr < childrenCount; ctr++)
						{
							if (_children.at(ctr)->getName() == literalCode::ID_TOKEN_NAME ||
								_children.at(ctr)->getName() == literalCode::INT_TOKEN_NAME)
							{
								result += _children.at(ctr)->getCode();
							}
							else
							{
								result += literalCode::LEFT_PARENTHESIS + _children.at(ctr)->getCode() + literalCode::RIGHT_PARENTHESIS;
							}

							if (ctr < childrenCount - 1)
							{
								result += literalCode::SPACE + _name + literalCode::SPACE;
							}
						}
					}
				}
				else if (_name == literalCode::ABORT_TOKEN_NAME)
				{
					result += literalCode::ABORT_TOKEN_NAME + literalCode::LEFT_PARENTHESIS + literalCode::QUOTATION +
						_children.at(0)->getName() + literalCode::QUOTATION + literalCode::RIGHT_PARENTHESIS + literalCode::SEMICOLON;
				}
				else if (_name == literalCode::FLUSH_TOKEN_NAME)
				{
					result += literalCode::FLUSH_TOKEN_NAME + literalCode::SEMICOLON;
				}
				else if (_name == literalCode::FENCE_TOKEN_NAME)
				{
					result += literalCode::FENCE_TOKEN_NAME + literalCode::SEMICOLON;
				}
				else if (_name == literalCode::GOTO_TOKEN_NAME)
				{
					result += literalCode::GOTO_TOKEN_NAME + literalCode::SPACE + _children.at(0)->getName() + literalCode::SEMICOLON;
				}
				else if (_name == literalCode::NOP_TOKEN_NAME)
				{
					result += literalCode::NOP_TOKEN_NAME + literalCode::SEMICOLON;
				}
				else if (_name == literalCode::ASSUME_TOKEN_NAME)
				{
					result += literalCode::ASSUME_TOKEN_NAME + literalCode::LEFT_PARENTHESIS + _children.at(0)->getCode() +
						literalCode::RIGHT_PARENTHESIS + literalCode::SEMICOLON;
				}
				else if (!isBooleanProgramNode)
				{
					config::throwError("Can't emit node: " + _name);
				}
			}
			else
			{
				if (_name == literalCode::PROGRAM_DECLARATION_TOKEN_NAME)
				{
					for (Ast* child : _children)
					{
						result += child->getCode() + "\n\n";
					}
				}
				else if (_name == literalCode::INITIALIZATION_BLOCK_TOKEN_NAME)
				{
					result += literalCode::BEGINIT_TAG_NAME + "\n\n";

					for (Ast* child : _children)
					{
						result += "\t" + child->getCode() + "\n";
					}

					result += "\n" + literalCode::ENDINIT_TAG_NAME;
				}
				else if (_name == literalCode::PSO_TSO_STORE_TOKEN_NAME)
				{
					result += literalCode::PSO_TSO_STORE_TOKEN_NAME + literalCode::SPACE +
						_children.at(0)->getCode() + literalCode::SPACE + literalCode::ASSIGN_OPERATOR +
						literalCode::SPACE + _children.at(1)->getCode() + literalCode::SEMICOLON;
				}
				else if (_name == literalCode::PSO_TSO_LOAD_TOKEN_NAME)
				{
					result += literalCode::PSO_TSO_LOAD_TOKEN_NAME + literalCode::SPACE +
						_children.at(0)->getCode() + literalCode::SPACE + literalCode::ASSIGN_OPERATOR +
						literalCode::SPACE + _children.at(1)->getCode() + literalCode::SEMICOLON;
				}
				else if (_name == literalCode::LOCAL_ASSIGN_TOKEN_NAME)
				{
					result += _children.at(0)->getCode() + literalCode::SPACE + literalCode::ASSIGN_OPERATOR +
						literalCode::SPACE + _children.at(1)->getCode() + literalCode::SEMICOLON;
				}
				else if (_name == literalCode::ID_TOKEN_NAME)
				{
					if (_children.at(0)->getName() == literalCode::PC_TOKEN_NAME)
					{
						result += literalCode::PC_TOKEN_NAME + literalCode::LEFT_CURLY_BRACKET + _children.at(0)->getChild(0)->getName() +
							literalCode::RIGHT_CURLY_BRACKET;
					}
					else
					{
						result += _children.at(0)->getName();
					}
				}
				else if (_name == literalCode::INT_TOKEN_NAME)
				{
					result += _children.at(0)->getName();
				}
				else if (_name == literalCode::BOOL_TOKEN_NAME)
				{
					result += _children.at(0)->getName();
				}
				else if (_name == literalCode::PROCESS_DECLARATION_TOKEN_NAME)
				{
					result += _children.at(0)->getCode() + "\n\n" + config::addTabs(_children.at(1)->getCode(), 1);
				}
				else if (_name == literalCode::PROCESS_HEADER_TOKEN_NAME)
				{
					result += literalCode::PROCESS_TAG_NAME + literalCode::SPACE + _children.at(0)->getName() + literalCode::COLON;
				}
				else if (_name == literalCode::STATEMENTS_TOKEN_NAME)
				{
					for (Ast* child : _children)
					{
						result += child->getCode() + "\n";
					}

					if (!result.empty())
					{
						result = result.substr(0, result.length() - 1);
					}
				}
				else if (_name == literalCode::LABEL_TOKEN_NAME)
				{
					if (_children.at(0)->getName() == literalCode::IF_ELSE_TOKEN_NAME)
					{
						result.insert(result.begin(), '\n');
					}

					result += _children.at(0)->getName() + literalCode::COLON + literalCode::SPACE + _children.at(1)->getCode();
				}
				else if (_name == literalCode::IF_ELSE_TOKEN_NAME)
				{
					if (_parent != nullptr)
					{
						if (_parent->getName() != literalCode::LABEL_TOKEN_NAME)
						{
							result.insert(result.begin(), '\n');
						}
					}

					result += literalCode::IF_TAG_NAME + literalCode::SPACE + literalCode::LEFT_PARENTHESIS +
						_children.at(0)->getCode() + literalCode::RIGHT_PARENTHESIS + "\n" +
						config::addTabs(_children.at(1)->getCode(), 1) + "\n";

					if (_children.at(2)->getName() != literalCode::NONE_TOKEN_NAME)
					{
						result += literalCode::ELSE_TAG_NAME + "\n" + config::addTabs(_children.at(2)->getCode(), 1) + "\n";
					}

					result += literalCode::ENDIF_TAG_NAME + literalCode::SEMICOLON;
				}
				else if (find(literalCode::UNARY_OPERATORS.begin(), literalCode::UNARY_OPERATORS.end(), _name) !=
					literalCode::UNARY_OPERATORS.end())
				{
					result += _name + literalCode::LEFT_PARENTHESIS + _children.at(0)->getCode() + literalCode::RIGHT_PARENTHESIS;
				}
				else if (find(literalCode::BINARY_OPERATORS.begin(), literalCode::BINARY_OPERATORS.end(), _name) !=
					literalCode::BINARY_OPERATORS.end())
				{
					if (_name == literalCode::ASTERISK_TOKEN_NAME && _children.size() == 0)
					{
						result += _name;
					}
					else
					{

						childrenCount = _children.size();

						for (int ctr = 0; ctr < childrenCount; ctr++)
						{
							if (_children.at(ctr)->getName() == literalCode::ID_TOKEN_NAME || _children.at(ctr)->getName() ==
								literalCode::INT_TOKEN_NAME)
							{
								result += _children.at(ctr)->getCode();
							}
							else
							{
								result += literalCode::LEFT_PARENTHESIS + _children.at(ctr)->getCode() + literalCode::RIGHT_PARENTHESIS;
							}

							if (ctr < childrenCount - 1)
							{
								result += literalCode::SPACE + _name + literalCode::SPACE;
							}
						}
					}
				}
				else if (_name == literalCode::ABORT_TOKEN_NAME)
				{
					result += literalCode::ABORT_TOKEN_NAME + literalCode::LEFT_PARENTHESIS + literalCode::QUOTATION +
						_children.at(0)->getName() + literalCode::QUOTATION + literalCode::RIGHT_PARENTHESIS + literalCode::SEMICOLON;
				}
				else if (_name == literalCode::FLUSH_TOKEN_NAME)
				{
					result += literalCode::FLUSH_TOKEN_NAME + literalCode::SEMICOLON;
				}
				else if (_name == literalCode::FENCE_TOKEN_NAME)
				{
					result += literalCode::FENCE_TOKEN_NAME + literalCode::SEMICOLON;
				}
				else if (_name == literalCode::GOTO_TOKEN_NAME)
				{
					result += literalCode::GOTO_TOKEN_NAME + literalCode::SPACE + _children.at(0)->getName() + literalCode::SEMICOLON;
				}
				else if (_name == literalCode::NOP_TOKEN_NAME)
				{
					result += literalCode::NOP_TOKEN_NAME + literalCode::SEMICOLON;
				}
				else if (_name == literalCode::ASSUME_TOKEN_NAME)
				{
					result += literalCode::ASSUME_TOKEN_NAME + literalCode::LEFT_PARENTHESIS + _children.at(0)->getCode() +
						literalCode::RIGHT_PARENTHESIS + literalCode::SEMICOLON;
				}
				else if (_name == literalCode::BEGIN_ATOMIC_TOKEN_NAME)
				{
					result += literalCode::BEGIN_ATOMIC_TOKEN_NAME + literalCode::SEMICOLON;
				}
				else if (_name == literalCode::END_ATOMIC_TOKEN_NAME)
				{
					result += literalCode::END_ATOMIC_TOKEN_NAME + literalCode::SEMICOLON;
				}
				else if (_name == literalCode::CHOOSE_TOKEN_NAME)
				{
					result += literalCode::CHOOSE_TOKEN_NAME + literalCode::LEFT_PARENTHESIS + _children.at(0)->getCode() +
						literalCode::COMMA + literalCode::SPACE + _children.at(1)->getCode() +
						literalCode::RIGHT_PARENTHESIS;
				}
				else if (!isBooleanProgramNode)
				{
					config::throwError("Can't emit node: " + _name);
				}
			}

			if (!_endComment.empty())
			{
				result += "\n" + literalCode::MULTILINE_COMMENT_PREFIX + literalCode::SPACE + _endComment + literalCode::SPACE +
					literalCode::MULTILINE_COMMENT_SUFFIX + "\n";
			}

			if (_name == literalCode::IF_ELSE_TOKEN_NAME)
			{
				result += "\n";
			}

			_code = result;
			_codeIsValid = true;
		}

		return _code;
	}

	Ast* Ast::getParent()
	{
		return _parent;
	}

	void Ast::setParent(Ast* newParent)
	{
		_parent = newParent;
	}

	Ast* Ast::getChild(int index)
	{
		if (index < 0 || index >= _children.size())
		{
			config::throwCriticalError("Invalid child index: " + to_string(index) + " for Ast node {" + getCode() + "}");
		}

		_childrenCount = _children.size();
		return _children[index];
	}

	const vector<Ast*> Ast::getChildren()
	{
		return _children;
	}

	void Ast::insertChild(Ast* newChild)
	{
		newChild->setParent(this);
		_children.push_back(newChild);
		invalidateCode();

		_childrenCount = _children.size();
	}

	void Ast::insertChild(Ast* newChild, int index)
	{
		newChild->setParent(this);
		_children.insert(_children.begin() + index, newChild);
		invalidateCode();

		_childrenCount = _children.size();
	}

	void Ast::insertChildren(const vector<Ast*> &newChildren)
	{
		for (Ast* newChild : newChildren)
		{
			newChild->setParent(this);
			_children.push_back(newChild);
		}

		invalidateCode();

		_childrenCount = _children.size();
	}

	void Ast::insertChildren(const vector<Ast*> &newChildren, int index)
	{
		for (Ast* newChild : newChildren)
		{
			newChild->setParent(this);
		}

		_children.insert(_children.begin() + index, newChildren.begin(), newChildren.end());

		invalidateCode();
	}

	void Ast::deleteChild(int index)
	{
		//Ast* toBeDeleted = _children[index];
		_children.erase(_children.begin() + index);
		//delete toBeDeleted;

		invalidateCode();

		_childrenCount = _children.size();
	}

	void Ast::replaceChild(Ast* newChild, int index)
	{
		//cout << "\tReplacing \"" << getCode() << "\" by \"" << newChild->getCode() << "\"\n";

		//Ast* toBeDeleted = _children[index];
		_children.erase(_children.begin() + index);
		//delete toBeDeleted;

		newChild->setParent(this);
		_children.insert(_children.begin() + index, newChild);

		invalidateCode();

		_childrenCount = _children.size();
	}

	void Ast::replaceChild(const vector<Ast*> &newChildren, int index)
	{
		/*cout << "\tReplacing \"" << getCode() << "\" by \"";
		for (Ast* newChild : newChildren)
		{
			cout << "\n\t" << newChild->getCode();
		}
		cout << "\"\n";*/

		//Ast* toBeDeleted = _children[index];
		_children.erase(_children.begin() + index);
		//delete toBeDeleted;

		for (Ast* newChild : newChildren)
		{
			newChild->setParent(this);
		}

		_children.insert(_children.begin() + index, newChildren.begin(), newChildren.end());

		invalidateCode();

		_childrenCount = _children.size();
	}

	int Ast::getChildrenCount()
	{
		return _childrenCount;
	}

	const string Ast::getStartComment()
	{
		return _startComment;
	}

	void Ast::setStartComment(const string &value)
	{
		_startComment = value;
	}

	const string Ast::getEndComment()
	{
		return _endComment;
	}

	void Ast::setEndComment(const string &value)
	{
		_endComment = value;
	}

	int Ast::getIndexAsChild()
	{
		int childrenOfParentCount = _parent->getChildrenCount();

		for (int ctr = 0; ctr < childrenOfParentCount; ctr++)
		{
			if (_parent->getChild(ctr) == this)
			{
				return ctr;
			}
		}

		return -1;
	}

	z3::expr Ast::getZ3Expression(z3::context* c)
	{
		if (config::currentLanguage == config::language::RMA)
		{
			// TODO
			config::throwError("Z3 expression conversion not implemented for RMA");
		}
		else
		{
			if (_name == literalCode::ID_TOKEN_NAME)
			{
				return c->int_const(_children[0]->_name.c_str());
			}
			else if (_name == literalCode::INT_TOKEN_NAME)
			{
				return c->int_val(_children[0]->_name.c_str());
			}
			else if (_name == literalCode::BOOL_TOKEN_NAME)
			{
				return c->bool_const(_children[0]->_name.c_str());
			}
			else if (_name == literalCode::BINARY_OPERATORS[0])	// +
			{
				return _children[0]->getZ3Expression(c) + _children[1]->getZ3Expression(c);
			}
			else if (_name == literalCode::BINARY_OPERATORS[1])	// -
			{
				return _children[0]->getZ3Expression(c) - _children[1]->getZ3Expression(c);
			}
			else if (_name == literalCode::BINARY_OPERATORS[2])	// *
			{
				return _children[0]->getZ3Expression(c) * _children[1]->getZ3Expression(c);
			}
			else if (_name == literalCode::BINARY_OPERATORS[3])	// /
			{
				return _children[0]->getZ3Expression(c) / _children[1]->getZ3Expression(c);
			}
			else if (_name == literalCode::BINARY_OPERATORS[4])	// &
			{
				return _children[0]->getZ3Expression(c) && _children[1]->getZ3Expression(c);
			}
			else if (_name == literalCode::BINARY_OPERATORS[5])	// |
			{
				return _children[0]->getZ3Expression(c) || _children[1]->getZ3Expression(c);
			}
			else if (_name == literalCode::BINARY_OPERATORS[6])	// <
			{
				return _children[0]->getZ3Expression(c) < _children[1]->getZ3Expression(c);
			}
			else if (_name == literalCode::BINARY_OPERATORS[7])	// >
			{
				return _children[0]->getZ3Expression(c) > _children[1]->getZ3Expression(c);
			}
			else if (_name == literalCode::BINARY_OPERATORS[8])	// <=
			{
				return _children[0]->getZ3Expression(c) <= _children[1]->getZ3Expression(c);
			}
			else if (_name == literalCode::BINARY_OPERATORS[9])	// >=
			{
				return _children[0]->getZ3Expression(c) >= _children[1]->getZ3Expression(c);
			}
			else if (_name == literalCode::BINARY_OPERATORS[10])	// =
			{
				// Ignore
			}
			else if (_name == literalCode::BINARY_OPERATORS[11])	// ==
			{
				return _children[0]->getZ3Expression(c) == _children[1]->getZ3Expression(c);
			}
			else if (_name == literalCode::BINARY_OPERATORS[12])	// !=
			{
				return _children[0]->getZ3Expression(c) != _children[1]->getZ3Expression(c);
			}
			else if (_name == literalCode::UNARY_OPERATORS[0])	// !
			{
				return !_children[0]->getZ3Expression(c);
			}
		}
		
		// If all else fails
		config::throwCriticalError("Can't convert the following to a Z3 expression:\n" + getCode());
		return c->int_const("INVALID");
	}

/* Public methods */
	void Ast::invalidateCode()
	{
		_codeIsValid = false;

		if (_parent != nullptr)
		{
			_parent->invalidateCode();
		}
	}

	// Spawns a control flow visitor on the first statement of every process declaration statement block
	void Ast::visitAllProgramPoints()
	{
		if (_name == literalCode::PROGRAM_DECLARATION_TOKEN_NAME)
		{
			Ast* currentProcessDeclarationStatements;
			ControlFlowVisitor* newControlFlowVisitor;

			for (int ctr = 1; ctr < _childrenCount; ctr++)
			{
				if (_children[ctr]->getName() == literalCode::PROCESS_DECLARATION_TOKEN_NAME)
				{
					currentProcessDeclarationStatements = _children[ctr]->getChild(1);

					if (currentProcessDeclarationStatements->getChildrenCount() > 0)
					{
						newControlFlowVisitor = new ControlFlowVisitor;
						newControlFlowVisitor->traverseControlFlowGraph(currentProcessDeclarationStatements->getChild(0));
					}
				}
			}
		}
	}

	void Ast::unfoldIfElses()
	{
		assert(_name == literalCode::STATEMENTS_TOKEN_NAME);

		// of the format vector<pair<ifElse node, pair<hasElse, hasLabel ? label : "">>>
		vector<Ast*> replacementStatements;
		vector<Ast*> currentStatements;
		Ast* conditional;
		Ast* firstStatement;
		int elseLabel;
		int endLabel;

		vector<pair<Ast*, pair<bool, string>>> ifElseStatements;
		Ast* currentIfElseStatement;
		bool hasElse;
		string label;
		string currentName;

		for (Ast* child : _children)
		{
			currentName = child->getName();

			if (currentName == literalCode::IF_ELSE_TOKEN_NAME ||
				(currentName == literalCode::LABEL_TOKEN_NAME &&
				child->getChild(1)->getName() == literalCode::IF_ELSE_TOKEN_NAME))
			{
				if (currentName == literalCode::IF_ELSE_TOKEN_NAME)
				{
					currentIfElseStatement = child;
					label = "";
				}
				else
				{
					currentIfElseStatement = child->getChild(1);
					label = child->getChild(0)->getName();
				}

				hasElse = currentIfElseStatement->getChild(2)->getName() == literalCode::STATEMENTS_TOKEN_NAME;

				ifElseStatements.push_back(pair<Ast*, pair<bool, string>>(currentIfElseStatement,
					pair<bool, string>(hasElse, label)));
			}
		}

		for (pair<Ast*, pair<bool, string>> ifElseStatementData : ifElseStatements)
		{
			currentIfElseStatement = ifElseStatementData.first;
			hasElse = ifElseStatementData.second.first;

			currentIfElseStatement->getChild(1)->unfoldIfElses();

			if (hasElse)
			{
				currentIfElseStatement->getChild(2)->unfoldIfElses();
			}
		}

		for (pair<Ast*, pair<bool, string>> ifElseStatementData : ifElseStatements)
		{
			currentIfElseStatement = ifElseStatementData.first;
			hasElse = ifElseStatementData.second.first;
			label = ifElseStatementData.second.second;

			conditional = currentIfElseStatement->getChild(0);
			elseLabel = hasElse ? config::getCurrentAuxiliaryLabel() : -1;
			endLabel = config::getCurrentAuxiliaryLabel();

			replacementStatements.clear();
			currentStatements.clear();

			replacementStatements.push_back(newAssume(
					newReverseLargestImplicativeDisjunctionOfCubes(config::globalCubeSizeLimit, conditional))
				);

			currentStatements = currentIfElseStatement->getChild(1)->getChildren();
			replacementStatements.insert(replacementStatements.end(), currentStatements.begin(), currentStatements.end());

			if (hasElse)
			{
				replacementStatements.push_back(newGoto(endLabel));

				replacementStatements.push_back(
						newLabel(
							elseLabel,
							newAssume(
								newReverseLargestImplicativeDisjunctionOfCubes(
									config::globalCubeSizeLimit,
									conditional->negate()
								)
							)
						)
					);

				currentStatements = currentIfElseStatement->getChild(2)->getChildren();
				replacementStatements.insert(replacementStatements.end(), currentStatements.begin(), currentStatements.end());
			}

			replacementStatements.push_back(newLabel(endLabel, newNop()));

			firstStatement = newBooleanIf(newAsterisk(), newGoto(hasElse ? elseLabel : endLabel));

			if (!label.empty())
			{
				for (int ctr = replacementStatements.size() - 1; ctr >= 0; ctr--)
				{
					currentIfElseStatement->getParent()->getParent()->insertChild(replacementStatements.at(ctr), currentIfElseStatement->
						getParent()->getIndexAsChild() + 1);
				}

				currentIfElseStatement->replaceNodeWithoutDeletions(firstStatement);
			}
			else
			{
				for (int ctr = replacementStatements.size() - 1; ctr >= 0; ctr--)
				{
					currentIfElseStatement->getParent()->insertChild(replacementStatements.at(ctr), getIndexAsChild() + 1);
				}

				currentIfElseStatement->replaceNodeWithoutDeletions(firstStatement);
			}
		}
	}

	void Ast::replaceNodeWithoutDeletions(Ast* newData)
	{
		_children.clear();
		_name = newData->getName();
		_childrenCount = newData->getChildrenCount();
		vector<Ast*> newChildren = newData->getChildren();
		_children.insert(_children.begin(), newChildren.begin(), newChildren.end());
		_startComment = newData->getStartComment();
		_endComment = newData->getEndComment();

		invalidateCode();
	}

	// Returns a string representation of the node and its children
	string Ast::toString()
	{
		regex indentationRegex("\n");
		string result = _name;

		if (!_startComment.empty())
		{
			result += "\t" + _startComment;
		}

		if (!_endComment.empty())
		{
			result += "\t" + _endComment;
		}
		
		if (isProgramPoint()) //|| (!isRoot() && parent->name == config::IF_ELSE_TOKEN_NAME))
		{
			result += "\tpersistentReadCost = (" + bsm::toString(&persistentReadCost) + ")";
			result += "\tpersistentWriteCost = (" + bsm::toString(&persistentWriteCost) + ")";
		
			/*result += "\t";
			string startName, endName;
			for (ControlFlowEdge* edge : outgoingEdges)
			{
				startName = edge->start->name == config::LABEL_TOKEN_NAME ? edge->start->name + " " + edge->start->children.at(0)->name : edge->start->name;
				endName = edge->end->name == config::LABEL_TOKEN_NAME ? edge->end->name + " " + edge->end->children.at(0)->name : edge->end->name;
				result += "(" + startName + ", " + endName + ") ";
			}*/
		}
		
		int childrenCount = _children.size();
		
		for (int ctr = 0; ctr < childrenCount; ctr++)
		{
			result += "\n" + _children[ctr]->toString();
		}
		
		result = regex_replace(result, indentationRegex, "\n|");
		
		return result;
	}

/* Replicators */
	// Creates a clone of this and all successor nodes, all of which correspond to their source in all but bufferSizeMaps, comments and control flow data
	Ast* Ast::clone()
	{
		Ast* result = new Ast(_name);

		for (Ast* child : _children)
		{
			result->insertChild(child->clone());
		}

		return result;
	}

	Ast* Ast::negate()
	{
		Ast* result;

		if (_name == literalCode::EQUALS || _name == literalCode::LESS_THAN || _name == literalCode::LESS_EQUALS ||
			_name == string(1, literalCode::GREATER_THAN) || _name == literalCode::GREATER_EQUALS ||
			_name == literalCode::NOT_EQUALS || _name == literalCode::AND || _name == literalCode::OR ||
			_name == literalCode::DOUBLE_AND || _name == literalCode::DOUBLE_OR || _name == literalCode::ID_TOKEN_NAME)
		{
			Ast* selfClone = clone();
			result = new Ast(literalCode::NOT);
			result->insertChild(selfClone);
		}
		else if (_name == literalCode::NOT)
		{
			result = _children[0]->clone();
		}
		else if (_name == literalCode::BOOL_TOKEN_NAME)
		{
			if (_children[0]->getName() == literalCode::TRUE_TAG_NAME)
			{
				result = newFalse();
			}
			else if (_children[0]->getName() == literalCode::FALSE_TAG_NAME)
			{
				result = newTrue();
			}
			else
			{
				config::throwError("Can't negate BOOL(" + _children[0]->getName() + ")");
			}
		}
		else if (_name == literalCode::INT_TOKEN_NAME)
		{
			if (_children[0]->getName() == "0")
			{
				result = newINT(1);
			}
			else if (_children[0]->getName() == "1")
			{
				result = newINT(0);
			}
			else
			{
				config::throwError("Can't negate INT(" + _children[0]->getName() + ")");
			}
		}

		return result;
	}

/* Pseudo-constructors */
	Ast* Ast::newAstFromParsedProgram(const string &parsedProgramString)
	{
		string name = "";
		string currentlyRead = "";
		vector<string> parsedChildStrings;
		int parsedProgramStringLength = parsedProgramString.size();
		int parenthesisBalance = 0;

		for (int ctr = 0; ctr < parsedProgramStringLength; ctr++)
		{
			if (parsedProgramString[ctr] == literalCode::LEFT_PARENTHESIS)
			{
				if (parenthesisBalance == 0)
				{
					name = currentlyRead;
					currentlyRead = "";
				}
				else
				{
					currentlyRead += parsedProgramString[ctr];
				}

				parenthesisBalance++;
			}
			else if (parsedProgramString[ctr] == literalCode::RIGHT_PARENTHESIS)
			{
				if (parenthesisBalance == 1)
				{
					parsedChildStrings.push_back(currentlyRead);
					currentlyRead = "";
				}
				else
				{
					currentlyRead += parsedProgramString[ctr];
				}

				parenthesisBalance--;
			}
			else if (parsedProgramString[ctr] == literalCode::COMMA)
			{
				if (parenthesisBalance == 1)
				{
					parsedChildStrings.push_back(currentlyRead);
					currentlyRead = "";
				}
				else
				{
					currentlyRead += parsedProgramString[ctr];
				}
			}
			else
			{
				currentlyRead += parsedProgramString[ctr];
			}
		}

		if (!currentlyRead.empty())
		{
			if (name.empty())
			{
				name = currentlyRead;
			}
			else
			{
				config::throwCriticalError("Error while parsing \"" + parsedProgramString + "\":\n\tcurrentlyRead == \"" + currentlyRead + "\"\n\tname == \"" + name + "\"");
			}
		}

		Ast* result = new Ast(name);

		for (string parsedChildString : parsedChildStrings)
		{
			result->insertChild(newAstFromParsedProgram(parsedChildString));
		}

		return result;
	}

	// Initializes a localAssign(ID(variableName), INT(initialValue)) node
	Ast* Ast::newLocalAssign(const string &variableName, int initialValue)
	{
		Ast* result = new Ast(literalCode::LOCAL_ASSIGN_TOKEN_NAME);
		result->insertChild(newID(variableName));
		result->insertChild(newINT(initialValue));
		return result;
	}

	// Initializes a localAssign(ID(variableName), node) node
	Ast* Ast::newLocalAssign(const string &variableName, Ast* assignmentNode)
	{
		Ast* result = new Ast(literalCode::LOCAL_ASSIGN_TOKEN_NAME);
		result->insertChild(newID(variableName));
		result->insertChild(assignmentNode);
		return result;
	}

	// Initializes an ifElse(ifConditional, statements) node
	Ast* Ast::newIfElse(Ast* ifConditionalNode, const vector<Ast*> &statements)
	{
		Ast* result = new Ast(literalCode::IF_ELSE_TOKEN_NAME);
		result->insertChild(ifConditionalNode);
		result->insertChild(newStatements(statements));
		result->insertChild(newNone());
		return result;
	}

	// Initializes an ifElse(ifConditional, ifStatements, elseStatements) node
	Ast* Ast::newIfElse(Ast* ifConditionalNode, const vector<Ast*> &ifStatements, const vector<Ast*> &elseStatements)
	{
		Ast* result = new Ast(literalCode::IF_ELSE_TOKEN_NAME);
		result->insertChild(ifConditionalNode);
		result->insertChild(newStatements(ifStatements));
		result->insertChild(newStatements(elseStatements));
		return result;
	}

	// Initializes a binary operation node
	Ast* Ast::newBinaryOp(Ast* leftOperand, Ast* rightOperand, const string &operation)
	{
		Ast* result = new Ast(operation);
		result->insertChild(leftOperand);
		result->insertChild(rightOperand);
		return result;
	}

	Ast* Ast::newMultipleOperation(const vector<Ast*> &operands, const string &operation)
	{
		int operandCount = operands.size();

		if (operandCount > 1)
		{
			Ast* result = new Ast(operation);
			result->insertChildren(operands);

			return result;
		}
		else if (operandCount == 1)
		{
			return operands[0];
		}
		else
		{
			return nullptr;
		}
	}

	// Initializes an ID node
	Ast* Ast::newID(const string &variableName)
	{
		Ast* result = new Ast(literalCode::ID_TOKEN_NAME);
		result->insertChild(new Ast(variableName));
		return result;
	}

	// Initializes an INT node
	Ast* Ast::newINT(int value)
	{
		Ast* result = new Ast(literalCode::INT_TOKEN_NAME);
		result->insertChild(new Ast(to_string(value)));
		return result;
	}

	Ast* Ast::newTrue()
	{
		Ast* result = new Ast(literalCode::BOOL_TOKEN_NAME);
		result->insertChild(new Ast(literalCode::TRUE_TAG_NAME));
		return result;
	}

	Ast* Ast::newFalse()
	{
		Ast* result = new Ast(literalCode::BOOL_TOKEN_NAME);
		result->insertChild(new Ast(literalCode::FALSE_TAG_NAME));
		return result;
	}

	Ast* Ast::newStatements(const vector<Ast*> &statements)
	{
		Ast* result = new Ast(literalCode::STATEMENTS_TOKEN_NAME);
		result->insertChildren(statements);
		return result;
	}

	Ast* Ast::newNone()
	{
		Ast* result = new Ast(literalCode::NONE_TOKEN_NAME);
		return result;
	}

	Ast* Ast::newNop()
	{
		Ast* result = new Ast(literalCode::NOP_TOKEN_NAME);
		return result;
	}

	Ast* Ast::newAbort(const string &abortMessage)
	{
		Ast* result = new Ast(literalCode::ABORT_TOKEN_NAME);
		result->insertChild(new Ast(abortMessage));
		return result;
	}

	Ast* Ast::newAsterisk()
	{
		Ast* result = new Ast(literalCode::ASTERISK_TOKEN_NAME);
		return result;
	}

	Ast* Ast::newLabel(int value, Ast* statement)
	{
		Ast* result = new Ast(literalCode::LABEL_TOKEN_NAME);
		result->insertChild(new Ast(to_string(value)));
		result->insertChild(statement);
		return result;
	}

	Ast* Ast::newGoto(int value)
	{
		Ast* result = new Ast(literalCode::GOTO_TOKEN_NAME);
		result->insertChild(new Ast(to_string(value)));
		return result;
	}

	// Initializes an assume(conditional) node
	Ast* Ast::newAssume(Ast* assumeConditionalNode)
	{
		Ast* result = new Ast(literalCode::ASSUME_TOKEN_NAME);
		result->insertChild(assumeConditionalNode);
		return result;
	}

	Ast* Ast::newChoose(Ast* firstChoice, Ast* secondChoice)
	{
		Ast* result = new Ast(literalCode::CHOOSE_TOKEN_NAME);
		result->insertChild(firstChoice);
		result->insertChild(secondChoice);
		return result;
	}

	Ast* Ast::newStore(const string &variableName, Ast* rightSide)
	{
		Ast* result = new Ast(literalCode::PSO_TSO_STORE_TOKEN_NAME);
		result->insertChild(newID(variableName));
		result->insertChild(rightSide);
		return result;
	}

	Ast* Ast::newLoad(const string &variableName, Ast* rightSide)
	{
		Ast* result = new Ast(literalCode::PSO_TSO_LOAD_TOKEN_NAME);
		result->insertChild(newID(variableName));
		result->insertChild(rightSide);
		return result;
	}

	Ast* Ast::newSharedVariables(const vector<string> &variableNames)
	{
		Ast* result = new Ast(literalCode::BL_SHARED_VARIABLES_BLOCK_TOKEN_NAME);

		for (string variableName : variableNames)
		{
			result->insertChild(newID(variableName));
		}

		return result;
	}

	Ast* Ast::newLocalVariables(const vector<string> &variableNames)
	{
		Ast* result = new Ast(literalCode::BL_LOCAL_VARIABLES_BLOCK_TOKEN_NAME);

		for (string variableName : variableNames)
		{
			result->insertChild(newID(variableName));
		}

		return result;
	}

	Ast* Ast::newBeginAtomic()
	{
		Ast* result = new Ast(literalCode::BEGIN_ATOMIC_TOKEN_NAME);
		return result;
	}

	Ast* Ast::newEndAtomic()
	{
		Ast* result = new Ast(literalCode::END_ATOMIC_TOKEN_NAME);
		return result;
	}

	Ast* Ast::newBooleanIf(Ast* ifConditionalNode, Ast* statement)
	{
		Ast* result = new Ast(literalCode::BL_IF_TOKEN_NAME);
		result->insertChild(ifConditionalNode);
		result->insertChild(statement);
		return result;
	}

	Ast* Ast::newBooleanVariableCube(const string &definition, bool useTemporaryVariables)
	{
		int numberOfPredicates = config::globalPredicates.size();
		vector<Ast*> cubeTerms;
		string currentVariableName;

		for (int ctr = 0; ctr < numberOfPredicates; ctr++)
		{
			currentVariableName = useTemporaryVariables ? config::globalPredicates[ctr]->getSingleTemporaryVariableName() :
				config::globalPredicates[ctr]->getSingleBooleanVariableName();

			if (definition[ctr] == CubeTreeNode::CUBE_STATE_DECIDED_FALSE)
			{
				cubeTerms.push_back(
						newBinaryOp(
							newID(currentVariableName),
							newINT(0),
							literalCode::EQUALS
						)
					);
			}
			else if (definition[ctr] == CubeTreeNode::CUBE_STATE_DECIDED_TRUE)
			{
				cubeTerms.push_back(
						newBinaryOp(
							newID(currentVariableName),
							newINT(0),
							literalCode::NOT_EQUALS
						)
					);
			}
		}

		return newMultipleOperation(cubeTerms, literalCode::DOUBLE_AND);
	}

	Ast* Ast::newLargestImplicativeDisjunctionOfCubes(int cubeSizeUpperLimit, Ast* predicate, bool useTemporaryVariables)
	{
		cout << "\t\tComputing F(" << predicate->getCode() << ")\t\t\t \n";
		int numberOfPredicates = config::globalPredicates.size();
		vector<int> relevantIndices = config::getRelevantAuxiliaryTemporaryVariableIndices(predicate);

		vector<Ast*> implicativeCubes;
		vector<string> implicativeCubeStringRepresentations = config::getMinimalImplyingCubes(predicate, relevantIndices);

		for (string implicativeCubeStringRepresentation : implicativeCubeStringRepresentations)
		{
			implicativeCubes.push_back(newBooleanVariableCube(implicativeCubeStringRepresentation, useTemporaryVariables));
		}

		if (implicativeCubes.size() == 0)
		{
			if (config::isTautology(predicate))
			{
				return newTrue();
			}
			else
			{
				return newFalse();
			}
		}

		return newMultipleOperation(implicativeCubes, literalCode::DOUBLE_OR);
	}

	Ast* Ast::newReverseLargestImplicativeDisjunctionOfCubes(int cubeSizeUpperLimit, Ast* predicate)
	{
		return newLargestImplicativeDisjunctionOfCubes(cubeSizeUpperLimit, predicate->negate(), false)->negate();
	}

	Ast* Ast::newAbstractAssignmentFragment(Ast* assignment, Ast* predicate)
	{
		return newChoose(
				newLargestImplicativeDisjunctionOfCubes(config::globalCubeSizeLimit,
					config::getWeakestLiberalPrecondition(assignment, predicate)),
				newLargestImplicativeDisjunctionOfCubes(config::globalCubeSizeLimit,
					config::getWeakestLiberalPrecondition(assignment, predicate->negate()))
			);
	}
		
	Ast* Ast::newWeakestLiberalPrecondition(Ast* assignment, Ast* predicate)
	{
		Ast* result = nullptr;
		
		if (config::currentLanguage == config::language::RMA)
		{
			// TODO
			config::throwError("Predicate abstraction operations not implemented for RMA");
		}
		else
		{
			cout << "\t\tComputing WP(" << assignment->getCode() << ", " << predicate->getCode() << ")\t\t\t \n";
	
			if (assignment->getName() == literalCode::PSO_TSO_STORE_TOKEN_NAME ||
				assignment->getName() == literalCode::PSO_TSO_LOAD_TOKEN_NAME ||
				assignment->getName() == literalCode::LOCAL_ASSIGN_TOKEN_NAME)
			{
				string leftVariable = assignment->getChild(0)->getChild(0)->getName();
				Ast* rightExpression = assignment->getChild(1);
				result = predicate->clone();
				vector<Ast*> toBeReplaced = result->search(leftVariable);
				int toBeReplacedCount = toBeReplaced.size();

				for (int ctr = 0; ctr < toBeReplacedCount; ctr++)
				{
					/*cout << "\t\tconfig::replaceNode(" + rightExpression->clone()->getCode() + ", " +
						toBeReplaced[ctr]->getParent()->getCode() + ")\t\t\t \n";*/
					config::replaceNode(rightExpression->clone(), toBeReplaced[ctr]->getParent());
				}
			}
		}
		
		return result;
	}

/* Cascading methods */
	void Ast::registerIDsAsGlobal()
	{
		vector<string> IDs = getIDs();

		for (string ID : IDs)
		{
			if (!config::tryRegisterGlobalSymbol(ID))
			{
				config::throwCriticalError("Couldn't register global symbol \"" + ID + "\"");
			}
		}
	}

	void Ast::registerIDsAsLocal()
	{
		vector<string> IDs = getIDs();

		for (string ID : IDs)
		{
			config::tryRegisterLocalSymbol(ID);
		}
	}

	// Cascades from top to bottom. If for a read- or write-statement is encountered, the cascade stops and the IDs read and written by its children are gathered
	void Ast::getCostsFromChildren()
	{
		if (config::currentLanguage == config::language::RMA)
		{
			if (_name == literalCode::LOCAL_ASSIGN_TOKEN_NAME)
			{
				vector<string> writtenVariables = _children.at(0)->getIDs();
				vector<string> readVariables = _children.at(2)->getIDs();

				for (string member : writtenVariables)
				{
					bsm::incrementCost(member, 1, &causedWriteCost);
				}

				for (string member : readVariables)
				{
					bsm::incrementCost(member, 1, &causedReadCost);
				}
			}
			else if (_name == literalCode::RMA_PUT_TOKEN_NAME)
			{
				bsm::incrementCost(_children.at(2)->_name, 1, &causedWriteCost);
				bsm::incrementCost(_children.at(4)->_name, 1, &causedReadCost);
			}
			else if (_name == literalCode::RMA_GET_TOKEN_NAME)
			{
				bsm::incrementCost(_children.at(0)->_name, 1, &causedWriteCost);
				bsm::incrementCost(_children.at(3)->_name, 1, &causedReadCost);
			}
			else
			{
				for (Ast* child : _children)
				{
					child->getCostsFromChildren();
				}
			}
		}
		else	// TSO/PSO: critical statements are store and load
		{
			if (_name == literalCode::PSO_TSO_STORE_TOKEN_NAME || _name == literalCode::PSO_TSO_LOAD_TOKEN_NAME)
			{
				vector<string> writtenVariables = _children.at(0)->getIDs();
				vector<string> readVariables = _children.at(1)->getIDs();

				for (string member : writtenVariables)
				{
					bsm::incrementCost(member, 1, &causedWriteCost);
				}

				for (string member : readVariables)
				{
					bsm::incrementCost(member, 1, &causedReadCost);
				}
			}
			else
			{
				for (Ast* child : _children)
				{
					child->getCostsFromChildren();
				}
			}
		}
	}

	// Cascades from top to bottom and back. Returns a string vector containing all identifiers along the downward path, including the current node's own.
	vector<string> Ast::getIDs()
	{
		vector<string> results;

		if (_name == literalCode::ID_TOKEN_NAME)	// If this is an identifier, push it on the vector to return it upwards
		{
			results.push_back(_children.at(0)->getName());
		}
		else // If this isn't an identifier, gather all identifiers from the progeny
		{
			vector<string> subResults;

			for (Ast* child : _children)
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

	// Cascades from top to bottom. Causes nodes to initialize the global variables initialized in the program's first block into their persistent maps
	void Ast::initializePersistentCosts()
	{
		if (_name == literalCode::PROGRAM_DECLARATION_TOKEN_NAME)
		{
			// Collect all buffer costs caused by the variable initialization statements
			if (config::currentLanguage == config::language::RMA)
			{
				for (int ctr = 0; ctr < _children[0]->getChildrenCount(); ctr++)
				{
					if (_children[0]->getChild(ctr)->getName() != literalCode::RMA_PROCESS_INITIALIZATION_TOKEN_NAME)
					{
						break;
					}

					for (int subCtr = 0; subCtr < _children[0]->getChild(ctr)->getChild(1)->getChildrenCount(); subCtr++)
					{
						bsm::additiveMergeBufferSizes(&(_children[0]->getChild(ctr)->getChild(1)->getChild(subCtr)->causedWriteCost),
							&persistentWriteCost);
						bsm::additiveMergeBufferSizes(&(_children[0]->getChild(ctr)->getChild(1)->getChild(subCtr)->causedWriteCost),
							&persistentWriteCost);
					}
				}
			}
			else
			{
				for (int ctr = 0; ctr < _children[0]->getChildrenCount(); ctr++)
				{
					bsm::additiveMergeBufferSizes(&(_children[0]->getChild(ctr)->causedWriteCost), &persistentWriteCost);
					bsm::additiveMergeBufferSizes(&(_children[0]->getChild(ctr)->causedWriteCost), &persistentReadCost);
				}
			}

			// Sets the values of the recently initialized entries to 0
			resetBufferSizes();

			// Propagates the 0-initialized global variable map to the progeny of the process declarations
			for (Ast* child : _children)
			{
				if (child->getName() == literalCode::PROCESS_DECLARATION_TOKEN_NAME)
				{
					child->topDownCascadingCopyPersistentBufferSizes(this);
					child->topDownCascadingAddInitializedCausedCostsToPersistentCosts();
				}
			}
		}
	}

	// Cascades from top to bottom. Copies all persistent buffer size maps to the successors.
	void Ast::topDownCascadingCopyPersistentBufferSizes(Ast* source)
	{
		copyPersistentBufferSizes(source);

		for (Ast* child : _children)
		{
			child->topDownCascadingCopyPersistentBufferSizes(source);
		}
	}

	// Cascades from top to bottom. Causes those nodes that have their own caused costs to add them to their persistent maps.
	void Ast::topDownCascadingAddInitializedCausedCostsToPersistentCosts()
	{
		bsm::conditionalAdditiveMergeBufferSizes(&causedReadCost, &persistentReadCost);
		bsm::conditionalAdditiveMergeBufferSizes(&causedWriteCost, &persistentWriteCost);

		for (Ast* child : _children)
		{
			child->topDownCascadingAddInitializedCausedCostsToPersistentCosts();
		}
	}

	// Cascades from top to bottom. Causes label nodes to register themselves with the global register.
	void Ast::topDownCascadingRegisterLabels()
	{
		if (_name == literalCode::LABEL_TOKEN_NAME)
		{
			if (!config::tryRegisterLabel(getParentProcessNumber(), stoi(_children[0]->getName())))
			{
				config::throwCriticalError("Couldn't register label " + _children[0]->getName() + " for " + getCode());
			}
		}

		for (Ast* child : _children)
		{
			child->topDownCascadingRegisterLabels();
		}
	}

	// Cascades from top to bottom. Causes label nodes to generate outgoing directional control flow edges, if applicable.
	void Ast::topDownCascadingGenerateOutgoingEdges()
	{
		generateOutgoingEdges();

		for (Ast* child : _children)
		{
			child->topDownCascadingGenerateOutgoingEdges();
		}
	}

	void Ast::topDownCascadingUnfoldIfElses()
	{
		if (_name == literalCode::STATEMENTS_TOKEN_NAME)
		{
			unfoldIfElses();
		}
		else
		{
			for (int ctr = 0; ctr < _children.size(); ctr++)
			{
				_children[ctr]->topDownCascadingUnfoldIfElses();
			}
		}
	}

	void Ast::topDownCascadingPerformPredicateAbstraction()
	{
		if (!performPredicateAbstraction())
		{
			Ast* child;

			for (int ctr = 0; ctr < _children.size(); ctr++)
			{
				child = _children[ctr];

				if (_name != literalCode::IF_ELSE_TOKEN_NAME &&
					child->getName() != literalCode::BL_SHARED_VARIABLES_BLOCK_TOKEN_NAME &&
					child->getName() != literalCode::BL_LOCAL_VARIABLES_BLOCK_TOKEN_NAME &&
					child->getName() != literalCode::BL_IF_TOKEN_NAME)
				{
					child->topDownCascadingPerformPredicateAbstraction();
				}
			}
		}
	}

	void Ast::topDownCascadingReplaceIDNames(const string &oldName, const string &newName)
	{
		if (_name == literalCode::ID_TOKEN_NAME)
		{
			if (_children[0]->getName() == oldName)
			{
				_children[0]->setName(newName);
			}
		}
		else if (_name != literalCode::INT_TOKEN_NAME)
		{
			for (Ast* child : _children)
			{
				child->topDownCascadingReplaceIDNames(oldName, newName);
			}
		}
	}
		
	// Cascades down the control flow graph. If any successor node has a different persistent map, causes this to propagate its TOP values down the line.
	void Ast::controlFlowDirectionCascadingPropagateTops()
	{
		bool changesMade = false;
		bufferSizeMap topContainer;
			
		// Creates a map containing 0s for each non-TOP, and TOP for each TOP value in the persistent write cost map, and adds it to the successor's map if necessary
		bsm::copyAndSetNonTopToZero(&persistentWriteCost, &topContainer);
			
		for (ControlFlowEdge* edge : outgoingEdges)
		{
			if (!bsm::bufferSizeMapCompare(&persistentWriteCost, &(edge->end->persistentWriteCost)))
			{
				changesMade = true;
				bsm::additiveMergeBufferSizes(&topContainer, &(edge->end->persistentWriteCost));
			}
		}
			
		// Creates a map containing 0s for each non-TOP, and TOP for each TOP value in the persistent read cost map, and adds it to the successor's map if necessary
		bsm::copyAndSetNonTopToZero(&persistentReadCost, &topContainer);
			
		for (ControlFlowEdge* edge : outgoingEdges)
		{
			if (!bsm::bufferSizeMapCompare(&persistentReadCost, &(edge->end->persistentReadCost)))
			{
				changesMade = true;
				bsm::additiveMergeBufferSizes(&topContainer, &(edge->end->persistentReadCost));
			}
		}

		if (changesMade)
		{
			for (ControlFlowEdge* edge : outgoingEdges)
			{
				edge->end->controlFlowDirectionCascadingPropagateTops();
			}
		}
	}

	void Ast::performBufferSizeAnalysisReplacements()
	{
		if (config::currentLanguage == config::language::RMA)
		{
			// TODO
			config::throwError("Buffer size analysis not implemented for RMA");
		}
		else if (config::currentLanguage == config::language::PSO)
		{
			// TODO
			config::throwError("Buffer size analysis not implemented for PSO");
		}
		else
		{
			if (_name == literalCode::PSO_TSO_STORE_TOKEN_NAME ||
				_name == literalCode::PSO_TSO_LOAD_TOKEN_NAME ||
				_name == literalCode::FENCE_TOKEN_NAME ||
				_name == literalCode::FLUSH_TOKEN_NAME)
			{
				vector<Ast*> replacement;
				int s;
				int processNumber = getParentProcessNumber();
				string globalVariableName;
				string x_cnt_t;
				string x_fst_t;
				vector<string> bufferVariables;

				if (_name == literalCode::PSO_TSO_STORE_TOKEN_NAME)
				{
					vector<Ast*> currentStatements;

					globalVariableName = _children[0]->getChild(0)->getName();
					x_cnt_t = config::symbolMap[globalVariableName]->getAuxiliaryCounterName(processNumber);
					x_fst_t = config::symbolMap[globalVariableName]->getAuxiliaryFirstPointerName(processNumber);
					bufferVariables = config::symbolMap[globalVariableName]->getAuxiliaryBufferNames(processNumber);

					if (persistentWriteCost[globalVariableName] == bsm::TOP_VALUE || persistentWriteCost[globalVariableName] > config::K)
					{
						s = config::K;
					}
					else
					{
						s = persistentWriteCost[globalVariableName];
					}

					currentStatements.push_back(newAbort(config::OVERFLOW_MESSAGE));

					// Adding if (x_cnt_t == s) { abort(OVERFLOW_MESSAGE); }
					replacement.push_back(newIfElse(newBinaryOp(newID(x_cnt_t), newINT(s), literalCode::EQUALS), currentStatements));

					currentStatements.clear();
					currentStatements.push_back(newLocalAssign(x_fst_t, 1));
					currentStatements.push_back(newLocalAssign(bufferVariables[0], _children[1]->clone()));

					// Adding if (x_fst_t == 0) { x_fst_t = 1; x_1_t = r; }
					replacement.push_back(newIfElse(newBinaryOp(newID(x_fst_t), newINT(0), literalCode::EQUALS), currentStatements));

					// Adding x_cnt_t = x_cnt_t + 1;
					replacement.push_back(newLocalAssign(x_cnt_t, newBinaryOp(newID(x_cnt_t), newINT(1), string(1, literalCode::PLUS))));

					for (int i = 1; i <= s; i++)
					{
						currentStatements.clear();
						currentStatements.push_back(newLocalAssign(bufferVariables[i], _children[1]->clone()));

						// Adding if (x_cnt_t == i + 1) { x_(i+1)_t = r; }
						replacement.push_back(newIfElse(newBinaryOp(newID(x_cnt_t), newINT(i + 1), literalCode::EQUALS), currentStatements));
					}
				}
				else if (_name == literalCode::PSO_TSO_LOAD_TOKEN_NAME)
				{
					vector<Ast*> currentStatements;
					string r = _children[0]->getChild(0)->getName();

					globalVariableName = _children[1]->getChild(0)->getName();
					x_cnt_t = config::symbolMap[globalVariableName]->getAuxiliaryCounterName(processNumber);
					bufferVariables = config::symbolMap[globalVariableName]->getAuxiliaryBufferNames(processNumber);

					if (persistentWriteCost[globalVariableName] == bsm::TOP_VALUE || persistentWriteCost[globalVariableName] > config::K)
					{
						s = config::K;
					}
					else
					{
						s = persistentWriteCost[globalVariableName];
					}

					currentStatements.push_back(newLoad(r, _children[1]));

					// Adding if (x_cnt_t == 0) { load r = X; }
					replacement.push_back(newIfElse(newBinaryOp(newID(x_cnt_t), newINT(0), literalCode::EQUALS), currentStatements));

					for (int i = 0; i < s; i++)
					{
						currentStatements.clear();
						currentStatements.push_back(newLocalAssign(r, newID(bufferVariables[i])));

						// Adding if (x_cnt_t == i + 1) { r = x_(i+1)_t; }
						replacement.push_back(newIfElse(newBinaryOp(newID(x_cnt_t), newINT(i + 1), literalCode::EQUALS), currentStatements));
					}
				}
				else if (_name == literalCode::FENCE_TOKEN_NAME)
				{
					for (pair<string, VariableEntry*> symbol : config::symbolMap)
					{
						if (symbol.second->getType() == VariableEntry::GLOBAL)
						{
							replacement.push_back(
									newAssume(newBinaryOp(newID(symbol.second->getAuxiliaryCounterName(processNumber)), newINT(0), literalCode::EQUALS))
								);
							replacement.push_back(
									newAssume(newBinaryOp(newID(symbol.second->getAuxiliaryFirstPointerName(processNumber)), newINT(0), literalCode::EQUALS))
								);
						}
					}
				}
				else if (_name == literalCode::FLUSH_TOKEN_NAME)
				{
					int p = config::getCurrentAuxiliaryLabel();
					vector<Ast*> flushStatements;
					vector<Ast*> currentThenStatements;
					vector<Ast*> currentElseStatements;
					Ast* currentIfElse;

					for (pair<string, VariableEntry*> symbol : config::symbolMap)
					{
						if (symbol.second->getType() == VariableEntry::GLOBAL)
						{
							globalVariableName = symbol.second->getName();
							x_cnt_t = config::symbolMap[globalVariableName]->getAuxiliaryCounterName(processNumber);
							x_fst_t = config::symbolMap[globalVariableName]->getAuxiliaryFirstPointerName(processNumber);
							bufferVariables = config::symbolMap[globalVariableName]->getAuxiliaryBufferNames(processNumber);

							if (persistentWriteCost[globalVariableName] == bsm::TOP_VALUE || persistentWriteCost[globalVariableName] > config::K)
							{
								s = config::K;
							}
							else
							{
								s = persistentWriteCost[globalVariableName];
							}

							currentThenStatements.push_back(newStore(globalVariableName, newID(bufferVariables[s - 1])));
							currentElseStatements.push_back(newStore(globalVariableName, newID(bufferVariables[s - 2])));
							currentIfElse = newIfElse(
									newBinaryOp(newID(x_fst_t), newINT(s - 1), string(1, literalCode::GREATER_THAN)),
									currentThenStatements,
									currentElseStatements
								);

							for (int i = s - 2; i >= 0; i++)
							{
								currentThenStatements.clear();
								currentElseStatements.clear();

								currentThenStatements.push_back(currentIfElse);
								currentElseStatements.push_back(newStore(globalVariableName, newID(bufferVariables[i - 1])));

								currentIfElse = newIfElse(
										newBinaryOp(newID(x_fst_t), newINT(i), string(1, literalCode::GREATER_THAN)),
										currentThenStatements,
										currentElseStatements
									);
							}

							currentThenStatements.clear();

							currentThenStatements.push_back(currentIfElse);
							currentThenStatements.push_back(newLocalAssign(x_fst_t, newBinaryOp(newID(x_fst_t), newINT(1), string(1, literalCode::PLUS))));

							currentIfElse = newIfElse(newAsterisk(), currentThenStatements);

							currentThenStatements.clear();

							currentThenStatements.push_back(currentIfElse);

							flushStatements.push_back(
									newIfElse(newBinaryOp(newID(x_cnt_t), newINT(0), string(1, literalCode::GREATER_THAN)), currentThenStatements)
								);
						}
					}

					flushStatements.push_back(newGoto(p));

					// Adding p: if (*) { [flushStatements] goto p; }
					replacement.push_back(newLabel(p, newIfElse(newAsterisk(), flushStatements)));
				}

				if (!replacement.empty() && _parent != nullptr)
				{
					Ast* toBeReplaced;

					if (_parent->getName() == literalCode::LABEL_TOKEN_NAME)
					{
						// labelled
						toBeReplaced = _parent;
						Ast* labelledFirstStatement = newLabel(stoi(_parent->getChild(0)->getName()), replacement[0]);

						labelledFirstStatement->setStartComment(literalCode::REPLACING_CAPTION + literalCode::SPACE + literalCode::QUOTATION + getCode() +
							literalCode::QUOTATION);

						replacement.erase(replacement.begin());
						replacement.insert(replacement.begin(), labelledFirstStatement);
					}
					else
					{
						// unlabelled
						toBeReplaced = this;

						replacement[0]->setStartComment(literalCode::REPLACING_CAPTION + literalCode::SPACE + literalCode::QUOTATION + getCode() +
							literalCode::QUOTATION);
					}

					replacement[replacement.size() - 1]->setEndComment(literalCode::FINISHED_REPLACING_CAPTION + literalCode::SPACE +
						literalCode::QUOTATION + getCode() + literalCode::QUOTATION);

					config::prepareNodeForLazyReplacement(replacement, toBeReplaced);
				}
			}
			else if (_name != literalCode::INITIALIZATION_BLOCK_TOKEN_NAME &&
				_name != literalCode::BL_INITIALIZATION_BLOCK_TOKEN_NAME &&
				_name != literalCode::ASSERT_TOKEN_NAME)
			{
				for (Ast* child : _children)
				{
					child->performBufferSizeAnalysisReplacements();
				}
			}
		}
	}

	void Ast::generateBooleanVariableInitializations()
	{
		if (_name == literalCode::BL_PROGRAM_DECLARATION_TOKEN_NAME)
		{
			int numberOfGlobalPredicates = config::globalPredicates.size();

			for (int ctr = numberOfGlobalPredicates - 1; ctr >= 0; ctr--)
			{
				_children[2]->insertChild(
						newLabel(
							config::getCurrentAuxiliaryLabel(),
							newLoad(
								config::globalPredicates[ctr]->getSingleTemporaryVariableName(),
								newID(config::globalPredicates[ctr]->getSingleBooleanVariableName())
							)
						),
						0
					);
			}

			for (int ctr = numberOfGlobalPredicates - 1; ctr >= 0; ctr--)
			{
				_children[2]->insertChild(
						newLabel(
							config::getCurrentAuxiliaryLabel(),
							newStore(config::globalPredicates[ctr]->getSingleBooleanVariableName(), newAsterisk())
						),
						0
					);
			}

			_children[2]->getChild(0)->setStartComment("Initializing local variables");

			vector<Ast*> postFix;

			for (int ctr = 0; ctr < numberOfGlobalPredicates; ctr++)
			{
				postFix.push_back(newLabel(config::getCurrentAuxiliaryLabel(),
					newLocalAssign(config::globalPredicates[ctr]->getSingleTemporaryVariableName(), newINT(0))));
			}

			postFix[0]->setStartComment("Resetting local variables");
			_children[2]->insertChildren(postFix);

			Ast* maxCube = config::getAssumptionOfNegatedLargestFalseImplicativeDisjunctionOfCubes();

			_children[2]->insertChild(newLabel(config::getCurrentAuxiliaryLabel(), maxCube));

			for (int ctr = 3; ctr < _children.size(); ctr++)
			{
				if (_children[ctr]->getName() == literalCode::BL_PROCESS_DECLARATION_TOKEN_NAME ||
					_children[ctr]->getName() == literalCode::PROCESS_DECLARATION_TOKEN_NAME)
				{
					_children[ctr]->getChild(1)->insertChild(newLabel(config::getCurrentAuxiliaryLabel(), maxCube), 0);
				}
			}
		}
	}
		
	vector<Ast*> Ast::search(const string &soughtName)
	{
		vector<Ast*> results;
		
		if (_name != soughtName)
		{
			vector<Ast*> subResults;
		
			for (Ast* child : _children)
			{
				subResults = child->search(soughtName);
				results.insert(results.end(), subResults.begin(), subResults.end());
			}
		}
		else
		{
			results.push_back(this);
		}
		
		return results;
	}










//
//
//	Ast::Ast()
//	{
//		// The index by which this node can be referred to from its parent's children vector. The root always has an index of -1
//		indexAsChild = -1;
//		name = "";
//		parent = nullptr;
//
//		//updateShortStringRepresentation();
//	}
//
//	Ast::~Ast()
//	{
//	}
//
///* Access */
//	// Adds a child node and sets its variables connecting it to this node
//	void Ast::addChild(Ast* child)
//	{
//		child->parent = this;
//		child->indexAsChild = children.size();
//		children.push_back(child);
//	
//		//updateShortStringRepresentation();
//	}
//	
//	// Adds a child node at a specific index and sets its variables connecting it to this node
//	void Ast::addChild(Ast* child, int index)
//	{
//		child->parent = this;
//		child->indexAsChild = index;
//
//		if (index == children.size())
//		{
//			children.insert(children.end(), child);
//		}
//		else if (index < children.size())
//		{
//			children.insert(children.begin() + index, child);
//		}
//		else
//		{
//			/*config::throwCriticalError("Error trying to insert: " + child->emitCode() + " on the index " + to_string(index) +
//				". The current vector size is " + to_string(children.size()) + ".");*/
//			children.insert(children.begin() + index, child);
//		}
//	
//		refreshChildIndices();
//		//updateShortStringRepresentation();
//	}
//	
//	void Ast::addChildren(const vector<Ast*> &newChildren)
//	{
//		for (Ast* child : newChildren)
//		{
//			child->parent = this;
//			child->indexAsChild = children.size();
//			children.push_back(child);
//		}
//	}
//	
//	void Ast::refreshChildIndices()
//	{
//		int childCount = children.size();
//	
//		for (int ctr = 0; ctr < childCount; ctr++)
//		{
//			children.at(ctr)->indexAsChild = ctr;
//		}
//	}
//	
//	// Gets the qualified name representing this label
//	string Ast::getLabelCode()
//	{
//		if (name == literalCode::LABEL_TOKEN_NAME)
//		{
//			string parentProcessNumber;
//			string label = children.at(0)->name;
//	
//			if (tryGetParentProcessNumber(&parentProcessNumber))
//			{
//				return parentProcessNumber + literalCode::LABEL_SEPARATOR + label;
//			}
//		}
//	
//		return "";
//	}
//	
//
//	const string Ast::getCode()
//	{
//		if (_code.empty())
//		{
//			_code = emitCode();
//		}
//
//		return _code;
//	}
//
///* Cascading operations */
//	string Ast::emitCode()
//	{
//		string result = "";
//		bool isBooleanProgramNode = false;
//		int childrenCount;
//
//		if (name == literalCode::BL_PROGRAM_DECLARATION_TOKEN_NAME)
//		{
//			for (Ast* child : children)
//			{
//				result += child->emitCode() + "\n\n";
//			}
//
//			isBooleanProgramNode = true;
//		}
//		else if (name == literalCode::BL_INITIALIZATION_BLOCK_TOKEN_NAME)
//		{
//			string subResult = "";
//
//			for (Ast* child : children)
//			{
//				subResult += child->emitCode() + "\n";
//			}
//
//			if (!subResult.empty())
//			{
//				subResult = subResult.substr(0, subResult.length() - 1);
//			}
//
//			result += literalCode::INIT_TAG_NAME + "\n\n" + config::addTabs(subResult, 1) + "\n\n";
//
//			isBooleanProgramNode = true;
//		}
//		else if (name == literalCode::BL_SHARED_VARIABLES_BLOCK_TOKEN_NAME)
//		{
//			result += literalCode::BL_SHARED_VARIABLES_BLOCK_TOKEN_NAME;
//
//			for (Ast* child : children)
//			{
//				result += " " + child->emitCode() + ",";
//			}
//
//			if (result.size() > literalCode::BL_SHARED_VARIABLES_BLOCK_TOKEN_NAME.size())
//			{
//				result[result.size() - 1] = literalCode::SEMICOLON;
//			}
//			else
//			{
//				result += literalCode::SEMICOLON;
//			}
//
//			isBooleanProgramNode = true;
//		}
//		else if (name == literalCode::BL_LOCAL_VARIABLES_BLOCK_TOKEN_NAME)
//		{
//			result += literalCode::BL_LOCAL_VARIABLES_BLOCK_TOKEN_NAME;
//
//			for (Ast* child : children)
//			{
//				result += " " + child->emitCode() + ",";
//			}
//
//			if (result.size() > literalCode::BL_LOCAL_VARIABLES_BLOCK_TOKEN_NAME.size())
//			{
//				result[result.size() - 1] = literalCode::SEMICOLON;
//			}
//			else
//			{
//				result += literalCode::SEMICOLON;
//			}
//
//			isBooleanProgramNode = true;
//		}
//		else if (name == literalCode::BL_PROCESS_DECLARATION_TOKEN_NAME)
//		{
//			result += literalCode::PROCESS_TAG_NAME + " " + children.at(0)->children.at(0)->name + "\n\n" + config::addTabs(children.at(1)->emitCode(), 1);
//
//			isBooleanProgramNode = true;
//		}
//		else if (name == literalCode::BL_IF_TOKEN_NAME)
//		{
//			result += literalCode::IF_TAG_NAME + literalCode::SPACE + literalCode::LEFT_PARENTHESIS +
//				children.at(0)->emitCode() + literalCode::RIGHT_PARENTHESIS + literalCode::SPACE + children.at(1)->emitCode();
//
//			isBooleanProgramNode = true;
//		}
//		else if (name == literalCode::ASSERT_TOKEN_NAME)
//		{
//			result += literalCode::ASSERT_TOKEN_NAME + literalCode::SPACE + literalCode::BL_ALWAYS_TOKEN_NAME +
//				literalCode::LEFT_PARENTHESIS +	children.at(0)->emitCode() + literalCode::RIGHT_PARENTHESIS +
//				literalCode::SEMICOLON;
//
//			isBooleanProgramNode = true;
//		}
//
//		if (!startComment.empty())
//		{
//			result += "\n" + literalCode::MULTILINE_COMMENT_PREFIX + literalCode::SPACE + startComment + literalCode::SPACE + literalCode::MULTILINE_COMMENT_SUFFIX + "\n";
//		}
//
//		/*if (!startComment.empty())
//		{
//		result += literalCode::COMMENT_PREFIX + literalCode::SPACE + startComment + "\n";
//		}*/
//
//		if (config::currentLanguage == config::language::RMA)
//		{
//			if (name == literalCode::PROGRAM_DECLARATION_TOKEN_NAME)
//			{
//				for (Ast* child : children)
//				{
//					result += child->emitCode() + "\n\n";
//				}
//			}
//			else if (name == literalCode::INITIALIZATION_BLOCK_TOKEN_NAME)
//			{
//				result += literalCode::BEGINIT_TAG_NAME + "\n\n";
//
//				for (Ast* child : children)
//				{
//					result += config::addTabs(child->emitCode(), 1) + "\n";
//				}
//
//				result += "\n" + literalCode::ENDINIT_TAG_NAME;
//			}
//			else if (name == literalCode::RMA_PROCESS_INITIALIZATION_TOKEN_NAME)
//			{
//				result += children.at(0)->emitCode() + "\n" + config::addTabs(children.at(1)->emitCode(), 1);
//			}
//			else if (name == literalCode::PROCESS_DECLARATION_TOKEN_NAME)
//			{
//				result += children.at(0)->emitCode() + "\n\n" + config::addTabs(children.at(1)->emitCode(), 1);
//			}
//			else if (name == literalCode::PROCESS_HEADER_TOKEN_NAME)
//			{
//				result += literalCode::PROCESS_TAG_NAME + literalCode::SPACE + children.at(0)->name + literalCode::COLON;
//			}
//			else if (name == literalCode::STATEMENTS_TOKEN_NAME)
//			{
//				for (Ast* child : children)
//				{
//					result += child->emitCode() + "\n";
//				}
//
//				if (!result.empty())
//				{
//					result = result.substr(0, result.length() - 1);
//				}
//			}
//			else if (name == literalCode::LABEL_TOKEN_NAME)
//			{
//				if (children.at(0)->name == literalCode::IF_ELSE_TOKEN_NAME)
//				{
//					result.insert(result.begin(), '\n');
//				}
//
//				result += children.at(0)->name + literalCode::COLON + literalCode::SPACE + children.at(1)->emitCode();
//			}
//			else if (name == literalCode::LOCAL_ASSIGN_TOKEN_NAME)
//			{
//				result += children.at(0)->emitCode() + literalCode::SPACE + literalCode::ASSIGN_OPERATOR + literalCode::LEFT_PARENTHESIS +
//					children.at(1)->name + literalCode::RIGHT_PARENTHESIS + literalCode::SPACE +
//					children.at(2)->emitCode() + literalCode::SEMICOLON;
//			}
//			else if (name == literalCode::ID_TOKEN_NAME)
//			{
//				if (children.at(0)->name == literalCode::PC_TOKEN_NAME)
//				{
//					result += literalCode::PC_TOKEN_NAME + literalCode::LEFT_CURLY_BRACKET + children.at(0)->children.at(0)->name +
//						literalCode::RIGHT_CURLY_BRACKET;
//				}
//				else
//				{
//					result += children.at(0)->name;
//				}
//			}
//			else if (name == literalCode::INT_TOKEN_NAME)
//			{
//				result += children.at(0)->name;
//			}
//			else if (name == literalCode::BOOL_TOKEN_NAME)
//			{
//				result += children.at(0)->name;
//			}
//			else if (name == literalCode::RMA_PUT_TOKEN_NAME)
//			{
//				result += literalCode::RMA_PUT_TOKEN_NAME + literalCode::LEFT_PARENTHESIS + children.at(0)->name +
//					literalCode::COMMA + literalCode::SPACE + children.at(1)->name + literalCode::RIGHT_PARENTHESIS +
//					literalCode::SPACE + literalCode::LEFT_PARENTHESIS + children.at(2)->name + literalCode::COMMA +
//					literalCode::SPACE + children.at(3)->name + literalCode::COMMA + literalCode::SPACE + children.at(4)->name +
//					literalCode::RIGHT_PARENTHESIS + literalCode::SEMICOLON;
//			}
//			else if (name == literalCode::RMA_GET_TOKEN_NAME)
//			{
//				result += children.at(0)->name + literalCode::SPACE + literalCode::EQUALS + literalCode::SPACE +
//					literalCode::RMA_GET_TOKEN_NAME + literalCode::LEFT_PARENTHESIS + children.at(1)->name +
//					literalCode::COMMA + literalCode::SPACE + children.at(2)->name + literalCode::RIGHT_PARENTHESIS +
//					literalCode::SPACE + literalCode::LEFT_PARENTHESIS + children.at(3)->name + literalCode::COMMA +
//					literalCode::SPACE + children.at(4)->name + literalCode::RIGHT_PARENTHESIS + literalCode::SEMICOLON;
//			}
//			else if (name == literalCode::IF_ELSE_TOKEN_NAME)
//			{
//				if (parent->name != literalCode::LABEL_TOKEN_NAME)
//				{
//					result.insert(result.begin(), '\n');
//				}
//
//				result += literalCode::IF_TAG_NAME + literalCode::SPACE + literalCode::LEFT_PARENTHESIS +
//					children.at(0)->emitCode() + literalCode::RIGHT_PARENTHESIS + "\n" +
//					config::addTabs(children.at(1)->emitCode(), 1) + "\n";
//
//				if (children.at(2)->name != literalCode::NONE_TOKEN_NAME)
//				{
//					result += literalCode::ELSE_TAG_NAME + "\n" + config::addTabs(children.at(2)->emitCode(), 1) + "\n";
//				}
//
//				result += literalCode::ENDIF_TAG_NAME + literalCode::SEMICOLON;
//			}
//			else if (find(literalCode::UNARY_OPERATORS.begin(), literalCode::UNARY_OPERATORS.end(), name) != literalCode::UNARY_OPERATORS.end())
//			{
//				result += name + literalCode::LEFT_PARENTHESIS + children.at(0)->emitCode() + literalCode::RIGHT_PARENTHESIS;
//			}
//			else if (find(literalCode::BINARY_OPERATORS.begin(), literalCode::BINARY_OPERATORS.end(), name) != literalCode::BINARY_OPERATORS.end())
//			{
//				if (name == literalCode::ASTERISK_TOKEN_NAME && children.size() == 0)
//				{
//					result += name;
//				}
//				else
//				{
//					childrenCount = children.size();
//
//					for (int ctr = 0; ctr < childrenCount; ctr++)
//					{
//						if (children.at(ctr)->name == literalCode::ID_TOKEN_NAME || children.at(ctr)->name == literalCode::INT_TOKEN_NAME)
//						{
//							result += children.at(ctr)->emitCode();
//						}
//						else
//						{
//							result += literalCode::LEFT_PARENTHESIS + children.at(ctr)->emitCode() + literalCode::RIGHT_PARENTHESIS;
//						}
//
//						if (ctr < childrenCount - 1)
//						{
//							result += literalCode::SPACE + name + literalCode::SPACE;
//						}
//					}
//
//					/*if (children.at(0)->name == literalCode::ID_TOKEN_NAME || children.at(0)->name == literalCode::INT_TOKEN_NAME)
//					{
//						result += children.at(0)->emitCode();
//					}
//					else
//					{
//						result += literalCode::LEFT_PARENTHESIS + children.at(0)->emitCode() + literalCode::RIGHT_PARENTHESIS;
//					}
//
//					result += literalCode::SPACE + name + literalCode::SPACE;
//
//					if (children.at(1)->name == literalCode::ID_TOKEN_NAME || children.at(1)->name == literalCode::INT_TOKEN_NAME)
//					{
//						result += children.at(1)->emitCode();
//					}
//					else
//					{
//						result += literalCode::LEFT_PARENTHESIS + children.at(1)->emitCode() + literalCode::RIGHT_PARENTHESIS;
//					}*/
//				}
//			}
//			else if (name == literalCode::ABORT_TOKEN_NAME)
//			{
//				result += literalCode::ABORT_TOKEN_NAME + literalCode::LEFT_PARENTHESIS + literalCode::QUOTATION +
//					children.at(0)->name + literalCode::QUOTATION + literalCode::RIGHT_PARENTHESIS + literalCode::SEMICOLON;
//			}
//			else if (name == literalCode::FLUSH_TOKEN_NAME)
//			{
//				result += literalCode::FLUSH_TOKEN_NAME + literalCode::SEMICOLON;
//			}
//			else if (name == literalCode::FENCE_TOKEN_NAME)
//			{
//				result += literalCode::FENCE_TOKEN_NAME + literalCode::SEMICOLON;
//			}
//			else if (name == literalCode::GOTO_TOKEN_NAME)
//			{
//				result += literalCode::GOTO_TOKEN_NAME + literalCode::SPACE + children.at(0)->name + literalCode::SEMICOLON;
//			}
//			else if (name == literalCode::NOP_TOKEN_NAME)
//			{
//				result += literalCode::NOP_TOKEN_NAME + literalCode::SEMICOLON;
//			}
//			else if (name == literalCode::ASSUME_TOKEN_NAME)
//			{
//				result += literalCode::ASSUME_TOKEN_NAME + literalCode::LEFT_PARENTHESIS + children.at(0)->emitCode() +
//					literalCode::RIGHT_PARENTHESIS + literalCode::SEMICOLON;
//			}
//			else if (!isBooleanProgramNode)
//			{
//				config::throwError("Can't emit node: " + name);
//			}
//		}
//		else
//		{
//			if (name == literalCode::PROGRAM_DECLARATION_TOKEN_NAME)
//			{
//				for (Ast* child : children)
//				{
//					result += child->emitCode() + "\n\n";
//				}
//			}
//			else if (name == literalCode::INITIALIZATION_BLOCK_TOKEN_NAME)
//			{
//				result += literalCode::BEGINIT_TAG_NAME + "\n\n";
//
//				for (Ast* child : children)
//				{
//					result += "\t" + child->emitCode() + "\n";
//				}
//
//				result += "\n" + literalCode::ENDINIT_TAG_NAME;
//			}
//			else if (name == literalCode::PSO_TSO_STORE_TOKEN_NAME)
//			{
//				result += literalCode::PSO_TSO_STORE_TOKEN_NAME + literalCode::SPACE +
//					children.at(0)->emitCode() + literalCode::SPACE + literalCode::ASSIGN_OPERATOR +
//					literalCode::SPACE + children.at(1)->emitCode() + literalCode::SEMICOLON;
//			}
//			else if (name == literalCode::PSO_TSO_LOAD_TOKEN_NAME)
//			{
//				result += literalCode::PSO_TSO_LOAD_TOKEN_NAME + literalCode::SPACE +
//					children.at(0)->emitCode() + literalCode::SPACE + literalCode::ASSIGN_OPERATOR +
//					literalCode::SPACE + children.at(1)->emitCode() + literalCode::SEMICOLON;
//			}
//			else if (name == literalCode::LOCAL_ASSIGN_TOKEN_NAME)
//			{
//				result += children.at(0)->emitCode() + literalCode::SPACE + literalCode::ASSIGN_OPERATOR +
//					literalCode::SPACE + children.at(1)->emitCode() + literalCode::SEMICOLON;
//			}
//			else if (name == literalCode::ID_TOKEN_NAME)
//			{
//				if (children.at(0)->name == literalCode::PC_TOKEN_NAME)
//				{
//					result += literalCode::PC_TOKEN_NAME + literalCode::LEFT_CURLY_BRACKET + children.at(0)->children.at(0)->name +
//						literalCode::RIGHT_CURLY_BRACKET;
//				}
//				else
//				{
//					result += children.at(0)->name;
//				}
//			}
//			else if (name == literalCode::INT_TOKEN_NAME)
//			{
//				result += children.at(0)->name;
//			}
//			else if (name == literalCode::BOOL_TOKEN_NAME)
//			{
//				result += children.at(0)->name;
//			}
//			else if (name == literalCode::PROCESS_DECLARATION_TOKEN_NAME)
//			{
//				result += children.at(0)->emitCode() + "\n\n" + config::addTabs(children.at(1)->emitCode(), 1);
//			}
//			else if (name == literalCode::PROCESS_HEADER_TOKEN_NAME)
//			{
//				result += literalCode::PROCESS_TAG_NAME + literalCode::SPACE + children.at(0)->name + literalCode::COLON;
//			}
//			else if (name == literalCode::STATEMENTS_TOKEN_NAME)
//			{
//				for (Ast* child : children)
//				{
//					result += child->emitCode() + "\n";
//				}
//
//				if (!result.empty())
//				{
//					result = result.substr(0, result.length() - 1);
//				}
//			}
//			else if (name == literalCode::LABEL_TOKEN_NAME)
//			{
//				if (children.at(0)->name == literalCode::IF_ELSE_TOKEN_NAME)
//				{
//					result.insert(result.begin(), '\n');
//				}
//
//				result += children.at(0)->name + literalCode::COLON + literalCode::SPACE + children.at(1)->emitCode();
//			}
//			else if (name == literalCode::IF_ELSE_TOKEN_NAME)
//			{
//				if (parent->name != literalCode::LABEL_TOKEN_NAME)
//				{
//					result.insert(result.begin(), '\n');
//				}
//
//				result += literalCode::IF_TAG_NAME + literalCode::SPACE + literalCode::LEFT_PARENTHESIS +
//					children.at(0)->emitCode() + literalCode::RIGHT_PARENTHESIS + "\n" +
//					config::addTabs(children.at(1)->emitCode(), 1) + "\n";
//
//				if (children.at(2)->name != literalCode::NONE_TOKEN_NAME)
//				{
//					result += literalCode::ELSE_TAG_NAME + "\n" + config::addTabs(children.at(2)->emitCode(), 1) + "\n";
//				}
//
//				result += literalCode::ENDIF_TAG_NAME + literalCode::SEMICOLON;
//			}
//			else if (find(literalCode::UNARY_OPERATORS.begin(), literalCode::UNARY_OPERATORS.end(), name) != literalCode::UNARY_OPERATORS.end())
//			{
//				result += name + literalCode::LEFT_PARENTHESIS + children.at(0)->emitCode() + literalCode::RIGHT_PARENTHESIS;
//			}
//			else if (find(literalCode::BINARY_OPERATORS.begin(), literalCode::BINARY_OPERATORS.end(), name) != literalCode::BINARY_OPERATORS.end())
//			{
//				if (name == literalCode::ASTERISK_TOKEN_NAME && children.size() == 0)
//				{
//					result += name;
//				}
//				else
//				{
//
//					childrenCount = children.size();
//
//					for (int ctr = 0; ctr < childrenCount; ctr++)
//					{
//						if (children.at(ctr)->name == literalCode::ID_TOKEN_NAME || children.at(ctr)->name == literalCode::INT_TOKEN_NAME)
//						{
//							result += children.at(ctr)->emitCode();
//						}
//						else
//						{
//							result += literalCode::LEFT_PARENTHESIS + children.at(ctr)->emitCode() + literalCode::RIGHT_PARENTHESIS;
//						}
//
//						if (ctr < childrenCount - 1)
//						{
//							result += literalCode::SPACE + name + literalCode::SPACE;
//						}
//					}
//
//					/*if (children.at(0)->name == literalCode::ID_TOKEN_NAME || children.at(0)->name == literalCode::INT_TOKEN_NAME)
//					{
//					result += children.at(0)->emitCode();
//					}
//					else
//					{
//					result += literalCode::LEFT_PARENTHESIS + children.at(0)->emitCode() + literalCode::RIGHT_PARENTHESIS;
//					}
//
//					result += literalCode::SPACE + name + literalCode::SPACE;
//
//					if (children.at(1)->name == literalCode::ID_TOKEN_NAME || children.at(1)->name == literalCode::INT_TOKEN_NAME)
//					{
//					result += children.at(1)->emitCode();
//					}
//					else
//					{
//					result += literalCode::LEFT_PARENTHESIS + children.at(1)->emitCode() + literalCode::RIGHT_PARENTHESIS;
//					}*/
//				}
//			}
//			else if (name == literalCode::ABORT_TOKEN_NAME)
//			{
//				result += literalCode::ABORT_TOKEN_NAME + literalCode::LEFT_PARENTHESIS + literalCode::QUOTATION +
//					children.at(0)->name + literalCode::QUOTATION + literalCode::RIGHT_PARENTHESIS + literalCode::SEMICOLON;
//			}
//			else if (name == literalCode::FLUSH_TOKEN_NAME)
//			{
//				result += literalCode::FLUSH_TOKEN_NAME + literalCode::SEMICOLON;
//			}
//			else if (name == literalCode::FENCE_TOKEN_NAME)
//			{
//				result += literalCode::FENCE_TOKEN_NAME + literalCode::SEMICOLON;
//			}
//			else if (name == literalCode::GOTO_TOKEN_NAME)
//			{
//				result += literalCode::GOTO_TOKEN_NAME + literalCode::SPACE + children.at(0)->name + literalCode::SEMICOLON;
//			}
//			else if (name == literalCode::NOP_TOKEN_NAME)
//			{
//				result += literalCode::NOP_TOKEN_NAME + literalCode::SEMICOLON;
//			}
//			else if (name == literalCode::ASSUME_TOKEN_NAME)
//			{
//				result += literalCode::ASSUME_TOKEN_NAME + literalCode::LEFT_PARENTHESIS + children.at(0)->emitCode() +
//					literalCode::RIGHT_PARENTHESIS + literalCode::SEMICOLON;
//			}
//			else if (name == literalCode::BEGIN_ATOMIC_TOKEN_NAME)
//			{
//				result += literalCode::BEGIN_ATOMIC_TOKEN_NAME + literalCode::SEMICOLON;
//			}
//			else if (name == literalCode::END_ATOMIC_TOKEN_NAME)
//			{
//				result += literalCode::END_ATOMIC_TOKEN_NAME + literalCode::SEMICOLON;
//			}
//			else if (name == literalCode::CHOOSE_TOKEN_NAME)
//			{
//				result += literalCode::CHOOSE_TOKEN_NAME + literalCode::LEFT_PARENTHESIS + children.at(0)->emitCode() +
//					literalCode::COMMA + literalCode::SPACE + children.at(1)->emitCode() +
//					literalCode::RIGHT_PARENTHESIS;
//			}
//			else if (!isBooleanProgramNode)
//			{
//				config::throwError("Can't emit node: " + name);
//			}
//		}
//
//		/*if (!endComment.empty())
//		{
//		result += "\n" + literalCode::COMMENT_PREFIX + literalCode::SPACE + endComment;
//		}*/
//
//		if (!endComment.empty())
//		{
//			result += "\n" + literalCode::MULTILINE_COMMENT_PREFIX + literalCode::SPACE + endComment + literalCode::SPACE + literalCode::MULTILINE_COMMENT_SUFFIX + "\n";
//		}
//
//		if (name == literalCode::IF_ELSE_TOKEN_NAME)
//		{
//			result += "\n";
//		}
//
//		return result;
//	}
//	
//	void Ast::cascadingUnifyVariableNames()
//	{
//		if (!unifyVariableNames())
//		{
//			for (Ast* child : children)
//			{
//				child->cascadingUnifyVariableNames();
//			}
//		}
//	}
//	
//	
//	
//	void Ast::cascadingInitializeAuxiliaryVariables()
//	{
//		initializeAuxiliaryVariables();
//	
//		for (Ast* child : children)
//		{
//			child->cascadingInitializeAuxiliaryVariables();
//		}
//	}
//
//	void Ast::labelAllStatements()
//	{
//		if (name == literalCode::STATEMENTS_TOKEN_NAME)
//		{
//			vector<Ast*> replacementVector;
//
//			for (Ast* child : children)
//			{
//				if (child->name != literalCode::LABEL_TOKEN_NAME)
//				{
//					replacementVector.clear();
//					replacementVector.push_back(newLabel(config::getCurrentAuxiliaryLabel(), child));
//					config::lazyReplacements[child] = replacementVector;
//				}
//			}
//		}
//	}
//
//	bool Ast::isEquivalent(Ast* otherAst)
//	{
//		return emitCode().compare(otherAst->emitCode()) == 0;
//	}
//
//	vector<int> Ast::getAllProcessDeclarations()
//	{
//		vector<int> result;
//
//		if (name == literalCode::PROCESS_DECLARATION_TOKEN_NAME)
//		{
//			result.push_back(stoi(children.at(0)->children.at(0)->name));
//		}
//		else if (name == literalCode::BL_PROCESS_DECLARATION_TOKEN_NAME)
//		{
//			result.push_back(stoi(children.at(0)->name));
//		}
//		else
//		{
//			vector<int> subResult;
//
//			for (Ast* child : children)
//			{
//				subResult = child->getAllProcessDeclarations();
//				result.insert(result.end(), subResult.begin(), subResult.end());
//			}
//		}
//
//		return result;
//	}
//
//	void Ast::cascadingReplaceIDNames(const string &oldName, const string &newName)
//	{
//		replaceIDNames(oldName, newName);
//
//		if (name != literalCode::ID_TOKEN_NAME && name != literalCode::INT_TOKEN_NAME)
//		{
//			for (Ast* child : children)
//			{
//				child->cascadingReplaceIDNames(oldName, newName);
//			}
//		}
//	}
//
///* Static operations */
//	void Ast::replaceNode(const vector<Ast*> &nodes, Ast* oldNode)
//	{
//		Ast* newParent = oldNode->parent;
//		int newIndex = oldNode->indexAsChild;
//		int nodesCount = nodes.size();
//		int parentChildrenCount = newParent->children.size();
//	
//		newParent->children.erase(newParent->children.begin() + newIndex);
//	
//		for (int ctr = newIndex + nodesCount - 1; ctr >= newIndex; ctr--)
//		{
//			nodes[ctr - newIndex]->parent = newParent;
//			newParent->children.insert(newParent->children.begin() + newIndex, nodes[ctr - newIndex]);
//		}
//	
//		newParent->refreshChildIndices();
//	}
//	
//	void Ast::replaceNode(Ast* newNode, Ast* oldNode)
//	{
//		Ast* newParent = oldNode->parent;
//		int newIndex = oldNode->indexAsChild;
//	
//		newParent->children.erase(newParent->children.begin() + newIndex);
//		newParent->children.insert(newParent->children.begin() + newIndex, newNode);
//	
//		newParent->refreshChildIndices();
//	
//		//newParent->updateShortStringRepresentation();
//	}
//
///* Static pseudo-constructors */
//
///* Local access */
//	
//	// Outputs the number string of the superior process in the value of the out pointer and returns true if applicable, otherwise returns false
//	bool Ast::tryGetParentProcessNumber(string* out)
//	{
//		bool result = false;
//	
//		if (!isRoot())
//		{
//			if (parent->name == literalCode::PROCESS_DECLARATION_TOKEN_NAME)
//			{
//				(*out) = parent->children.at(0)->children.at(0)->name;
//				result = true;
//			}
//			else
//			{
//				result = parent->tryGetParentProcessNumber(out);
//			}
//		}
//	
//		return result;
//	}
//	
//	int Ast::numberOfVariablesInPersistentWriteBuffer()
//	{
//		int result = 0;
//	
//		for (bufferSizeMapIterator iterator = persistentWriteCost.begin(); iterator != persistentWriteCost.end(); iterator++)
//		{
//			if (iterator->second > 0 || iterator->second == bsm::TOP_VALUE)
//			{
//				result++;
//			}
//		}
//	
//		return result;
//	}
//	
//	// Returns whether the node has no parent
//	bool Ast::isRoot()
//	{
//		return indexAsChild == -1;
//	}
//	
//	// Returns whether the current node is an ifElse node with an Else statement block.
//	bool Ast::hasElse()
//	{
//		if (name == literalCode::IF_ELSE_TOKEN_NAME)
//		{
//			return children.at(2)->name != literalCode::NONE_TOKEN_NAME;
//		}
//	
//		return false;
//	}
//	
//	// Gets the qualified name representing the label to which this goto node links
//	string Ast::getGotoCode()
//	{
//		if (name == literalCode::GOTO_TOKEN_NAME)
//		{
//			string parentProcessNumber;
//			string label = children.at(0)->name;
//	
//			if (tryGetParentProcessNumber(&parentProcessNumber))
//			{
//				return parentProcessNumber + literalCode::LABEL_SEPARATOR + label;
//			}
//		}
//	
//		return "";
//	}
//	
//	// Contains all buffer size maps
//	vector<bufferSizeMap*> Ast::allBufferSizeContainers()
//	{
//		if (_allBufferSizeContainers.empty())
//		{
//			_allBufferSizeContainers.push_back(&causedWriteCost);
//			_allBufferSizeContainers.push_back(&causedReadCost);
//			_allBufferSizeContainers.push_back(&persistentWriteCost);
//			_allBufferSizeContainers.push_back(&persistentReadCost);
//		}
//	
//		return _allBufferSizeContainers;
//	}
//
///* Cascade elements */
//	bool Ast::unifyVariableNames()
//	{
//		if (name == literalCode::ID_TOKEN_NAME)
//		{
//			string variableName = children.at(0)->name;
//	
//			if (find(config::variableNames.begin(), config::variableNames.end(), children.at(0)->name) == config::variableNames.end())
//			{
//				config::variableNames.push_back(variableName);
//			}
//	
//			return true;
//		}
//	
//		return false;
//	}
//	
//	// Registers this label node with the global label container
//	void Ast::registerLabel()
//	{
//		if (name == literalCode::LABEL_TOKEN_NAME)
//		{
//			config::labelLookupMap[getLabelCode()] = this;
//		}
//	}
//	
//	
//	
//	void Ast::initializeAuxiliaryVariables()
//	{
//		if (config::currentLanguage == config::language::RMA)
//		{
//			// TODO
//			config::throwError("Auxiliary variable generation not implemented for RMA");
//		}
//		else
//		{
//			if (name == literalCode::PROGRAM_DECLARATION_TOKEN_NAME)
//			{
//				vector<string> globalVariableNames = bsm::getAllKeys(&persistentWriteCost);
//				vector<int> processes;
//	
//				for (Ast* child : children)
//				{
//					if (child->name == literalCode::PROCESS_DECLARATION_TOKEN_NAME)
//					{
//						processes.push_back(stoi(child->children.at(0)->children.at(0)->name));
//					}
//				}
//	
//				for (string globalVariableName : globalVariableNames)
//				{
//					config::globalVariables[globalVariableName] = new GlobalVariable(globalVariableName, processes);
//				}
//			}
//		}
//	}
//	
//	vector<Ast*> Ast::reportBack()
//	{
//		vector<Ast*> result;
//		vector<Ast*> subResult;
//	
//		if (name == literalCode::PSO_TSO_STORE_TOKEN_NAME || name == literalCode::PSO_TSO_LOAD_TOKEN_NAME ||
//			name == literalCode::FENCE_TOKEN_NAME || name == literalCode::FLUSH_TOKEN_NAME)
//		{
//			result.push_back(this);
//		}
//		else if (children.size() > 0)
//		{
//			for (Ast* child : children)
//			{
//				subResult = child->reportBack();
//	
//				for (Ast* reported : subResult)
//				{
//					result.push_back(reported);
//				}
//			}
//		}
//	
//		return result;
//	}
//	
//	// Compares the persistent buffer size maps of those of the direct control flow successors, and if they don't match, it sets the locally TOP values
//	// remotely to TOP. If no changes have been made, returns false.
//	bool Ast::propagateTops()
//	{
//	}
//
//	//
//	//
//	//// Initializes a unary operation node
//	//Ast* Ast::newUnaryOp(Ast* operand, string operation)
//	//{
//	//	Ast* result = new Ast();
//	//	result->name = operation;
//	//	result->addChild(operand);
//	//	return result;
//	//}
////
////void Ast::updateShortStringRepresentation()
////{
////	if (children.size() > 0)
////	{
////		string result = name + "(";
////
////		for (Ast* child : children)
////		{
////			child->updateShortStringRepresentation();
////			result += child->shortStringRepresentation + ", ";
////		}
////
////		shortStringRepresentation = result.substr(0, result.size() - 2) + ")";
////	}
////	else
////	{
////		shortStringRepresentation = name;
////	}
////
////	/*if (name == "")
////	{
////		shortStringRepresentation = name;
////	}
////	else
////	{
////		shortStringRepresentation = emitCode();
////	}*/
////}
////
////Ast::Ast(string initialName)
////{
////	// The index by which this node can be referred to from its parent's children vector. The root always has an index of -1
////	indexAsChild = -1;
////	name = initialName;
////}
////
////vector<vector<Ast*>> Ast::allCubes(vector<int> relevantAuxiliaryTemporaryVariableIndices, int cubeSizeUpperLimit)
////{
////	int numberOfVariables = relevantAuxiliaryTemporaryVariableIndices.size();
////	vector<Ast*> allPredicates;
////
////	for (int ctr = 0; ctr < numberOfVariables; ctr++)
////	{
////		allPredicates.push_back(config::globalPredicates[relevantAuxiliaryTemporaryVariableIndices[ctr]]->clone());
////		//allVariables.push_back(newID(config::auxiliaryTemporaryVariableNames[relevantAuxiliaryTemporaryVariableIndices[ctr]]));
////	}
////
////	vector<vector<Ast*>> limitedPowerSet = config::powerSetOfLimitedCardinality(allPredicates, cubeSizeUpperLimit);
////	vector<vector<Ast*>> negationPatterns;
////	vector<vector<Ast*>> allCubeSets;
////
////	for (vector<Ast*> subset : limitedPowerSet)
////	{
////		negationPatterns = allNegationPatterns(subset);
////		allCubeSets.insert(allCubeSets.end(), negationPatterns.begin(), negationPatterns.end());
////	}
////
////	return allCubeSets;
////}
////
//////Ast* Ast::newLargestImplicativeDisjunctionOfCubes(int cubeSizeUpperLimit, Ast* predicate, bool useTemporaryVariables)
//////{
//////	int numberOfPredicates = config::globalPredicates.size();
//////	vector<int> relevantIndices;
//////	string emptyPool = config::getCubeStatePool(relevantIndices);
//////	relevantIndices = config::getRelevantAuxiliaryTemporaryVariableIndices(predicate);
//////	string pool = config::getCubeStatePool(relevantIndices);
//////	vector<string> implicativeCubeStates;
//////	vector<string> unmaskedImplicativeCubeStates;
//////	vector<string> cubeStateCombinations;
//////	vector<Ast*> implicativeCubes;
//////
//////	if (relevantIndices.size() > 0)
//////	{
//////		for (int ctr = 1; ctr <= cubeSizeUpperLimit; ctr++)
//////		{
//////			implicativeCubeStates.clear();
//////			cubeStateCombinations = config::getNaryCubeStateCombinations(relevantIndices, ctr);
//////			
//////			for (string cubeStateCombination : cubeStateCombinations)
//////			{
//////				unmaskedImplicativeCubeStates = config::getImplicativeCubeStates(config::applyDecisionMask(cubeStateCombination, pool), predicate);
//////				implicativeCubeStates.insert(implicativeCubeStates.end(), unmaskedImplicativeCubeStates.begin(), unmaskedImplicativeCubeStates.end());
//////			}
//////
//////			//cout << pool << "\t";
//////			pool = config::removeDecisionsFromPool(pool, implicativeCubeStates);
//////			//cout << pool << "\n";
//////			
//////			relevantIndices.clear();
//////
//////			for (int subCtr = 0; subCtr < numberOfPredicates; subCtr++)
//////			{
//////				if (pool[subCtr] != config::CUBE_STATE_OMIT)
//////				{
//////					relevantIndices.push_back(subCtr);
//////				}
//////			}
//////
//////			for (string implicativeCubeState : implicativeCubeStates)
//////			{
//////				implicativeCubes.push_back(newBooleanVariableCube(implicativeCubeState, useTemporaryVariables));
//////			}
//////
//////			if (pool.compare(emptyPool) == 0)
//////			{
//////				break;
//////			}
//////		}
//////	}
//////
//////	if (implicativeCubes.size() == 0)
//////	{
//////		z3::context c;
//////		z3::expr trueConstant = c.bool_val(true);
//////
//////		if (config::expressionImpliesPredicate(trueConstant, predicate))
//////		{
//////			return newTrue();
//////		}
//////		else
//////		{
//////			return newFalse();
//////		}
//////	}
//////
//////	return newMultipleOperation(implicativeCubes, config::DOUBLE_OR);
//////}
////
////Ast* Ast::newLargestImplicativeDisjunctionOfCubes(vector<int> relevantAuxiliaryTemporaryVariableIndices, int cubeSizeUpperLimit, Ast* predicate)
////{
////	vector<vector<Ast*>> cubes = allCubes(relevantAuxiliaryTemporaryVariableIndices, cubeSizeUpperLimit);
////	vector<Ast*> implicativeCubeNodes;
////
////	for (vector<Ast*> cube : cubes)
////	{
////		if (config::cubeImpliesPredicate(cube, predicate))
////		{
////			implicativeCubeNodes.push_back(newMultipleOperation(cube, config::AND));
////		}
////	}
////
////	return newMultipleOperation(implicativeCubeNodes, config::OR);
////}
////
////bool Ast::equals(Ast* other)
////{
////	if (name.compare(other->name) == 0)
////	{
////		int childrenCount = children.size();
////
////		for (int ctr = 0; ctr < childrenCount; ctr++)
////		{
////			if (!(children.at(ctr)->equals(other->children.at(ctr))))
////			{
////				return false;
////			}
////		}
////
////		return true;
////	}
////
////	return false;
////}
////
////// Returns the qualified name described by the process- and label numbers
////int Ast::toLabelCode(string processNumber, string labelNumber)
////{
////	hash<string> labelHash;
////	return labelHash(processNumber + config::LABEL_SEPARATOR + labelNumber);
////}
////
////int Ast::effectiveMaxWriteBufferSize(string variableName)
////{
////	if (bufferSizeMapContains(&persistentWriteCost, variableName))
////	{
////		int maxBufferSize = persistentWriteCost[variableName];
////
////		if (maxBufferSize == config::TOP_VALUE || maxBufferSize > config::K)
////		{
////			return config::K;
////		}
////		else
////		{
////			return maxBufferSize;
////		}
////	}
////	else
////	{
////		return config::UNDEFINED_VALUE;
////	}
////}
////
////vector<vector<Ast*>> Ast::allNegationPatterns(std::vector<Ast*> idSet)
////{
////	int numberOfVariables = idSet.size();
////	vector<vector<Ast*>> result;
////
////	if (numberOfVariables > 0)
////	{
////		string firstMask = string('0', numberOfVariables);
////		string mask = string(firstMask);
////		vector<Ast*> subResult;
////
////		do {
////			subResult.clear();
////
////			for (int ctr = 0; ctr < numberOfVariables; ctr++)
////			{
////				if (mask[ctr] == '0')
////				{
////					subResult.push_back(idSet[ctr]->clone());
////				}
////				else
////				{
////					subResult.push_back(idSet[ctr]->negate());
////				}
////			}
////
////			result.push_back(subResult);
////			mask = config::nextBinaryRepresentation(mask, numberOfVariables);
////		} while (mask.compare(firstMask) != 0);
////	}
////
////	return result;
//	//}