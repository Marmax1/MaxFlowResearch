#include "Graph.h"
#include <numeric>
#include <functional>

typedef pair<string, long long> edgePair;

class Compare
{
public:
	bool operator() (edgePair a, edgePair b)
	{
		return b.second < a.second;
	}
};

class CompareWay {
public:
	bool operator() (way& a, way& b)
	{
		return b.second < a.second;
	}
};

struct Graph::FlowEdge {
	string from;
	string to;
	long long capacity;
	long long flow;

	FlowEdge(string f, string t, long long c) : from(f), to(t), capacity(c), flow(0) {}
	FlowEdge(string f, string t, long long c, long long flow) : from(f), to(t), capacity(c), flow(flow) {}
};

struct Graph::Cluster {
	string center; // Центр кластера
	unordered_set<string> nodes; // Узлы в кластере
	vector<pair<string, string>> edges; // Ребра кластера
	long long local_flow; // Локальный максимальный поток
};


bool Graph::IsNodeVisited(string node) {
	if (visited.find(node) == visited.end())
		return false;
	return true;
}

vector<string> Graph::GetWayByPrev(string from, string to, unordered_map<string, string>& prev) {
	vector<string> way;
	while (to != from) {
		way.push_back(to);
		to = prev[to];
	}
	way.push_back(from);
	reverse(way.begin(), way.end());
	return way;
}

vector<string> Graph::MergeVectors(vector<string> first, vector<string> second) {
	vector<string> vec(first);
	vec.insert(vec.end(), second.begin(), second.end());
	return vec;
}

long long Graph::Remains(FlowEdge edge) {
	return edge.capacity - edge.flow;
}

long long Graph::GetIndexReverseEdge(vector<FlowEdge>& e, long long curIndex) {
	if (curIndex == 0)
		return 1;

	if (curIndex == e.size() - 1)
		return curIndex - 1;

	if (e[curIndex - 1].from == e[curIndex].to && e[curIndex - 1].to == e[curIndex].from)
		return curIndex - 1;

	if (e[curIndex + 1].from == e[curIndex].to && e[curIndex + 1].to == e[curIndex].from)
		return curIndex + 1;
}

long long Graph::DfsForFlow(string& curNode, long long curCapacity, string& T, vector<FlowEdge>& e, unordered_map<string, vector<long long>>& indexes) {
	if (curNode == T) {
		cout << curNode << " <- ";
		return curCapacity;
	}

	if (IsNodeVisited(curNode))
		return 0;

	visited.insert(curNode);

	for (long long ind : indexes[curNode]) {
		if (Remains(e[ind]) == 0)
			continue;

		long long res = DfsForFlow(e[ind].to, min(Remains(e[ind]), curCapacity), T, e, indexes);

		if (res > 0) {
			e[ind].flow += res;	// прямое ребро
			long long indexRev = GetIndexReverseEdge(e, ind);
			e[indexRev].flow -= res;	// обратное
			cout << curNode << " <- ";
			return res;
		}
	}

	return 0;
}

long long Graph::DFSWithDelta(string u, string t, long long delta, unordered_map<string, string>& parent) {
	visited.insert(u);

	if (u == t) {
		return delta; // Нашли путь до стока
	}

	for (auto& edge : nodes[u]) {
		string v = edge.first;
		long long capacity = edge.second;

		if (!IsNodeVisited(v) && capacity >= delta) {
			parent[v] = u;
			long long min_flow = DFSWithDelta(v, t, delta, parent);

			if (min_flow > 0) {
				// Обновляем пропускные способности
				nodes[u][v] -= min_flow;
				if (!oriented) {
					nodes[v][u] += min_flow;
				}
				else {
					// Для ориентированного графа добавляем обратное ребро
					if (nodes[v].find(u) == nodes[v].end()) {
						nodes[v][u] = 0;
					}
					nodes[v][u] += min_flow;
				}
				return min_flow;
			}
		}
	}
	return 0;
}




Graph::Graph() {
	name = "";
	weighted = false;
	oriented = false;
}

Graph::Graph(string name, bool weighted, bool oriented) {
	this->name = name;
	this->weighted = weighted;
	this->oriented = oriented;
}

Graph::Graph(const Graph& gr) {
	name = gr.name;
	nodes = gr.nodes;
	/*for (const auto& node : gr.nodes) {
		for (const auto& way : node.second) {
			nodes[node.first][way.first] = way.second;
		}
	}*/
	weighted = gr.weighted;
	oriented = gr.oriented;
	adjacencyMatrix = gr.adjacencyMatrix;
}

void Graph::BuildAdjacencyMatrix() {

	string source = "source";
	string sink = "sink";

	// Собираем все уникальные узлы
	vector<string> nodeNames;
	for (const auto& nodePair : nodes) {
		const string& from = nodePair.first;
		nodeNames.push_back(from);
	}

	// Перемещаем source в начало и sink в конец
	auto it = find(nodeNames.begin(), nodeNames.end(), source);
	if (it != nodeNames.begin() && it != nodeNames.end()) {
		rotate(nodeNames.begin(), it, it + 1);
	}

	it = find(nodeNames.begin(), nodeNames.end(), sink);
	if (it != nodeNames.end() - 1 && it != nodeNames.end()) {
		iter_swap(nodeNames.end() - 1, it);
	}

	// Создаем маппинг имени узла в индекс
	unordered_map<string, size_t> nodeIndex;
	for (size_t i = 0; i < nodeNames.size(); ++i) {
		nodeIndex[nodeNames[i]] = i;
	}

	// Инициализируем матрицу смежности
	size_t n = nodeNames.size();
	adjacencyMatrix = vector<vector<long long>>(n, vector<long long>(n, 0));

	// Заполняем матрицу
	for (const auto& nodePair : nodes) {
		const string& from = nodePair.first;
		const unordered_map<string, long long>& edges = nodePair.second;

		size_t fromIndex = nodeIndex[from];
		for (const auto& edgePair : edges) {
			const string& to = edgePair.first;
			long long weight = edgePair.second;
			size_t toIndex = nodeIndex[to];
			adjacencyMatrix[fromIndex][toIndex] = weight;
		}
	}
}

