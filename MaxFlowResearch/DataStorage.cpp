#include "DataStorage.h"

int DataStorage::Size() {
	return graphs.size();
}

void DataStorage::SetGraph(string name) {
	for (int i = 0; i < graphs.size(); i++) {
		if (graphs[i].GetName() == name) {
			iter = graphs.begin() + i;
			return;
		}
	}
	throw "Графа " + name + " нет в базе";
}

void DataStorage::CreateRandomFlowGraph(unsigned int countNodes, float density, unsigned int maxWeightValue) {
	if (abs(density) > 1) {
		throw "Значение плотности может быть только от 0 до 1. Сейчас: " + to_string(density);
	}
	
	string name = "RandomFlowGraph";
	int secondPartName = 1;
	while (IsGraphExist(name + to_string(secondPartName))) {
		secondPartName++;
	}
	name = name + to_string(secondPartName);

	unsigned int maxPossibleEdges = countNodes * (countNodes - 1) / 2;
	unsigned int requestedEdges = static_cast<unsigned int>(maxPossibleEdges * density);
	unsigned int minEdges = countNodes - 1;

	if (requestedEdges < minEdges) {
		throw invalid_argument("Невозможно создать связный граф. Требуется минимум " +
			to_string(minEdges) + " дуг, запрошено " +
			to_string(requestedEdges));
	}

	if (requestedEdges > maxPossibleEdges) {
		throw invalid_argument("Превышено максимальное количество дуг. Максимум: " +
			to_string(maxPossibleEdges) + ", запрошено: " +
			to_string(requestedEdges));
	}
	graphs.push_back(Graph());
	iter = --graphs.end();
	iter->TransformToRandomFlowGraph(name, countNodes, density, maxWeightValue);
}

void DataStorage::CreateCopyGraph(string newName) {
	if (IsGraphExist(newName)) {
		throw "Граф " + newName + " уже существует";
	}
	Graph gr = Graph(*iter);
	gr.SetName(newName);
	graphs.push_back(gr);
	iter = --graphs.end();
}

void DataStorage::DeleteGraph() {
	graphs.erase(iter);
	iter = graphs.begin();
}

string DataStorage::GetName() {
	return iter->GetName();
}

void DataStorage::SetName(string name) {
	iter->SetName(name);
}

bool DataStorage::IsGraphExist(string name) {
	for (auto& graph : graphs) {
		if (graph.GetName() == name)
			return true;
	}
	return false;
}

bool DataStorage::IsWeighted() {
	return iter->IsWeighted();
}

bool DataStorage::IsOriented() {
	return iter->IsOriented();
}

void DataStorage::AddNode(string node) {
	if (iter->IsNodeExist(node)) {
		throw "Вершина " + node + " уже существует";
	}
	else
		iter->AddNode(node);
}

void DataStorage::AddEdge(string nodeFrom, string nodeWhere, int value) {
	if (nodeFrom == nodeWhere && !IsOriented())
		throw string("Нельзя создать петлю в неориентированном графе");

	int error = 0;
	if (!iter->IsNodeExist(nodeFrom))
		error += 1;
	if (!iter->IsNodeExist(nodeWhere))
		error += 2;
	if (error != 0)
		throw error;

	if (iter->IsEdgeExist(nodeFrom, nodeWhere)) {
		if (IsOriented()) {
			throw "Дуга " + nodeFrom + " -> " + nodeWhere + " уже существует";
		}
		else {
			throw "Ребро " + nodeFrom + " -- " + nodeWhere + " уже существует";
		}
	}

	iter->AddEdge(nodeFrom, nodeWhere, value);
}

void DataStorage::DeleteNode(string node) {
	if (iter->IsNodeExist(node)) {
		iter->DeleteNode(node);
	}
	else
		throw "Вершины " + node + " не существует";
}

void DataStorage::DeleteEdge(string nodeFrom, string nodeWhere) {
	if (iter->IsEdgeExist(nodeFrom, nodeWhere)) {
		iter->DeleteEdge(nodeFrom, nodeWhere);
	}
	else
		throw "Дуга " + nodeFrom + " -> " + nodeWhere + " и так не существует";
}

void DataStorage::ChangeValueWay(string nodeFrom, string nodeWhere, int value) {
	int error = 0;
	if (!iter->IsNodeExist(nodeFrom))
		error += 1;
	if (!iter->IsNodeExist(nodeWhere))
		error += 2;
	if (error != 0)
		throw error;

	iter->AddEdge(nodeFrom, nodeWhere, value);
}

void DataStorage::WriteToConsole() {
	cout << *iter;
}

void DataStorage::WriteToConsoleAll() {
	for (const auto& item : graphs) {
		cout << item << '\n';
	}
}

