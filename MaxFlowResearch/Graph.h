#pragma once
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <vector>
#include <fstream>
#include <iostream>
#include <string>
#include <random>
#include <iomanip>
#include <algorithm>
#include <string>
#include <stack>

using namespace std;

typedef pair<vector<string>, long long> way;

class Graph
{
private:
	struct Edge {
		int to, rev;  // rev - индекс обратного ребра
		long long cap, flow;
	};

	string name;
	unordered_map<int, unordered_map<int, long long>> nodes;
	bool weighted;
	bool oriented;
	unordered_set<int> visited;

	vector<vector<long long>> adjacencyMatrix;
	vector<vector<Edge>> adj;
	unordered_map<string, int> nodeIndexMap;

	bool IsNodeVisited(int node);
	vector<string> MergeVectors(vector<string> first, vector<string> second);
	//long long DFSWithDelta(string u, string t, long long delta, unordered_map<string, string>& parent);

	static const long long INF = (long long)1e18 + 1;

	// Вспомогательные методы для алгоритма Диница
	bool bfs(int s, int t, vector<int>& level);
	bool bfsWithCapacity(int s, int t, vector<int>& level, long long delta);
	bool bfsUnord_mapWithCapacity(int s, int t, unordered_map<int, int>& level, long long delta);
	long long dfs(int u, int t, long long flow, vector<int>& level);
	long long dfsOptimazed(int u, int t, long long flow, vector<int>& ptr, vector<int>& level);
	long long dfsWithCapacity(int u, int t, long long flow, vector<int>& ptr, vector<int>& level, long long delta);
	long long dfsUnord_mapWithCapacity(int u, int t, long long flow, unordered_map<int, int>& ptr, unordered_map<int, int>& level, long long delta);
	

	bool bfsUnord_map(int s, int t, unordered_map<int, int>& level);
	long long dfsUnord_map(int u, int t, long long flow, unordered_map<int, int>& ptr, unordered_map<int, int>& level);


	bool bfsWithEdges(int s, int t, vector<int>& level);
	long long dfsWithEdges(int u, int t, long long flow, vector<int>& ptr, vector<int>& level);


	//для приближённого
	vector<vector<long long>> residual;  // Остаточные пропускные способности
	vector<vector<double>> dual_weights; // Двойственные веса рёбер
	vector<double> distance;            // Расстояния для кратчайших путей
	vector<int> parent;                 // Для восстановления пути

	// Инициализация остаточной сети и двойственных весов
	void initGargKonemann();

public:
	Graph();
	Graph(string name, bool weighted, bool oriented);
	Graph(const Graph&);
	bool IsNodeExist(int node);
	bool IsEdgeExist(int nodeFrom, int nodeWhere);
	bool IsWeighted();
	bool IsOriented();
	int GetCountOfNodes();
	void BuildAdjacencyMatrix();
	void BuildAdjEdgesFromMatrix();
	void BuildAdjacencyListFromMatrix();
	string GetName();
	void SetName(string name);
	long long GetWeight(int nodeFrom, int nodeWhere);
	void AddNode(int node);
	void CopyNode(int node, int nameNewNode);
	void AddEdge(int nodeFrom, int nodeWhere, long long value = 1);
	void DeleteNode(int node);
	void DeleteAllNodes();
	void DeleteEdge(int nodeFrom, int nodeWhere);
	void DeleteAllEdgeTo(int node);
	void ClearVisited();

	/*long long FordFulkersonCheck(string s, string t);*/
	long long FordFulkersonMatrix(int s, int t);
	long long FordFulkersonBFS(int s, int t);
	long long FordFulkersonScaling(int s, int t, double eps = 0.01);
	long long FordFulkersonScalingApproximate(int s, int t, double eps = 0.01);

	long long getMaxFlowPushRelabel(int source, int sink);
	long long getMaxFlowPushRelabel_HLF(int source, int sink);
	long long getMaxFlowPushRelabel_HLF_GlRel(int source, int sink);
	long long getMaxFlowPushRelabel_HLF_GlRelUnordered_map(int source, int sink);

	// Новый метод для алгоритма Диница
	long long getMaxFlowDinicMatrix(int source, int sink);
	long long getMaxFlowDinicMatrixOptimized(int source, int sink);
	long long getMaxFlowDinicCapacityScaling(int source, int sink, long long maxCapacity = 10000);
	long long getMaxFlowDinicUnord_map(int source, int sink);
	long long getMaxFlowDinicUnord_mapCapacityScaling(int source, int sink, long long maxCapacity = 10000);
	//long long getMaxFlowDinicWithEdges(int source, int sink);

	// Новый метод для алгоритма MaxFlow-WO
	//long long MaxFlowWO(const string& source, const string& sink, int max_iterations = 10);

	//Приближённый
	bool findShortestPath(int s, int t);
	long long augmentFlow(int s, int t, double epsilon);
	long long gargKonemannMaxFlow(int s, int t, double epsilon = 0.01);

	long long DinicMaxFlow(int s, int t);
	long long DinicMaxFlowMatrix(int s, int t);


	void TransformToRandomFlowGraph(string name, unsigned int countNodes, float density, unsigned int maxWeightValues);

	friend std::ostream& operator<< (std::ostream& out, const Graph& graph);
	~Graph();
};