#pragma once

#include <string>
#include <vector>
#include <regex>
#include <z3++.h>

#include "config.h"
#include "bufferSizeMap.h"
#include "ControlFlowVisitor.h"

class ControlFlowEdge;

class Ast
{
public:

	std::string shortStringRepresentation;
	void updateShortStringRepresentation();
	std::string name;

	bufferSizeMap causedWriteCost;
	bufferSizeMap causedReadCost;
	bufferSizeMap persistentWriteCost;
	bufferSizeMap persistentReadCost;

	Ast* parent;
	std::string startComment;
	std::string endComment;
	std::vector<Ast*> children;
	std::vector<ControlFlowEdge*> outgoingEdges;
	int indexAsChild;

	void addChild(Ast* child);
	void addChildren(std::vector<Ast*> newChildren);
	void refreshChildIndices();
	std::string astToString();
	z3::expr astToZ3Expression(z3::context* c);
	std::vector<Ast*> search(std::string soughtName);
	std::vector<std::string> getIDs();
	bool equals(Ast* other);

	std::string getLabelCode();

	void controlFlowDirectionCascadingPropagateTops();
	void cascadingGenerateOutgoingEdges();
	void cascadingUnifyVariableNames();
	void cascadingInitializeAuxiliaryVariables();
	void carryOutReplacements();
	void topDownCascadingRegisterLabels();
	void getCostsFromChildren();
	void initializePersistentCosts();
	void visitAllProgramPoints();
	void cascadingPerformPredicateAbstraction();
	std::string emitCode();

	Ast* weakestLiberalPrecondition(Ast* predicate);
	Ast* negate();
	Ast* clone();

	Ast();
	Ast(std::string initialName);
	~Ast();

	static Ast* newID(std::string variableName);
	static Ast* newINT(int value);
	static Ast* newLocalAssign(std::string variableName, int initialValue);
	static Ast* newLocalAssign(std::string variableName, Ast* assignmentNode);
	static Ast* newIfElse(Ast* ifConditionalNode, std::vector<Ast*> statements);
	static Ast* newIfElse(Ast* ifConditionalNode, std::vector<Ast*> ifStatements, std::vector<Ast*> elseStatements);
	static Ast* newBinaryOp(Ast* leftOperand, Ast* rightOperand, std::string operation);
	static Ast* newUnaryOp(Ast* operand, std::string operation);
	static Ast* newAssume(Ast* assumeConditionalNode);
	static Ast* newBeginAtomic();
	static Ast* newEndAtomic();
	static Ast* newNop();
	static Ast* newAsterisk();
	static Ast* newNone();
	static Ast* newStatements(std::vector<Ast*> statements);
	static Ast* newLabel(int value, Ast* statement);
	static Ast* newLoad(std::string variableName, Ast* rightSide);
	static Ast* newStore(std::string variableName, Ast* rightSide);
	static Ast* newChoose(Ast* firstChoice, Ast* secondChoice);
	static Ast* newAbstractAssignmentFragment(Ast* assignment, Ast* predicate);

	static Ast* newMultipleOperation(std::vector<Ast*> operands, std::string operation);
	static Ast* newBooleanVariableCube(std::string definition);
	static std::vector<std::vector<Ast*>> allCubes(std::vector<int> relevantAuxiliaryTemporaryVariableIndices, int cubeSizeUpperLimit);
	static Ast* newLargestImplicativeDisjunctionOfCubes(int cubeSizeUpperLimit, Ast* predicate);
	static Ast* newLargestImplicativeDisjunctionOfCubes(std::vector<int> relevantAuxiliaryTemporaryVariableIndices, int cubeSizeUpperLimit, Ast* predicate);
	static Ast* newReverseLargestImplicativeDisjunctionOfCubes(int cubeSizeUpperLimit, Ast* predicate);

	static std::vector<std::vector<Ast*>> allNegationPatterns(std::vector<Ast*> idSet);

	static void replaceNode(std::vector<Ast*> nodes, Ast* oldNode);
	static void replaceNode(Ast* newNode, Ast* oldNode);

private:

	std::vector<bufferSizeMap*> _allBufferSizeContainers;
	std::vector<bufferSizeMap*> allBufferSizeContainers();
	bool isProgramPoint();
	void resetBufferSizes();
	void copyPersistentBufferSizes(Ast* source);
	bool propagateTops();
	void topDownCascadingCopyPersistentBufferSizes(Ast* source);
	void topDownCascadingAddInitializedCausedCostsToPersistentCosts();
	bool hasElse();
	bool generateOutgoingEdges();
	bool unifyVariableNames();
	std::string getGotoCode();
	int toLabelCode(std::string processNumber, std::string labelNumber);
	void registerLabel();
	Ast* tryGetNextSibling();
	Ast* tryGetNextStatement();
	bool isRoot();
	bool tryGetParentProcessNumber(std::string* out);
	Ast* tryGetLastChild();
	Ast* tryGetLastStatement();
	int effectiveMaxWriteBufferSize(std::string variableName);
	int numberOfVariablesInPersistentWriteBuffer();
	bool performPredicateAbstraction();
	void initializeAuxiliaryVariables();
	std::vector<Ast*> reportBack();
};