long long DataStorage::GetMaxFlowFordFulkerson(string s, string t) {
	if (!iter->IsOriented() || !iter->IsWeighted())
		throw string("Граф должен быть ориентированным и взвешенным");

	if (!(iter->IsNodeExist(s) && iter->IsNodeExist(t)))
		throw string("Какой-то из вершин не существует");

	long long maxFlow;

	//cout << iter->FordFulkersonMatrix(s, t) << '\n';
	cout << iter->FordFulkersonBFS(s, t) << '\n';

	CreateCopyGraph(iter->GetName() + "Copy");
	cout << iter->FordFulkersonScaling(s, t);
	DeleteGraph();
	iter = --graphs.end();

	CreateCopyGraph(iter->GetName() + "Copy");
	maxFlow = iter->FordFulkersonScalingApproximate(s, t);
	DeleteGraph();
	iter = --graphs.end();
	return maxFlow;
}


long long DataStorage::GetMaxFlowPushRelabel() {
	if (!iter->IsOriented() || !iter->IsWeighted())
		throw string("Граф должен быть ориентированным и взвешенным");

	if (!(iter->IsNodeExist("source") && iter->IsNodeExist("sink")))
		throw string("Какой-то из вершин не существует");

	cout << iter->getMaxFlowPushRelabel(0, iter->GetCountOfNodes() - 1) << '\n';
	cout << iter->getMaxFlowPushRelabel_HLF(0, iter->GetCountOfNodes() - 1) << '\n';
	return iter->getMaxFlowPushRelabel_HLF_GlRel(0, iter->GetCountOfNodes() - 1);
}

long long DataStorage::GetMaxFlowDinic(string s, string t) {
	if (!iter->IsOriented() || !iter->IsWeighted())
		throw string("Граф должен быть ориентированным и взвешенным");

	if (!(iter->IsNodeExist(s) && iter->IsNodeExist(t)))
		throw string("Какой-то из вершин не существует");

	long long maxFlow;

	//cout << iter->DinicMaxFlow(s, t) << '\n';

	CreateCopyGraph(iter->GetName() + "Copy");
	cout << iter->DinicMaxFlowMatrix(s, t) << '\n';
	DeleteGraph();
	iter = --graphs.end();

	CreateCopyGraph(iter->GetName() + "Copy");
	maxFlow = iter->getMaxFlowDinic(0, iter->GetCountOfNodes() - 1) << '\n';
	DeleteGraph();
	iter = --graphs.end();
	return maxFlow;
}

long long DataStorage::gargKonemannMaxFlow() {
	if (!iter->IsOriented() || !iter->IsWeighted())
		throw string("Граф должен быть ориентированным и взвешенным");

	if (!(iter->IsNodeExist("source") && iter->IsNodeExist("sink")))
		throw string("Какой-то из вершин не существует");

	return iter->gargKonemannMaxFlow(0, iter->GetCountOfNodes() - 1);
}