/*
распределяем доступное нам количество дуг по ним, но с учётом того, что компоненты должны быть связные
	a. Cтроим случайное остовное дерево от 1 вершины до последней (для связности)
	b. Остальные дуги, которые надо распределить, кладём в массив в виде true, перемешиваем с false-ами (общее кол-во true = макс. возможное кол-во дуг
		 - (минус) кол-во дуг в остовнjv дереве). Теперь,идя по всем дугам по очереди в матрице смежности и одновременно по массиву, я могу расставить
		 дуги случайным образом, не забывая пропускать уже созданные дуги из-за остового дерева
*/
void Graph::TransformToRandomFlowGraph(string name, unsigned int countNodes, float density, unsigned int maxWeightValues) {

	if (countNodes < 2) {
		throw invalid_argument("Flow graph must have at least 2 nodes");
	}

	DeleteAllNodes();
	this->name = name;
	this->weighted = true;
	this->oriented = true;

	// Add nodes
	for (unsigned int i = 0; i < countNodes; i++) {
		AddNode(to_string(i));
	}

	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<long long> weight_dist(1, maxWeightValues);

	// Step 1: Build directed spanning tree (Krusky-like)
	vector<unsigned int> parent(countNodes);
	for (unsigned int i = 0; i < countNodes; i++) {
		parent[i] = i;
	}

	auto find_root = [&parent](unsigned int u) {
		while (parent[u] != u) {
			parent[u] = parent[parent[u]];
			u = parent[u];
		}
		return u;
		};

	vector<pair<unsigned int, unsigned int>> possible_edges;
	for (unsigned int i = 0; i < countNodes; i++) {
		for (unsigned int j = i + 1; j < countNodes; j++) {
			possible_edges.push_back(make_pair(i, j));
		}
	}

	shuffle(possible_edges.begin(), possible_edges.end(), gen);

	unsigned int edges_added = 0;
	for (const auto& edge : possible_edges) {
		unsigned int u = edge.first;
		unsigned int v = edge.second;
		unsigned int root_u = find_root(u);
		unsigned int root_v = find_root(v);

		if (root_u != root_v) {
			AddEdge(to_string(u), to_string(v), weight_dist(gen));
			parent[root_v] = root_u;
			edges_added++;

			if (edges_added == countNodes - 1) break;
		}
	}

	// Step 2: Add random edges
	unsigned int max_possible_edges = countNodes * (countNodes - 1) / 2;
	unsigned int total_edges_needed = static_cast<unsigned int>(max_possible_edges * density);
	unsigned int additional_edges = total_edges_needed - edges_added;

	unsigned int edges_available = possible_edges.size() - edges_added;
	unsigned int edges_to_add = min(additional_edges, edges_available);

	for (unsigned int i = 0, added = 0; added < edges_to_add && i < possible_edges.size(); i++) {
		const auto& edge = possible_edges[i];
		if (!IsEdgeExist(to_string(edge.first), to_string(edge.second))) {
			AddEdge(to_string(edge.first), to_string(edge.second), weight_dist(gen));
			added++;
		}
	}

	// Step 3: Detect sources and sinks
	unordered_set<string> sources;
	unordered_set<string> sinks;
	unordered_map<string, bool> count_incoming;

	// Initialize count_incoming
	for (unsigned int i = 0; i < countNodes; i++) {
		count_incoming[to_string(i)] = false;
	}

	// Find all nodes with incoming edges
	for (const auto& node : nodes) {
		for (const auto& edge : node.second) {
			count_incoming[edge.first] = true;
		}
	}

	// Identify sources and sinks
	for (unsigned int i = 0; i < countNodes; i++) {
		string node = to_string(i);
		if (!count_incoming[node]) {
			sources.insert(node);
		}
		if (nodes[node].empty()) {
			sinks.insert(node);
		}
	}

	// Step 4: Normalize sources and sinks
	string super_source = "source";
	string super_sink = "sink";
	bool need_super_source = sources.size() != 1;
	bool need_super_sink = sinks.size() != 1;

	if (need_super_source) {
		AddNode(super_source);
		for (const auto& src : sources) {
			AddEdge(super_source, src, weight_dist(gen));
		}
	}
	else {
		CopyNode("0", super_source);
		DeleteNode("0");
	}

	if (need_super_sink) {
		AddNode(super_sink);
		for (const auto& snk : sinks) {
			AddEdge(snk, super_sink, weight_dist(gen));
		}
	}
	else {
		CopyNode(to_string(countNodes - 1), super_sink);
		DeleteNode(to_string(countNodes - 1));
	}

	BuildAdjacencyMatrix();
}

void Graph::TransformToDirected() {
	oriented = false;
	unordered_map<string, unordered_map<string, long long>> oldNodes = this->nodes;
	DeleteAllNodes();

	for (auto pair : oldNodes) {
		string nodeFirst = pair.first;
		AddNode(nodeFirst);
	}

	for (auto pair : oldNodes) {
		string nodeFirst = pair.first;
		for (auto edge : pair.second) {
			string nodeSecond = edge.first;
			long long w = edge.second;
			AddEdge(nodeFirst, nodeSecond, w);
		}
	}

	BuildAdjacencyMatrix();
}


bool Graph::IsNodeExist(string node) {
	return nodes.find(node) != nodes.end();
}

bool Graph::IsEdgeExist(string nodeFrom, string nodeWhere) {
	if (IsNodeExist(nodeFrom) && IsNodeExist(nodeWhere)) {
		return nodes[nodeFrom].find(nodeWhere) != nodes[nodeFrom].end();
	}
	return false;
}

bool Graph::IsWeighted() {
	return weighted;
}

bool Graph::IsOriented() {
	return oriented;
}

int Graph::GetCountOfNodes() {
	return nodes.size();
}

string Graph::GetName() {
	return name;
}

void Graph::SetName(string name) {
	this->name = name;
}

long long Graph::GetWeight(string nodeFrom, string nodeWhere) {
	if (IsEdgeExist(nodeFrom, nodeWhere)) {
		return nodes[nodeFrom][nodeWhere];
	}
	return INF;
}

void Graph::AddNode(string node) {
	nodes[node];
}

void Graph::CopyNode(string node, string nameNewNode) {
	if (!IsNodeExist(node)) {
		throw invalid_argument("Source node doesn't exist");
	}
	if (IsNodeExist(nameNewNode)) {
		throw invalid_argument("Node with this name already exists");
	}

	AddNode(nameNewNode);

	// Копируем все исходящие рёбра
	for (auto& edge : nodes[node]) {
		string neighbor = edge.first;
		long long weight = edge.second;

		AddEdge(nameNewNode, neighbor, weight);
	}

	// Копируем все входящие рёбра
	for (auto& graphNode : nodes) {
		string source = graphNode.first;

		// Пропускаем саму вершину и только что созданные рёбра
		if (source == node || source == nameNewNode) continue;

		if (graphNode.second.find(node) != graphNode.second.end()) {
			long long weight = graphNode.second[node];
			AddEdge(source, nameNewNode, weight);
		}
	}
}

