#pragma once
#include "Graph.h"
#include <chrono>
#include <numeric>
#include <functional>

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
	void AddNode(string node);
	void AddEdge(string nodeFrom, string nodeWhere, int value);
	void DeleteNode(string node);
	void DeleteEdge(string nodeFrom, string nodeWhere);
	void ChangeValueWay(string nodeFrom, string nodeWhere, int value);
	void TransformGraphToDirected();
	void WriteToConsole();
	void WriteToConsoleAll();



	long long GetMaxFlowFordFulkerson(string s, string t);
	long long GetMaxFlowPushRelabel();
	long long GetMaxFlowDinic(string s, string t);
	
	long long MaxFlowWO(string s, string t);

	// Finding the shortest paths
	void WriteSpeedExecutionMaxFlowMethodsFor(unsigned int countNodes, float density, unsigned int maxWeightValue, unsigned int countGraphs = 25);
	void CompareMaxFlowMetods();

	/*
	Create random graph with the specified parameters
	density - плотность (from 0 to 1)
	weightValues - массив возможных весов (empty is any number)
	*/
	void CreateRandomFlowGraph(unsigned int countNodes = 10, float density = 0.4, unsigned int maxWeightValue = 10000);

	~DataStorage();
};