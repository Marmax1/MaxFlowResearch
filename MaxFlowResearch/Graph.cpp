#include "Graph.h"
#include <numeric>
#include <functional>

bool Graph::IsNodeVisited(int node) {
	if (visited.find(node) == visited.end())
		return false;
	return true;
}

//long long Graph::DFSWithDelta(string u, string t, long long delta, unordered_map<string, string>& parent) {
//	visited.insert(u);
//
//	if (u == t) {
//		return delta; // Нашли путь до стока
//	}
//
//	for (auto& edge : nodes[u]) {
//		string v = edge.first;
//		long long capacity = edge.second;
//
//		if (!IsNodeVisited(v) && capacity >= delta) {
//			parent[v] = u;
//			long long min_flow = DFSWithDelta(v, t, delta, parent);
//
//			if (min_flow > 0) {
//				// Обновляем пропускные способности
//				nodes[u][v] -= min_flow;
//				if (!oriented) {
//					nodes[v][u] += min_flow;
//				}
//				else {
//					// Для ориентированного графа добавляем обратное ребро
//					if (nodes[v].find(u) == nodes[v].end()) {
//						nodes[v][u] = 0;
//					}
//					nodes[v][u] += min_flow;
//				}
//				return min_flow;
//			}
//		}
//	}
//	return 0;
//}




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
	adj = gr.adj;
	initGargKonemann();
}

//void Graph::BuildAdjacencyMatrix() {
//
//	// Собираем все уникальные узлы
//	vector<int> nodeNames;
//	for (const auto& nodePair : nodes) {
//		const int& from = nodePair.first;
//		nodeNames.push_back(from);
//	}
//
//	// Перемещаем source в начало и sink в конец
//	auto it = find(nodeNames.begin(), nodeNames.end(), source);
//	if (it != nodeNames.begin() && it != nodeNames.end()) {
//		rotate(nodeNames.begin(), it, it + 1);
//	}
//
//	it = find(nodeNames.begin(), nodeNames.end(), sink);
//	if (it != nodeNames.end() - 1 && it != nodeNames.end()) {
//		iter_swap(nodeNames.end() - 1, it);
//	}
//
//	// Создаем маппинг имени узла в индекс
//	unordered_map<string, size_t> nodeIndex;
//	for (size_t i = 0; i < nodeNames.size(); ++i) {
//		nodeIndex[nodeNames[i]] = i;
//	}
//
//	// Инициализируем матрицу смежности
//	size_t n = nodeNames.size();
//	adjacencyMatrix = vector<vector<long long>>(n, vector<long long>(n, 0));
//
//	// Заполняем матрицу
//	for (const auto& nodePair : nodes) {
//		const string& from = nodePair.first;
//		const unordered_map<string, long long>& edges = nodePair.second;
//
//		size_t fromIndex = nodeIndex[from];
//		for (const auto& edgePair : edges) {
//			const string& to = edgePair.first;
//			long long weight = edgePair.second;
//			size_t toIndex = nodeIndex[to];
//			adjacencyMatrix[fromIndex][toIndex] = weight;
//		}
//	}
//}

void Graph::BuildAdjEdgesFromMatrix() {
	int n = adjacencyMatrix.size();
	adj.resize(n);

	// Для каждого направленного ребра в матрице
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			long long capacity = adjacencyMatrix[i][j];

			if (capacity != 0) {
				// Проверяем, не создаем ли мы петлю (цикл из вершины в себя)
				if (i == j) {
					continue;
				}

				// Проверяем существование обратного ребра (j -> i)
				bool reverseExists = false;
				for (Edge& e : adj[i]) {
					if (e.to == j) {
						// Ребро уже существует - обновляем пропускную способность
						e.cap += capacity;
						reverseExists = true;
						break;
					}
				}

				if (!reverseExists) {
					// Создаем новое ребро
					Edge forward(j, adj[j].size(), capacity);
					Edge backward(i, adj[i].size(), 0);

					adj[i].push_back(forward);
					adj[j].push_back(backward);
				}
			}
		}
	}
}

void Graph::BuildAdjacencyListFromMatrix() {
	int n = adjacencyMatrix.size();

	nodes.reserve(n);

	for (int i = 0; i < n; ++i) {
		std::unordered_map<int, long long> edges;

		for (int j = 0; j < n; ++j) {
			if (adjacencyMatrix[i][j] != 0) {
				edges[j] = adjacencyMatrix[i][j];
			}
		}

		// ВСЕГДА добавляем вершину, даже если edges пустой
		nodes[i] = std::move(edges);
	}
}