void Graph::AddEdge(string nodeFrom, string nodeWhere, long long value) {
	nodes[nodeFrom][nodeWhere] = value;
	if (!oriented) {
		nodes[nodeWhere][nodeFrom] = value;
	}
}

void Graph::DeleteNode(string node) {
	for (auto& vertex : nodes) {
		nodes[vertex.first].erase(node);
	}
	nodes.erase(node);
}

void Graph::DeleteAllNodes() {
	nodes.clear();
}

void Graph::DeleteEdge(string nodeFrom, string nodeWhere) {
	if (!oriented) {
		nodes[nodeWhere].erase(nodeFrom);
	}
	nodes[nodeFrom].erase(nodeWhere);
}

void Graph::DeleteAllEdgeTo(string node) {
	if (oriented) {
		for (auto& vertex : nodes) {
			DeleteEdge(vertex.first, node);
		}
	}
	else {
		DeleteNode(node);
		AddNode(node);
	}
}

void Graph::ClearVisited() {
	visited.clear();
}



long long Graph::FordFulkersonMatrix(string s, string t) {

	size_t n = adjacencyMatrix.size();
	size_t source = 0;
	size_t sink = n - 1;

	vector<vector<long long>> residual(n, vector<long long>(n));
	for (size_t i = 0; i < n; ++i) {
		for (size_t j = 0; j < n; ++j) {
			residual[i][j] = adjacencyMatrix[i][j];
		}
	}

	vector<int> parent(n);
	long long max_flow = 0;

	// Поиск увеличивающего пути с помощью BFS
	auto bfs = [&]() {
		fill(parent.begin(), parent.end(), -1);
		parent[source] = -2;
		queue<pair<size_t, long long>> q;
		q.push({ source, INF });

		while (!q.empty()) {
			size_t u = q.front().first;
			long long flow = q.front().second;
			q.pop();

			for (size_t v = 0; v < n; ++v) {
				if (parent[v] == -1 && residual[u][v] > 0) {
					parent[v] = u;
					long long new_flow = min(flow, residual[u][v]);
					if (v == sink) {
						return new_flow;
					}
					q.push({ v, new_flow });
				}
			}
		}
		return 0LL;
		};

	long long new_flow;
	while ((new_flow = bfs()) != 0) {
		max_flow += new_flow;
		size_t cur = sink;
		while (cur != source) {
			size_t prev = parent[cur];
			residual[prev][cur] -= new_flow;
			residual[cur][prev] += new_flow;
			cur = prev;
		}
	}

	return max_flow;
}

long long Graph::FordFulkerson(string s, string t) {

	// Создаем список всех рёбер (прямых и обратных)
	vector<FlowEdge> edges;
	unordered_map<string, vector<long long>> adj;

	// Добавляем прямые рёбра
	for (auto it = nodes.begin(); it != nodes.end(); ++it) {
		const string& from = it->first;
		const auto& neighbors = it->second;

		for (auto neighbor_it = neighbors.begin(); neighbor_it != neighbors.end(); ++neighbor_it) {
			const string& to = neighbor_it->first;
			long long capacity = neighbor_it->second;

			edges.emplace_back(from, to, capacity);
			adj[from].push_back(edges.size() - 1);

			// Добавляем обратное ребро с нулевой пропускной способностью
			edges.emplace_back(to, from, 0);
			adj[to].push_back(edges.size() - 1);
		}
	}

	long long maxFlow = 0;

	// Функция поиска увеличивающего пути через DFS
	function<long long(string, string, long long, unordered_map<string, long long>&)> dfs =
		[&](string u, string t, long long flow, unordered_map<string, long long>& parent) -> long long {
		visited.insert(u);

		if (u == t) return flow;

		for (long long edgeIndex : adj[u]) {
			FlowEdge& edge = edges[edgeIndex];
			long long residual = edge.capacity - edge.flow;

			if (residual > 0 && !IsNodeVisited(edge.to)) {
				parent[edge.to] = edgeIndex;
				long long minFlow = min(flow, residual);
				long long result = dfs(edge.to, t, minFlow, parent);

				if (result > 0) {
					return result;
				}
			}
		}

		return 0;
		};

	while (true) {
		ClearVisited();
		unordered_map<string, long long> parent;

		long long flow = dfs(s, t, INF, parent);
		if (flow == 0) break;

		maxFlow += flow;

		// Обновляем потоки вдоль пути
		string current = t;
		while (current != s) {
			long long edgeIndex = parent[current];
			edges[edgeIndex].flow += flow;
			edges[edgeIndex ^ 1].flow -= flow; // обратное ребро
			current = edges[edgeIndex].from;
		}
	}

	return maxFlow;
}


long long Graph::FordFulkersonBFS(string s, string t) {
	if (!IsNodeExist(s) || !IsNodeExist(t)) {
		return 0;
	}

	// Создаем остаточную сеть и инициализируем её исходными пропускными способностями
	unordered_map<string, unordered_map<string, long long>> residual;
	for (const auto& node : nodes) {
		for (const auto& edge : node.second) {
			residual[node.first][edge.first] = edge.second;
			// Инициализируем обратные рёбра нулевой пропускной способностью
			if (residual[edge.first].find(node.first) == residual[edge.first].end()) {
				residual[edge.first][node.first] = 0;
			}
		}
	}

	long long max_flow = 0;
	unordered_map<string, string> parent;

	// BFS для поиска увеличивающего пути
	auto bfs = [&]() {
		parent.clear();
		queue<string> q;
		q.push(s);
		parent[s] = ""; // Маркер для истока

		while (!q.empty()) {
			string u = q.front();
			q.pop();

			for (const auto& edge : residual[u]) {
				string v = edge.first;
				long long capacity = edge.second;

				// Если вершина ещё не посещена и есть остаточная пропускная способность
				if (parent.find(v) == parent.end() && capacity > 0) {
					parent[v] = u;
					if (v == t) {
						return true; // Нашли путь до стока
					}
					q.push(v);
				}
			}
		}
		return false; // Путь до стока не найден
		};

	// Пока существует увеличивающий путь
	while (bfs()) {
		// Находим минимальную остаточную пропускную способность на пути
		long long path_flow = INF;
		for (string v = t; v != s; v = parent[v]) {
			string u = parent[v];
			path_flow = min(path_flow, residual[u][v]);
		}

		// Обновляем остаточные пропускные способности
		for (string v = t; v != s; v = parent[v]) {
			string u = parent[v];
			residual[u][v] -= path_flow;
			residual[v][u] += path_flow;
		}

		max_flow += path_flow;
	}

	return max_flow;
}

