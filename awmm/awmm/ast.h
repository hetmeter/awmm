#pragma once

#ifndef MAP_INCLUDED
#define MAP_INCLUDED
#include <map>
#endif

#ifndef VECTOR_INCLUDED
#define VECTOR_INCLUDED
#include <vector>
#endif

typedef std::map<std::string, int> bufferSizeMap;
typedef bufferSizeMap::iterator bufferSizeMapIterator;

struct controlFlowEdge;
typedef struct controlFlowEdge controlFlowEdge;

//char LABEL_SEPARATOR;

const std::string ID_TOKEN_NAME = "ID";
const std::string PROGRAM_DECLARATION_TOKEN_NAME = "programDeclaration";
const std::string PROCESS_DECLARATION_TOKEN_NAME = "processDeclaration";
const std::string INITIALIZATION_BLOCK_TOKEN_NAME = "initializationBlock";
const std::string STATEMENTS_TOKEN_NAME = "statements";
const std::string STORE_TOKEN_NAME = "store";
const std::string LOAD_TOKEN_NAME = "load";
const std::string FENCE_TOKEN_NAME = "fence";
const std::string LABEL_TOKEN_NAME = "label";
const std::string GOTO_TOKEN_NAME = "goto";
const std::string IF_ELSE_TOKEN_NAME = "ifElse";
const std::string WHILE_TOKEN_NAME = "while";
const std::string NONE_TAG_NAME = "none";

struct ast
{
private:
		std::vector<bufferSizeMap*> _allBufferSizeContainers;
		std::vector<bufferSizeMap*> allBufferSizeContainers();
		std::string bufferSizeMapString(bufferSizeMap* source);
		bool isProgramPoint();
public:
	   std::string name;
	   bufferSizeMap causedWriteCost;
	   bufferSizeMap causedReadCost;
	   bufferSizeMap persistentWriteCost;
	   bufferSizeMap persistentReadCost;
	   ast* parent;
	   std::vector<ast*> children;
	   std::vector<controlFlowEdge*> outgoingEdges;
	   int indexAsChild = -1;
	   bufferSizeMap writeBufferSizeMap;
	   bufferSizeMap readBufferSizeMap;
	   void getCostsFromChildren();
	   void resetBufferSizes();
	   void copyPersistentBufferSizes(ast* source);
	   void propagateTops();
	   std::vector<std::string> getIDs();
	   void topDownCascadingCopyPersistentBufferSizes(ast* source);
	   void topDownCascadingAddInitializedCausedCostsToPersistentCosts();
	   void topDownCascadingRegisterLabels();
	   void controlFlowDirectionCascadingPropagateTops();
	   void cascadingGenerateOutgoingEdges();
	   bool has;
	   bool generateOutgoingEdges();
	   int getGotoCode();
	   int getLabelCode();
	   int toLabelCode(std::string processNumber, std::string labelNumber);
	   void registerLabel();
	   void initializePersistentCosts();
	   void visitAllProgramPoints();
	   void addChild(ast* child);
	   bool tryGetNextSibling(ast* out);
	   bool isRoot();
	   bool tryGetParentProcessNumber(int* out);
	   bool tryGetLastChild(ast* out);
	   std::string toString();
};
typedef struct ast ast;