void DataStorage::CompareMethods(unsigned int countNodes, float density, unsigned int maxWeightValue, unsigned int countGraphs, const vector<string>& methodNames) {
	// Таблица для хранения методов и их времени выполнения
	map<string, function<long long(int, int, long long&)>> methods = {
		// PushRelabel методы
		{"PushRelabelMatrix", [this](int s, int t, long long& time) {
			auto start = chrono::steady_clock::now();
			long long maxFlow = iter->getMaxFlowPushRelabel(s, t);
			auto end = chrono::steady_clock::now();
			time = chrono::duration_cast<chrono::microseconds>(end - start).count();
			return maxFlow;
		}},
		{"PushRelabelMatrix_HLF", [this](int s, int t, long long& time) {
			auto start = chrono::steady_clock::now();
			long long maxFlow = iter->getMaxFlowPushRelabel_HLF(s, t);
			auto end = chrono::steady_clock::now();
			time = chrono::duration_cast<chrono::microseconds>(end - start).count();
			return maxFlow;
		}},
		{"PushRelabelMatrix_v3", [this](int s, int t, long long& time) {
			auto start = chrono::steady_clock::now();
			long long maxFlow = iter->getMaxFlowPushRelabel_HLF_GlRel(s, t);
			auto end = chrono::steady_clock::now();
			time = chrono::duration_cast<chrono::microseconds>(end - start).count();
			return maxFlow;
		}},
		{"PushRelabelMatrixMaxPushes", [this](int s, int t, long long& time) {
			auto start = chrono::steady_clock::now();
			long long maxFlow = iter->PushRelabelMatrix(to_string(s), to_string(t));
			auto end = chrono::steady_clock::now();
			time = chrono::duration_cast<chrono::microseconds>(end - start).count();
			return maxFlow;
		}},

		// Dinic методы
		{"DinicMaxFlow_unordered_map", [this](int s, int t, long long& time) {
			CreateCopyGraph(iter->GetName() + "Copy");
			auto start = chrono::steady_clock::now();
			long long maxFlow = iter->DinicMaxFlowMatrix("source", "sink");
			auto end = chrono::steady_clock::now();
			time = chrono::duration_cast<chrono::microseconds>(end - start).count();
			DeleteGraph();
			iter = --graphs.end();
			return maxFlow;
		}},
		{"DinicMaxFlowMatrix", [this](int s, int t, long long& time) {
			CreateCopyGraph(iter->GetName() + "Copy");
			auto start = chrono::steady_clock::now();
			long long maxFlow = iter->getMaxFlowDinic(s, t);
			auto end = chrono::steady_clock::now();
			time = chrono::duration_cast<chrono::microseconds>(end - start).count();
			DeleteGraph();
			iter = --graphs.end();
			return maxFlow;
		}},
		/*{"DinicMaxFlowWithEdges", [this](int s, int t, long long& time) {
			CreateCopyGraph(iter->GetName() + "Copy");
			auto start = chrono::steady_clock::now();
			long long maxFlow = iter->getMaxFlowDinicWithEdges(s, t);
			auto end = chrono::steady_clock::now();
			time = chrono::duration_cast<chrono::microseconds>(end - start).count();
			DeleteGraph();
			iter = --graphs.end();
			return maxFlow;
		}},*/

		// другие...
		{"gargKonemannMaxFlow", [this](int s, int t, long long& time) {
			CreateCopyGraph(iter->GetName() + "Copy");
			auto start = chrono::steady_clock::now();
			long long maxFlow = iter->gargKonemannMaxFlow(s, t);
			auto end = chrono::steady_clock::now();
			time = chrono::duration_cast<chrono::microseconds>(end - start).count();
			DeleteGraph();
			iter = --graphs.end();
			return maxFlow;
		}},
	};

	// Вектор для хранения общего времени
	vector<long long> totalTime(methodNames.size(), 0);
	vector<vector<long long>> maxFlows(countGraphs, vector<long long>(methodNames.size(), 0)); // Результаты выполнения методов

	int n = 0;
	while (n < countGraphs) {
		CreateRandomFlowGraph(countNodes, density, maxWeightValue);
		int currentCountNodes = iter->GetCountOfNodes();
		int s = 0;
		int t = currentCountNodes - 1;

		// Проверяем каждый метод
		for (size_t i = 0; i < methodNames.size(); ++i) {
			const auto& name = methodNames[i];
			if (methods.find(name) == methods.end()) {
				cout << "Unknown method: " << name << endl;
				continue;
			}

			long long time = 0;
			long long maxFlow = methods[name](s, t, time);
			totalTime[i] += time;
			maxFlows[n][i] = maxFlow; // Сохраняем последний maxFlow (можно добавить проверку)
		}

		system("cls");

		cout << "--------------------------------------------------------------------------------------\n";
		cout << "Сравнение методов для графов с " << countNodes << " вершинами и плотностью " << density << '\n';
		cout << "Количество тестовых графов: " << n << "\n\n";
		cout << "Среднее время выполнения (микросекунды):\n";

		for (size_t i = 0; i < methodNames.size(); i++) {
			if (i < totalTime.size()) {
				double avgTime = static_cast<double>(totalTime[i]) / n;
				cout << methodNames[i] << ": " << avgTime / 1000 << " мс\n";
			}
		}
		cout << "--------------------------------------------------------------------------------------\n";

		n++;
	}

	cout << "\nСравнение результатов методов:\n";
	cout << "Метод\t";
	for (size_t i = 0; i < methodNames.size(); i++) {
		cout << methodNames[i] << '\t';
	}
	cout << '\n';

	for (size_t i = 0; i < countGraphs; i++) {
		cout << "Граф " << i << ":\t";
		for (size_t j = 0; j < methodNames.size(); j++) {
			cout << maxFlows[i][j] << '\t';
		}
		cout << '\n';
	}
}

void DataStorage::ComparePushRelabelMethods() {
	CompareMethods(200, 0.3, 10000, 25, { "PushRelabelMatrix",
		"PushRelabelMatrix_HLF",
		"PushRelabelMatrix_v3",
		"PushRelabelMatrixMaxPushes"
		});
}

void DataStorage::CompareDinicMethods() {
	CompareMethods(5000, 0.6, 10000, 25, { "DinicMaxFlow_unordered_map",
		"DinicMaxFlowMatrix"});
}

void DataStorage::CompareBestMaxFlowMetods() {
	CompareMethods(300, 0.8, 10000, 25, { "PushRelabelMatrix_v3", "DinicMaxFlowMatrix", "gargKonemannMaxFlow"});
}

DataStorage::~DataStorage() {
	cout << "DataStorage: start deleting...\n";
}