//long long Graph::FordFulkersonCheck(string s, string t) {
//	ClearVisited();
//
//	vector<FlowEdge> e;
//	unordered_map<string, vector<long long>> indexes;
//
//	// получение всех дуг
//	for (auto& node : nodes) {
//		string from = node.first;
//		for (auto& edge : node.second) {
//			string to = edge.first;
//			//прямая дуга
//			indexes[from].push_back(e.size());
//			e.emplace_back(from, to, edge.second, 0);
//			// и обратная
//			indexes[to].push_back(e.size());
//			e.emplace_back(to, from, 0, 0);
//		}
//	}
//
//	long long ans = 0;
//	long long flow = -1;
//	while (flow != 0) {
//		flow = DfsForFlow(s, INF, t, e, indexes);
//		ans += flow;
//		cout << " Размер: " << flow << '\n';
//		ClearVisited();
//	}
//
//	return ans;
//}

long long Graph::FordFulkersonScalingApproximate(string s, string t, double eps) {
	if (!IsNodeExist(s) || !IsNodeExist(t)) return 0;

	// 1. Находим максимальную конечную пропускную способность (исключая INF)
	long long max_cap = 0;
	for (const auto& u : nodes) {
		for (const auto& edge : u.second) {
			if (edge.second != INF && edge.second > max_cap) {
				max_cap = edge.second;
			}
		}
	}

	// Если все рёбра имеют INF пропускную способность
	if (max_cap == 0) max_cap = INF;

	// 2. Инициализация delta
	long long delta = 1;
	while (delta * 2 <= max_cap) {
		delta *= 2;
	}

	// 3. Создаём остаточную сеть
	unordered_map<string, unordered_map<string, long long>> residual;
	for (const auto& u : nodes) {
		for (const auto& edge : u.second) {
			residual[u.first][edge.first] = edge.second;
		}
	}

	long long max_flow = 0;

	// 4. BFS с delta-ограничением
	auto bfs = [&]() {
		unordered_map<string, string> parent;
		queue<string> q;
		q.push(s);
		parent[s] = "";

		while (!q.empty()) {
			string u = q.front();
			q.pop();

			for (const auto& edge : residual[u]) {
				string v = edge.first;
				long long capacity = edge.second;
				if (parent.find(v) == parent.end() && capacity >= delta) {
					parent[v] = u;
					q.push(v);
					if (v == t) return parent;
				}
			}
		}
		return parent;
		};

	// 5. Основной цикл масштабирования
	while (delta >= 1) {
		// Условие останова для приближённого алгоритма
		if (max_cap != INF && delta < eps * max_cap) break;

		auto parent = bfs();
		while (parent.find(t) != parent.end()) {
			// Находим минимальный остаток на пути
			long long path_flow = INF;
			for (string v = t; v != s; v = parent[v]) {
				string u = parent[v];
				path_flow = min(path_flow, residual[u][v]);
			}

			// Обновляем остаточную сеть
			for (string v = t; v != s; v = parent[v]) {
				string u = parent[v];
				residual[u][v] -= path_flow;
				// Добавляем обратное ребро, если его нет
				if (residual[v].find(u) == residual[v].end()) {
					residual[v][u] = 0;
				}
				residual[v][u] += path_flow;
			}

			max_flow += path_flow;
			parent = bfs();
		}
		delta /= 2;
	}

	return max_flow;
}


long long Graph::PushRelabelApproximate(string s, string t, int max_pushes) {

	// 1. Инициализация
	unordered_map<string, long long> height, excess;
	unordered_map<string, unordered_map<string, long long>> residual;

	// Копируем пропускные способности в остаточную сеть
	for (const auto& u : nodes) {
		height[u.first] = 0;
		excess[u.first] = 0;
		for (const auto& edge : u.second) {
			residual[u.first][edge.first] = edge.second;
		}
	}

	// Высота истока = |V|, избыток = INF
	height[s] = nodes.size();
	excess[s] = INF;

	// 2. Начальное проталкивание из истока
	for (auto& edge : nodes[s]) {
		string v = edge.first;
		long long flow = residual[s][v]; // Можно протолкнуть весь доступный поток
		if (flow > 0) {
			residual[s][v] -= flow;
			residual[v][s] += flow; // Добавляем обратное ребро
			excess[s] -= flow;
			excess[v] += flow;
		}
	}

	// 3. Основной цикл
	int pushes = 0;
	bool updated;
	do {
		updated = false;
		for (const auto& u_node : nodes) {
			string u = u_node.first;
			if (u == s || u == t || excess[u] == 0) continue;

			// 3.1. Пытаемся протолкнуть поток
			for (auto& edge : residual[u]) {
				string v = edge.first;
				if (height[u] > height[v] && edge.second > 0) {
					long long flow = min(excess[u], edge.second);
					residual[u][v] -= flow;
					residual[v][u] += flow;
					excess[u] -= flow;
					excess[v] += flow;
					updated = true;
					pushes++;

					if (excess[u] == 0) break; // Избыток исчерпан
				}
			}

			// 3.2. Если избыток остался — поднимаем вершину
			if (excess[u] > 0) {
				long long min_height = INF;
				for (const auto& edge : residual[u]) {
					if (edge.second > 0) {
						min_height = min(min_height, height[edge.first]);
					}
				}
				if (min_height != INF) {
					height[u] = min_height + 1;
					updated = true;
				}
			}
		}
	} while (updated && pushes < max_pushes);

	// 4. Возвращаем поток, вошедший в сток
	return excess[t];
}

