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

/*
A node of the abstract semantic tree. Its type is contained in the name variable, unless it's an identifier's name or an integer value.
It can hold buffer size maps containing the buffer size increases caused by the program point it's representing, and the buffer size
the program would at most need at this point. May also contain directed edges to other AST nodes, representing control flow.

Cascading operations may go top to bottom or bottom to top in the tree, or down along the control flow graph, which might be cyclic.
*/

#include "Ast.h"
#include "config.h"
#include "ControlFlowEdge.h"
#include "ControlFlowVisitor.h"
#include "CubeTreeNode.h"
#include "literalCode.h"
#include "PredicateData.h"
#include "VariableEntry.h"

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
				newEdge = new ControlFlowEdge(this, config::labelLookupMap[pair<int, int>(getParentProcessNumber(),
					stoi(_children[0]->getName()))]);
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
			return _name.compare(literalCode::LABEL_TOKEN_NAME) == 0 || _name.compare(literalCode::GOTO_TOKEN_NAME) == 0 ||
				_name.compare(literalCode::RMA_GET_TOKEN_NAME) == 0 || _name.compare(literalCode::PSO_TSO_LOAD_TOKEN_NAME) == 0 ||
				_name.compare(literalCode::IF_ELSE_TOKEN_NAME) == 0 || _name.compare(literalCode::FENCE_TOKEN_NAME) == 0 ||
				_name.compare(literalCode::RMA_PUT_TOKEN_NAME) == 0 || _name.compare(literalCode::LOCAL_ASSIGN_TOKEN_NAME) == 0 ||
				_name.compare(literalCode::NOP_TOKEN_NAME) == 0 || _name.compare(literalCode::FLUSH_TOKEN_NAME) == 0 ||
				_name.compare(literalCode::BEGIN_ATOMIC_TOKEN_NAME) == 0 || _name.compare(literalCode::END_ATOMIC_TOKEN_NAME) == 0;
		}
		else
		{
			return _name.compare(literalCode::LABEL_TOKEN_NAME) == 0 || _name.compare(literalCode::GOTO_TOKEN_NAME) == 0 ||
				_name.compare(literalCode::PSO_TSO_STORE_TOKEN_NAME) == 0 || _name.compare(literalCode::PSO_TSO_LOAD_TOKEN_NAME) == 0 ||
				_name.compare(literalCode::IF_ELSE_TOKEN_NAME) == 0 || _name.compare(literalCode::FENCE_TOKEN_NAME) == 0 ||
				_name.compare(literalCode::NOP_TOKEN_NAME) == 0 || _name.compare(literalCode::FLUSH_TOKEN_NAME) == 0 ||
				_name.compare(literalCode::LOCAL_ASSIGN_TOKEN_NAME) == 0 || _name.compare(literalCode::BEGIN_ATOMIC_TOKEN_NAME) == 0 ||
				_name.compare(literalCode::END_ATOMIC_TOKEN_NAME) == 0;
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
			/*if (_parent->getParent()->getName().compare(literalCode::IF_ELSE_TOKEN_NAME) == 0)
			{
				return _parent->getParent()->tryGetNextSibling();
			}*/

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

				for (map<string, PredicateData*>::iterator it = config::predicates.begin(); it != config::predicates.end(); ++it)
				{
					auxiliaryBooleanVariableNames.push_back(it->second->getSingleBooleanVariableName());
					auxiliaryTemporaryVariableNames.push_back(it->second->getSingleTemporaryVariableName());
				}

				Ast* sharedVars = newSharedVariables(auxiliaryBooleanVariableNames);
				sharedVars->insertChild(newID(literalCode::ABORT_VARIABLE_NAME));
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
				if (config::verboseMode)
				{
					cout << "\tPerforming predicate abstraction on: " << getCode() << "\t\t\t \n\n";
				}
				
				vector<Ast*> replacementStatements;
				vector<Ast*> parallelAssignments;
				vector<string> LHSidentifers = _children[0]->getIDs();
				int process = getParentProcessNumber();

				if (LHSidentifers.size() != 1)
				{
					config::throwCriticalError("Invalid number of left-hand-side identifiers in " + getCode());
				}

				string LHSidentifier = LHSidentifers[0];
				string effectiveLHSidentifier;
				VariableEntry* LHSsymbol = config::symbolMap[LHSidentifier];

				if (LHSsymbol->getType() == VariableEntry::AUXILIARY && LHSsymbol->getAuxiliaryType() == VariableEntry::BUFFER)
				{
					effectiveLHSidentifier = LHSsymbol->getGlobalName();
				}
				else
				{
					effectiveLHSidentifier = LHSidentifier;
				}

				Ast* effectiveAssignment = newBinaryOp(newID(effectiveLHSidentifier), _children[1]->clone(), literalCode::EQUALS);
				vector<string> phis = config::getOriginalPredicateCodesContainingSymbol(effectiveLHSidentifier);
				Ast* WP_phi;
				Ast* WP_notPhi;
				vector<PredicateData*> currentParallelLHS;
				PredicateData* currentPhiPredicate;
				PredicateData* originalReplacement;
				Ast* chooseExpression;

				for (string phi : phis)
				{
					currentParallelLHS.clear();
					currentPhiPredicate = config::predicates[phi];

					WP_phi = config::getWeakestLiberalPrecondition(effectiveAssignment, currentPhiPredicate->getPredicateAst());
					WP_notPhi = config::getWeakestLiberalPrecondition(effectiveAssignment, currentPhiPredicate->getPredicateAst()->negate());

					if (LHSidentifier.compare(effectiveLHSidentifier) == 0)
					{
						currentParallelLHS.push_back(currentPhiPredicate);
					}
					else
					{
						originalReplacement = config::predicates[currentPhiPredicate->getXReplacedByYCode(effectiveLHSidentifier, LHSidentifier)];
						currentParallelLHS = originalReplacement->getAllReplacementVariants(process);
						currentParallelLHS.insert(currentParallelLHS.begin(), originalReplacement);
					}

					chooseExpression = newChoose(
							config::getLargestImplicativeDisjunctionOfCubes(config::globalCubeSizeLimit, WP_phi),
							config::getLargestImplicativeDisjunctionOfCubes(config::globalCubeSizeLimit, WP_notPhi)
						);

					if (LHSidentifier.compare(effectiveLHSidentifier) != 0)
					{
						chooseExpression->expandIDNodes(LHSidentifier, process);
					}

					for (PredicateData* LHS : currentParallelLHS)
					{
						parallelAssignments.push_back(newStore(LHS->getSingleBooleanVariableName(), chooseExpression));
					}
				}

				bool isInInitializationBlock = _parent->getName() != literalCode::INITIALIZATION_BLOCK_TOKEN_NAME ||
					_parent->getName() != literalCode::BL_INITIALIZATION_BLOCK_TOKEN_NAME;

				if (!isInInitializationBlock)
				{
					replacementStatements.push_back(newBeginAtomic());
				}

				string currentBooleanVariableName;

				for (Ast* parallelAssignment : parallelAssignments)
				{
					currentBooleanVariableName = parallelAssignment->getChild(0)->getChild(0)->getName();
					replacementStatements.push_back(
							newLoad(
								config::symbolMap[currentBooleanVariableName]->getTemporaryVariantName(),
								newID(currentBooleanVariableName)
							)
						);
				}

				replacementStatements.insert(replacementStatements.end(), parallelAssignments.begin(),
					parallelAssignments.end());

				for (Ast* parallelAssignment : parallelAssignments)
				{
					replacementStatements.push_back(
							newLocalAssign(config::symbolMap[parallelAssignment->getChild(0)->getChild(0)->getName()]->getTemporaryVariantName(),
							newINT(0))
						);
				}

				replacementStatements.push_back(
					config::getAssumptionOfNegatedLargestFalseImplicativeDisjunctionOfCubes());
				if (!isInInitializationBlock)
				{
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
				Ast* newConjunction = newBinaryOp(_children[0], newBinaryOp(newID(literalCode::ABORT_VARIABLE_NAME), newINT(0), literalCode::EQUALS),
					literalCode::DOUBLE_AND);
				_children[0]->setParent(newConjunction);
				newConjunction->setParent(this);
				_children.erase(_children.begin());
				_children.push_back(newConjunction);

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

				set<string> tempSet;

				for (Ast* nonPcAssertion : nonPcAssertions)
				{
					tempSet = nonPcAssertion->getIDset();

					if (tempSet.find(literalCode::ABORT_VARIABLE_NAME) == tempSet.end())
					{
						currentReplacement.clear();
						currentReplacement.push_back(config::getLargestImplicativeDisjunctionOfCubes(
							config::globalCubeSizeLimit, nonPcAssertion, false));
						config::prepareNodeForLazyReplacement(currentReplacement, nonPcAssertion);
					}
				}

				result = true;
			}
			else if (_name == literalCode::ABORT_TOKEN_NAME)
			{
				replaceNodeWithoutDeletions(newStore(literalCode::ABORT_VARIABLE_NAME, newINT(1)));

				result = true;
			}
			else if (_name == literalCode::GOTO_TOKEN_NAME && _parent->getName() != literalCode::BL_IF_TOKEN_NAME)
			{
				replaceNodeWithoutDeletions(newBooleanIf(newTrue(), clone()));

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
			if (_name == literalCode::LABEL_TOKEN_NAME && (!_children[0]->getStartComment().empty() || _children[0]->getEndComment().empty()))
			{
				_startComment = _children[0]->getStartComment();
				_endComment = _children[0]->getEndComment();
				_children[0]->setStartComment("");
				_children[0]->setEndComment("");
			}

			if (_name == literalCode::LABEL_TOKEN_NAME && _children[0]->getName() == "6680")
			{
				int c = 25;
				//config::throwCriticalError("");
			}

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

					string interjection = "";

					if (_children.at(1)->getName() == literalCode::LABEL_TOKEN_NAME)
					{
						interjection = literalCode::NOP_TOKEN_NAME + literalCode::SEMICOLON + "\n";
					}

					result += _children.at(0)->getName() + literalCode::COLON + literalCode::SPACE + interjection + _children.at(1)->getCode();
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

					string interjection = "";

					if (_children.at(1)->getName() == literalCode::LABEL_TOKEN_NAME)
					{
						interjection = literalCode::NOP_TOKEN_NAME + literalCode::SEMICOLON + "\n";
					}

					result += _children.at(0)->getName() + literalCode::COLON + literalCode::SPACE + interjection + _children.at(1)->getCode();
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

		if (index < _children.size())
		{
			_children.insert(_children.begin() + index, newChild);
		}
		else
		{
			_children.push_back(newChild);
		}

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
		_children.erase(_children.begin() + index);

		invalidateCode();

		_childrenCount = _children.size();
	}

	void Ast::replaceChild(Ast* newChild, int index)
	{
		_children.erase(_children.begin() + index);

		newChild->setParent(this);
		_children.insert(_children.begin() + index, newChild);

		invalidateCode();

		_childrenCount = _children.size();
	}

	void Ast::replaceChild(const vector<Ast*> &newChildren, int index)
	{
		_children.erase(_children.begin() + index);

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
		invalidateCode();
	}

	const string Ast::getEndComment()
	{
		return _endComment;
	}

	void Ast::setEndComment(const string &value)
	{
		_endComment = value;
		invalidateCode();
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
		bool conditionalIsAsterisk;
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
			conditionalIsAsterisk = conditional->getName() == literalCode::ASTERISK_TOKEN_NAME;
			elseLabel = hasElse ? config::getCurrentAuxiliaryLabel() : -1;
			endLabel = config::getCurrentAuxiliaryLabel();

			replacementStatements.clear();
			currentStatements.clear();

			if (!conditionalIsAsterisk)
			{
				replacementStatements.push_back(newAssume(
						newReverseLargestImplicativeDisjunctionOfCubes(config::globalCubeSizeLimit, conditional))
					);
			}

			currentStatements = currentIfElseStatement->getChild(1)->getChildren();
			replacementStatements.insert(replacementStatements.end(), currentStatements.begin(), currentStatements.end());

			if (hasElse)
			{
				replacementStatements.push_back(newGoto(endLabel));

				replacementStatements.push_back(
						newLabel(
							elseLabel,
							conditionalIsAsterisk ? newAssume(
								newReverseLargestImplicativeDisjunctionOfCubes(config::globalCubeSizeLimit, conditional->negate())
							) : newNop()
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

	void Ast::expandIDNodes(const std::string &bufferIdentifier, int process)
	{
		set<string> IDs;
		set<string> currentIDs;
		VariableEntry* currentVariable;
		vector<PredicateData*> currentPredicates;
		int currentPredicatesCount;
		int currentBufferSize;
		map<string, vector<string>> replacementsMap;
		vector<string> IDvector = getIDs();
		vector<string> currentBufferVariableNames;
		PredicateData* currentPredicate;
		string globalIdentifier = config::symbolMap[bufferIdentifier]->getGlobalName();
		bool usingTemporaryVariables = false;

		for (string ID : IDvector)
		{
			IDs.insert(ID);
		}

		if (IDvector.size() > 0)
		{
			usingTemporaryVariables = IDvector[0][0] == literalCode::TEMPORARY_VARIABLE_PREFIX;
		}

		for (set<string>::iterator it = IDs.begin(); it != IDs.end(); ++it)
		{
			currentPredicate = config::symbolMap[*it]->getAssociatedPredicate();
			currentIDs = currentPredicate->getPredicateIDs();

			if (currentIDs.find(globalIdentifier) != currentIDs.end())
			{
				currentPredicates.clear();

				currentPredicate = config::predicates[currentPredicate->getXReplacedByYCode(globalIdentifier, bufferIdentifier)];

				currentPredicates.push_back(currentPredicate);

				currentIDs = currentPredicate->getPredicateIDs();
				currentIDs.erase(bufferIdentifier);

				for (set<string>::iterator subIt = currentIDs.begin(); subIt != currentIDs.end(); ++subIt)
				{
					currentVariable = config::symbolMap[*subIt];

					if (currentVariable->getType() == VariableEntry::GLOBAL)
					{
						currentBufferSize = currentVariable->getMaximumBufferSize(process);
						currentBufferVariableNames = currentVariable->getAuxiliaryBufferNames(process);
						currentBufferVariableNames.erase(currentBufferVariableNames.begin() + currentBufferSize, currentBufferVariableNames.end());
						currentPredicatesCount = currentPredicates.size();

						for (int ctr = 0; ctr < currentPredicatesCount; ++ctr)
						{
							for (string bufferVariableName : currentBufferVariableNames)
							{
								currentPredicates.push_back(config::predicates[currentPredicates[ctr]->getXReplacedByYCode(*subIt, bufferVariableName)]);
							}
						}
					}
				}

				for (PredicateData* predicate : currentPredicates)
				{
					replacementsMap[*it].push_back(usingTemporaryVariables ? predicate->getSingleTemporaryVariableName() :
						predicate->getSingleBooleanVariableName());
				}
			}
		}

		vector<Ast*> toBeChecked;
		vector<Ast*> currentChildren;
		vector<Ast*> currentReplacements;
		toBeChecked.insert(toBeChecked.begin(), _children.begin(), _children.end());
		Ast* currentlyChecked;
		string currentIdentifier;
		string currentOperator;

		while (!toBeChecked.empty())
		{
			currentlyChecked = toBeChecked[0];

			if (currentlyChecked->getName() == literalCode::EQUALS || currentlyChecked->getName() == literalCode::NOT_EQUALS)
			{
				currentIdentifier = currentlyChecked->getChild(0)->getChild(0)->getName();

				if (replacementsMap.find(currentIdentifier) != replacementsMap.end())
				{
					IDvector = replacementsMap[currentIdentifier];

					if (IDvector.size() == 1)
					{
						currentlyChecked->getChild(0)->getChild(0)->setName(IDvector[0]);
					}
					else
					{
						currentOperator = currentlyChecked->getName();
						
						currentReplacements.clear();
						for (string ID : IDvector)
						{
							currentReplacements.push_back(newBinaryOp(newID(ID), newINT(0), currentOperator));
						}

						currentlyChecked->getParent()->replaceChild(currentReplacements, currentlyChecked->getIndexAsChild());
					}
				}
			}
			else
			{
				currentChildren = currentlyChecked->getChildren();
				toBeChecked.insert(toBeChecked.end(), currentChildren.begin(), currentChildren.end());
			}

			toBeChecked.erase(toBeChecked.begin());
		}
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
		string startComment = statement->getStartComment();
		statement->setStartComment("");
		string endComment = statement->getEndComment();
		statement->setEndComment("");

		Ast* result = new Ast(literalCode::LABEL_TOKEN_NAME);
		result->insertChild(new Ast(to_string(value)));
		result->insertChild(statement);

		result->setStartComment(startComment);
		result->setEndComment(endComment);

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
		vector<Ast*> cubeTerms;
		PredicateData* currentPredicate;
		bool currentDecision;
		int originalPredicatesCount = config::originalPredicateCodes.size();

		for (int ctr = 0; ctr < originalPredicatesCount; ctr++)
		{
			if (definition[ctr] == CubeTreeNode::CUBE_STATE_DECIDED_FALSE ||
				definition[ctr] == CubeTreeNode::CUBE_STATE_DECIDED_TRUE)
			{
				currentDecision = definition[ctr] == CubeTreeNode::CUBE_STATE_DECIDED_TRUE;
				currentPredicate = config::predicates[config::originalPredicateCodes[ctr]];

				cubeTerms.push_back(useTemporaryVariables ? currentPredicate->getTemporaryRHS(currentDecision) :
					currentPredicate->getBooleanRHS(currentDecision));
			}
		}

		return newMultipleOperation(cubeTerms, literalCode::DOUBLE_AND);
	}

	Ast* Ast::newLargestImplicativeDisjunctionOfCubes(int cubeSizeUpperLimit, Ast* predicate, bool useTemporaryVariables)
	{
		time_t startedAt;
		time_t finishedAt;
		double elapsedSeconds;

		if (config::verboseMode)
		{
			startedAt = time(0);
		}

		vector<Ast*> implicativeCubes;
		set<string> predicateIDs;

		if (config::verboseMode)
		{
			cout << "\t\tComputing F(" << predicate->getCode() << ")";
		}

		vector<string> predicateIDvector = predicate->getIDs();

		for (string id : predicateIDvector)
		{
			predicateIDs.insert(id);
		}

		vector<int> indices = config::getVariableTransitiveClosureFromOriginalPredicates(predicateIDs);
		vector<string> implicativeCubeStringRepresentations = config::getMinimalImplyingCubeStringRepresentations(predicate, indices);

		for (string implicativeCubeStringRepresentation : implicativeCubeStringRepresentations)
		{
			implicativeCubes.push_back(newBooleanVariableCube(implicativeCubeStringRepresentation, useTemporaryVariables));
		}

		if (implicativeCubes.size() == 0)
		{
			if (config::isTautology(predicate))
			{
				if (config::verboseMode)
				{
					finishedAt = time(0);
					elapsedSeconds = difftime(finishedAt, startedAt);
					cout << "\t" << elapsedSeconds << "s\t\t \n";
				}

				return newTrue();
			}
			else
			{
				if (config::verboseMode)
				{
					finishedAt = time(0);
					elapsedSeconds = difftime(finishedAt, startedAt);
					cout << "\t" << elapsedSeconds << "s\t\t \n";
				}

				return newFalse();
			}
		}

		if (config::verboseMode)
		{
			finishedAt = time(0);
			elapsedSeconds = difftime(finishedAt, startedAt);
			cout << "\t" << elapsedSeconds << "s\t\t \n";
		}

		return newMultipleOperation(implicativeCubes, literalCode::DOUBLE_OR);
	}

	Ast* Ast::newReverseLargestImplicativeDisjunctionOfCubes(int cubeSizeUpperLimit, Ast* predicate)
	{
		return config::getLargestImplicativeDisjunctionOfCubes(cubeSizeUpperLimit, predicate->negate(), false)->negate();
	}

	Ast* Ast::newAbstractAssignmentFragment(Ast* assignment, Ast* predicate)
	{
		return newChoose(
				config::getLargestImplicativeDisjunctionOfCubes(config::globalCubeSizeLimit,
					config::getWeakestLiberalPrecondition(assignment, predicate)),
				config::getLargestImplicativeDisjunctionOfCubes(config::globalCubeSizeLimit,
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
			if (config::verboseMode)
			{
				cout << "\t\tComputing WP(" << assignment->getCode() << ", " << predicate->getCode() << ")\t\t\t \n";
			}
	
			if (assignment->getName() == literalCode::EQUALS ||
				assignment->getName() == literalCode::PSO_TSO_STORE_TOKEN_NAME ||
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
				//config::throwCriticalError("Couldn't register global symbol \"" + ID + "\"");
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
				results.insert(results.begin(), subResults.begin(), subResults.end());
			}
		}

		return results;
	}

	// Cascades from top to bottom and back. Returns a string set containing all identifiers along the downward path, including the current node's own.
	set<string> Ast::getIDset()
	{
		set<string> results;

		if (_name == literalCode::ID_TOKEN_NAME)	// If this is an identifier, push it on the vector to return it upwards
		{
			results.insert(_children.at(0)->getName());
		}
		else // If this isn't an identifier, gather all identifiers from the progeny
		{
			set<string> subResults;

			for (Ast* child : _children)
			{
				subResults = child->getIDset();
				results.insert(subResults.begin(), subResults.end());
			}
		}

		return results;
	}

	// Cascades from top to bottom and back. Returns an Ast* vector containing all identifier nodes along the downward path, including the current node's own.
	vector<Ast*> Ast::fetchIDs()
	{
		vector<Ast*> results;

		if (_name == literalCode::ID_TOKEN_NAME)	// If this is an identifier, push it on the vector to return it upwards
		{
			results.push_back(this);
		}
		else // If this isn't an identifier, gather all identifiers from the progeny
		{
			vector<Ast*> subResults;

			for (Ast* child : _children)
			{
				subResults = child->fetchIDs();
				results.insert(results.begin(), subResults.begin(), subResults.end());
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
			int process = getParentProcessNumber();
			int label = stoi(_children[0]->getName());

			if (!config::tryRegisterLabel(process, label))
			{
				config::throwCriticalError("Couldn't register label " + _children[0]->getName() + " for " + getCode());
			}
			else
			{
				config::labelLookupMap[pair<int, int>(process, label)] = this;
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

			for (int ctr = 0; ctr < (int)_children.size(); ctr++)
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

	void Ast::topDownCascadingReportBufferSizes(int process)
	{
		int effectiveProcess = process == -1 ? getParentProcessNumber() : process;

		if (effectiveProcess != -1)
		{
			for (bufferSizeMapIterator it = persistentWriteCost.begin(); it != persistentWriteCost.end(); it++)
			{
				config::symbolMap[it->first]->increaseMaximumBufferSize(effectiveProcess, it->second);
			}
		}

		for (Ast* child : _children)
		{
			child->topDownCascadingReportBufferSizes(effectiveProcess);
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

	void Ast::topDownCascadingLabelAllStatements()
	{
		if (_name == literalCode::STATEMENTS_TOKEN_NAME || _name == literalCode::INITIALIZATION_BLOCK_TOKEN_NAME ||
			_name == literalCode::BL_INITIALIZATION_BLOCK_TOKEN_NAME)
		{
			Ast* currentLabel;

			for (int ctr = 0; ctr < _childrenCount; ++ctr)
			{
				if (_children[ctr]->getName() != literalCode::LABEL_TOKEN_NAME)
				{
					currentLabel = newLabel(config::getCurrentAuxiliaryLabel(), _children[ctr]);
					currentLabel->setStartComment(_children[ctr]->getStartComment());
					currentLabel->setEndComment(_children[ctr]->getEndComment());
					_children[ctr]->setStartComment("");
					_children[ctr]->setEndComment("");
					_children[ctr]->setParent(currentLabel);
					_children.erase(_children.begin() + ctr);
					_children.insert(_children.begin() + ctr, currentLabel);
					_children[ctr]->invalidateCode();
				}
			}
		}

		for (int ctr = 0; ctr < _childrenCount; ++ctr)
		{
			_children[ctr]->topDownCascadingLabelAllStatements();
		}
	}

	void Ast::performBufferSizeAnalysisReplacements()
	{
		if (config::currentLanguage == config::language::RMA)
		{
			// TODO
			config::throwError("Buffer size analysis not implemented for RMA");
		}
		else if (config::currentLanguage == config::language::SALPL && config::memoryOrdering == config::TSO)
		{
			// TODO
			config::throwError("Buffer size analysis not implemented for TSO");
		}
		else if (config::currentLanguage == config::language::SALPL && config::memoryOrdering == config::PSO)
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
						// For statement replacement purposes, the initialization assignments count as one buffer write
						s = max(1, persistentWriteCost[globalVariableName]);
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
						currentStatements.push_back(newLocalAssign(bufferVariables[i - 1], _children[1]->clone()));

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
						// For statement replacement purposes, the initialization assignments count as one buffer write
						s = max(1, persistentWriteCost[globalVariableName]);
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
								// For statement replacement purposes, the initialization assignments count as one buffer write
								s = max(1, persistentWriteCost[globalVariableName]);
							}

							if (s > 1)
							{
								currentThenStatements.push_back(newStore(globalVariableName, newID(bufferVariables[s - 1])));
								currentElseStatements.push_back(newStore(globalVariableName, newID(bufferVariables[s - 2])));
								currentIfElse = newIfElse(
										newBinaryOp(newID(x_fst_t), newINT(s - 1), string(1, literalCode::GREATER_THAN)),
										currentThenStatements,
										currentElseStatements
									);
							}
							else
							{
								currentIfElse = newStore(globalVariableName, newID(bufferVariables[s - 1]));
							}

							for (int i = s - 1; i > 0; --i)
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
			vector<Ast*> assignmentBlock;

			for (map<string, PredicateData*>::iterator it = config::predicates.begin(); it != config::predicates.end(); ++it)
			{
				assignmentBlock.push_back(
						newLabel(
							config::getCurrentAuxiliaryLabel(),
							newStore(it->second->getSingleBooleanVariableName(), newAsterisk())
						)
					);
			}

			for (map<string, PredicateData*>::iterator it = config::predicates.begin(); it != config::predicates.end(); ++it)
			{
				assignmentBlock.push_back(
						newLabel(
								config::getCurrentAuxiliaryLabel(),
								newLoad(it->second->getSingleTemporaryVariableName(), newID(it->second->getSingleBooleanVariableName()))
							)
					);
			}

			_children[2]->insertChildren(assignmentBlock, 0);
			assignmentBlock.clear();
			_children[2]->getChild(0)->setStartComment("Initializing local variables");

			for (map<string, PredicateData*>::iterator it = config::predicates.begin(); it != config::predicates.end(); ++it)
			{
				assignmentBlock.push_back(newLabel(config::getCurrentAuxiliaryLabel(),
					newLocalAssign(it->second->getSingleTemporaryVariableName(), newINT(0))));
			}

			assignmentBlock.push_back(newLabel(config::getCurrentAuxiliaryLabel(), newStore(literalCode::ABORT_VARIABLE_NAME, newINT(0))));

			assignmentBlock[0]->setStartComment("Resetting local variables");
			_children[2]->insertChildren(assignmentBlock);

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