/*
Распределяем доступное нам количество дуг по ним, но с учётом того, что компоненты должны быть связные
	a. Cтроим случайное остовное дерево от 1 вершины до последней (для связности)
	b. Остальные дуги, которые надо распределить, кладём в массив в виде true, перемешиваем с false-ами (общее кол-во true = макс. возможное кол-во дуг
		 - (минус) кол-во дуг в остовном дереве). Теперь,идя по всем дугам по очереди в матрице смежности и одновременно по массиву, я могу расставить
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

	//// Создаём матрицу смежности
	adjacencyMatrix = vector<vector<long long>>(countNodes, vector<long long>(countNodes, 0));

	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<long long> weight_dist(1, maxWeightValues);

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

	// Создание массива со случайно расположенными дугами
	
	vector<pair<unsigned int, unsigned int>> possible_edges;
	possible_edges.reserve(countNodes * (countNodes - 1) / 2); // резервирование памяти под кол-во дуг
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
			adjacencyMatrix[u][v] = weight_dist(gen);
			parent[root_v] = root_u;
			edges_added++;

			if (edges_added == countNodes - 1) break;
		}
	}

	// Шаг 2: добавляем дуги до нужной плотности
	unsigned int max_possible_edges = countNodes * (countNodes - 1) / 2;
	unsigned int total_edges_needed = static_cast<unsigned int>(max_possible_edges * density);
	unsigned int additional_edges = total_edges_needed - edges_added;

	unsigned int edges_available = possible_edges.size() - edges_added;
	unsigned int edges_to_add = min(additional_edges, edges_available);

	for (unsigned int i = 0, added = 0; added < edges_to_add && i < possible_edges.size(); i++) {
		unsigned int u = possible_edges[i].first;
		unsigned int v = possible_edges[i].second;

		// Проверка, не была ли дуга уже добавлена при формировании остовного дерева
		if (adjacencyMatrix[u][v] == 0) { 
			adjacencyMatrix[u][v] = weight_dist(gen);
			added++;
		}
	}

	// Шаг 3: найти все источники и стоки и свести их к одному
	vector<int> sources, sinks;
	vector<char> hasOutgoing(countNodes, false), hasIncoming(countNodes, false);	// поменял с bool на char, стало в 10 раз быстрее (что???)

	for (int i = 0; i < countNodes; ++i) {
		for (int j = i + 1; j < countNodes; ++j) {
			if (adjacencyMatrix[i][j]) {
				hasOutgoing[i] = 1;
				hasIncoming[j] = 1;
			}
		}
	}

	// Собираем результаты
	for (int i = 0; i < countNodes; ++i) {
		if (hasOutgoing[i] && !hasIncoming[i]) sources.push_back(i);
		if (!hasOutgoing[i] && hasIncoming[i]) sinks.push_back(i);
	}

	// Step 4: Normalize sources and sinks			// Можно НЕ СОЗДАВАТЬ СУПЕРИСТОЧНИК И СУПЕРИСТОК, а соеденить с 0 и последним

	for (auto& sourse : sources) {
		if (sourse != 0) {
			adjacencyMatrix[0][sourse] = weight_dist(gen);
		}
	}
	for (auto& sink : sinks) {
		if (sink != countNodes - 1) {
			adjacencyMatrix[sink][countNodes - 1] = weight_dist(gen);
		}
	}

	BuildAdjacencyListFromMatrix();
	BuildAdjEdgesFromMatrix();
	//initGargKonemann();
}

void Graph::TransformToRandomGraph(string name, unsigned int countNodes, float density, unsigned int maxWeightValues) {
	if (countNodes < 2) {
		throw invalid_argument("Flow graph must have at least 2 nodes");
	}

	DeleteAllNodes();
	this->name = name;
	this->weighted = true;
	this->oriented = true;

	// Инициализация генератора случайных чисел
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> weightDis(1, maxWeightValues);

	// Создание матрицы смежности
	adjacencyMatrix.assign(countNodes, std::vector<long long>(countNodes, 0));

	// Заполнение графа с заданной плотностью
	int totalPossibleEdges = countNodes * (countNodes - 1); // Для ориентированного графа без петель
	int targetEdges = static_cast<int>(density * totalPossibleEdges);
	int edgesAdded = 0;

	// Добавляем ребра случайным образом до достижения нужной плотности
	while (edgesAdded < targetEdges) {
		int i = gen() % countNodes;
		int j = gen() % countNodes;

		// Пропускаем петли 
		if (i == j) continue;

		// Если ребра еще нет, добавляем его
		if (adjacencyMatrix[i][j] == 0) {
			long long weight = weightDis(gen);
			adjacencyMatrix[i][j] = weight;
			edgesAdded++;
		}
	}

	BuildAdjacencyListFromMatrix();
	BuildAdjEdgesFromMatrix();
}

void Graph::TransformToCompleteGraph(string name, unsigned int countNodes, unsigned int maxWeightValues) {
	if (countNodes < 2) {
		throw invalid_argument("Flow graph must have at least 2 nodes");
	}

	DeleteAllNodes();
	this->name = name;
	this->weighted = true;
	this->oriented = true;
	
	// Инициализация генератора случайных чисел
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> weightDis(1, maxWeightValues);

	// Создание полной матрицы смежности
	adjacencyMatrix.assign(countNodes, std::vector<long long>(countNodes, 0));

	// Заполнение всех возможных ребер (кроме петель)
	for (unsigned int i = 0; i < countNodes; i++) {
		for (unsigned int j = 0; j < countNodes; j++) {
			if (i != j) {
				adjacencyMatrix[i][j] = weightDis(gen);
			}
		}
	}

	BuildAdjacencyListFromMatrix();
	BuildAdjEdgesFromMatrix();
}
// Случайное разбиение на доли
// Добавление рёбер между долями
// Подключение источника и стока
void Graph::TransformToBipartiteGraph(string name, unsigned int countNodes, float density, unsigned int maxWeightValues) {
	if (countNodes < 2) {
		throw invalid_argument("Flow graph must have at least 2 nodes");
	}
	
	DeleteAllNodes();
	this->name = name;
	this->weighted = true;
	this->oriented = true;
	
	// Инициализация генератора случайных чисел
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> weightDis(1, maxWeightValues);
	std::uniform_int_distribution<> partitionDis(1, countNodes - 1);

	// Определяем размеры долей (случайное разбиение, но каждая доля не пуста)
	unsigned int firstPartSize = partitionDis(gen);
	unsigned int secondPartSize = countNodes - firstPartSize;

	// Общее количество вершин: исходные вершины + источник (index = countNodes) + сток (index = countNodes + 1)
	unsigned int totalNodes = countNodes + 2;
	adjacencyMatrix.assign(totalNodes, std::vector<long long>(totalNodes, 0));

	// Источник (source) имеет индекс countNodes
	unsigned int sourceIdx = countNodes;
	// Сток (sink) имеет индекс countNodes + 1
	unsigned int sinkIdx = countNodes + 1;

	// Вычисляем максимальное количество возможных ребер между долями
	int totalPossibleEdges = firstPartSize * secondPartSize * 2; // Умножаем на 2 для ориентированного графа
	int targetEdges = static_cast<int>(density * totalPossibleEdges);
	int edgesAdded = 0;

	// Добавляем ребра только между разными долями
	while (edgesAdded < targetEdges) {
		// Выбираем случайную пару вершин из разных долей
		unsigned int i, j;
		if (gen() % 2 == 0) {
			// Ребро из первой доли во вторую
			i = gen() % firstPartSize;
			j = firstPartSize + (gen() % secondPartSize);
		}
		else {
			// Ребро из второй доли в первую
			i = firstPartSize + (gen() % secondPartSize);
			j = gen() % firstPartSize;
		}

		// Если ребра еще нет, добавляем его
		if (adjacencyMatrix[i][j] == 0) {
			long long weight = weightDis(gen);
			adjacencyMatrix[i][j] = weight;
			edgesAdded++;
		}
	}

	// Добавляем ребра от источника ко всем вершинам первой доли
	for (unsigned int i = 0; i < firstPartSize; i++) {
		long long weight = weightDis(gen);
		adjacencyMatrix[sourceIdx][i] = weight;
	}
	
	// Добавляем ребра от всех вершин второй доли к стоку
	for (unsigned int i = 0; i < secondPartSize; i++) {
		unsigned int nodeIdx = firstPartSize + i;
		long long weight = weightDis(gen);
		adjacencyMatrix[nodeIdx][sinkIdx] = weight;
	}

	BuildAdjacencyListFromMatrix();
	BuildAdjEdgesFromMatrix();
}

bool Graph::IsNodeExist(int node) {
	return nodes.find(node) != nodes.end();
}

bool Graph::IsEdgeExist(int nodeFrom, int nodeWhere) {
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
	return adjacencyMatrix.size();
}

string Graph::GetName() {
	return name;
}

void Graph::SetName(string name) {
	this->name = name;
}

vector<vector<long long>> Graph::GetMatrix() {
	return adjacencyMatrix;
}

void Graph::SetMatrix(vector<vector<long long>> martix) {
	this->adjacencyMatrix = martix;
}

unordered_map<int, unordered_map<int, long long>> Graph::GetAdjList() {
	return nodes;
}

void Graph::SetAdjList(unordered_map<int, unordered_map<int, long long>> nodes) {
	this->nodes = nodes;
}

long long Graph::GetWeight(int nodeFrom, int nodeWhere) {
	if (IsEdgeExist(nodeFrom, nodeWhere)) {
		return nodes[nodeFrom][nodeWhere];
	}
	return INF;
}

void Graph::AddNode(int node) {
	nodes[node];
}

void Graph::CopyNode(int node, int nameNewNode) {
	if (!IsNodeExist(node)) {
		throw invalid_argument("Вершины \"" + to_string(node) + "\" нет в графе");
	}
	if (IsNodeExist(nameNewNode)) {
		throw invalid_argument("Вершина \"" + to_string(nameNewNode) + "\" уже есть в графе, выберите другое имя");
	}

	AddNode(nameNewNode);

	// Копируем все исходящие рёбра
	for (auto& edge : nodes[node]) {
		int nodeTo = edge.first;
		long long weight = edge.second;

		AddEdge(nameNewNode, nodeTo, weight);
	}

	// Копируем все входящие рёбра
	for (auto& graphNode : nodes) {
		int source = graphNode.first;

		// Пропускаем саму вершину и только что созданную вершину
		if (source == node || source == nameNewNode) continue;

		if (graphNode.second.find(node) != graphNode.second.end()) {
			long long weight = graphNode.second[node];
			AddEdge(source, nameNewNode, weight);
		}
	}
}

void Graph::AddEdge(int nodeFrom, int nodeWhere, long long value) {
	nodes[nodeFrom][nodeWhere] = value;
	if (!oriented) {
		nodes[nodeWhere][nodeFrom] = value;
	}
}

void Graph::DeleteNode(int node) {
	for (auto& vertex : nodes) {
		nodes[vertex.first].erase(node);
	}
	nodes.erase(node);
}

void Graph::DeleteAllNodes() {
	nodes.clear();
}

void Graph::DeleteEdge(int nodeFrom, int nodeWhere) {
	if (!oriented) {
		nodes[nodeWhere].erase(nodeFrom);
	}
	nodes[nodeFrom].erase(nodeWhere);
}

void Graph::DeleteAllEdgeTo(int node) {
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



long long Graph::FordFulkersonMatrix(int s, int t) {

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


long long Graph::FordFulkersonBFS(int s, int t) {
	if (!IsNodeExist(s) || !IsNodeExist(t)) {
		return 0;
	}

	// Создаем остаточную сеть и инициализируем её исходными пропускными способностями
	unordered_map<int, unordered_map<int, long long>> residual;
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
	unordered_map<int, int> parent;

	// BFS для поиска увеличивающего пути
	auto bfs = [&]() {
		parent.clear();
		queue<int> q;
		q.push(s);
		parent[s] = -1; // Маркер для истока

		while (!q.empty()) {
			int u = q.front();
			q.pop();

			for (const auto& edge : residual[u]) {
				int v = edge.first;
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
		for (int v = t; v != s; v = parent[v]) {
			int u = parent[v];
			path_flow = min(path_flow, residual[u][v]);
		}

		// Обновляем остаточные пропускные способности
		for (int v = t; v != s; v = parent[v]) {
			int u = parent[v];
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

long long Graph::FordFulkersonScalingApproximate(int s, int t, double eps) {
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
	unordered_map<int, unordered_map<int, long long>> residual;
	for (const auto& u : nodes) {
		for (const auto& edge : u.second) {
			residual[u.first][edge.first] = edge.second;
		}
	}

	long long max_flow = 0;

	// 4. BFS с delta-ограничением
	auto bfs = [&]() {
		unordered_map<int, int> parent;
		queue<int> q;
		q.push(s);
		parent[s] = -1;

		while (!q.empty()) {
			int u = q.front();
			q.pop();

			for (const auto& edge : residual[u]) {
				int v = edge.first;
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
			for (int v = t; v != s; v = parent[v]) {
				int u = parent[v];
				path_flow = min(path_flow, residual[u][v]);
			}

			// Обновляем остаточную сеть
			for (int v = t; v != s; v = parent[v]) {
				int u = parent[v];
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



long long Graph::DinicMaxFlow(int s, int t) {

	unordered_map<int, size_t> level;

	long long max_flow = 0;

	// BFS для построения слоистой сети
	auto bfs = [&]() {
		level.clear();
		queue<int> q;
		q.push(s);
		level[s] = 1;

		while (!q.empty()) {
			int u = q.front();
			q.pop();

			for (const Edge& e : adj[u]) {
				if (!level.count(e.to) && e.flow < e.cap) {
					level[e.to] = level[u] + 1;
					q.push(e.to);
				}
			}
		}
		return level.count(t);
		};

	// DFS для поиска блокирующего потока
	function<long long(int, long long)> dfs = [&](int u, long long flow) {
		if (u == t) {
			return flow;
		}

		for (size_t i = 0; i < adj[u].size(); ++i) {
			Edge& e = adj[u][i];
			if (level[e.to] == level[u] + 1 && e.flow < e.cap) {
				long long min_flow = min(flow, e.cap - e.flow);
				long long pushed = dfs(e.to, min_flow);

				if (pushed > 0) {
					e.flow += pushed;
					adj[e.to][e.rev].flow -= pushed;
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
long long Graph::getMaxFlowPushRelabel_HLF(int source, int sink) {
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



void globalRelabel(const vector<vector<long long>>& capacity, const vector<vector<long long>>& flow, vector<long long>& height, int sink) {
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
long long Graph::getMaxFlowPushRelabel_HLF_GlRel(int source, int sink) {
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



// С Edges
void Graph::globalRelabelEdges(vector<long long>& height, int sink) {
	fill(height.begin(), height.end(), adj.size());
	height[sink] = 0;
	queue<int> q;
	q.push(sink);

	while (!q.empty()) {
		int u = q.front();
		q.pop();

		for (const auto& e : adj[u]) {
			// Проверяем обратное ребро (e.to -> u)
			Edge& revEdge = adj[e.to][e.rev];
			if (revEdge.cap - revEdge.flow > 0 && height[e.to] == adj.size()) {
				height[e.to] = height[u] + 1;
				q.push(e.to);
			}
		}
	}
}

long long Graph::getMaxFlowPushRelabel_HLF_GlRelEdges(int source, int sink) {
	int n = adj.size();
	vector<long long> height(n, 0);
	vector<long long> excess(n, 0);
	vector<vector<int>> buckets(2 * n);
	long long max_height = 0;
	int relabel_counter = 0;

	// Инициализация
	height[source] = n;

	// Начальное насыщение рёбер из истока
	for (auto& e : adj[source]) {
		if (e.cap > 0) {
			e.flow = e.cap;
			adj[e.to][e.rev].flow = -e.cap;
			excess[e.to] += e.cap;
			excess[source] -= e.cap;

			if (e.to != sink && e.to != source) {
				buckets[height[e.to]].push_back(e.to);
				max_height = max(max_height, height[e.to]);
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

		// Проталкивание потока
		for (auto& e : adj[u]) {
			if (excess[u] == 0) break;
			if (e.cap - e.flow > 0 && height[u] == height[e.to] + 1) {
				long long delta = min(excess[u], e.cap - e.flow);
				e.flow += delta;
				adj[e.to][e.rev].flow -= delta;
				excess[u] -= delta;
				excess[e.to] += delta;

				if (excess[e.to] == delta && e.to != source && e.to != sink) {
					buckets[height[e.to]].push_back(e.to);
					max_height = max(max_height, height[e.to]);
				}
			}
		}

		// Подъём вершины
		if (excess[u] > 0) {
			long long min_height = LLONG_MAX;
			for (auto& e : adj[u]) {
				if (e.cap - e.flow > 0) {
					min_height = min(min_height, height[e.to]);
				}
			}

			if (min_height != LLONG_MAX) {
				height[u] = min_height + 1;
				relabel_counter++;

				if (relabel_counter >= n) {
					globalRelabelEdges(height, sink);
					relabel_counter = 0;
					// Перестраиваем buckets
					max_height = 0;
					for (int v = 0; v < n; v++) {
						if (excess[v] > 0 && v != source && v != sink && height[v] < n) {
							buckets[height[v]].push_back(v);
							max_height = max(max_height, height[v]);
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
long long Graph::dfs(int u, int t, long long flow, vector<int>& level) {
	if (u == t || flow == 0)
		return flow;

	for (int v = 0; v < adjacencyMatrix.size(); ++v) {
		if (level[v] == level[u] + 1 && adjacencyMatrix[u][v] > 0) {
			long long pushed = dfs(v, t, min(flow, adjacencyMatrix[u][v]), level);
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
long long Graph::getMaxFlowDinicMatrix(int source, int sink) {
	if (source == sink)
		return 0;

	long long maxFlow = 0;
	vector<int> level(adjacencyMatrix.size());

	while (bfs(source, sink, level)) {
		while (long long pushed = dfs(source, sink, INF, level)) {
			maxFlow += pushed;
		}
	}

	return maxFlow;
}


// Улучшенный вспомогательный метод DFS для поиска блокирующего потока
long long Graph::dfsOptimazed(int u, int t, long long flow, vector<int>& ptr, vector<int>& level) {
	if (u == t || flow == 0)
		return flow;

	for (int& v = ptr[u]; v < adjacencyMatrix.size(); ++v) {
		if (level[v] == level[u] + 1 && adjacencyMatrix[u][v] > 0) {
			long long pushed = dfsOptimazed(v, t, min(flow, adjacencyMatrix[u][v]), ptr, level);
			if (pushed > 0) {
				adjacencyMatrix[u][v] -= pushed;
				adjacencyMatrix[v][u] += pushed;
				return pushed;
			}
		}
	}

	return 0;
}

// Улучшенный метод алгоритма Диница
long long Graph::getMaxFlowDinicMatrixOptimized(int source, int sink) {
	if (source == sink)
		return 0;

	long long maxFlow = 0;
	vector<int> level(adjacencyMatrix.size());
	vector<int> ptr(adjacencyMatrix.size());

	while (bfs(source, sink, level)) {
		fill(ptr.begin(), ptr.end(), 0);

		while (long long pushed = dfsOptimazed(source, sink, INF, ptr, level)) {
			maxFlow += pushed;
		}
	}

	return maxFlow;
}


// Для Unordered_map


// BFS для построения слоистой сети
bool Graph::bfsUnord_map(int s, int t, unordered_map<int, int>& level) {
	// Очищаем level для всех вершин
	for (const auto& pair : nodes) {
		level[pair.first] = -1;
	}
	level[s] = 0;

	queue<int> q;
	q.push(s);

	while (!q.empty()) {
		int u = q.front();
		q.pop();

		// Итерация по ребрам узла u
		for (auto edgeIt = nodes[u].begin(); edgeIt != nodes[u].end(); ++edgeIt) {
			int v = edgeIt->first;
			long long capacity = edgeIt->second;

			if (capacity > 0 && level[v] == -1) {
				level[v] = level[u] + 1;
				q.push(v);
			}
		}
	}

	return level[t] != -1;
}

// DFS для поиска блокирующего потока
long long Graph::dfsUnord_map(int u, int t, long long flow, unordered_map<int, int>& ptr, unordered_map<int, int>& level) {
	if (u == t || flow == 0)
		return flow;

	// Получаем ссылку на map исходящих ребер
	auto& edges = nodes[u];

	// Используем ptr для отслеживания, какие ребра уже просмотрены
	auto it = edges.begin();
	// Пропускаем уже обработанные ребра
	advance(it, ptr[u]);

	for (; it != edges.end(); ++it) {
		int v = it->first;
		long long& capacity = it->second;

		if (level[v] == level[u] + 1 && capacity > 0) {
			long long pushed = dfsUnord_map(v, t, min(flow, capacity), ptr, level);
			if (pushed > 0) {

				capacity -= pushed;
				// обновляем обратное ребро
				nodes[v][u] += pushed;

				return pushed;
			}
		}
		ptr[u]++; // Увеличиваем указатель
	}

	return 0;
}

// Основной метод алгоритма Диница
long long Graph::getMaxFlowDinicUnord_map(int source, int sink) {
	if (source == sink)
		return 0;

	long long maxFlow = 0;
	unordered_map<int, int> level;
	unordered_map<int, int> ptr;

	while (bfsUnord_map(source, sink, level)) {
		// Инициализируем ptr для всех вершин
		for (const auto& pair : nodes) {
			ptr[pair.first] = 0;
		}

		while (long long pushed = dfsUnord_map(source, sink, INF, ptr, level)) {
			maxFlow += pushed;
		}
	}

	return maxFlow;
}







// с масштабированием

bool Graph::bfsWithCapacity(int s, int t, vector<int>& level, long long delta) {
	fill(level.begin(), level.end(), -1);
	level[s] = 0;

	queue<int> q;
	q.push(s);

	while (!q.empty()) {
		int u = q.front();
		q.pop();

		for (int v = 0; v < adjacencyMatrix.size(); ++v) {
			if (level[v] == -1 && adjacencyMatrix[u][v] >= delta) {
				level[v] = level[u] + 1;
				q.push(v);
			}
		}
	}

	return level[t] != -1;
}

long long Graph::dfsWithCapacity(int u, int t, long long flow, vector<int>& ptr, vector<int>& level, long long delta) {
	if (u == t || flow == 0)
		return flow;

	for (int& v = ptr[u]; v < adjacencyMatrix.size(); ++v) {
		if (level[v] == level[u] + 1 && adjacencyMatrix[u][v] >= delta) {
			long long pushed = dfsWithCapacity(v, t, min(flow, adjacencyMatrix[u][v]),
				ptr, level, delta);
			if (pushed > 0) {
				adjacencyMatrix[u][v] -= pushed;
				adjacencyMatrix[v][u] += pushed;
				return pushed;
			}
		}
	}

	return 0;
}

long long Graph::getMaxFlowDinicCapacityScaling(int source, int sink, long long maxCapacity) {
	if (source == sink)
		return 0;

	long long maxFlow = 0;
	vector<int> level(adjacencyMatrix.size());
	vector<int> ptr(adjacencyMatrix.size());

	// Начальный порог Δ (наибольшая степень двойки <= maxCapacity)
	long long delta = 1;
	while (delta * 2 <= maxCapacity) {
		delta *= 2;
	}

	// Основной цикл масштабирования
	while (delta > 0) {
		// Многократно увеличиваем поток с текущим Δ
		while (bfsWithCapacity(source, sink, level, delta)) {
			fill(ptr.begin(), ptr.end(), 0);

			while (long long pushed = dfsWithCapacity(source, sink, INF, ptr, level, delta)) {
				maxFlow += pushed;
			}
		}

		// Уменьшаем порог
		delta /= 2;
	}

	return maxFlow;
}



// BFS для построения слоистой сети
bool Graph::bfsUnord_mapWithCapacity(int s, int t, unordered_map<int, int>& level, long long delta) {
	// Очищаем level для всех вершин
	for (const auto& pair : nodes) {
		level[pair.first] = -1;
	}
	level[s] = 0;

	queue<int> q;
	q.push(s);

	while (!q.empty()) {
		int u = q.front();
		q.pop();

		// Итерация по ребрам узла u
		for (auto edgeIt = nodes[u].begin(); edgeIt != nodes[u].end(); ++edgeIt) {
			int v = edgeIt->first;
			long long capacity = edgeIt->second;

			if (capacity >= delta && level[v] == -1) {
				level[v] = level[u] + 1;
				q.push(v);
			}
		}
	}

	return level[t] != -1;
}

// DFS для поиска блокирующего потока
long long Graph::dfsUnord_mapWithCapacity(int u, int t, long long flow, unordered_map<int, int>& ptr, unordered_map<int, int>& level, long long delta) {
	if (u == t || flow == 0)
		return flow;

	// Получаем ссылку на map исходящих ребер
	auto& edges = nodes[u];

	// Используем ptr для отслеживания, какие ребра уже просмотрены
	auto it = edges.begin();
	// Пропускаем уже обработанные ребра
	advance(it, ptr[u]);

	for (; it != edges.end(); ++it) {
		int v = it->first;
		long long& capacity = it->second;

		if (level[v] == level[u] + 1 && capacity >= delta) {
			long long pushed = dfsUnord_mapWithCapacity(v, t, min(flow, capacity), ptr, level, delta);
			if (pushed > 0) {

				capacity -= pushed;
				// обновляем обратное ребро
				nodes[v][u] += pushed;

				return pushed;
			}
		}
		ptr[u]++; // Увеличиваем указатель
	}

	return 0;
}

// Основной метод алгоритма Диница
long long Graph::getMaxFlowDinicUnord_mapCapacityScaling(int source, int sink, long long maxCapacity) {
	if (source == sink)
		return 0;

	long long maxFlow = 0;
	unordered_map<int, int> level;
	unordered_map<int, int> ptr;

	// Начальный порог Δ (наибольшая степень двойки <= maxCapacity)
	long long delta = 1;
	while (delta * 2 <= maxCapacity) {
		delta *= 2;
	}

	// Основной цикл масштабирования
	while (delta > 0) {
		while (bfsUnord_mapWithCapacity(source, sink, level, delta)) {
			// Инициализируем ptr для всех вершин
			for (const auto& pair : nodes) {
				ptr[pair.first] = 0;
			}

			while (long long pushed = dfsUnord_mapWithCapacity(source, sink, INF, ptr, level, delta)) {
				maxFlow += pushed;
			}
		}

		// Уменьшаем порог
		delta /= 2;
	}

	return maxFlow;
}

//
//
//bool Graph::bfsWithEdges(int s, int t, vector<int>& level) {
//	fill(level.begin(), level.end(), -1);
//	level[s] = 0;
//
//	queue<int> q;
//	q.push(s);
//
//	while (!q.empty()) {
//		int u = q.front();
//		q.pop();
//
//		for (FlowEdge& e : adj[u]) {
//			if (e.capacity > 0 && level[e.to] == -1) {
//				level[e.to] = level[u] + 1;
//				q.push(e.to);
//			}
//		}
//	}
//
//	return level[t] != -1;
//}
//
//long long Graph::dfsWithEdges(int u, int t, long long flow, vector<int>& ptr, vector<int>& level) {
//	if (u == t || flow == 0)
//		return flow;
//
//	for (int& i = ptr[u]; i < adj[u].size(); ++i) {
//		FlowEdge& e = adj[u][i];
//		if (level[i] == level[u] + 1 && e.capacity > 0) {
//			long long pushed = dfsWithEdges(i, t, min(flow, e.capacity), ptr, level);
//			if (pushed > 0) {
//				e.capacity -= pushed;
//				adj[i][u].capacity += pushed;
//				return pushed;
//			}
//		}
//	}
//
//	return 0;
//}
//
//
//// Основной метод алгоритма Диница
///////////////////////// ГДЕ ОШИБКА, ВЫДАЁТ НЕПРАВИЛЬНЫЕ РЕЗУЛЬТАТЫ НА БОЛЬШИХ ГРАФАХ
//long long Graph::getMaxFlowDinicWithEdges(int source, int sink) { 
//	if (source == sink)
//		return 0;
//
//	long long maxFlow = 0;
//	vector<int> level(adj.size());
//	vector<int> ptr(adj.size());
//
//	while (bfsWithEdges(source, sink, level)) {
//		fill(ptr.begin(), ptr.end(), 0);
//
//		while (long long pushed = dfsWithEdges(source, sink, INF, ptr, level)) {
//			maxFlow += pushed;
//		}
//	}
//
//	return maxFlow;
//}
//
//
//
//

// Приближённый

void Graph::initGargKonemann() {
	int n = adjacencyMatrix.size();
	residual = adjacencyMatrix;
	dual_weights.assign(n, vector<double>(n, 1.0));
	distance.assign(n, numeric_limits<double>::max());
	parent.assign(n, -1);
}

bool Graph::findShortestPath(int s, int t) {
	int n = adjacencyMatrix.size();
	distance.assign(n, numeric_limits<double>::max());
	parent.assign(n, -1);
	distance[s] = 0;

	priority_queue<pair<double, int>, vector<pair<double, int>>, greater<pair<double, int>>> pq;
	pq.push({ 0, s });

	while (!pq.empty()) {
		auto pair = pq.top();
		double dist_u = pair.first;
		int u = pair.second;
		pq.pop();

		if (u == t) break;
		if (dist_u > distance[u]) continue;

		for (int v = 0; v < n; ++v) {
			if (residual[u][v] > 0) {
				double weight = dual_weights[u][v] / residual[u][v];
				if (distance[v] > distance[u] + weight) {
					distance[v] = distance[u] + weight;
					parent[v] = u;
					pq.push({ distance[v], v });
				}
			}
		}
	}
	return parent[t] != -1;
}


long long Graph::augmentFlow(int s, int t, double epsilon) {
	long long delta = numeric_limits<long long>::max();
	// Находим минимальную остаточную способность
	for (int v = t; v != s; v = parent[v]) {
		int u = parent[v];
		delta = min(delta, residual[u][v]);
	}
	// Увеличиваем поток и обновляем двойственные веса
	for (int v = t; v != s; v = parent[v]) {
		int u = parent[v];
		residual[u][v] -= delta;
		residual[v][u] += delta;
		dual_weights[u][v] *= (1 + epsilon * delta / adjacencyMatrix[u][v]);
	}
	return delta;
}


long long Graph::gargKonemannMaxFlow(int s, int t, double epsilon) {
	initGargKonemann();
	long long max_flow = 0;
	while (findShortestPath(s, t)) {
		max_flow += augmentFlow(s, t, epsilon);
	}
	return max_flow;
}





long long Graph::FordFulkersonScaling(int s, int t, double eps) {
	// Проверка на существование узлов
	if (!IsNodeExist(s) || !IsNodeExist(t)) {
		return -1;
	}

	// Инициализация остаточных пропускных способностей
	long long max_cap = 0;
	unordered_map<int, unordered_map<int, long long>> residual;
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
		unordered_map<int, int> parent;
		queue<int> q;
		q.push(s);
		parent[s] = -1;

		while (!q.empty()) {
			int u = q.front();
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
			for (int v = t; v != s; v = parent[v]) {
				int u = parent[v];
				pathFlow = min(pathFlow, residual[u][v]);
			}

			// Обновляем остаточные способности
			for (int v = t; v != s; v = parent[v]) {
				int u = parent[v];
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