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
#include "ControlFlowEdge.h"
//#include "Cube.h"
#include "CubeTreeNode.h"

using namespace std;

/* Constructors and destructor */
	Ast::Ast()
	{
		// The index by which this node can be referred to from its parent's children vector. The root always has an index of -1
		indexAsChild = -1;
		name = "";
		parent = nullptr;

		//updateShortStringRepresentation();
	}

	Ast::~Ast()
	{
	}

/* Replicators */
	// Creates a clone of this and all successor nodes, all of which correspond to their source in all but bufferSizeMaps, comments and control flow data
	Ast* Ast::clone()
	{
		Ast* result = new Ast();
		result->name = name;
	
		for (Ast* child : children)
		{
			result->addChild(child->clone());
		}
	
		return result;
	}
	
	Ast* Ast::negate()
	{
		Ast* result;
	
		if (name == literalCode::EQUALS || name == literalCode::LESS_THAN || name == literalCode::LESS_EQUALS ||
			name == string(1, literalCode::GREATER_THAN) || name == literalCode::GREATER_EQUALS || name == literalCode::NOT_EQUALS ||
			name == literalCode::AND || name == literalCode::OR || name == literalCode::DOUBLE_AND || name == literalCode::DOUBLE_OR ||
			name == literalCode::ID_TOKEN_NAME)
		{
			Ast* selfClone = clone();
			result = new Ast();
			result->name = literalCode::NOT;
			result->addChild(selfClone);
		}
		else if (name == literalCode::NOT)
		{
			result = children.at(0)->clone();
		}
		else if (name == literalCode::BOOL_TOKEN_NAME)
		{
			if (children.at(0)->name == literalCode::TRUE_TAG_NAME)
			{
				result = newFalse();
			}
			else if (children.at(0)->name == literalCode::FALSE_TAG_NAME)
			{
				result = newTrue();
			}
			else
			{
				config::throwError("Can't negate BOOL(" + children.at(0)->name + ")");
			}
		}
		else if (name == literalCode::INT_TOKEN_NAME)
		{
			if (children.at(0)->name == "0")
			{
				result = newINT(1);
			}
			else if (children.at(0)->name == "1")
			{
				result = newINT(0);
			}
			else
			{
				config::throwError("Can't negate INT(" + children.at(0)->name + ")");
			}
		}
	
		return result;
	}