long long Graph::PushRelabelMatrix(string s, string t, int max_pushes) {
	// 1. Построение индексов
	size_t n = adjacencyMatrix.size();
	size_t source = 0;
	size_t sink = n - 1;

	// 2. Инициализация
	vector<long long> height(n, 0), excess(n, 0);
	vector<vector<long long>> residual = adjacencyMatrix;  // Остаточная сеть

	height[source] = n;
	excess[source] = INF;

	// 3. Начальное проталкивание из истока
	for (size_t v = 0; v < n; ++v) {
		if (residual[source][v] > 0) {
			long long flow = residual[source][v];
			residual[source][v] -= flow;
			residual[v][source] += flow;
			excess[source] -= flow;
			excess[v] += flow;
		}
	}

	// 4. Основной цикл
	int pushes = 0;
	bool updated;
	do {
		updated = false;
		for (size_t u = 0; u < n; ++u) {
			if (u == source || u == sink || excess[u] == 0) continue;

			// 4.1. Проталкивание потока
			for (size_t v = 0; v < n; ++v) {
				if (height[u] > height[v] && residual[u][v] > 0) {
					long long flow = min(excess[u], residual[u][v]);
					residual[u][v] -= flow;
					residual[v][u] += flow;
					excess[u] -= flow;
					excess[v] += flow;
					updated = true;
					pushes++;

					if (excess[u] == 0) break;
				}
			}

			// 4.2. Подъём вершины
			if (excess[u] > 0) {
				long long min_height = INF;
				for (size_t v = 0; v < n; ++v) {
					if (residual[u][v] > 0) {
						min_height = min(min_height, height[v]);
					}
				}
				if (min_height != INF) {
					height[u] = min_height + 1;
					updated = true;
				}
			}
		}
	} while (updated && pushes < max_pushes);

	return excess[sink];
}

long long Graph::PushRelabelParallel(string s, string t, int max_pushes) {
	if (!IsNodeExist(s) || !IsNodeExist(t)) {
		throw invalid_argument("Source or sink node doesn't exist");
	}

	// 1. Инициализация
	unordered_map<string, std::atomic<long long>> height, excess;
	unordered_map<string, unordered_map<string, std::atomic<long long>>> residual;

	// Копируем пропускные способности в остаточную сеть
	for (const auto& u : nodes) {
		height[u.first].store(0);
		excess[u.first].store(0);
		for (const auto& edge : u.second) {
			residual[u.first][edge.first].store(edge.second);
		}
	}

	// Высота истока = |V|, избыток = INF
	height[s].store(nodes.size());
	excess[s].store(INF);

	// 2. Начальное проталкивание из истока (последовательно)
	for (auto& edge : nodes[s]) {
		string v = edge.first;
		long long flow = residual[s][v].load();
		if (flow > 0) {
			residual[s][v] -= flow;
			residual[v][s] += flow;
			excess[s] -= flow;
			excess[v] += flow;
		}
	}

	// 3. Основной цикл (параллельный)
	int pushes = 0;
	bool updated;
	do {
		updated = false;
		vector<string> active_nodes;

		// Собираем активные вершины
		for (const auto& u_node : nodes) {
			string u = u_node.first;
			if (u != s && u != t && excess[u].load() > 0) {
				active_nodes.push_back(u);
			}
		}

		// Параллельная обработка активных вершин
#pragma omp parallel for shared(updated)
		for (size_t i = 0; i < active_nodes.size(); ++i) {
			string u = active_nodes[i];
			long long curr_excess = excess[u].load();
			if (curr_excess <= 0) continue;

			// 3.1. Пытаемся протолкнуть поток
			for (auto& edge : nodes[u]) {
				string v = edge.first;
				long long cap = residual[u][v].load();
				if (height[u].load() > height[v].load() && cap > 0) {
					long long flow = min(curr_excess, cap);

					// Атомарное обновление
					residual[u][v] -= flow;
					residual[v][u] += flow;
					excess[u] -= flow;
					excess[v] += flow;

#pragma omp atomic write
					updated = true;
					pushes++;

					curr_excess = excess[u].load();
					if (curr_excess == 0) break;
				}
			}

			// 3.2. Подъём вершины, если избыток остался
			if (curr_excess > 0) {
				long long min_height = INF;
				for (auto& edge : nodes[u]) {
					string v = edge.first;
					if (residual[u][v].load() > 0) {
						min_height = min(min_height, height[v].load());
					}
				}
				if (min_height != INF) {
					height[u].store(min_height + 1);
#pragma omp atomic write
					updated = true;
				}
			}
		}
	} while (updated && pushes < max_pushes);

	return excess[t].load();
}


long long Graph::DinicMaxFlow(string s, string t) {

	// Структура для хранения рёбер
	struct Edge {
		string to;
		long long capacity;
		long long flow;
		size_t reverse_edge;
	};

	unordered_map<string, vector<Edge>> adj;
	unordered_map<string, size_t> level;

	// Построение остаточной сети
	for (const auto& u : nodes) {
		for (const auto& edge : u.second) {
			string v = edge.first;
			long long cap = edge.second;

			// Прямое ребро
			Edge forward = { v, cap, 0, adj[v].size() };
			// Обратное ребро (для остаточной сети)
			Edge backward = { u.first, 0, 0, adj[u.first].size() };

			adj[u.first].push_back(forward);
			adj[v].push_back(backward);
		}
	}

	long long max_flow = 0;

	// BFS для построения слоистой сети
	auto bfs = [&]() {
		level.clear();
		queue<string> q;
		q.push(s);
		level[s] = 1;

		while (!q.empty()) {
			string u = q.front();
			q.pop();

			for (const Edge& e : adj[u]) {
				if (!level.count(e.to) && e.flow < e.capacity) {
					level[e.to] = level[u] + 1;
					q.push(e.to);
				}
			}
		}
		return level.count(t);
		};

	// DFS для поиска блокирующего потока
	function<long long(string, long long)> dfs = [&](string u, long long flow) {
		if (u == t) {
			return flow;
		}

		for (size_t i = 0; i < adj[u].size(); ++i) {
			Edge& e = adj[u][i];
			if (level[e.to] == level[u] + 1 && e.flow < e.capacity) {
				long long min_flow = min(flow, e.capacity - e.flow);
				long long pushed = dfs(e.to, min_flow);

				if (pushed > 0) {
					e.flow += pushed;
					adj[e.to][e.reverse_edge].flow -= pushed;
					return pushed;
				}
			}
		}
		return 0LL;
		};

	// Основной цикл
	while (bfs()) {
		long long pushed;
		while ((pushed = dfs(s, INF)) > 0) {
			max_flow += pushed;
		}
	}

	return max_flow;
}

