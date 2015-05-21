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
	void addChildren(std::vector<Ast*> newChildren);
	void refreshChildIndices();
	std::vector<Ast*> search(std::string soughtName);
	std::string getLabelCode();
	Ast* weakestLiberalPrecondition(Ast* predicate);
	std::string toString();
	z3::expr toZ3Expression(z3::context* c);

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
	void setVariableInitializations();
	void topDownCascadingCopyPersistentBufferSizes(Ast* source);
	void topDownCascadingAddInitializedCausedCostsToPersistentCosts();
	void controlFlowDirectionCascadingPropagateTops();

/* Static operations */
	static void replaceNode(std::vector<Ast*> nodes, Ast* oldNode);
	static void replaceNode(Ast* newNode, Ast* oldNode);

/* Static pseudo-constructors */
	static Ast* newID(std::string variableName);
	static Ast* newINT(int value);
	static Ast* newBinaryOp(Ast* leftOperand, Ast* rightOperand, std::string operation);
	static Ast* newMultipleOperation(std::vector<Ast*> operands, std::string operation);
	static Ast* newLocalAssign(std::string variableName, int initialValue);
	static Ast* newLocalAssign(std::string variableName, Ast* assignmentNode);
	static Ast* newStore(std::string variableName, Ast* rightSide);
	static Ast* newLoad(std::string variableName, Ast* rightSide);
	static Ast* newIfElse(Ast* ifConditionalNode, std::vector<Ast*> statements);
	static Ast* newIfElse(Ast* ifConditionalNode, std::vector<Ast*> ifStatements, std::vector<Ast*> elseStatements);
	static Ast* newStatements(std::vector<Ast*> statements);
	static Ast* newNone();
	static Ast* newNop();
	static Ast* newLabel(int value, Ast* statement);
	static Ast* newAsterisk();
	static Ast* newTrue();
	static Ast* newFalse();
	static Ast* newAssume(Ast* assumeConditionalNode);
	static Ast* newChoose(Ast* firstChoice, Ast* secondChoice);
	static Ast* newBeginAtomic();
	static Ast* newEndAtomic();

	static Ast* newAstFromParsedProgram(std::string parsedProgramString);
	static Ast* newSharedVariables(std::vector<std::string> variableNames);
	static Ast* newLocalVariables(std::vector<std::string> variableNames);
	static Ast* newBooleanVariableCube(std::string definition, bool useTemporaryVariables = true);
	static Ast* newAbstractAssignmentFragment(Ast* assignment, Ast* predicate);
	static Ast* newLargestImplicativeDisjunctionOfCubes(int cubeSizeUpperLimit, Ast* predicate, bool useTemporaryVariables = true);
	static Ast* newReverseLargestImplicativeDisjunctionOfCubes(int cubeSizeUpperLimit, Ast* predicate);
	//
	//	static Ast* newUnaryOp(Ast* operand, std::string operation);

private:

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
	void copyPersistentBufferSizes(Ast* source);
	bool propagateTops();
	Ast* tryGetNextStatement();
	Ast* tryGetLastStatement();
	Ast* tryGetNextSibling();
	Ast* tryGetLastChild();
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