/* Access */
	// Adds a child node and sets its variables connecting it to this node
	void Ast::addChild(Ast* child)
	{
		child->parent = this;
		child->indexAsChild = children.size();
		children.push_back(child);
	
		//updateShortStringRepresentation();
	}
	
	// Adds a child node at a specific index and sets its variables connecting it to this node
	void Ast::addChild(Ast* child, int index)
	{
		child->parent = this;
		child->indexAsChild = index;

		if (index == children.size())
		{
			children.insert(children.end(), child);
		}
		else if (index < children.size())
		{
			children.insert(children.begin() + index, child);
		}
		else
		{
			/*config::throwCriticalError("Error trying to insert: " + child->emitCode() + " on the index " + to_string(index) +
				". The current vector size is " + to_string(children.size()) + ".");*/
			children.insert(children.begin() + index, child);
		}
	
		refreshChildIndices();
		//updateShortStringRepresentation();
	}
	
	void Ast::addChildren(const vector<Ast*> &newChildren)
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
	
	vector<Ast*> Ast::search(const string &soughtName)
	{
		vector<Ast*> results;
	
		if (name != soughtName)
		{
			vector<Ast*> subResults;
	
			for (Ast* child : children)
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
	
	// Gets the qualified name representing this label
	string Ast::getLabelCode()
	{
		if (name == literalCode::LABEL_TOKEN_NAME)
		{
			string parentProcessNumber;
			string label = children.at(0)->name;
	
			if (tryGetParentProcessNumber(&parentProcessNumber))
			{
				return parentProcessNumber + literalCode::LABEL_SEPARATOR + label;
			}
		}
	
		return "";
	}
	
	Ast* Ast::weakestLiberalPrecondition(Ast* predicate)
	{
		Ast* result;
	
		if (config::currentLanguage == config::language::RMA)
		{
			// TODO
			config::throwError("Predicate abstraction operations not implemented for RMA");
		}
		else
		{
			cout << "\t\tComputing WP(" << emitCode() << ", " << predicate->emitCode() << ")\t\t\t \n";

			if (name == literalCode::PSO_TSO_STORE_TOKEN_NAME || name == literalCode::PSO_TSO_LOAD_TOKEN_NAME
				|| name == literalCode::LOCAL_ASSIGN_TOKEN_NAME)
			{
				string leftVariable = children.at(0)->children.at(0)->name;
				Ast* rightExpression = children.at(1);
				result = predicate->clone();
				vector<Ast*> toBeReplaced = result->search(leftVariable);
	
				for (Ast* oldId : toBeReplaced)
				{
					replaceNode(rightExpression->clone(), oldId->parent);
				}
	
				//result->updateShortStringRepresentation();
			}
			else
			{
				result = new Ast();
			}
		}
	
		//cout << "\tWP(" + emitCode() + ", " + predicate->emitCode() + ") = (" + result->emitCode() + ")\n";
	
		return result;
	}
	
	// Returns a string representation of the node and its children
	string Ast::toString()
	{
		regex indentationRegex("\n");
		string result = name;
	
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
	
		int childrenCount = children.size();
	
		for (int ctr = 0; ctr < childrenCount; ctr++)
		{
			result += "\n" + children.at(ctr)->toString();
		}
	
		result = regex_replace(result, indentationRegex, "\n|");
	
		return result;
	}
	
	z3::expr Ast::toZ3Expression(z3::context* c)
	{
		if (config::currentLanguage == config::language::RMA)
		{
			// TODO
			config::throwError("Z3 expression conversion not implemented for RMA");
		}
		else
		{
			if (name == literalCode::ID_TOKEN_NAME)
			{
				return c->int_const(children.at(0)->name.c_str());
			}
			else if (name == literalCode::INT_TOKEN_NAME)
			{
				return c->int_val(children.at(0)->name.c_str());
			}
			else if (name == literalCode::BOOL_TOKEN_NAME)
			{
				return c->bool_const(children.at(0)->name.c_str());
			}
			else if (name == literalCode::BINARY_OPERATORS[0])	// +
			{
				return children.at(0)->toZ3Expression(c) + children.at(1)->toZ3Expression(c);
			}
			else if (name == literalCode::BINARY_OPERATORS[1])	// -
			{
				return children.at(0)->toZ3Expression(c) - children.at(1)->toZ3Expression(c);
			}
			else if (name == literalCode::BINARY_OPERATORS[2])	// *
			{
				return children.at(0)->toZ3Expression(c) * children.at(1)->toZ3Expression(c);
			}
			else if (name == literalCode::BINARY_OPERATORS[3])	// /
			{
				return children.at(0)->toZ3Expression(c) / children.at(1)->toZ3Expression(c);
			}
			else if (name == literalCode::BINARY_OPERATORS[4])	// &
			{
				return children.at(0)->toZ3Expression(c) && children.at(1)->toZ3Expression(c);
			}
			else if (name == literalCode::BINARY_OPERATORS[5])	// |
			{
				return children.at(0)->toZ3Expression(c) || children.at(1)->toZ3Expression(c);
			}
			else if (name == literalCode::BINARY_OPERATORS[6])	// <
			{
				return children.at(0)->toZ3Expression(c) < children.at(1)->toZ3Expression(c);
			}
			else if (name == literalCode::BINARY_OPERATORS[7])	// >
			{
				return children.at(0)->toZ3Expression(c) > children.at(1)->toZ3Expression(c);
			}
			else if (name == literalCode::BINARY_OPERATORS[8])	// <=
			{
				return children.at(0)->toZ3Expression(c) <= children.at(1)->toZ3Expression(c);
			}
			else if (name == literalCode::BINARY_OPERATORS[9])	// >=
			{
				return children.at(0)->toZ3Expression(c) >= children.at(1)->toZ3Expression(c);
			}
			else if (name == literalCode::BINARY_OPERATORS[10])	// =
			{
				// Ignore
			}
			else if (name == literalCode::BINARY_OPERATORS[11])	// ==
			{
				return children.at(0)->toZ3Expression(c) == children.at(1)->toZ3Expression(c);
			}
			else if (name == literalCode::BINARY_OPERATORS[12])	// !=
			{
				return children.at(0)->toZ3Expression(c) != children.at(1)->toZ3Expression(c);
			}
			else if (name == literalCode::UNARY_OPERATORS[0])	// !
			{
				return !children.at(0)->toZ3Expression(c);
			}
		}
	
		// If all else fails
		config::throwCriticalError("Can't convert the following to a Z3 expression:\n" + toString());
		return c->int_const("INVALID");
	}

/* Cascading operations */
	string Ast::emitCode()
	{
		string result = "";
		bool isBooleanProgramNode = false;

		if (name == literalCode::BL_PROGRAM_DECLARATION_TOKEN_NAME)
		{
			for (Ast* child : children)
			{
				result += child->emitCode() + "\n\n";
			}

			isBooleanProgramNode = true;
		}
		else if (name == literalCode::BL_INITIALIZATION_BLOCK_TOKEN_NAME)
		{
			string subResult = "";

			for (Ast* child : children)
			{
				subResult += child->emitCode() + "\n";
			}

			if (!subResult.empty())
			{
				subResult = subResult.substr(0, subResult.length() - 1);
			}

			result += literalCode::INIT_TAG_NAME + "\n\n" + config::addTabs(subResult, 1) + "\n\n";

			isBooleanProgramNode = true;
		}
		else if (name == literalCode::BL_SHARED_VARIABLES_BLOCK_TOKEN_NAME)
		{
			result += literalCode::BL_SHARED_VARIABLES_BLOCK_TOKEN_NAME;

			for (Ast* child : children)
			{
				result += " " + child->emitCode() + ",";
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
		else if (name == literalCode::BL_LOCAL_VARIABLES_BLOCK_TOKEN_NAME)
		{
			result += literalCode::BL_LOCAL_VARIABLES_BLOCK_TOKEN_NAME;

			for (Ast* child : children)
			{
				result += " " + child->emitCode() + ",";
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
		else if (name == literalCode::BL_PROCESS_DECLARATION_TOKEN_NAME)
		{
			result += literalCode::PROCESS_TAG_NAME + " " + children.at(0)->children.at(0)->name + "\n\n" + config::addTabs(children.at(1)->emitCode(), 1);

			isBooleanProgramNode = true;
		}
		else if (name == literalCode::BL_IF_TOKEN_NAME)
		{
			result += literalCode::IF_TAG_NAME + literalCode::SPACE + literalCode::LEFT_PARENTHESIS +
				children.at(0)->emitCode() + literalCode::RIGHT_PARENTHESIS + literalCode::SPACE + children.at(1)->emitCode();

			isBooleanProgramNode = true;
		}
		else if (name == literalCode::ASSERT_TOKEN_NAME)
		{
			result += literalCode::ASSERT_TOKEN_NAME + literalCode::SPACE + literalCode::BL_ALWAYS_TOKEN_NAME +
				literalCode::LEFT_PARENTHESIS +	children.at(0)->emitCode() + literalCode::RIGHT_PARENTHESIS +
				literalCode::SEMICOLON;

			isBooleanProgramNode = true;
		}

		if (!startComment.empty())
		{
			result += "\n" + literalCode::MULTILINE_COMMENT_PREFIX + literalCode::SPACE + startComment + literalCode::SPACE + literalCode::MULTILINE_COMMENT_SUFFIX + "\n";
		}

		/*if (!startComment.empty())
		{
		result += literalCode::COMMENT_PREFIX + literalCode::SPACE + startComment + "\n";
		}*/

		if (config::currentLanguage == config::language::RMA)
		{
			if (name == literalCode::PROGRAM_DECLARATION_TOKEN_NAME)
			{
				for (Ast* child : children)
				{
					result += child->emitCode() + "\n\n";
				}
			}
			else if (name == literalCode::INITIALIZATION_BLOCK_TOKEN_NAME)
			{
				result += literalCode::BEGINIT_TAG_NAME + "\n\n";

				for (Ast* child : children)
				{
					result += config::addTabs(child->emitCode(), 1) + "\n";
				}

				result += "\n" + literalCode::ENDINIT_TAG_NAME;
			}
			else if (name == literalCode::RMA_PROCESS_INITIALIZATION_TOKEN_NAME)
			{
				result += children.at(0)->emitCode() + "\n" + config::addTabs(children.at(1)->emitCode(), 1);
			}
			else if (name == literalCode::PROCESS_DECLARATION_TOKEN_NAME)
			{
				result += children.at(0)->emitCode() + "\n\n" + config::addTabs(children.at(1)->emitCode(), 1);
			}
			else if (name == literalCode::PROCESS_HEADER_TOKEN_NAME)
			{
				result += literalCode::PROCESS_TAG_NAME + literalCode::SPACE + children.at(0)->name + literalCode::COLON;
			}
			else if (name == literalCode::STATEMENTS_TOKEN_NAME)
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
			else if (name == literalCode::LABEL_TOKEN_NAME)
			{
				if (children.at(0)->name == literalCode::IF_ELSE_TOKEN_NAME)
				{
					result.insert(result.begin(), '\n');
				}

				result += children.at(0)->name + literalCode::COLON + literalCode::SPACE + children.at(1)->emitCode();
			}
			else if (name == literalCode::LOCAL_ASSIGN_TOKEN_NAME)
			{
				result += children.at(0)->emitCode() + literalCode::SPACE + literalCode::ASSIGN_OPERATOR + literalCode::LEFT_PARENTHESIS +
					children.at(1)->name + literalCode::RIGHT_PARENTHESIS + literalCode::SPACE +
					children.at(2)->emitCode() + literalCode::SEMICOLON;
			}
			else if (name == literalCode::ID_TOKEN_NAME)
			{
				if (children.at(0)->name == literalCode::PC_TOKEN_NAME)
				{
					result += literalCode::PC_TOKEN_NAME + literalCode::LEFT_CURLY_BRACKET + children.at(0)->children.at(0)->name +
						literalCode::RIGHT_CURLY_BRACKET;
				}
				else
				{
					result += children.at(0)->name;
				}
			}
			else if (name == literalCode::INT_TOKEN_NAME)
			{
				result += children.at(0)->name;
			}
			else if (name == literalCode::BOOL_TOKEN_NAME)
			{
				result += children.at(0)->name;
			}
			else if (name == literalCode::RMA_PUT_TOKEN_NAME)
			{
				result += literalCode::RMA_PUT_TOKEN_NAME + literalCode::LEFT_PARENTHESIS + children.at(0)->name +
					literalCode::COMMA + literalCode::SPACE + children.at(1)->name + literalCode::RIGHT_PARENTHESIS +
					literalCode::SPACE + literalCode::LEFT_PARENTHESIS + children.at(2)->name + literalCode::COMMA +
					literalCode::SPACE + children.at(3)->name + literalCode::COMMA + literalCode::SPACE + children.at(4)->name +
					literalCode::RIGHT_PARENTHESIS + literalCode::SEMICOLON;
			}
			else if (name == literalCode::RMA_GET_TOKEN_NAME)
			{
				result += children.at(0)->name + literalCode::SPACE + literalCode::EQUALS + literalCode::SPACE +
					literalCode::RMA_GET_TOKEN_NAME + literalCode::LEFT_PARENTHESIS + children.at(1)->name +
					literalCode::COMMA + literalCode::SPACE + children.at(2)->name + literalCode::RIGHT_PARENTHESIS +
					literalCode::SPACE + literalCode::LEFT_PARENTHESIS + children.at(3)->name + literalCode::COMMA +
					literalCode::SPACE + children.at(4)->name + literalCode::RIGHT_PARENTHESIS + literalCode::SEMICOLON;
			}
			else if (name == literalCode::IF_ELSE_TOKEN_NAME)
			{
				if (parent->name != literalCode::LABEL_TOKEN_NAME)
				{
					result.insert(result.begin(), '\n');
				}

				result += literalCode::IF_TAG_NAME + literalCode::SPACE + literalCode::LEFT_PARENTHESIS +
					children.at(0)->emitCode() + literalCode::RIGHT_PARENTHESIS + "\n" +
					config::addTabs(children.at(1)->emitCode(), 1) + "\n";

				if (children.at(2)->name != literalCode::NONE_TOKEN_NAME)
				{
					result += literalCode::ELSE_TAG_NAME + "\n" + config::addTabs(children.at(2)->emitCode(), 1) + "\n";
				}

				result += literalCode::ENDIF_TAG_NAME + literalCode::SEMICOLON;
			}
			else if (find(literalCode::UNARY_OPERATORS.begin(), literalCode::UNARY_OPERATORS.end(), name) != literalCode::UNARY_OPERATORS.end())
			{
				result += name + literalCode::LEFT_PARENTHESIS + children.at(0)->emitCode() + literalCode::RIGHT_PARENTHESIS;
			}
			else if (find(literalCode::BINARY_OPERATORS.begin(), literalCode::BINARY_OPERATORS.end(), name) != literalCode::BINARY_OPERATORS.end())
			{
				if (name == literalCode::ASTERISK_TOKEN_NAME && children.size() == 0)
				{
					result += name;
				}
				else
				{
					if (children.at(0)->name == literalCode::ID_TOKEN_NAME || children.at(0)->name == literalCode::INT_TOKEN_NAME)
					{
						result += children.at(0)->emitCode();
					}
					else
					{
						result += literalCode::LEFT_PARENTHESIS + children.at(0)->emitCode() + literalCode::RIGHT_PARENTHESIS;
					}

					result += literalCode::SPACE + name + literalCode::SPACE;

					if (children.at(1)->name == literalCode::ID_TOKEN_NAME || children.at(1)->name == literalCode::INT_TOKEN_NAME)
					{
						result += children.at(1)->emitCode();
					}
					else
					{
						result += literalCode::LEFT_PARENTHESIS + children.at(1)->emitCode() + literalCode::RIGHT_PARENTHESIS;
					}
				}
			}
			else if (name == literalCode::ABORT_TOKEN_NAME)
			{
				result += literalCode::ABORT_TOKEN_NAME + literalCode::LEFT_PARENTHESIS + literalCode::QUOTATION +
					children.at(0)->name + literalCode::QUOTATION + literalCode::RIGHT_PARENTHESIS + literalCode::SEMICOLON;
			}
			else if (name == literalCode::FLUSH_TOKEN_NAME)
			{
				result += literalCode::FLUSH_TOKEN_NAME + literalCode::SEMICOLON;
			}
			else if (name == literalCode::FENCE_TOKEN_NAME)
			{
				result += literalCode::FENCE_TOKEN_NAME + literalCode::SEMICOLON;
			}
			else if (name == literalCode::GOTO_TOKEN_NAME)
			{
				result += literalCode::GOTO_TOKEN_NAME + literalCode::SPACE + children.at(0)->name + literalCode::SEMICOLON;
			}
			else if (name == literalCode::NOP_TOKEN_NAME)
			{
				result += literalCode::NOP_TOKEN_NAME + literalCode::SEMICOLON;
			}
			else if (name == literalCode::ASSUME_TOKEN_NAME)
			{
				result += literalCode::ASSUME_TOKEN_NAME + literalCode::LEFT_PARENTHESIS + children.at(0)->emitCode() +
					literalCode::RIGHT_PARENTHESIS + literalCode::SEMICOLON;
			}
			else if (!isBooleanProgramNode)
			{
				config::throwError("Can't emit node: " + name);
			}
		}
		else
		{
			if (name == literalCode::PROGRAM_DECLARATION_TOKEN_NAME)
			{
				for (Ast* child : children)
				{
					result += child->emitCode() + "\n\n";
				}
			}
			else if (name == literalCode::INITIALIZATION_BLOCK_TOKEN_NAME)
			{
				result += literalCode::BEGINIT_TAG_NAME + "\n\n";

				for (Ast* child : children)
				{
					result += "\t" + child->emitCode() + "\n";
				}

				result += "\n" + literalCode::ENDINIT_TAG_NAME;
			}
			else if (name == literalCode::PSO_TSO_STORE_TOKEN_NAME)
			{
				result += literalCode::PSO_TSO_STORE_TOKEN_NAME + literalCode::SPACE +
					children.at(0)->emitCode() + literalCode::SPACE + literalCode::ASSIGN_OPERATOR +
					literalCode::SPACE + children.at(1)->emitCode() + literalCode::SEMICOLON;
			}
			else if (name == literalCode::PSO_TSO_LOAD_TOKEN_NAME)
			{
				result += literalCode::PSO_TSO_LOAD_TOKEN_NAME + literalCode::SPACE +
					children.at(0)->emitCode() + literalCode::SPACE + literalCode::ASSIGN_OPERATOR +
					literalCode::SPACE + children.at(1)->emitCode() + literalCode::SEMICOLON;
			}
			else if (name == literalCode::LOCAL_ASSIGN_TOKEN_NAME)
			{
				result += children.at(0)->emitCode() + literalCode::SPACE + literalCode::ASSIGN_OPERATOR +
					literalCode::SPACE + children.at(1)->emitCode() + literalCode::SEMICOLON;
			}
			else if (name == literalCode::ID_TOKEN_NAME)
			{
				if (children.at(0)->name == literalCode::PC_TOKEN_NAME)
				{
					result += literalCode::PC_TOKEN_NAME + literalCode::LEFT_CURLY_BRACKET + children.at(0)->children.at(0)->name +
						literalCode::RIGHT_CURLY_BRACKET;
				}
				else
				{
					result += children.at(0)->name;
				}
			}
			else if (name == literalCode::INT_TOKEN_NAME)
			{
				result += children.at(0)->name;
			}
			else if (name == literalCode::BOOL_TOKEN_NAME)
			{
				result += children.at(0)->name;
			}
			else if (name == literalCode::PROCESS_DECLARATION_TOKEN_NAME)
			{
				result += children.at(0)->emitCode() + "\n\n" + config::addTabs(children.at(1)->emitCode(), 1);
			}
			else if (name == literalCode::PROCESS_HEADER_TOKEN_NAME)
			{
				result += literalCode::PROCESS_TAG_NAME + literalCode::SPACE + children.at(0)->name + literalCode::COLON;
			}
			else if (name == literalCode::STATEMENTS_TOKEN_NAME)
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
			else if (name == literalCode::LABEL_TOKEN_NAME)
			{
				if (children.at(0)->name == literalCode::IF_ELSE_TOKEN_NAME)
				{
					result.insert(result.begin(), '\n');
				}

				result += children.at(0)->name + literalCode::COLON + literalCode::SPACE + children.at(1)->emitCode();
			}
			else if (name == literalCode::IF_ELSE_TOKEN_NAME)
			{
				if (parent->name != literalCode::LABEL_TOKEN_NAME)
				{
					result.insert(result.begin(), '\n');
				}

				result += literalCode::IF_TAG_NAME + literalCode::SPACE + literalCode::LEFT_PARENTHESIS +
					children.at(0)->emitCode() + literalCode::RIGHT_PARENTHESIS + "\n" +
					config::addTabs(children.at(1)->emitCode(), 1) + "\n";

				if (children.at(2)->name != literalCode::NONE_TOKEN_NAME)
				{
					result += literalCode::ELSE_TAG_NAME + "\n" + config::addTabs(children.at(2)->emitCode(), 1) + "\n";
				}

				result += literalCode::ENDIF_TAG_NAME + literalCode::SEMICOLON;
			}
			else if (find(literalCode::UNARY_OPERATORS.begin(), literalCode::UNARY_OPERATORS.end(), name) != literalCode::UNARY_OPERATORS.end())
			{
				result += name + literalCode::LEFT_PARENTHESIS + children.at(0)->emitCode() + literalCode::RIGHT_PARENTHESIS;
			}
			else if (find(literalCode::BINARY_OPERATORS.begin(), literalCode::BINARY_OPERATORS.end(), name) != literalCode::BINARY_OPERATORS.end())
			{
				if (name == literalCode::ASTERISK_TOKEN_NAME && children.size() == 0)
				{
					result += name;
				}
				else
				{
					if (children.at(0)->name == literalCode::ID_TOKEN_NAME || children.at(0)->name == literalCode::INT_TOKEN_NAME)
					{
						//result += children.at(0)->children.at(0)->name;
						result += children.at(0)->emitCode();
					}
					else
					{
						result += literalCode::LEFT_PARENTHESIS + children.at(0)->emitCode() + literalCode::RIGHT_PARENTHESIS;
					}

					result += literalCode::SPACE + name + literalCode::SPACE;

					if (children.at(1)->name == literalCode::ID_TOKEN_NAME || children.at(1)->name == literalCode::INT_TOKEN_NAME)
					{
						//result += children.at(1)->children.at(0)->name;
						result += children.at(1)->emitCode();
					}
					else
					{
						result += literalCode::LEFT_PARENTHESIS + children.at(1)->emitCode() + literalCode::RIGHT_PARENTHESIS;
					}
				}
			}
			else if (name == literalCode::ABORT_TOKEN_NAME)
			{
				result += literalCode::ABORT_TOKEN_NAME + literalCode::LEFT_PARENTHESIS + literalCode::QUOTATION +
					children.at(0)->name + literalCode::QUOTATION + literalCode::RIGHT_PARENTHESIS + literalCode::SEMICOLON;
			}
			else if (name == literalCode::FLUSH_TOKEN_NAME)
			{
				result += literalCode::FLUSH_TOKEN_NAME + literalCode::SEMICOLON;
			}
			else if (name == literalCode::FENCE_TOKEN_NAME)
			{
				result += literalCode::FENCE_TOKEN_NAME + literalCode::SEMICOLON;
			}
			else if (name == literalCode::GOTO_TOKEN_NAME)
			{
				result += literalCode::GOTO_TOKEN_NAME + literalCode::SPACE + children.at(0)->name + literalCode::SEMICOLON;
			}
			else if (name == literalCode::NOP_TOKEN_NAME)
			{
				result += literalCode::NOP_TOKEN_NAME + literalCode::SEMICOLON;
			}
			else if (name == literalCode::ASSUME_TOKEN_NAME)
			{
				result += literalCode::ASSUME_TOKEN_NAME + literalCode::LEFT_PARENTHESIS + children.at(0)->emitCode() +
					literalCode::RIGHT_PARENTHESIS + literalCode::SEMICOLON;
			}
			else if (name == literalCode::BEGIN_ATOMIC_TOKEN_NAME)
			{
				result += literalCode::BEGIN_ATOMIC_TOKEN_NAME + literalCode::SEMICOLON;
			}
			else if (name == literalCode::END_ATOMIC_TOKEN_NAME)
			{
				result += literalCode::END_ATOMIC_TOKEN_NAME + literalCode::SEMICOLON;
			}
			else if (name == literalCode::CHOOSE_TOKEN_NAME)
			{
				result += literalCode::CHOOSE_TOKEN_NAME + literalCode::LEFT_PARENTHESIS + children.at(0)->emitCode() +
					literalCode::COMMA + literalCode::SPACE + children.at(1)->emitCode() +
					literalCode::RIGHT_PARENTHESIS;
			}
			else if (!isBooleanProgramNode)
			{
				config::throwError("Can't emit node: " + name);
			}
		}

		/*if (!endComment.empty())
		{
		result += "\n" + literalCode::COMMENT_PREFIX + literalCode::SPACE + endComment;
		}*/

		if (!endComment.empty())
		{
			result += "\n" + literalCode::MULTILINE_COMMENT_PREFIX + literalCode::SPACE + endComment + literalCode::SPACE + literalCode::MULTILINE_COMMENT_SUFFIX + "\n";
		}

		if (name == literalCode::IF_ELSE_TOKEN_NAME)
		{
			result += "\n";
		}

		return result;
	}
	
	// Cascades from top to bottom and back. Returns a string vector containing all identifiers along the downward path, including the current node's own.
	vector<string> Ast::getIDs()
	{
		vector<string> results;
	
		if (name == literalCode::ID_TOKEN_NAME)	// If this is an identifier, push it on the vector to return it upwards
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
	
	// Cascades from top to bottom. If for a read- or write-statement is encountered, the cascade stops and the IDs read and written by its children are gathered
	void Ast::getCostsFromChildren()
	{
		if (config::currentLanguage == config::language::RMA)
		{
			if (name == literalCode::LOCAL_ASSIGN_TOKEN_NAME)
			{
				vector<string> writtenVariables = children.at(0)->getIDs();
				vector<string> readVariables = children.at(2)->getIDs();
	
				for (string member : writtenVariables)
				{
					bsm::incrementCost(member, 1, &causedWriteCost);
				}
	
				for (string member : readVariables)
				{
					bsm::incrementCost(member, 1, &causedReadCost);
				}
			}
			else if (name == literalCode::RMA_PUT_TOKEN_NAME)
			{
				bsm::incrementCost(children.at(2)->name, 1, &causedWriteCost);
				bsm::incrementCost(children.at(4)->name, 1, &causedReadCost);
			}
			else if (name == literalCode::RMA_GET_TOKEN_NAME)
			{
				bsm::incrementCost(children.at(0)->name, 1, &causedWriteCost);
				bsm::incrementCost(children.at(3)->name, 1, &causedReadCost);
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
			if (name == literalCode::PSO_TSO_STORE_TOKEN_NAME || name == literalCode::PSO_TSO_LOAD_TOKEN_NAME)
			{
				vector<string> writtenVariables = children.at(0)->getIDs();
				vector<string> readVariables = children.at(1)->getIDs();
	
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
				for (Ast* child : children)
				{
					child->getCostsFromChildren();
				}
			}
		}
	}
	
	// Cascades from top to bottom. Causes nodes to initialize the global variables initialized in the program's first block into their persistent maps
	void Ast::initializePersistentCosts()
	{
		if (name == literalCode::PROGRAM_DECLARATION_TOKEN_NAME)
		{
			// Collect all buffer costs caused by the variable initialization statements
			if (config::currentLanguage == config::language::RMA)
			{
				for (Ast* processInitialization : children.at(0)->children)
				{
					if (processInitialization->name != literalCode::RMA_PROCESS_INITIALIZATION_TOKEN_NAME)
					{
						break;
					}
	
					for (Ast* initializationStatement : processInitialization->children.at(1)->children)
					{
						bsm::additiveMergeBufferSizes(&(initializationStatement->causedWriteCost), &(persistentWriteCost));
						bsm::additiveMergeBufferSizes(&(initializationStatement->causedWriteCost), &(persistentReadCost));
					}
				}
			}
			else
			{
				for (Ast* initializationStatement : children.at(0)->children)
				{
					bsm::additiveMergeBufferSizes(&(initializationStatement->causedWriteCost), &(persistentWriteCost));
					bsm::additiveMergeBufferSizes(&(initializationStatement->causedWriteCost), &(persistentReadCost));
				}
			}
	
			// Sets the values of the recently initialized entries to 0
			resetBufferSizes();
	
			// Propagates the 0-initialized global variable map to the progeny of the process declarations
			for (Ast* child : children)
			{
				if (child->name == literalCode::PROCESS_DECLARATION_TOKEN_NAME)
				{
					child->topDownCascadingCopyPersistentBufferSizes(this);
					child->topDownCascadingAddInitializedCausedCostsToPersistentCosts();
				}
			}
		}
	}
	
	// Cascades from top to bottom. Causes label nodes to register themselves with the global register.
	void Ast::topDownCascadingRegisterLabels()
	{
		if (name == literalCode::LABEL_TOKEN_NAME)
		{
			registerLabel();
		}
	
		for (Ast* child : children)
		{
			child->topDownCascadingRegisterLabels();
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
	
	// Spawns a control flow visitor on the first statement of every process declaration statement block
	void Ast::visitAllProgramPoints()
	{
		if (name == literalCode::PROGRAM_DECLARATION_TOKEN_NAME)
		{
			int childrenCount = children.size();
			Ast* currentProcessDeclarationStatements;
			ControlFlowVisitor* newControlFlowVisitor;
	
			for (int ctr = 1; ctr < childrenCount; ctr++)
			{
				if (children.at(ctr)->name == literalCode::PROCESS_DECLARATION_TOKEN_NAME)
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
	}
	
	void Ast::cascadingInitializeAuxiliaryVariables()
	{
		initializeAuxiliaryVariables();
	
		for (Ast* child : children)
		{
			child->cascadingInitializeAuxiliaryVariables();
		}
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
			if (name == literalCode::PROGRAM_DECLARATION_TOKEN_NAME)
			{
				for (Ast* child : children)
				{
					if (child->name == literalCode::PROCESS_DECLARATION_TOKEN_NAME)
					{
						child->carryOutReplacements();
					}
				}
			}
			else if (name == literalCode::PROCESS_DECLARATION_TOKEN_NAME)
			{
				vector<Ast*> toBeReplaced = children.at(1)->reportBack();
	
				for (Ast* statement : toBeReplaced)
				{
					bsm::additiveMergeBufferSizes(&(statement->persistentWriteCost), &persistentWriteCost);
				}
				
				int process = stoi(children.at(0)->children.at(0)->name);
				int currentMaximum;
				Ast* currentNode;
				vector<string> globalVariableNames = bsm::getAllKeys(&persistentWriteCost);
	
				for (string globalVariableName : globalVariableNames)
				{
					if (persistentWriteCost[globalVariableName] == bsm::TOP_VALUE || persistentWriteCost[globalVariableName] > config::K)
					{
						currentMaximum = config::K;
					}
					else
					{
						currentMaximum = persistentWriteCost[globalVariableName];
					}
	
					for (int ctr = currentMaximum; ctr > 0; ctr--)
					{
						currentNode = newLocalAssign(config::globalVariables[globalVariableName]->auxiliaryWriteBufferVariableNames[pair<int, int>(process, ctr)], 0);
						currentNode->parent = children.at(1);
						children.at(1)->children.insert(children.at(1)->children.begin(), currentNode);
					}
	
					currentNode = newLocalAssign(config::globalVariables[globalVariableName]->auxiliaryCounterVariableNames[process], 0);
					currentNode->parent = children.at(1);
					children.at(1)->children.insert(children.at(1)->children.begin(), currentNode);
	
					currentNode = newLocalAssign(config::globalVariables[globalVariableName]->auxiliaryFirstPointerVariableNames[process], 0);
					currentNode->parent = children.at(1);
					children.at(1)->children.insert(children.at(1)->children.begin(), currentNode);
				}
	
				children.at(1)->refreshChildIndices();
	
				for (Ast* statement : toBeReplaced)
				{
					statement->carryOutReplacements();
				}
			}
			else if (name == literalCode::PSO_TSO_STORE_TOKEN_NAME || name == literalCode::PSO_TSO_LOAD_TOKEN_NAME ||
				name == literalCode::FENCE_TOKEN_NAME || name == literalCode::FLUSH_TOKEN_NAME)
			{
				vector<Ast*> replacement;
	
				if (name == literalCode::PSO_TSO_STORE_TOKEN_NAME)
				{
					string globalVariableName = children.at(0)->children.at(0)->name;
					string processId;
					tryGetParentProcessNumber(&processId);
					int process = stoi(processId);
					string x_fst_t = config::globalVariables[globalVariableName]->auxiliaryFirstPointerVariableNames[process];
					string x_cnt_t = config::globalVariables[globalVariableName]->auxiliaryCounterVariableNames[process];
	
					vector<Ast*> statements;
	
					int s;
					if (persistentWriteCost[globalVariableName] == bsm::TOP_VALUE || persistentWriteCost[globalVariableName] > config::K)
					{
						s = config::K;
	
						Ast* abortAst = new Ast();
						abortAst->name = literalCode::ABORT_TOKEN_NAME;
						abortAst->addChild(new Ast());
						abortAst->children.at(0)->name = config::OVERFLOW_MESSAGE;
	
						statements.push_back(abortAst);
	
						replacement.push_back(newIfElse(
								newBinaryOp(
										newID(x_cnt_t),
										newINT(config::K),
										literalCode::EQUALS
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
						replacement.push_back(newLocalAssign(x_fst_t, 1));
						replacement.push_back(newLocalAssign(config::globalVariables[globalVariableName]->auxiliaryWriteBufferVariableNames[pair<int, int>(process, 1)], children.at(1)));
					}
					else
					{
						statements.push_back(newLocalAssign(x_fst_t, 1));
						statements.push_back(newLocalAssign(config::globalVariables[globalVariableName]->auxiliaryWriteBufferVariableNames[pair<int, int>(process, 1)], children.at(1)));
	
						replacement.push_back(newIfElse(
								newBinaryOp(
										newID(x_fst_t),
										newINT(0),
										literalCode::EQUALS
									),
								statements
							));
					}
	
					replacement.push_back(newLocalAssign(
							x_cnt_t,
							newBinaryOp(
								newID(x_cnt_t),
								newINT(1),
								string(1, literalCode::PLUS)
							)
						));
	
					for (int ctr = 2; ctr <= s; ctr++)
					{
						statements.clear();
						statements.push_back(newLocalAssign(config::globalVariables[globalVariableName]->auxiliaryWriteBufferVariableNames[pair<int, int>(process, ctr)], children.at(1)));
	
						replacement.push_back(newIfElse(
								newBinaryOp(
									newID(x_cnt_t),
									newINT(ctr),
									literalCode::EQUALS
								),
								statements
							));
					}
				}
				else if (name == literalCode::PSO_TSO_LOAD_TOKEN_NAME)
				{
					string globalVariableName = children.at(1)->children.at(0)->name;
					string processId;
					tryGetParentProcessNumber(&processId);
					int process = stoi(processId);
					string x_cnt_t = config::globalVariables[globalVariableName]->auxiliaryCounterVariableNames[process];
	
					int s;
					if (persistentWriteCost[globalVariableName] == bsm::TOP_VALUE || persistentWriteCost[globalVariableName] > config::K)
					{
						s = config::K;
					}
					else
					{
						s = persistentWriteCost[globalVariableName];
					}
	
					vector<Ast*> statements;
					statements.push_back(newBinaryOp(children.at(0), children.at(1), literalCode::PSO_TSO_LOAD_TOKEN_NAME));
	
					replacement.push_back(newIfElse(
							newBinaryOp(
								newID(x_cnt_t),
								newINT(0),
								literalCode::EQUALS
							),
							statements
						));
	
					for (int ctr = 1; ctr <= s; ctr++)
					{
						statements.clear();
						statements.push_back(newBinaryOp(
								children.at(0),
								newID(
									config::globalVariables[globalVariableName]->auxiliaryWriteBufferVariableNames[pair<int, int>(process, ctr)]
								),
								literalCode::LOCAL_ASSIGN_TOKEN_NAME
							));
	
						replacement.push_back(newIfElse(
								newBinaryOp(
									newID(x_cnt_t),
									newINT(ctr),
									literalCode::EQUALS
								),
								statements
							));
					}
				}
				else if (name == literalCode::FENCE_TOKEN_NAME)
				{
					string processId;
					tryGetParentProcessNumber(&processId);
					int process = stoi(processId);
					vector<string> globalVariableNames = bsm::getAllKeys(&persistentWriteCost);
					string x_cnt_t;
					string x_fst_t;
	
					for (string globalVariableName : globalVariableNames)
					{
						x_fst_t = config::globalVariables[globalVariableName]->auxiliaryFirstPointerVariableNames[process];
						x_cnt_t = config::globalVariables[globalVariableName]->auxiliaryCounterVariableNames[process];
	
						replacement.push_back(newAssume(
								newBinaryOp(
									newID(x_cnt_t),
									newINT(0),
									literalCode::EQUALS
								)
							));
	
						replacement.push_back(newAssume(
								newBinaryOp(
									newID(x_fst_t),
									newINT(0),
									literalCode::EQUALS
								)
							));
					}
				}
				else if (name == literalCode::FLUSH_TOKEN_NAME && numberOfVariablesInPersistentWriteBuffer() > 0)
				{
					string processId;
					tryGetParentProcessNumber(&processId);
					int process = stoi(processId);
					vector<string> globalVariableNames = bsm::getAllKeys(&persistentWriteCost);
					string x_cnt_t;
					string x_fst_t;
					srand(time(NULL));
					int uniqueLabel = rand();
					int s;
					Ast* currentIfElse;
	
					vector<Ast*> currentIfStatements;
					vector<Ast*> oldIfStatements;
					vector<Ast*> currentElseStatements;
	
					vector<Ast*> flushStatements;
					for (string globalVariableName : globalVariableNames)
					{
						if (persistentWriteCost[globalVariableName] == bsm::TOP_VALUE || persistentWriteCost[globalVariableName] > config::K)
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
								currentIfStatements.push_back(newStore(
										globalVariableName,
										newID(config::globalVariables[globalVariableName]->auxiliaryWriteBufferVariableNames[pair<int, int>(process, ctr + 1)])
									));
							}
							else
							{
								currentIfStatements.clear();
								currentIfStatements.push_back(currentIfElse);
							}
	
							currentElseStatements.clear();
	
							currentElseStatements.push_back(newStore(
									globalVariableName,
									newID(config::globalVariables[globalVariableName]->auxiliaryWriteBufferVariableNames[pair<int, int>(process, ctr)])
								));
	
							currentIfElse = newIfElse(
									newBinaryOp(
										newID(x_fst_t),
										newINT(ctr),
										string(1, literalCode::GREATER_THAN)
									),
									currentIfStatements,
									currentElseStatements
								);
						}
	
						if (s >= 1)
						{
							currentIfStatements.clear();
	
							if (s > 1)
							{
								currentIfStatements.push_back(currentIfElse);
							}
	
							currentIfStatements.push_back(newLocalAssign(
									x_fst_t,
									newBinaryOp(
										newID(x_fst_t),
										newINT(1),
										string(1, literalCode::PLUS)
									)
								));
	
							if (numberOfVariablesInPersistentWriteBuffer() > 1)
							{
								currentIfElse = newIfElse(
										newAsterisk(),
										currentIfStatements
									);
	
								currentIfStatements.clear();
								currentIfStatements.push_back(currentIfElse);
							}
	
							currentIfElse = newIfElse(
									newBinaryOp(
										newID(x_cnt_t),
										newINT(0),
										string(1, literalCode::GREATER_THAN)
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
						gotoNode->name = literalCode::GOTO_TOKEN_NAME;
						gotoNode->children.at(0)->name = to_string(uniqueLabel);
	
						flushStatements.push_back(gotoNode);
	
						Ast* envelopingIfNode = newIfElse(
								newAsterisk(),
								flushStatements
							);
	
						Ast* labelAst = new Ast();
						labelAst->name = literalCode::LABEL_TOKEN_NAME;
						labelAst->addChild(new Ast());
						labelAst->children.at(0)->name = to_string(uniqueLabel);
						labelAst->addChild(envelopingIfNode);
	
						replacement.push_back(labelAst);
					}
				}
	
				if (!replacement.empty())
				{
					if (this->parent->name == literalCode::LABEL_TOKEN_NAME)
					{
						this->parent->startComment = literalCode::REPLACING_CAPTION + literalCode::SPACE + literalCode::QUOTATION + emitCode() + literalCode::QUOTATION;
					}
					else
					{
						replacement.at(0)->startComment = literalCode::REPLACING_CAPTION + literalCode::SPACE + literalCode::QUOTATION + emitCode() + literalCode::QUOTATION;
					}
					replacement.at(replacement.size() - 1)->endComment = literalCode::FINISHED_REPLACING_CAPTION + literalCode::SPACE + literalCode::QUOTATION + emitCode() + literalCode::QUOTATION;
				}
	
				replaceNode(replacement, this);
			}
		}
	}
	
	void Ast::cascadingPerformPredicateAbstraction()
	{
		if (!performPredicateAbstraction())
		{
			Ast* child;

			//for (Ast* child : children)
			for (int ctr = 0; ctr < children.size(); ctr++)
			{
				child = children.at(ctr);

				if (name != literalCode::IF_ELSE_TOKEN_NAME && child->name != literalCode::BL_SHARED_VARIABLES_BLOCK_TOKEN_NAME &&
					child->name != literalCode::BL_LOCAL_VARIABLES_BLOCK_TOKEN_NAME && child->name != literalCode::BL_IF_TOKEN_NAME)
				{
					child->cascadingPerformPredicateAbstraction();
				}
			}
		}
	}

	void Ast::cascadingUnfoldIfElses()
	{
		if (name == literalCode::STATEMENTS_TOKEN_NAME)
		{
			bool hasChanged = true;

			while (hasChanged)
			{
				hasChanged = false;

				for (int ctr = 0; ctr < children.size(); ctr++)
				{
					hasChanged = hasChanged || children.at(ctr)->unfoldIfElses();
				}
			}
		}
		else
		{
			for (int ctr = 0; ctr < children.size(); ctr++)
			{
				children.at(ctr)->cascadingUnfoldIfElses();
			}
		}
	}
	
	void Ast::setVariableInitializations()
	{
		if (name == literalCode::BL_PROGRAM_DECLARATION_TOKEN_NAME)
		{
			int numberOfGlobalPredicates = config::globalPredicates.size();
	
			for (int ctr = numberOfGlobalPredicates - 1; ctr >= 0; ctr--)
			{
				children.at(2)->addChild(newLabel(config::getCurrentAuxiliaryLabel(), newLoad(config::auxiliaryTemporaryVariableNames[ctr], newID(config::auxiliaryBooleanVariableNames[ctr]))), 0);
			}
	
			for (int ctr = numberOfGlobalPredicates - 1; ctr >= 0; ctr--)
			{
				children.at(2)->addChild(newLabel(config::getCurrentAuxiliaryLabel(), newStore(config::auxiliaryBooleanVariableNames[ctr], newAsterisk())), 0);
			}
	
			children.at(2)->children.at(0)->startComment = "Initializing local variables";
	
			vector<Ast*> postFix;
	
			for (int ctr = 0; ctr < numberOfGlobalPredicates; ctr++)
			{
				postFix.push_back(newLabel(config::getCurrentAuxiliaryLabel(), newLocalAssign(config::auxiliaryTemporaryVariableNames[ctr], newINT(0))));
			}
	
			postFix.at(0)->startComment = "Resetting local variables";
			children.at(2)->addChildren(postFix);

			Ast* maxCube = config::getAssumptionOfNegatedLargestFalseImplicativeDisjunctionOfCubes();

			children.at(2)->addChild(newLabel(config::getCurrentAuxiliaryLabel(), maxCube));

			for (int ctr = 3; ctr < children.size(); ctr++)
			{
				if (children.at(ctr)->name == literalCode::BL_PROCESS_DECLARATION_TOKEN_NAME ||
					children.at(ctr)->name == literalCode::PROCESS_DECLARATION_TOKEN_NAME)
				{
					children.at(ctr)->children.at(1)->addChild(newLabel(config::getCurrentAuxiliaryLabel(), maxCube), 0);
				}
			}
	
			/*vector<string> implicativeCubeStates;
			vector<string> cubeStateCombinations;
			vector<string> unmaskedImplicativeCubeStates;
			vector<int> relevantIndices;
			vector<Ast*> implicativeCubes;
	
			string emptyPool = CubeTreeNode::getCubeStatePool(relevantIndices);
	
			for (int ctr = 0; ctr < config::globalCubeSizeLimit; ctr++)
			{
				relevantIndices.push_back(ctr);
			}
	
			string pool = CubeTreeNode::getCubeStatePool(relevantIndices);
	
			for (int ctr = 1; ctr <= config::globalCubeSizeLimit; ctr++)
			{
				implicativeCubeStates.clear();
				cubeStateCombinations = CubeTreeNode::getNaryCubeStateCombinations(relevantIndices, ctr);
	
				for (string cubeStateCombination : cubeStateCombinations)
				{
					unmaskedImplicativeCubeStates = CubeTreeNode::getImplicativeCubeStates(CubeTreeNode::applyDecisionMask(cubeStateCombination, pool), newFalse());
					implicativeCubeStates.insert(implicativeCubeStates.end(), unmaskedImplicativeCubeStates.begin(), unmaskedImplicativeCubeStates.end());
				}
	
				//cout << pool << "\t";
				pool = CubeTreeNode::removeDecisionsFromPool(pool, implicativeCubeStates);
				//cout << pool << "\n";
	
				relevantIndices.clear();
	
				for (int subCtr = 0; subCtr < config::globalCubeSizeLimit; subCtr++)
				{
					if (pool[subCtr] != CubeTreeNode::CUBE_STATE_OMIT)
					{
						relevantIndices.push_back(subCtr);
					}
				}
	
				for (string implicativeCubeState : implicativeCubeStates)
				{
					implicativeCubes.push_back(newBooleanVariableCube(implicativeCubeState));
				}
	
				if (pool.compare(emptyPool) == 0)
				{
					break;
				}
			}
	
			if (implicativeCubes.size() > 0)
			{
				children.at(2)->addChild(newLabel(config::getCurrentAuxiliaryLabel(), newAssume(newMultipleOperation(implicativeCubes, literalCode::DOUBLE_OR)->negate())));
	
				for (int ctr = 3; ctr < children.size(); ctr++)
				{
					children.at(ctr)->children.at(1)->addChild(newLabel(config::getCurrentAuxiliaryLabel(), newAssume(newMultipleOperation(implicativeCubes, literalCode::DOUBLE_OR)->negate())), 0);
				}
			}*/
		}
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
		bsm::conditionalAdditiveMergeBufferSizes(&causedReadCost, &persistentReadCost);
		bsm::conditionalAdditiveMergeBufferSizes(&causedWriteCost, &persistentWriteCost);
	
		for (Ast* child : children)
		{
			child->topDownCascadingAddInitializedCausedCostsToPersistentCosts();
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

	void Ast::labelAllStatements()
	{
		if (name == literalCode::STATEMENTS_TOKEN_NAME)
		{
			vector<Ast*> replacementVector;

			for (Ast* child : children)
			{
				if (child->name != literalCode::LABEL_TOKEN_NAME)
				{
					replacementVector.clear();
					replacementVector.push_back(newLabel(config::getCurrentAuxiliaryLabel(), child));
					config::lazyReplacements[child] = replacementVector;
				}
			}
		}
	}

	bool Ast::isEquivalent(Ast* otherAst)
	{
		return emitCode().compare(otherAst->emitCode()) == 0;
	}

/* Static operations */
	void Ast::replaceNode(const vector<Ast*> &nodes, Ast* oldNode)
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
	
	void Ast::replaceNode(Ast* newNode, Ast* oldNode)
	{
		Ast* newParent = oldNode->parent;
		int newIndex = oldNode->indexAsChild;
	
		newParent->children.erase(newParent->children.begin() + newIndex);
		newParent->children.insert(newParent->children.begin() + newIndex, newNode);
	
		newParent->refreshChildIndices();
	
		//newParent->updateShortStringRepresentation();
	}

/* Static pseudo-constructors */
	// Initializes an ID node
	Ast* Ast::newID(const string &variableName)
	{
		Ast* result = new Ast();
		result->name = literalCode::ID_TOKEN_NAME;
		result->addChild(new Ast());
		result->children.at(0)->name = variableName;
		return result;
	}
	
	// Initializes an INT node
	Ast* Ast::newINT(int value)
	{
		Ast* result = new Ast();
		result->name = literalCode::INT_TOKEN_NAME;
		result->addChild(new Ast());
		result->children.at(0)->name = to_string(value);
		return result;
	}
	
	// Initializes a binary operation node
	Ast* Ast::newBinaryOp(Ast* leftOperand, Ast* rightOperand, const string &operation)
	{
		Ast* result = new Ast();
		result->name = operation;
		result->addChild(leftOperand);
		result->addChild(rightOperand);
		return result;
	}

	Ast* Ast::newMultipleOperation(const vector<Ast*> &operands, const string &operation)
	{
		int operandCount = operands.size();

		if (operandCount > 1)
		{
			return newBinaryOp(operands[0], newMultipleOperation(vector<Ast*>(operands.begin() + 1, operands.end()), operation), operation);
		}
		else if (operandCount == 1)
		{
			return operands[0];
		}
		else
		{
			return NULL;
		}
	}
	
	// Initializes a localAssign(ID(variableName), INT(initialValue)) node
	Ast* Ast::newLocalAssign(const string &variableName, int initialValue)
	{
		Ast* result = new Ast();
		result->name = literalCode::LOCAL_ASSIGN_TOKEN_NAME;
		result->addChild(newID(variableName));
		result->addChild(newINT(initialValue));
		return result;
	}
	
	// Initializes a localAssign(ID(variableName), node) node
	Ast* Ast::newLocalAssign(const string &variableName, Ast* assignmentNode)
	{
		Ast* result = new Ast();
		result->name = literalCode::LOCAL_ASSIGN_TOKEN_NAME;
		result->addChild(newID(variableName));
		result->addChild(assignmentNode);
		return result;
	}
	
	Ast* Ast::newStore(const string &variableName, Ast* rightSide)
	{
		Ast* result = new Ast();
		result->name = literalCode::PSO_TSO_STORE_TOKEN_NAME;
		result->addChild(newID(variableName));
		result->addChild(rightSide);
		return result;
	}
	
	Ast* Ast::newLoad(const string &variableName, Ast* rightSide)
	{
		Ast* result = new Ast();
		result->name = literalCode::PSO_TSO_LOAD_TOKEN_NAME;
		result->addChild(newID(variableName));
		result->addChild(rightSide);
		return result;
	}
	
	// Initializes an ifElse(ifConditional, statements) node
	Ast* Ast::newIfElse(Ast* ifConditionalNode, const vector<Ast*> &statements)
	{
		Ast* result = new Ast();
		result->name = literalCode::IF_ELSE_TOKEN_NAME;
		result->addChild(ifConditionalNode);
		result->addChild(newStatements(statements));
		result->addChild(newNone());
		return result;
	}
	
	// Initializes an ifElse(ifConditional, ifStatements, elseStatements) node
	Ast* Ast::newIfElse(Ast* ifConditionalNode, const vector<Ast*> &ifStatements, const vector<Ast*> &elseStatements)
	{
		Ast* result = new Ast();
		result->name = literalCode::IF_ELSE_TOKEN_NAME;
		result->addChild(ifConditionalNode);
		result->addChild(newStatements(ifStatements));
		result->addChild(newStatements(elseStatements));
		return result;
	}
	
	Ast* Ast::newStatements(const vector<Ast*> &statements)
	{
		Ast* result = new Ast();
		result->name = literalCode::STATEMENTS_TOKEN_NAME;
		result->addChildren(statements);
		return result;
	}
	
	Ast* Ast::newNone()
	{
		Ast* result = new Ast();
		result->name = literalCode::NONE_TOKEN_NAME;
		return result;
	}
	
	Ast* Ast::newNop()
	{
		Ast* result = new Ast();
		result->name = literalCode::NOP_TOKEN_NAME;
		return result;
	}

	Ast* Ast::newGoto(int value)
	{
		Ast* result = new Ast();
		result->name = literalCode::GOTO_TOKEN_NAME;
		result->children.push_back(new Ast());
		result->children.at(0)->name = to_string(value);
		return result;
	}
	
	Ast* Ast::newLabel(int value, Ast* statement)
	{
		Ast* result = new Ast();
		result->name = literalCode::LABEL_TOKEN_NAME;
		result->children.push_back(new Ast());
		result->children.at(0)->name = to_string(value);
		result->addChild(statement);
		return result;
	}
	
	Ast* Ast::newAsterisk()
	{
		Ast* result = new Ast();
		result->name = literalCode::ASTERISK_TOKEN_NAME;
		return result;
	}
	
	Ast* Ast::newTrue()
	{
		Ast* result = new Ast();
		result->name = literalCode::BOOL_TOKEN_NAME;
		result->addChild(new Ast());
		result->children.at(0)->name = literalCode::TRUE_TAG_NAME;
		return result;
	}
	
	Ast* Ast::newFalse()
	{
		Ast* result = new Ast();
		result->name = literalCode::BOOL_TOKEN_NAME;
		result->addChild(new Ast());
		result->children.at(0)->name = literalCode::FALSE_TAG_NAME;
		return result;
	}
	
	// Initializes an assume(conditional) node
	Ast* Ast::newAssume(Ast* assumeConditionalNode)
	{
		Ast* result = new Ast();
		result->name = literalCode::ASSUME_TOKEN_NAME;
		result->addChild(assumeConditionalNode);
		return result;
	}
	
	Ast* Ast::newChoose(Ast* firstChoice, Ast* secondChoice)
	{
		Ast* result = new Ast();
		result->name = literalCode::CHOOSE_TOKEN_NAME;
		result->addChild(firstChoice);
		result->addChild(secondChoice);
		return result;
	}
	
	Ast* Ast::newBeginAtomic()
	{
		Ast* result = new Ast();
		result->name = literalCode::BEGIN_ATOMIC_TOKEN_NAME;
		return result;
	}
	
	Ast* Ast::newEndAtomic()
	{
		Ast* result = new Ast();
		result->name = literalCode::END_ATOMIC_TOKEN_NAME;
		return result;
	}

	Ast* Ast::newBooleanIf(Ast* ifConditionalNode, Ast* statement)
	{
		Ast* result = new Ast();
		result->name = literalCode::BL_IF_TOKEN_NAME;
		result->addChild(ifConditionalNode);
		result->addChild(statement);
		return result;
	}

	Ast* Ast::newAstFromParsedProgram(const string &parsedProgramString)
	{
		// Parse through the AST representation string character by character
		Ast* currentAst = new Ast();
		Ast* result = currentAst;
		char currentChar;
		std::string currentName = "";
		int parsedProgramStringLength = parsedProgramString.length();

		for (int ctr = 0; ctr < parsedProgramStringLength; ctr++)
		{
			currentChar = parsedProgramString.at(ctr);

			if (currentChar == literalCode::LEFT_PARENTHESIS)	// Create a new node with the word parsed so far as its name.
			{													// Subsequently found nodes are to be added as this one's children
				currentAst->name = currentName;					// until the corresponding ')' is found.
				currentName = "";
				currentAst->addChild(new Ast);
				currentAst = currentAst->children.at(0);
			}
			else if (currentChar == literalCode::COMMA)			// Add the currently parsed node as the previous one's sibling
			{
				if (!currentName.empty())
				{
					currentAst->name = currentName;
					currentName = "";
				}

				currentAst = currentAst->parent;
				currentAst->addChild(new Ast);
				currentAst = currentAst->children.at(currentAst->children.size() - 1);
			}
			else if (currentChar == literalCode::RIGHT_PARENTHESIS)	// Move up one level
			{
				if (!currentName.empty())
				{
					currentAst->name = currentName;
					currentName = "";
				}

				currentAst = currentAst->parent;
			}
			else // Continue parsing the name of the next node
			{
				currentName += currentChar;
			}
		}

		return result;
	}
	
	Ast* Ast::newSharedVariables(const vector<string> &variableNames)
	{
		Ast* result = new Ast();
		result->name = literalCode::BL_SHARED_VARIABLES_BLOCK_TOKEN_NAME;
	
		for (string variableName : variableNames)
		{
			result->addChild(newID(variableName));
		}
	
		return result;
	}
	
	Ast* Ast::newLocalVariables(const vector<string> &variableNames)
	{
		Ast* result = new Ast();
		result->name = literalCode::BL_LOCAL_VARIABLES_BLOCK_TOKEN_NAME;
	
		for (string variableName : variableNames)
		{
			result->addChild(newID(variableName));
		}
	
		return result;
	}
	
	Ast* Ast::newBooleanVariableCube(const string &definition, bool useTemporaryVariables)
	{
		int numberOfPredicates = config::globalPredicates.size();
		vector<Ast*> cubeTerms;
		string currentVariableName;
	
		for (int ctr = 0; ctr < numberOfPredicates; ctr++)
		{
			currentVariableName = useTemporaryVariables ? config::auxiliaryTemporaryVariableNames[ctr] : config::auxiliaryBooleanVariableNames[ctr];
	
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
	
	Ast* Ast::newAbstractAssignmentFragment(Ast* assignment, Ast* predicate)
	{
		return newChoose(
				newLargestImplicativeDisjunctionOfCubes(config::globalCubeSizeLimit, assignment->weakestLiberalPrecondition(predicate)),
				newLargestImplicativeDisjunctionOfCubes(config::globalCubeSizeLimit, assignment->weakestLiberalPrecondition(predicate->negate()))
			);
	}
	
	Ast* Ast::newLargestImplicativeDisjunctionOfCubes(int cubeSizeUpperLimit, Ast* predicate, bool useTemporaryVariables)
	{
		cout << "\t\tComputing F(" << predicate->emitCode() << ")\t\t\t \n";
		int numberOfPredicates = config::globalPredicates.size();
		vector<int> relevantIndices = config::getRelevantAuxiliaryTemporaryVariableIndices(predicate);
	
		vector<Ast*> implicativeCubes;
		vector<string> implicativeCubeStringRepresentations = config::getMinimalImplyingCubes(predicate, relevantIndices);
		//vector<string> implicativeCubeStringRepresentations = config::getImplicativeCubes()->getMinimalImplyingCubes(predicate, relevantIndices);
	
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

/* Local access */
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
	
	// Outputs the number string of the superior process in the value of the out pointer and returns true if applicable, otherwise returns false
	bool Ast::tryGetParentProcessNumber(string* out)
	{
		bool result = false;
	
		if (!isRoot())
		{
			if (parent->name == literalCode::PROCESS_DECLARATION_TOKEN_NAME)
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
	
	int Ast::numberOfVariablesInPersistentWriteBuffer()
	{
		int result = 0;
	
		for (bufferSizeMapIterator iterator = persistentWriteCost.begin(); iterator != persistentWriteCost.end(); iterator++)
		{
			if (iterator->second > 0 || iterator->second == bsm::TOP_VALUE)
			{
				result++;
			}
		}
	
		return result;
	}
	
	// Returns whether the node has no parent
	bool Ast::isRoot()
	{
		return indexAsChild == -1;
	}
	
	// Returns whether the node represents one of the program point types
	bool Ast::isProgramPoint()
	{
		if (config::currentLanguage == config::language::RMA)
		{
			return name == literalCode::LABEL_TOKEN_NAME || name == literalCode::GOTO_TOKEN_NAME || name == literalCode::RMA_GET_TOKEN_NAME
				|| name == literalCode::PSO_TSO_LOAD_TOKEN_NAME || name == literalCode::IF_ELSE_TOKEN_NAME || name == literalCode::FENCE_TOKEN_NAME
				|| name == literalCode::RMA_PUT_TOKEN_NAME || name == literalCode::LOCAL_ASSIGN_TOKEN_NAME || name == literalCode::NOP_TOKEN_NAME
				|| name == literalCode::FLUSH_TOKEN_NAME;
		}
		else
		{
			return name == literalCode::LABEL_TOKEN_NAME || name == literalCode::GOTO_TOKEN_NAME || name == literalCode::PSO_TSO_STORE_TOKEN_NAME
				|| name == literalCode::PSO_TSO_LOAD_TOKEN_NAME || name == literalCode::IF_ELSE_TOKEN_NAME || name == literalCode::FENCE_TOKEN_NAME
				|| name == literalCode::NOP_TOKEN_NAME || name == literalCode::FLUSH_TOKEN_NAME || name == literalCode::LOCAL_ASSIGN_TOKEN_NAME;
		}
	}
	
	// Returns whether the current node is an ifElse node with an Else statement block.
	bool Ast::hasElse()
	{
		if (name == literalCode::IF_ELSE_TOKEN_NAME)
		{
			return children.at(2)->name != literalCode::NONE_TOKEN_NAME;
		}
	
		return false;
	}
	
	// Gets the qualified name representing the label to which this goto node links
	string Ast::getGotoCode()
	{
		if (name == literalCode::GOTO_TOKEN_NAME)
		{
			string parentProcessNumber;
			string label = children.at(0)->name;
	
			if (tryGetParentProcessNumber(&parentProcessNumber))
			{
				return parentProcessNumber + literalCode::LABEL_SEPARATOR + label;
			}
		}
	
		return "";
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

/* Cascade elements */
	bool Ast::unifyVariableNames()
	{
		if (name == literalCode::ID_TOKEN_NAME)
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
	
	// Registers this label node with the global label container
	void Ast::registerLabel()
	{
		if (name == literalCode::LABEL_TOKEN_NAME)
		{
			config::labelLookupMap[getLabelCode()] = this;
		}
	}
	
	// Checks whether the node is a program point and has any control flow successors, and if so, creates control flow edges between this node and its successors.
	bool Ast::generateOutgoingEdges()
	{
		if (isProgramPoint())	// Checks whether the node is a program point
		{
			ControlFlowEdge* newEdge;
			Ast* nextStatement = NULL;
	
			if (name == literalCode::GOTO_TOKEN_NAME)	// Goto nodes only lead to their corresponding label
			{
				newEdge = new ControlFlowEdge(this, config::labelLookupMap[getGotoCode()]);
				outgoingEdges.push_back(newEdge);
			}
			else if (name == literalCode::LABEL_TOKEN_NAME)	// Label nodes only lead to their corresponding statement
			{
				newEdge = new ControlFlowEdge(this, children.at(1));
				outgoingEdges.push_back(newEdge);
			}
			else if (name == literalCode::IF_ELSE_TOKEN_NAME)	// IfElse nodes lead to their conditionals, which lead to both statement blocks' first statements,
			{												// of which the last statements lead to the statement following the IfElse block, unless they're goto statements
				Ast* lastChild = NULL;
				nextStatement = tryGetNextStatement();
				bool nextStatementExists = nextStatement != NULL;
	
				newEdge = new ControlFlowEdge(this, children.at(0));	// Create an edge to the conditional
				outgoingEdges.push_back(newEdge);
	
				newEdge = new ControlFlowEdge(children.at(0), children.at(1)->children.at(0));	// Create an edge from the conditional to the first statement of the If block
				children.at(0)->outgoingEdges.push_back(newEdge);
	
				lastChild = children.at(1)->tryGetLastStatement();		// If there is a statement after the IfElse block, connect to it from the last statement of the If block, if it's not a goto statement
				if (nextStatementExists && lastChild != NULL && lastChild->name != literalCode::GOTO_TOKEN_NAME)
				{
					newEdge = new ControlFlowEdge(lastChild, nextStatement);
					lastChild->outgoingEdges.push_back(newEdge);
				}
	
				if (this->hasElse())	// If there is a non-empty Else block, connect to its first statement from the conditional
				{
					newEdge = new ControlFlowEdge(children.at(0), children.at(2)->children.at(0));
					children.at(0)->outgoingEdges.push_back(newEdge);
	
					lastChild = children.at(2)->tryGetLastStatement();		// If there is a statement after the IfElse block, connect to it from the last statement of the Else block, if it's not a goto statement
					if (nextStatementExists && lastChild != NULL && lastChild->name != literalCode::GOTO_TOKEN_NAME)
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
	
	void Ast::initializeAuxiliaryVariables()
	{
		if (config::currentLanguage == config::language::RMA)
		{
			// TODO
			config::throwError("Auxiliary variable generation not implemented for RMA");
		}
		else
		{
			if (name == literalCode::PROGRAM_DECLARATION_TOKEN_NAME)
			{
				vector<string> globalVariableNames = bsm::getAllKeys(&persistentWriteCost);
				vector<int> processes;
	
				for (Ast* child : children)
				{
					if (child->name == literalCode::PROCESS_DECLARATION_TOKEN_NAME)
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
	
		if (name == literalCode::PSO_TSO_STORE_TOKEN_NAME || name == literalCode::PSO_TSO_LOAD_TOKEN_NAME ||
			name == literalCode::FENCE_TOKEN_NAME || name == literalCode::FLUSH_TOKEN_NAME)
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
	
	bool Ast::performPredicateAbstraction()
	{
		bool result = false;
		int firstLabel = config::getCurrentAuxiliaryLabel();

		if (!isRoot())
		{
			if (parent->name == literalCode::LABEL_TOKEN_NAME)
			{
				firstLabel = stoi(parent->children.at(0)->name);
			}
		}
	
		//cout << "\tPerforming predicate abstraction on: " << emitCode() << "\n\n";
	
		if (config::currentLanguage == config::language::RMA)
		{
			// TODO
			config::throwError("Predicate abstraction operations not implemented for RMA");
		}
		else
		{
			if (name == literalCode::PROGRAM_DECLARATION_TOKEN_NAME)
			{
				name = literalCode::BL_PROGRAM_DECLARATION_TOKEN_NAME;
	
				Ast* sharedVars = newSharedVariables(config::auxiliaryBooleanVariableNames);
				Ast* localVars = newLocalVariables(config::auxiliaryTemporaryVariableNames);
	
				addChild(localVars, 0);
				addChild(sharedVars, 0);
			}
			else if (name == literalCode::INITIALIZATION_BLOCK_TOKEN_NAME)
			{
				name = literalCode::BL_INITIALIZATION_BLOCK_TOKEN_NAME;
			}
			else if (name == literalCode::PROCESS_DECLARATION_TOKEN_NAME)
			{
				name = literalCode::BL_PROCESS_DECLARATION_TOKEN_NAME;
			}
			else if (name == literalCode::PSO_TSO_STORE_TOKEN_NAME || name == literalCode::PSO_TSO_LOAD_TOKEN_NAME
				|| name == literalCode::LOCAL_ASSIGN_TOKEN_NAME)
			{
				cout << "\tPerforming predicate abstraction on: " << emitCode() << "\t\t\t \n\n";

				vector<int> relevantPredicateIndices = config::getRelevantAuxiliaryBooleanVariableIndices(children.at(0)->children.at(0)->name);
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
						positiveWeakestLiberalPrecondition = weakestLiberalPrecondition(config::globalPredicates[ctr]);
						negativeWeakestLiberalPrecondition = weakestLiberalPrecondition(config::globalPredicates[ctr]->negate());
	
						parallelAssignments.push_back(newStore(
								config::auxiliaryBooleanVariableNames[ctr],
								newAbstractAssignmentFragment(this, config::globalPredicates[ctr]))
							);
					}
	
					relevantPredicateIndices = config::getRelevantAuxiliaryTemporaryVariableIndices(parallelAssignments);
	
					replacementStatements.push_back(newBeginAtomic());
	
					for (int ctr : relevantPredicateIndices)
					{
						replacementStatements.push_back(newLoad(
								config::auxiliaryTemporaryVariableNames[ctr],
								newID(config::auxiliaryBooleanVariableNames[ctr])
							));
					}
	
					replacementStatements.insert(replacementStatements.end(), parallelAssignments.begin(), parallelAssignments.end());
	
					for (int ctr : relevantPredicateIndices)
					{
						replacementStatements.push_back(newLocalAssign(
								config::auxiliaryTemporaryVariableNames[ctr],
								newINT(0)
							));
					}

					replacementStatements.push_back(config::getAssumptionOfNegatedLargestFalseImplicativeDisjunctionOfCubes());
					replacementStatements.push_back(Ast::newEndAtomic());
				}
				
				replacementStatements[0]->startComment = literalCode::PREDICATE_ABSTRACTION_COMMENT_PREFIX + emitCode();

				if (parent->name == literalCode::LABEL_TOKEN_NAME)
				{
					config::lazyReplacements[parent] = replacementStatements;
				}
				else
				{
					config::lazyReplacements[this] = replacementStatements;
				}
	
				result = true;
			}
			else if (name == literalCode::ASSERT_TOKEN_NAME)
			{
				vector<Ast*> nonPcAssertions;
				vector<Ast*> currentReplacement;
				Ast* leftSide;
				Ast* rightSide;
				Ast* currentAssertion;
				nonPcAssertions.push_back(children.at(0));

				for (int ctr = 0; ctr < nonPcAssertions.size(); ctr++)
				{
					currentAssertion = nonPcAssertions.at(0);
					leftSide = currentAssertion->children.at(0);
					rightSide = currentAssertion->children.at(1);

					if (currentAssertion->name == literalCode::AND || currentAssertion->name == literalCode::OR ||
						currentAssertion->name == literalCode::DOUBLE_AND || currentAssertion->name == literalCode::DOUBLE_OR)
					{
						if (currentAssertion->name == literalCode::AND)
						{
							currentAssertion->name = literalCode::DOUBLE_AND;
						}
						else if (currentAssertion->name == literalCode::OR)
						{
							currentAssertion->name = literalCode::DOUBLE_OR;
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
					currentReplacement.push_back(newLargestImplicativeDisjunctionOfCubes(config::globalCubeSizeLimit, nonPcAssertion, false));
					config::lazyReplacements[nonPcAssertion] = currentReplacement;
				}

				result = true;
			}
		}
	
		return result;
	}

	bool Ast::unfoldIfElses()
	{
		if (name == literalCode::IF_ELSE_TOKEN_NAME)
		{
			cout << "\tPerforming predicate abstraction on: if(" << children.at(0)->emitCode() << ")...\n\n";

			bool isLabeled = parent->name == literalCode::LABEL_TOKEN_NAME;
			bool hasElse = children.at(2)->name != literalCode::NONE_TOKEN_NAME;
			vector<Ast*> replacementStatements;
			Ast* conditional = children.at(0)->clone();
			int elseLabel = hasElse ? config::getCurrentAuxiliaryLabel() : -1;
			int endLabel = config::getCurrentAuxiliaryLabel();

			replacementStatements.push_back(newAssume(
					newReverseLargestImplicativeDisjunctionOfCubes(
						config::globalCubeSizeLimit,
						conditional
					))
				);

			replacementStatements.insert(replacementStatements.end(),
				children.at(1)->children.begin(), children.at(1)->children.end());

			if (hasElse)
			{
				replacementStatements.push_back(newGoto(endLabel));

				replacementStatements.push_back(newLabel(elseLabel,
						newAssume(
							newReverseLargestImplicativeDisjunctionOfCubes(
								config::globalCubeSizeLimit,
								conditional->negate()
							)
					)));

				replacementStatements.insert(replacementStatements.end(),
					children.at(2)->children.begin(), children.at(2)->children.end());
			}

			replacementStatements.push_back(newLabel(endLabel, newNop()));

			Ast* firstStatement = newBooleanIf(newAsterisk(), newGoto(hasElse ? elseLabel : endLabel));

			if (isLabeled)
			{
				parent->parent->refreshChildIndices();

				for (int ctr = replacementStatements.size() - 1; ctr >= 0; ctr--)
				{
					parent->parent->addChild(replacementStatements.at(ctr), parent->indexAsChild + 1);
				}

				parent->parent->refreshChildIndices();

				replaceNode(firstStatement, this);
			}
			else
			{
				parent->refreshChildIndices();

				for (int ctr = replacementStatements.size() - 1; ctr >= 0; ctr--)
				{
					parent->addChild(replacementStatements.at(ctr), indexAsChild + 1);
				}

				parent->refreshChildIndices();

				replaceNode(firstStatement, this);
			}

			return true;
		}
		else if (name == literalCode::LABEL_TOKEN_NAME)
		{
			return children.at(1)->unfoldIfElses();
		}

		return false;
	}
	
	// Copies the contents of the persistent buffer size maps from another node
	void Ast::copyPersistentBufferSizes(Ast* source)
	{
		bsm::copyBufferSizes(&(source->persistentWriteCost), &persistentWriteCost);
		bsm::copyBufferSizes(&(source->persistentReadCost), &persistentReadCost);
	}
	
	// Compares the persistent buffer size maps of those of the direct control flow successors, and if they don't match, it sets the locally TOP values
	// remotely to TOP. If no changes have been made, returns false.
	bool Ast::propagateTops()
	{
		bool result = false;
		bufferSizeMap topContainer;
	
		// Creates a map containing 0s for each non-TOP, and TOP for each TOP value in the persistent write cost map, and adds it to the successor's map if necessary
		bsm::copyAndSetNonTopToZero(&persistentWriteCost, &topContainer);
	
		for (ControlFlowEdge* edge : outgoingEdges)
		{
			if (!bsm::bufferSizeMapCompare(&persistentWriteCost, &(edge->end->persistentWriteCost)))
			{
				result = true;
				bsm::additiveMergeBufferSizes(&topContainer, &(edge->end->persistentWriteCost));
			}
		}
	
		// Creates a map containing 0s for each non-TOP, and TOP for each TOP value in the persistent read cost map, and adds it to the successor's map if necessary
		bsm::copyAndSetNonTopToZero(&persistentReadCost, &topContainer);
	
		for (ControlFlowEdge* edge : outgoingEdges)
		{
			if (!bsm::bufferSizeMapCompare(&persistentReadCost, &(edge->end->persistentReadCost)))
			{
				result = true;
				bsm::additiveMergeBufferSizes(&topContainer, &(edge->end->persistentReadCost));
			}
		}
	
		return result;
	}
	
	// Returns the next statement node of the current statements block if applicable, otherwise returns NULL
	Ast* Ast::tryGetNextStatement()
	{
		if (parent->name == literalCode::LABEL_TOKEN_NAME)
		{
			return parent->tryGetNextSibling();
		}
		else if (parent->name == literalCode::STATEMENTS_TOKEN_NAME)
		{
			return tryGetNextSibling();
		}
	
		return NULL;
	}
	
	// Returns the last statement node of the current statements block if applicable, otherwise returns NULL
	Ast* Ast::tryGetLastStatement()
	{
		if (name == literalCode::STATEMENTS_TOKEN_NAME)
		{
			Ast* lastChild = tryGetLastChild();
	
			if (lastChild->name == literalCode::LABEL_TOKEN_NAME)
			{
				lastChild = lastChild->children.at(1);
			}
	
			return lastChild;
		}
	
		return NULL;
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

	//
	//
	//// Initializes a unary operation node
	//Ast* Ast::newUnaryOp(Ast* operand, string operation)
	//{
	//	Ast* result = new Ast();
	//	result->name = operation;
	//	result->addChild(operand);
	//	return result;
	//}
//
//void Ast::updateShortStringRepresentation()
//{
//	if (children.size() > 0)
//	{
//		string result = name + "(";
//
//		for (Ast* child : children)
//		{
//			child->updateShortStringRepresentation();
//			result += child->shortStringRepresentation + ", ";
//		}
//
//		shortStringRepresentation = result.substr(0, result.size() - 2) + ")";
//	}
//	else
//	{
//		shortStringRepresentation = name;
//	}
//
//	/*if (name == "")
//	{
//		shortStringRepresentation = name;
//	}
//	else
//	{
//		shortStringRepresentation = emitCode();
//	}*/
//}
//
//Ast::Ast(string initialName)
//{
//	// The index by which this node can be referred to from its parent's children vector. The root always has an index of -1
//	indexAsChild = -1;
//	name = initialName;
//}
//
//vector<vector<Ast*>> Ast::allCubes(vector<int> relevantAuxiliaryTemporaryVariableIndices, int cubeSizeUpperLimit)
//{
//	int numberOfVariables = relevantAuxiliaryTemporaryVariableIndices.size();
//	vector<Ast*> allPredicates;
//
//	for (int ctr = 0; ctr < numberOfVariables; ctr++)
//	{
//		allPredicates.push_back(config::globalPredicates[relevantAuxiliaryTemporaryVariableIndices[ctr]]->clone());
//		//allVariables.push_back(newID(config::auxiliaryTemporaryVariableNames[relevantAuxiliaryTemporaryVariableIndices[ctr]]));
//	}
//
//	vector<vector<Ast*>> limitedPowerSet = config::powerSetOfLimitedCardinality(allPredicates, cubeSizeUpperLimit);
//	vector<vector<Ast*>> negationPatterns;
//	vector<vector<Ast*>> allCubeSets;
//
//	for (vector<Ast*> subset : limitedPowerSet)
//	{
//		negationPatterns = allNegationPatterns(subset);
//		allCubeSets.insert(allCubeSets.end(), negationPatterns.begin(), negationPatterns.end());
//	}
//
//	return allCubeSets;
//}
//
////Ast* Ast::newLargestImplicativeDisjunctionOfCubes(int cubeSizeUpperLimit, Ast* predicate, bool useTemporaryVariables)
////{
////	int numberOfPredicates = config::globalPredicates.size();
////	vector<int> relevantIndices;
////	string emptyPool = config::getCubeStatePool(relevantIndices);
////	relevantIndices = config::getRelevantAuxiliaryTemporaryVariableIndices(predicate);
////	string pool = config::getCubeStatePool(relevantIndices);
////	vector<string> implicativeCubeStates;
////	vector<string> unmaskedImplicativeCubeStates;
////	vector<string> cubeStateCombinations;
////	vector<Ast*> implicativeCubes;
////
////	if (relevantIndices.size() > 0)
////	{
////		for (int ctr = 1; ctr <= cubeSizeUpperLimit; ctr++)
////		{
////			implicativeCubeStates.clear();
////			cubeStateCombinations = config::getNaryCubeStateCombinations(relevantIndices, ctr);
////			
////			for (string cubeStateCombination : cubeStateCombinations)
////			{
////				unmaskedImplicativeCubeStates = config::getImplicativeCubeStates(config::applyDecisionMask(cubeStateCombination, pool), predicate);
////				implicativeCubeStates.insert(implicativeCubeStates.end(), unmaskedImplicativeCubeStates.begin(), unmaskedImplicativeCubeStates.end());
////			}
////
////			//cout << pool << "\t";
////			pool = config::removeDecisionsFromPool(pool, implicativeCubeStates);
////			//cout << pool << "\n";
////			
////			relevantIndices.clear();
////
////			for (int subCtr = 0; subCtr < numberOfPredicates; subCtr++)
////			{
////				if (pool[subCtr] != config::CUBE_STATE_OMIT)
////				{
////					relevantIndices.push_back(subCtr);
////				}
////			}
////
////			for (string implicativeCubeState : implicativeCubeStates)
////			{
////				implicativeCubes.push_back(newBooleanVariableCube(implicativeCubeState, useTemporaryVariables));
////			}
////
////			if (pool.compare(emptyPool) == 0)
////			{
////				break;
////			}
////		}
////	}
////
////	if (implicativeCubes.size() == 0)
////	{
////		z3::context c;
////		z3::expr trueConstant = c.bool_val(true);
////
////		if (config::expressionImpliesPredicate(trueConstant, predicate))
////		{
////			return newTrue();
////		}
////		else
////		{
////			return newFalse();
////		}
////	}
////
////	return newMultipleOperation(implicativeCubes, config::DOUBLE_OR);
////}
//
//Ast* Ast::newLargestImplicativeDisjunctionOfCubes(vector<int> relevantAuxiliaryTemporaryVariableIndices, int cubeSizeUpperLimit, Ast* predicate)
//{
//	vector<vector<Ast*>> cubes = allCubes(relevantAuxiliaryTemporaryVariableIndices, cubeSizeUpperLimit);
//	vector<Ast*> implicativeCubeNodes;
//
//	for (vector<Ast*> cube : cubes)
//	{
//		if (config::cubeImpliesPredicate(cube, predicate))
//		{
//			implicativeCubeNodes.push_back(newMultipleOperation(cube, config::AND));
//		}
//	}
//
//	return newMultipleOperation(implicativeCubeNodes, config::OR);
//}
//
//bool Ast::equals(Ast* other)
//{
//	if (name.compare(other->name) == 0)
//	{
//		int childrenCount = children.size();
//
//		for (int ctr = 0; ctr < childrenCount; ctr++)
//		{
//			if (!(children.at(ctr)->equals(other->children.at(ctr))))
//			{
//				return false;
//			}
//		}
//
//		return true;
//	}
//
//	return false;
//}
//
//// Returns the qualified name described by the process- and label numbers
//int Ast::toLabelCode(string processNumber, string labelNumber)
//{
//	hash<string> labelHash;
//	return labelHash(processNumber + config::LABEL_SEPARATOR + labelNumber);
//}
//
//int Ast::effectiveMaxWriteBufferSize(string variableName)
//{
//	if (bufferSizeMapContains(&persistentWriteCost, variableName))
//	{
//		int maxBufferSize = persistentWriteCost[variableName];
//
//		if (maxBufferSize == config::TOP_VALUE || maxBufferSize > config::K)
//		{
//			return config::K;
//		}
//		else
//		{
//			return maxBufferSize;
//		}
//	}
//	else
//	{
//		return config::UNDEFINED_VALUE;
//	}
//}
//
//vector<vector<Ast*>> Ast::allNegationPatterns(std::vector<Ast*> idSet)
//{
//	int numberOfVariables = idSet.size();
//	vector<vector<Ast*>> result;
//
//	if (numberOfVariables > 0)
//	{
//		string firstMask = string('0', numberOfVariables);
//		string mask = string(firstMask);
//		vector<Ast*> subResult;
//
//		do {
//			subResult.clear();
//
//			for (int ctr = 0; ctr < numberOfVariables; ctr++)
//			{
//				if (mask[ctr] == '0')
//				{
//					subResult.push_back(idSet[ctr]->clone());
//				}
//				else
//				{
//					subResult.push_back(idSet[ctr]->negate());
//				}
//			}
//
//			result.push_back(subResult);
//			mask = config::nextBinaryRepresentation(mask, numberOfVariables);
//		} while (mask.compare(firstMask) != 0);
//	}
//
//	return result;
	//}