long long Graph::DinicMaxFlowMatrix(string s, string t) {
	size_t n = adjacencyMatrix.size();
	size_t source = 0;
	size_t sink = n - 1;

	vector<vector<long long>> residual(n, vector<long long>(n));
	for (size_t i = 0; i < n; ++i) {
		for (size_t j = 0; j < n; ++j) {
			residual[i][j] = adjacencyMatrix[i][j];
		}
	}

	vector<size_t> level(n);
	long long max_flow = 0;

	// BFS для построения слоистой сети
	auto bfs = [&]() {
		fill(level.begin(), level.end(), 0);
		queue<size_t> q;
		q.push(source);
		level[source] = 1;

		while (!q.empty()) {
			size_t u = q.front();
			q.pop();

			for (size_t v = 0; v < n; ++v) {
				if (!level[v] && residual[u][v] > 0) {
					level[v] = level[u] + 1;
					q.push(v);
				}
			}
		}
		return level[sink] != 0;
		};

	// DFS для поиска блокирующего потока
	function<long long(size_t, long long)> dfs = [&](size_t u, long long flow) {
		if (u == sink) {
			return flow;
		}

		for (size_t v = 0; v < n; ++v) {
			if (level[v] == level[u] + 1 && residual[u][v] > 0) {
				long long min_flow = min(flow, residual[u][v]);
				long long pushed = dfs(v, min_flow);

				if (pushed > 0) {
					residual[u][v] -= pushed;
					residual[v][u] += pushed;
					return pushed;
				}
			}
		}
		return 0LL;
		};

	// Основной цикл
	while (bfs()) {
		long long pushed;
		while ((pushed = dfs(source, INF)) > 0) {
			max_flow += pushed;
		}
	}

	return max_flow;
}

// Метод для вычисления максимального потока алгоритмом Push-Relabel
long long Graph::getMaxFlowPushRelabel(int source, int sink) {

	int n = adjacencyMatrix.size();
	vector<vector<long long>> flow(n, vector<long long>(n, 0));
	vector<long long> height(n, 0);
	vector<long long> excess(n, 0);

	// Инициализация
	height[source] = n; // Высота истока = количество вершин

	// Начальное насыщение рёбер из истока
	for (int v = 0; v < n; v++) {
		if (adjacencyMatrix[source][v] > 0) {
			flow[source][v] = adjacencyMatrix[source][v];
			flow[v][source] = -flow[source][v];
			excess[v] = flow[source][v];
			excess[source] -= flow[source][v];
		}
	}

	// Основной цикл алгоритма
	while (true) {
		bool foundActive = false;

		// Ищем активную вершину
		for (int u = 0; u < n; u++) {
			if (!(excess[u] > 0 && u != source && u != sink)) continue;

			foundActive = true;
			bool pushed = false;

			// Пытаемся протолкнуть поток в допустимое ребро
			for (int v = 0; v < n; v++) {
				if (adjacencyMatrix[u][v] - flow[u][v] > 0 && height[u] == height[v] + 1) {

					//операция push
					long long delta = min(excess[u], adjacencyMatrix[u][v] - flow[u][v]);
					flow[u][v] += delta;
					flow[v][u] -= delta; // Учитываем обратное ребро
					excess[u] -= delta;
					excess[v] += delta;


					pushed = true;
					break;
				}
			}

			// Если не удалось протолкнуть - поднимаем вершину (relabel)
			if (!pushed) {
				long long min_height = LLONG_MAX;
				// Находим минимальную высоту среди соседей с остаточной пропускной способностью
				for (int v = 0; v < n; v++) {
					if (adjacencyMatrix[u][v] - flow[u][v] > 0) {
						min_height = min(min_height, height[v]);
					}
				}
				if (min_height != LLONG_MAX) {
					height[u] = min_height + 1;
				}
			}
		}

		// Если не осталось активных вершин - завершаем
		if (!foundActive) break;
	}

	// Возвращаем значение максимального потока
	return excess[sink];
}

// Метод для вычисления максимального потока алгоритмом Push-Relabel c Highest Label First (HLF)
long long Graph::getMaxFlowPushRelabel_v2(int source, int sink) {
	int n = adjacencyMatrix.size();
	vector<vector<long long>> flow(n, vector<long long>(n, 0));
	vector<long long> height(n, 0);
	vector<long long> excess(n, 0);
	vector<vector<int>> buckets(2 * n);
	long long max_height = 0;

	// Инициализация
	height[source] = n;

	// Начальное насыщение рёбер из истока
	for (int v = 0; v < n; v++) {
		if (adjacencyMatrix[source][v] > 0) {
			flow[source][v] = adjacencyMatrix[source][v];
			flow[v][source] = -flow[source][v];
			excess[v] = flow[source][v];
			excess[source] -= flow[source][v];
			if (v != sink && v != source) {
				buckets[height[v]].push_back(v);
				max_height = max(max_height, height[v]);
			}
		}
	}

	while (max_height >= 0) {
		if (buckets[max_height].empty()) {
			max_height--;
			continue;
		}

		int u = buckets[max_height].back();
		buckets[max_height].pop_back();

		// Пытаемся протолкнуть поток
		bool pushed = false;
		for (int v = 0; v < n && excess[u] > 0; v++) {
			if (adjacencyMatrix[u][v] - flow[u][v] > 0 && height[u] == height[v] + 1) {
				long long delta = min(excess[u], adjacencyMatrix[u][v] - flow[u][v]);
				flow[u][v] += delta;
				flow[v][u] -= delta;
				excess[u] -= delta;
				excess[v] += delta;

				if (excess[v] == delta && v != source && v != sink) {
					buckets[height[v]].push_back(v);
					max_height = max(max_height, height[v]);
				}
				pushed = true;
			}
		}

		// Если не удалось протолкнуть - поднимаем вершину
		if (excess[u] > 0) {
			long long min_height = LLONG_MAX;
			for (int v = 0; v < n; v++) {
				if (adjacencyMatrix[u][v] - flow[u][v] > 0) {
					min_height = min(min_height, height[v]);
				}
			}
			if (min_height != LLONG_MAX) {
				height[u] = min_height + 1;
				if (height[u] < n) {  // Не добавляем вершины с высотой >= n
					buckets[height[u]].push_back(u);
					max_height = max(max_height, height[u]);
				}
			}
		}
	}

	return excess[sink];
}



void globalRelabel(const vector<vector<long long>>& capacity,
	const vector<vector<long long>>& flow,
	vector<long long>& height, int sink) {
	int n = height.size();
	fill(height.begin(), height.end(), n);
	height[sink] = 0;
	queue<int> q;
	q.push(sink);

	while (!q.empty()) {
		int u = q.front();
		q.pop();

		for (int v = 0; v < n; v++) {
			if (capacity[v][u] - flow[v][u] > 0 && height[v] == n) {
				height[v] = height[u] + 1;
				q.push(v);
			}
		}
	}
}

