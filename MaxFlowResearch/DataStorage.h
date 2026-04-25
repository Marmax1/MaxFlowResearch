#pragma once
#include "Graph.h"
#include <chrono>
#include <numeric>
#include <functional>
#include <map>

class DataStorage
{
private:
	vector<Graph> graphs;
	vector<Graph>::iterator iter;
public:
	int Size();
	void SetGraph(string name);
	void CreateCopyGraph(string newName);
	void DeleteGraph();
	void SetName(string name);
	string GetName();
	bool IsWeighted();
	bool IsOriented();
	bool IsGraphExist(string name);
	void AddNode(int node);
	void AddEdge(int nodeFrom, int nodeWhere, int value);
	void DeleteNode(int node);
	void DeleteEdge(int nodeFrom, int nodeWhere);
	void ChangeValueWay(int nodeFrom, int nodeWhere, int value);
	void WriteToConsole();
	void WriteToConsoleAll();



	long long GetMaxFlowFordFulkerson(int s, int t);
	long long GetMaxFlowPushRelabel();
	long long GetMaxFlowDinic(int s, int t);
	
	long long gargKonemannMaxFlow();

	//Сравнение методов по скорости
	// graphType:
	// 0 - Ориентированный ациклический граф
	// 1 - Ориентированный случайный граф
	// 2 - Полный граф
	// 3 - Двудольный граф
	void CompareMethods(unsigned int countNodes, float density, long long maxWeightValue, 
		unsigned int countGraphs, unsigned int graphType, const vector<string>& methodNames);
	void ComparePushRelabelMethods();
	void CompareDinicMethods();
	void CompareBestMaxFlowMetods();

	/*
	Create random graph with the specified parameters
	density - плотность (from 0 to 1)
	Значения дуг от 1 до maxWeightValue
	*/
	void CreateRandomFlowGraph(unsigned int& outEdgesCount, unsigned int countNodes = 10, float density = 0.4, unsigned int maxWeightValue = 10000);
	void CreateRandomGraph(unsigned int& outEdgesCount, unsigned int countNodes = 10, float density = 0.4, unsigned int maxWeightValue = 10000);
	void CreateCompleteGraph(unsigned int& outEdgesCount, unsigned int countNodes = 10, unsigned int maxWeightValue = 10000);
	void CreateBipartiteGraph(unsigned int& outEdgesCount, unsigned int countNodes = 10, float density = 0.4, unsigned int maxWeightValue = 10000);

	~DataStorage();
};