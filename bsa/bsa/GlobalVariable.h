//#pragma once
//
//#include <map>
//#include <vector>
//
//class GlobalVariable
//{
//private:
//
//	std::string _variableName;
//	std::map<int, int> maximumBufferSizePerProcess;
//	
//	std::string registerAuxiliaryVariable(const std::string &minimalName);
//
//public:
//
//	GlobalVariable(const std::string &variableName, const std::vector<int> &processes);
//	~GlobalVariable();
//
//	std::map<int, std::string> auxiliaryCounterVariableNames;
//	std::map<int, std::string> auxiliaryFirstPointerVariableNames;
//	std::map<std::pair<int, int>, std::string> auxiliaryWriteBufferVariableNames;
//
//	void setMaximumBufferSize(int process, int bufferSize);
//	int getMaximumBufferSize(int process);
//};