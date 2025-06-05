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

using namespace std;

typedef pair<vector<string>, long long> way;

class Graph
{
private:
	struct FlowEdge;
	
	struct Cluster;

	string name;
	unordered_map<string, unordered_map<string, long long>> nodes;
	bool weighted;
	bool oriented;
	unordered_set<string> visited;

	vector<vector<long long>> adjacencyMatrix;
	unordered_map<string, int> nodeIndexMap;

	bool IsNodeVisited(string node);
	vector<string> GetWayByPrev(string from, string to, unordered_map<string, string>& prev);
	vector<string> MergeVectors(vector<string> first, vector<string> second);
	long long Remains(FlowEdge edge);
	long long GetIndexReverseEdge(vector<FlowEdge>& e, long long curIndex);
	long long DfsForFlow(string& curNode, long long curFlow, string& T, vector<FlowEdge>& e, unordered_map<string, vector<long long>>& indexes);
	long long DFSWithDelta(string u, string t, long long delta, unordered_map<string, string>& parent);

	static const long long INF = (long long)1e18 + 1;


	// Вспомогательные методы для MaxFlow-WO
	vector<Cluster> formClusters(const string& source, const string& sink);
	long long calculateLocalFlow(Cluster& cluster, const string& source, const string& sink);
	void assignNodesToClusters(vector<Cluster>& clusters, const string& sink);
	void findConnectingEdges(vector<Cluster>& clusters);

	// Вспомогательные методы для алгоритма Диница
	bool bfs(int s, int t, vector<int>& level);
	long long dfs(int u, int t, long long flow, vector<int>& ptr, vector<int>& level);

public:
	Graph();
	Graph(string name, bool weighted, bool oriented);
	Graph(const Graph&);
	bool IsNodeExist(string node);
	bool IsEdgeExist(string nodeFrom, string nodeWhere);
	bool IsWeighted();
	bool IsOriented();
	int GetCountOfNodes();
	void BuildAdjacencyMatrix();
	string GetName();
	void SetName(string name);
	long long GetWeight(string nodeFrom, string nodeWhere);
	void AddNode(string node);
	void CopyNode(string node, string nameNewNode);
	void AddEdge(string nodeFrom, string nodeWhere, long long value = 1);
	void DeleteNode(string node);
	void DeleteAllNodes();
	void DeleteEdge(string nodeFrom, string nodeWhere);
	void DeleteAllEdgeTo(string node);
	void ClearVisited();

	/*long long FordFulkersonCheck(string s, string t);*/
	long long FordFulkersonMatrix(string s, string t);
	long long FordFulkerson(string s, string t);
	long long FordFulkersonBFS(string s, string t);
	long long FordFulkersonScaling(string s, string t, double eps = 0.01);
	long long FordFulkersonScalingApproximate(string s, string t, double eps = 0.01);

	long long getMaxFlowPushRelabel(int source, int sink);
	long long getMaxFlowPushRelabel_v2(int source, int sink);
	long long getMaxFlowPushRelabel_v3(int source, int sink);

	// Новый метод для алгоритма Диница
	long long getMaxFlowDinic(int source, int sink);
	long long getMaxFlowDinicScaling(int source, int sink);

	// Новый метод для алгоритма MaxFlow-WO
	long long MaxFlowWO(const string& source, const string& sink, int max_iterations = 10);

	long long PushRelabelApproximate(string s, string t, int max_pushes = 100000);
	long long PushRelabelMatrix(string s, string t, int max_pushes = 1000000);
	long long PushRelabelParallel(string s, string t, int max_pushes = 1000000);

	long long DinicMaxFlow(string s, string t);
	long long DinicMaxFlowMatrix(string s, string t);

	void TransformToRandomFlowGraph(string name, unsigned int countNodes, float density, unsigned int maxWeightValues);
	void TransformToDirected();

	friend std::ostream& operator<< (std::ostream& out, const Graph& graph);
	~Graph();
};