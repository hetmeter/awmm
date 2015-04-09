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
	std::vector<Ast*> search(std::string soughtName);

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

	Ast();
	Ast(std::string variableName);
	Ast(int value);
	Ast(std::string variableName, int initialValue);
	Ast(std::string variableName, Ast* assignmentNode);
	Ast(Ast* ifConditionalNode, std::vector<Ast*> statements);
	Ast(Ast* ifConditionalNode, std::vector<Ast*> ifStatements, std::vector<Ast*> elseStatements);
	Ast(Ast* assumeConditionalNode);
	Ast(Ast* leftOperand, Ast* rightOperand, std::string operation);
	Ast(Ast* operand, std::string operation);
	~Ast();

	static Ast* newBeginAtomic();
	static Ast* newEndAtomic();
	static Ast* newNop();
	static Ast* newAsterisk();
	static Ast* newLabel(int value, Ast* statement);
	static Ast* newLoad(std::string variableName, Ast* rightSide);
	static Ast* newStore(std::string variableName, Ast* rightSide);
	static Ast* newChoose(Ast* firstChoice, Ast* secondChoice);

	static void replaceNode(std::vector<Ast*> nodes, Ast* oldNode);
	static void replaceNode(Ast* newNode, Ast* oldNode);

private:

	std::vector<bufferSizeMap*> _allBufferSizeContainers;
	std::vector<bufferSizeMap*> allBufferSizeContainers();
	bool isProgramPoint();
	void resetBufferSizes();
	void copyPersistentBufferSizes(Ast* source);
	bool propagateTops();
	std::vector<std::string> getIDs();
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
	Ast* clone();
};