// Метод для вычисления максимального потока алгоритмом Push-Relabel c Highest Label First (HLF) и globalRelabel
long long Graph::getMaxFlowPushRelabel_v3(int source, int sink) {
	int n = adjacencyMatrix.size();
	vector<vector<long long>> flow(n, vector<long long>(n, 0));
	vector<long long> height(n, 0);
	vector<long long> excess(n, 0);
	vector<vector<int>> buckets(2 * n);
	long long max_height = 0;
	int relabel_counter = 0;

	// Инициализация
	height[source] = n;

	// Начальное насыщение рёбер из истока
	for (int v = 0; v < n; v++) {
		if (adjacencyMatrix[source][v] > 0) {
			flow[source][v] = adjacencyMatrix[source][v];
			flow[v][source] = -flow[source][v];
			excess[v] = flow[source][v];
			excess[source] -= flow[source][v];
			if (v != sink && v != source) {
				buckets[height[v]].push_back(v);
				max_height = max(max_height, height[v]);
			}
		}
	}

	while (max_height >= 0) {
		if (buckets[max_height].empty()) {
			max_height--;
			continue;
		}

		int u = buckets[max_height].back();
		buckets[max_height].pop_back();

		// Пытаемся протолкнуть поток
		bool pushed = false;
		for (int v = 0; v < n && excess[u] > 0; v++) {
			if (adjacencyMatrix[u][v] - flow[u][v] > 0 && height[u] == height[v] + 1) {
				long long delta = min(excess[u], adjacencyMatrix[u][v] - flow[u][v]);
				flow[u][v] += delta;
				flow[v][u] -= delta;
				excess[u] -= delta;
				excess[v] += delta;

				if (excess[v] == delta && v != source && v != sink) {
					buckets[height[v]].push_back(v);
					max_height = max(max_height, height[v]);
				}
				pushed = true;
			}
		}

		// Если не удалось протолкнуть - поднимаем вершину
		if (excess[u] > 0) {
			long long min_height = LLONG_MAX;
			for (int v = 0; v < n; v++) {
				if (adjacencyMatrix[u][v] - flow[u][v] > 0) {
					min_height = min(min_height, height[v]);
				}
			}
			if (min_height != LLONG_MAX) {
				height[u] = min_height + 1;
				relabel_counter++;

				// Периодически выполняем глобальное перемаркирование
				if (relabel_counter >= n) {
					globalRelabel(adjacencyMatrix, flow, height, sink);
					relabel_counter = 0;
					max_height = 0;
					for (int v = 0; v < n; v++) {
						if (excess[v] > 0 && v != source && v != sink) {
							if (height[v] < n) {
								buckets[height[v]].push_back(v);
								max_height = max(max_height, height[v]);
							}
						}
					}
					continue;
				}

				if (height[u] < n) {
					buckets[height[u]].push_back(u);
					max_height = max(max_height, height[u]);
				}
			}
		}
	}

	return excess[sink];
}





// Приближённый алгоритм
// Реализация методов MaxFlow-WO

// Реализация исправленных методов

vector<Graph::Cluster> Graph::formClusters(const string& source, const string& sink) {
	vector<Cluster> clusters;
	vector<string> nodes_list;

	// Собираем все узлы кроме стока
	for (const auto& node : nodes) {
		if (node.first != sink) {
			nodes_list.push_back(node.first);
		}
	}

	// Количество кластеров - корень из числа узлов (минимум 2)
	int k = max(2, (int)sqrt(nodes_list.size()));
	k = min(k, (int)nodes_list.size());

	// Гарантируем, что источник будет центром одного из кластеров
	Cluster source_cluster;
	source_cluster.center = source;
	source_cluster.nodes.insert(source);
	clusters.push_back(source_cluster);

	// Выбираем случайные центры для остальных кластеров
	random_shuffle(nodes_list.begin(), nodes_list.end());
	for (int i = 0; i < k - 1 && i < nodes_list.size(); ++i) {
		if (nodes_list[i] == source) continue;

		Cluster cluster;
		cluster.center = nodes_list[i];
		cluster.nodes.insert(nodes_list[i]);
		clusters.push_back(cluster);
	}

	return clusters;
}

void Graph::assignNodesToClusters(vector<Cluster>& clusters, const string& sink) {
	// Распределяем узлы по ближайшим кластерам
	for (const auto& node : nodes) {
		if (node.first == sink) continue;

		// Проверяем, не является ли узел уже центром кластера
		bool is_center = false;
		for (const auto& cluster : clusters) {
			if (node.first == cluster.center) {
				is_center = true;
				break;
			}
		}
		if (is_center) continue;

		// Находим ближайший кластер
		int nearest_cluster = 0;
		long long min_distance = numeric_limits<long long>::max();

		for (int i = 0; i < clusters.size(); ++i) {
			if (nodes.at(node.first).count(clusters[i].center)) {
				long long dist = nodes.at(node.first).at(clusters[i].center);
				if (dist < min_distance) {
					min_distance = dist;
					nearest_cluster = i;
				}
			}
		}

		if (min_distance != numeric_limits<long long>::max()) {
			clusters[nearest_cluster].nodes.insert(node.first);
		}
	}

	// Гарантируем, что сток есть в каждом кластере
	for (auto& cluster : clusters) {
		cluster.nodes.insert(sink);
	}
}

void Graph::findConnectingEdges(vector<Cluster>& clusters) {
	// Находим все ребра между узлами каждого кластера
	for (auto& cluster : clusters) {
		for (const auto& node : cluster.nodes) {
			for (const auto& edge : nodes.at(node)) {
				if (cluster.nodes.count(edge.first)) {
					cluster.edges.emplace_back(node, edge.first);
				}
			}
		}
	}
}

long long Graph::calculateLocalFlow(Cluster& cluster, const string& source, const string& sink) {
	// Проверяем, есть ли источник и сток в кластере
	if (!cluster.nodes.count(source) || !cluster.nodes.count(sink)) {
		return 0;
	}

	// Создаем временный подграф для этого кластера
	Graph temp_graph("temp", true, true);

	// Добавляем узлы
	for (const auto& node : cluster.nodes) {
		temp_graph.AddNode(node);
	}

	// Добавляем ребра с их пропускными способностями
	for (const auto& edge : cluster.edges) {
		long long capacity = nodes.at(edge.first).at(edge.second);
		temp_graph.AddEdge(edge.first, edge.second, capacity);
	}

	// Вычисляем максимальный поток в подграфе
	return temp_graph.DinicMaxFlow(source, sink);
}

