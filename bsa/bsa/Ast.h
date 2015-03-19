#pragma once

#include <string>
#include <vector>
#include <regex>

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
	std::vector<Ast*> children;
	std::vector<ControlFlowEdge*> outgoingEdges;
	int indexAsChild;

	void addChild(Ast* child);
	std::string astToString();

	std::string getLabelCode();

	void controlFlowDirectionCascadingPropagateTops();
	void cascadingGenerateOutgoingEdges();
	void cascadingUnifyVariableNames();
	void topDownCascadingRegisterLabels();
	void getCostsFromChildren();
	void initializePersistentCosts();
	void visitAllProgramPoints();
	std::string emitCode();

	Ast();
	~Ast();

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
	void initializeAuxiliaryVariables();
};

