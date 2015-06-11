#pragma once

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
	const Ast* getParent();
	int getIndexAsChild();
	int tryGetParentProcessNumber();

/* Local methods */
	void resetBufferSizes();
	void copyPersistentBufferSizes(Ast* source);
	bool generateOutgoingEdges();
	bool isProgramPoint();
	Ast* tryGetNextStatement();
	Ast* tryGetLastStatement();
	Ast* tryGetNextSibling();
	Ast* tryGetLastChild();

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

	void setParent(Ast* newParent);

	Ast* getChild(int index);
	void insertChild(Ast* newChild);
	void insertChild(Ast* newChild, int index);
	void insertChildren(const std::vector<Ast*> &newChildren);
	void insertChildren(const std::vector<Ast*> &newChildren, int index);
	void deleteChild(int index);
	void replaceChild(Ast* newChild, int index);
	void replaceChild(const std::vector<Ast*> &newChildren, int index);

	int getChildrenCount();

/* Public methods */
	void invalidateCode();
	void visitAllProgramPoints();

/* Pseudo-constructors */
	static Ast* newAstFromParsedProgram(const std::string &parsedProgramString);

	static Ast* newLocalAssign(const std::string &variableName, int initialValue);
	static Ast* newLocalAssign(const std::string &variableName, Ast* assignmentNode);

	static Ast* newIfElse(Ast* ifConditionalNode, const std::vector<Ast*> &statements);
	static Ast* newIfElse(Ast* ifConditionalNode, const std::vector<Ast*> &ifStatements, const std::vector<Ast*> &elseStatements);

	static Ast* newID(const std::string &variableName);
	static Ast* newINT(int value);
	static Ast* newStatements(const std::vector<Ast*> &statements);
	static Ast* newNone();
	static Ast* newAbort(const std::string &abortMessage);

/* Cascading methods */
	void registerIDsAsGlobal();
	void registerIDsAsLocal();
	void getCostsFromChildren();
	std::vector<std::string> getIDs();
	void initializePersistentCosts();
	void topDownCascadingCopyPersistentBufferSizes(Ast* source);
	void topDownCascadingAddInitializedCausedCostsToPersistentCosts();
	void topDownCascadingRegisterLabels();
	void topDownCascadingGenerateOutgoingEdges();
	void performBufferSizeAnalysisReplacements();
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