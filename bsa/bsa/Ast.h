#pragma once

#include <set>
#include <string>
#include <vector>
#include <z3++.h>

#include "bufferSizeMap.h"

class ControlFlowEdge;

class Ast
{
private:

/* Locals */
	std::string _name;

	std::string _code;
	bool _codeIsValid;

	Ast* _parent;
	std::vector<Ast*> _children;
	int _childrenCount;

	std::string _startComment;
	std::string _endComment;

/* Private fields */
	int getParentProcessNumber();

/* Local methods */
	void resetBufferSizes();
	void copyPersistentBufferSizes(Ast* source);
	bool generateOutgoingEdges();
	bool isProgramPoint();
	Ast* tryGetNextStatement();
	Ast* tryGetLastStatement();
	Ast* tryGetNextSibling();
	Ast* tryGetLastChild();
	bool performPredicateAbstraction();

public:

/* Constructors and destructor */
	Ast(const std::string &name);
	~Ast();

/* Public members */
	bufferSizeMap causedWriteCost;
	bufferSizeMap causedReadCost;
	bufferSizeMap persistentWriteCost;
	bufferSizeMap persistentReadCost;

	std::vector<ControlFlowEdge*> outgoingEdges;

/* Public fields */
	const std::string getName();
	void setName(const std::string value);

	const std::string getCode();

	Ast* getParent();
	void setParent(Ast* newParent);

	Ast* getChild(int index);
	const std::vector<Ast*> getChildren();
	void insertChild(Ast* newChild);
	void insertChild(Ast* newChild, int index);
	void insertChildren(const std::vector<Ast*> &newChildren);
	void insertChildren(const std::vector<Ast*> &newChildren, int index);
	void deleteChild(int index);
	void replaceChild(Ast* newChild, int index);
	void replaceChild(const std::vector<Ast*> &newChildren, int index);

	int getChildrenCount();

	const std::string getStartComment();
	void setStartComment(const std::string &value);
	const std::string getEndComment();
	void setEndComment(const std::string &value);

	int getIndexAsChild();
	z3::expr getZ3Expression(z3::context* c);

/* Public methods */
	void invalidateCode();
	void visitAllProgramPoints();
	void unfoldIfElses();
	void replaceNodeWithoutDeletions(Ast* newData);
	void expandIDNodes(const std::string &bufferIdentifier, int process);
	std::string toString();

/* Replicators */
	Ast* clone();
	Ast* negate();

/* Pseudo-constructors */
	static Ast* newAstFromParsedProgram(const std::string &parsedProgramString);

	static Ast* newLocalAssign(const std::string &variableName, int initialValue);
	static Ast* newLocalAssign(const std::string &variableName, Ast* assignmentNode);

	static Ast* newIfElse(Ast* ifConditionalNode, const std::vector<Ast*> &statements);
	static Ast* newIfElse(Ast* ifConditionalNode, const std::vector<Ast*> &ifStatements, const std::vector<Ast*> &elseStatements);

	static Ast* newBinaryOp(Ast* leftOperand, Ast* rightOperand, const std::string &operation);
	static Ast* newMultipleOperation(const std::vector<Ast*> &operands, const std::string &operation);

	static Ast* newID(const std::string &variableName);
	static Ast* newINT(int value);
	static Ast* newTrue();
	static Ast* newFalse();
	static Ast* newStatements(const std::vector<Ast*> &statements);
	static Ast* newNone();
	static Ast* newNop();
	static Ast* newAbort(const std::string &abortMessage);
	static Ast* newAsterisk();
	static Ast* newAssume(Ast* assumeConditionalNode);
	static Ast* newChoose(Ast* firstChoice, Ast* secondChoice);
	static Ast* newStore(const std::string &variableName, Ast* rightSide);
	static Ast* newLoad(const std::string &variableName, Ast* rightSide);
	static Ast* newSharedVariables(const std::vector<std::string> &variableNames);
	static Ast* newLocalVariables(const std::vector<std::string> &variableNames);
	static Ast* newBeginAtomic();
	static Ast* newEndAtomic();
	static Ast* newLabel(int value, Ast* statement);
	static Ast* newGoto(int value);
	static Ast* newBooleanIf(Ast* ifConditionalNode, Ast* statement);

	static Ast* newBooleanVariableCube(const std::string &definition, bool useTemporaryVariables);
	static Ast* newLargestImplicativeDisjunctionOfCubes(int cubeSizeUpperLimit, Ast* predicate, bool useTemporaryVariables = true);
	static Ast* newReverseLargestImplicativeDisjunctionOfCubes(int cubeSizeUpperLimit, Ast* predicate);
	static Ast* newAbstractAssignmentFragment(Ast* assignment, Ast* predicate);
	static Ast* newWeakestLiberalPrecondition(Ast* assignment, Ast* predicate);

/* Cascading methods */
	void registerIDsAsGlobal();
	void registerIDsAsLocal();
	void getCostsFromChildren();
	std::vector<std::string> getIDs();
	std::set<std::string> getIDset();
	std::vector<Ast*> fetchIDs();
	void initializePersistentCosts();
	void topDownCascadingCopyPersistentBufferSizes(Ast* source);
	void topDownCascadingAddInitializedCausedCostsToPersistentCosts();
	void topDownCascadingRegisterLabels();
	void topDownCascadingGenerateOutgoingEdges();
	void topDownCascadingUnfoldIfElses();
	void topDownCascadingPerformPredicateAbstraction();
	void topDownCascadingReplaceIDNames(const std::string &oldName, const std::string &newName);
	void topDownCascadingReportBufferSizes(int process = -1);
	void controlFlowDirectionCascadingPropagateTops();
	void performBufferSizeAnalysisReplacements();
	void generateBooleanVariableInitializations();
	std::vector<Ast*> search(const std::string &soughtName);
};