long long Graph::MaxFlowWO(const string& source, const string& sink, int max_iterations) {
	if (!IsNodeExist(source) || !IsNodeExist(sink)) {
		return 0;
	}

	long long max_flow = 0;

	for (int iter = 0; iter < max_iterations; ++iter) {
		// 1. Формируем кластеры
		vector<Cluster> clusters = formClusters(source, sink);

		// 2. Распределяем узлы по кластерам
		assignNodesToClusters(clusters, sink);

		// 3. Находим ребра внутри каждого кластера
		findConnectingEdges(clusters);

		// 4. Вычисляем локальные потоки и находим максимальный
		long long current_max = 0;
		for (auto& cluster : clusters) {
			cluster.local_flow = calculateLocalFlow(cluster, source, sink);
			if (cluster.local_flow > current_max) {
				current_max = cluster.local_flow;
			}
		}

		if (current_max > max_flow) {
			max_flow = current_max;
		}
	}

	return max_flow;
}







// Алгоритм Диница
// Вспомогательный метод BFS для построения слоистой сети
bool Graph::bfs(int s, int t, vector<int>& level) {
	fill(level.begin(), level.end(), -1);
	level[s] = 0;

	queue<int> q;
	q.push(s);

	while (!q.empty()) {
		int u = q.front();
		q.pop();

		for (int v = 0; v < adjacencyMatrix.size(); ++v) {
			if (adjacencyMatrix[u][v] > 0 && level[v] == -1) {
				level[v] = level[u] + 1;
				q.push(v);
			}
		}
	}

	return level[t] != -1;
}

// Вспомогательный метод DFS для поиска блокирующего потока
long long Graph::dfs(int u, int t, long long flow, vector<int>& ptr, vector<int>& level) {
	if (u == t || flow == 0)
		return flow;

	for (int& v = ptr[u]; v < adjacencyMatrix.size(); ++v) {
		if (level[v] == level[u] + 1 && adjacencyMatrix[u][v] > 0) {
			long long pushed = dfs(v, t, min(flow, adjacencyMatrix[u][v]), ptr, level);
			if (pushed > 0) {
				adjacencyMatrix[u][v] -= pushed;
				adjacencyMatrix[v][u] += pushed;
				return pushed;
			}
		}
	}

	return 0;
}

// Основной метод алгоритма Диница
long long Graph::getMaxFlowDinic(int source, int sink) {
	if (source == sink)
		return 0;

	long long maxFlow = 0;
	vector<int> level(adjacencyMatrix.size());
	vector<int> ptr(adjacencyMatrix.size());

	while (bfs(source, sink, level)) {
		fill(ptr.begin(), ptr.end(), 0);

		while (long long pushed = dfs(source, sink, INF, ptr, level)) {
			maxFlow += pushed;
		}
	}

	return maxFlow;
}


long long Graph::getMaxFlowDinicScaling(int source, int sink) {
	long long maxFlow = 0;
	long long maxCap = 0;
	for (size_t u = 0; u < adjacencyMatrix.size(); u++) {
		for (size_t v = 0; u < adjacencyMatrix.size(); u++) {
			if (u != v && adjacencyMatrix[u][v] != INF && adjacencyMatrix[u][v] > maxCap) {
				maxCap = adjacencyMatrix[u][v];
			}
		}
	}
	vector<int> level(adjacencyMatrix.size());
	vector<int> ptr(adjacencyMatrix.size());

	long long delta = 1;
	while (delta <= maxCap) delta <<= 1;

	for (; delta > 0; delta >>= 1) {
		while (bfs(source, sink, level)) {
			fill(ptr.begin(), ptr.end(), 0);
			while (long long pushed = dfs(source, sink, delta, ptr, level)) {
				maxFlow += pushed;
			}
		}
	}
	return maxFlow;
}



long long Graph::FordFulkersonScaling(string s, string t, double eps) {
	// Проверка на существование узлов
	if (!IsNodeExist(s) || !IsNodeExist(t)) {
		return -1;
	}

	// Инициализация остаточных пропускных способностей
	long long max_cap = 0;
	unordered_map<string, unordered_map<string, long long>> residual;
	for (auto& u : nodes) {
		for (auto& v : u.second) {
			residual[u.first][v.first] = v.second;
			if (v.second != INF && v.second > max_cap) {
				max_cap = v.second;
			}
		}
	}

	long long maxFlow = 0;
	long long delta = (long long)1 << 60; // Начинаем с большой степени двойки

	// Пока delta больше eps * максимального веса ребра
	while (delta >= eps * max_cap) {
		// Поиск увеличивающего пути с остаточной способностью >= delta
		unordered_map<string, string> parent;
		queue<string> q;
		q.push(s);
		parent[s] = "";

		while (!q.empty()) {
			string u = q.front();
			q.pop();

			for (auto& v : nodes[u]) {
				if (!parent.count(v.first) && residual[u][v.first] >= delta) {
					parent[v.first] = u;
					q.push(v.first);
					if (v.first == t) break;
				}
			}
		}

		// Если путь найден
		if (parent.count(t)) {
			// Находим минимальную остаточную способность на пути
			long long pathFlow = INF;
			for (string v = t; v != s; v = parent[v]) {
				string u = parent[v];
				pathFlow = min(pathFlow, residual[u][v]);
			}

			// Обновляем остаточные способности
			for (string v = t; v != s; v = parent[v]) {
				string u = parent[v];
				residual[u][v] -= pathFlow;
				residual[v][u] += pathFlow;
			}

			maxFlow += pathFlow;
		}
		else {
			delta /= 2; // Уменьшаем delta если путь не найден
		}
	}

	return maxFlow;
}



ostream& operator<< (std::ostream& out, const Graph& graph) {
	out << graph.name << endl;

	const auto& matrix = graph.adjacencyMatrix;

	if (matrix.size() > 13) {
		out << "Большая матрица\n";
		return out;
	}

	// Выводим заголовок с индексами узлов
	out << setw(6) << " ";
	for (size_t i = 0; i < matrix.size(); ++i) {
		out << setw(6) << i;
	}
	out << endl;

	// Выводим саму матрицу
	for (size_t i = 0; i < matrix.size(); ++i) {
		out << setw(6) << i;
		for (size_t j = 0; j < matrix[i].size(); ++j) {
			out << setw(6) << matrix[i][j];
		}
		out << endl;
	}

	return out;
}



Graph::~Graph() {

}