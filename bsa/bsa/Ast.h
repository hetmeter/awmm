#pragma once

#include <string>
#include <vector>
#include <z3++.h>

#include "bufferSizeMap.h"

class ControlFlowEdge;

class Ast
{
public:

/* Constructors and destructor */
	Ast();
	~Ast();

/* Replicators */
	Ast* clone();
	Ast* negate();

/* Attributes */
	std::string name;
	std::vector<Ast*> children;
	Ast* parent;
	int indexAsChild;
	std::string startComment;
	std::string endComment;

	bufferSizeMap causedWriteCost;
	bufferSizeMap causedReadCost;
	bufferSizeMap persistentWriteCost;
	bufferSizeMap persistentReadCost;

	std::vector<ControlFlowEdge*> outgoingEdges;

/* Access */
	void addChild(Ast* child);
	void addChild(Ast* child, int index);
	void addChildren(const std::vector<Ast*> &newChildren);
	void refreshChildIndices();
	std::vector<Ast*> search(const std::string &soughtName);
	std::string getLabelCode();
	Ast* weakestLiberalPrecondition(Ast* predicate);
	std::string toString();
	z3::expr toZ3Expression(z3::context* c);
	const std::string getCode();

/* Cascading operations */
	std::string emitCode();
	std::vector<std::string> getIDs();
	void cascadingUnifyVariableNames();
	void getCostsFromChildren();
	void initializePersistentCosts();
	void topDownCascadingRegisterLabels();
	void cascadingGenerateOutgoingEdges();
	void visitAllProgramPoints();
	void cascadingInitializeAuxiliaryVariables();
	void carryOutReplacements();
	void cascadingPerformPredicateAbstraction();
	void cascadingUnfoldIfElses();
	void setVariableInitializations();
	void topDownCascadingCopyPersistentBufferSizes(Ast* source);
	void topDownCascadingAddInitializedCausedCostsToPersistentCosts();
	void controlFlowDirectionCascadingPropagateTops();
	void labelAllStatements();
	bool isEquivalent(Ast* otherAst);

/* Static operations */
	static void replaceNode(const std::vector<Ast*> &nodes, Ast* oldNode);
	static void replaceNode(Ast* newNode, Ast* oldNode);

/* Static pseudo-constructors */
	static Ast* newID(const std::string &variableName);
	static Ast* newINT(int value);
	static Ast* newBinaryOp(Ast* leftOperand, Ast* rightOperand, const std::string &operation);
	static Ast* newMultipleOperation(const std::vector<Ast*> &operands, const std::string &operation);
	static Ast* newLocalAssign(const std::string &variableName, int initialValue);
	static Ast* newLocalAssign(const std::string &variableName, Ast* assignmentNode);
	static Ast* newStore(const std::string &variableName, Ast* rightSide);
	static Ast* newLoad(const std::string &variableName, Ast* rightSide);
	static Ast* newIfElse(Ast* ifConditionalNode, const std::vector<Ast*> &statements);
	static Ast* newIfElse(Ast* ifConditionalNode, const std::vector<Ast*> &ifStatements, const std::vector<Ast*> &elseStatements);
	static Ast* newStatements(const std::vector<Ast*> &statements);
	static Ast* newNone();
	static Ast* newNop();
	static Ast* newGoto(int value);
	static Ast* newLabel(int value, Ast* statement);
	static Ast* newAsterisk();
	static Ast* newTrue();
	static Ast* newFalse();
	static Ast* newAssume(Ast* assumeConditionalNode);
	static Ast* newChoose(Ast* firstChoice, Ast* secondChoice);
	static Ast* newBeginAtomic();
	static Ast* newEndAtomic();
	static Ast* newBooleanIf(Ast* ifConditionalNode, Ast* statement);

	static Ast* newAstFromParsedProgram(const std::string &parsedProgramString);
	static Ast* newSharedVariables(const std::vector<std::string> &variableNames);
	static Ast* newLocalVariables(const std::vector<std::string> &variableNames);
	static Ast* newBooleanVariableCube(const std::string &definition, bool useTemporaryVariables = true);
	static Ast* newAbstractAssignmentFragment(Ast* assignment, Ast* predicate);
	static Ast* newLargestImplicativeDisjunctionOfCubes(int cubeSizeUpperLimit, Ast* predicate, bool useTemporaryVariables = true);
	static Ast* newReverseLargestImplicativeDisjunctionOfCubes(int cubeSizeUpperLimit, Ast* predicate);
	//
	//	static Ast* newUnaryOp(Ast* operand, std::string operation);

private:
/* Locals */
	std::string _code = "";

/* Local access */
	void resetBufferSizes();
	bool tryGetParentProcessNumber(std::string* out);
	int numberOfVariablesInPersistentWriteBuffer();
	bool isRoot();
	bool isProgramPoint();
	bool hasElse();
	std::string getGotoCode();

	std::vector<bufferSizeMap*> _allBufferSizeContainers;
	std::vector<bufferSizeMap*> allBufferSizeContainers();

/* Cascade elements */
	bool unifyVariableNames();
	void registerLabel();
	bool generateOutgoingEdges();
	void initializeAuxiliaryVariables();
	std::vector<Ast*> reportBack();
	bool performPredicateAbstraction();
	bool unfoldIfElses();
	void copyPersistentBufferSizes(Ast* source);
	bool propagateTops();
	Ast* tryGetNextStatement();
	Ast* tryGetLastStatement();
	Ast* tryGetNextSibling();
	Ast* tryGetLastChild();
	void labelStatement();
};


//public:
//
//	std::string shortStringRepresentation;
//	void updateShortStringRepresentation();
//
//	bool equals(Ast* other);
//
//	Ast(std::string initialName);
//
//	static std::vector<std::vector<Ast*>> allCubes(std::vector<int> relevantAuxiliaryTemporaryVariableIndices, int cubeSizeUpperLimit);
//	static Ast* newLargestImplicativeDisjunctionOfCubes(std::vector<int> relevantAuxiliaryTemporaryVariableIndices, int cubeSizeUpperLimit, Ast* predicate);
//
//	static std::vector<std::vector<Ast*>> allNegationPatterns(std::vector<Ast*> idSet);
//
//private:
//
//	int toLabelCode(std::string processNumber, std::string labelNumber);
//	int effectiveMaxWriteBufferSize(std::string variableName);