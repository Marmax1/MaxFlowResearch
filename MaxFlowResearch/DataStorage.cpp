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

void DataStorage::CreateRandomFlowGraph(unsigned int& outEdgesCount, unsigned int countNodes, float density, unsigned int maxWeightValue) {
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
	outEdgesCount = requestedEdges;

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

void DataStorage::CreateRandomGraph(unsigned int& outEdgesCount, unsigned int countNodes, float density, unsigned int maxWeightValue) {
	// Проверка плотности
	if (density < 0 || density > 1) {
		throw "Значение плотности может быть только от 0 до 1. Сейчас: " + to_string(density);
	}

	// Генерация уникального имени
	string name = "RandomGraph";
	int secondPartName = 1;
	while (IsGraphExist(name + to_string(secondPartName))) {
		secondPartName++;
	}
	name = name + to_string(secondPartName);

	// Для ориентированного графа максимальное количество ребер
	unsigned int maxPossibleEdges = countNodes * (countNodes - 1); // без петель

	unsigned int requestedEdges = static_cast<unsigned int>(maxPossibleEdges * density);

	outEdgesCount = requestedEdges;

	if (requestedEdges > maxPossibleEdges) {
		throw invalid_argument("Превышено максимальное количество дуг. Максимум: " +
			to_string(maxPossibleEdges) + ", запрошено: " +
			to_string(requestedEdges));
	}

	// Добавление графа в хранилище
	graphs.push_back(Graph());
	iter = --graphs.end();
	iter->TransformToRandomGraph(name, countNodes, density, maxWeightValue);
}

void DataStorage::CreateCompleteGraph(unsigned int& outEdgesCount, unsigned int countNodes, unsigned int maxWeightValue) {
	if (countNodes == 0) {
		throw invalid_argument("Количество вершин должно быть больше 0. Сейчас: " + to_string(countNodes));
	}

	if (maxWeightValue == 0) {
		throw invalid_argument("Максимальный вес должен быть больше 0. Сейчас: " + to_string(maxWeightValue));
	}

	// Генерация уникального имени
	string name = "CompleteGraph";
	int secondPartName = 1;
	while (IsGraphExist(name + to_string(secondPartName))) {
		secondPartName++;
	}
	name = name + to_string(secondPartName);

	// Для полного ориентированного графа количество дуг
	unsigned int totalEdges = countNodes * (countNodes - 1); // Без петель

	outEdgesCount = totalEdges;

	// Добавление графа в хранилище
	graphs.push_back(Graph());
	iter = --graphs.end();
	iter->TransformToCompleteGraph(name, countNodes, maxWeightValue);
}

void DataStorage::CreateBipartiteGraph(unsigned int& outEdgesCount, unsigned int countNodes, float density, unsigned int maxWeightValue) {
	// Проверка плотности
	if (density < 0 || density > 1) {
		throw "Значение плотности может быть только от 0 до 1. Сейчас: " + to_string(density);
	}

	if (countNodes < 2) {
		throw invalid_argument("Для двудольного графа необходимо минимум 2 вершины. Сейчас: " + to_string(countNodes));
	}

	// Генерация уникального имени
	string name = "RandomBipartiteGraph";
	int secondPartName = 1;
	while (IsGraphExist(name + to_string(secondPartName))) {
		secondPartName++;
	}
	name = name + to_string(secondPartName);

	// Для двудольного графа максимальное количество ребер зависит от разбиения
	// Худший случай: доли по countNodes/2 вершин
	unsigned int maxFirstPart = countNodes / 2;
	unsigned int maxSecondPart = countNodes - maxFirstPart;
	unsigned int maxPossibleEdges = maxFirstPart * maxSecondPart * 2; // *2 для ориентированного графа

	unsigned int requestedEdges = static_cast<unsigned int>(maxPossibleEdges * density);

	outEdgesCount = requestedEdges;

	if (requestedEdges > maxPossibleEdges) {
		throw invalid_argument("Превышено максимальное количество дуг для двудольного графа. Максимум: " +
			to_string(maxPossibleEdges) + ", запрошено: " +
			to_string(requestedEdges));
	}

	// Добавление графа в хранилище
	graphs.push_back(Graph());
	iter = --graphs.end();
	iter->TransformToBipartiteGraph(name, countNodes, density, maxWeightValue);
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

void DataStorage::AddNode(int node) {
	if (iter->IsNodeExist(node)) {
		throw "Вершина " + to_string(node) + " уже существует";
	}
	else
		iter->AddNode(node);
}

void DataStorage::AddEdge(int nodeFrom, int nodeWhere, int value) {
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
			throw "Дуга " + to_string(nodeFrom) + " -> " + to_string(nodeWhere) + " уже существует";
		}
		else {
			throw "Ребро " + to_string(nodeFrom) + " -- " + to_string(nodeWhere) + " уже существует";
		}
	}

	iter->AddEdge(nodeFrom, nodeWhere, value);
}

void DataStorage::DeleteNode(int node) {
	if (iter->IsNodeExist(node)) {
		iter->DeleteNode(node);
	}
	else
		throw "Вершины " + to_string(node) + " не существует";
}

void DataStorage::DeleteEdge(int nodeFrom, int nodeWhere) {
	if (iter->IsEdgeExist(nodeFrom, nodeWhere)) {
		iter->DeleteEdge(nodeFrom, nodeWhere);
	}
	else
		throw "Дуга " + to_string(nodeFrom) + " -> " + to_string(nodeWhere) + " и так не существует";
}

void DataStorage::ChangeValueWay(int nodeFrom, int nodeWhere, int value) {
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

long long DataStorage::GetMaxFlowFordFulkerson(int s, int t) {
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

	/*if (!(iter->IsNodeExist(0) && iter->IsNodeExist("sink")))
		throw string("Какой-то из вершин не существует");*/

	cout << iter->getMaxFlowPushRelabel(0, iter->GetCountOfNodes() - 1) << '\n';
	cout << iter->getMaxFlowPushRelabel_HLF(0, iter->GetCountOfNodes() - 1) << '\n';
	return iter->getMaxFlowPushRelabel_HLF_GlRel(0, iter->GetCountOfNodes() - 1);
}

long long DataStorage::GetMaxFlowDinic(int s, int t) {
	if (!iter->IsOriented() || !iter->IsWeighted())
		throw string("Граф должен быть ориентированным и взвешенным");

	/*if (!(iter->IsNodeExist(s) && iter->IsNodeExist(t)))
		throw string("Какой-то из вершин не существует");*/

	long long maxFlow;

	//cout << iter->DinicMaxFlow(s, t) << '\n';

	/*CreateCopyGraph(iter->GetName() + "Copy");
	cout << iter->DinicMaxFlowMatrix(s, t) << '\n';
	DeleteGraph();
	iter = --graphs.end();*/

	CreateCopyGraph(iter->GetName() + "Copy");
	maxFlow = iter->getMaxFlowDinicMatrix(s, t);
	DeleteGraph();
	iter = --graphs.end();
	return maxFlow;
}

long long DataStorage::gargKonemannMaxFlow() {
	if (!iter->IsOriented() || !iter->IsWeighted())
		throw string("Граф должен быть ориентированным и взвешенным");

	/*if (!(iter->IsNodeExist("source") && iter->IsNodeExist("sink")))
		throw string("Какой-то из вершин не существует");*/

	return iter->gargKonemannMaxFlow(0, iter->GetCountOfNodes() - 1);
}

void DataStorage::CompareMethods(unsigned int countNodes, float density, long long maxWeightValue, unsigned int countGraphs, 
	unsigned int graphType, const vector<string>& methodNames) {
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
		{"PushRelabelEdges", [this](int s, int t, long long& time) {
			auto start = chrono::steady_clock::now();
			long long maxFlow = iter->getMaxFlowPushRelabel_HLF_GlRelEdges(s, t);
			auto end = chrono::steady_clock::now();
			time = chrono::duration_cast<chrono::microseconds>(end - start).count();
			return maxFlow;
		}},

		// Dinic методы
		
		{"DinicMaxFlowMatrix", [this](int s, int t, long long& time) {
			auto copyMatrix = iter->GetMatrix();
			auto start = chrono::steady_clock::now();
			long long maxFlow = iter->getMaxFlowDinicMatrix(s, t);
			auto end = chrono::steady_clock::now();
			time = chrono::duration_cast<chrono::microseconds>(end - start).count();
			iter->SetMatrix(copyMatrix);
			return maxFlow;
		}},
		{"DinicMaxFlowMatrixOptimized", [this](int s, int t, long long& time) {
			auto copyMatrix = iter->GetMatrix();
			auto start = chrono::steady_clock::now();
			long long maxFlow = iter->getMaxFlowDinicMatrixOptimized(s, t);
			auto end = chrono::steady_clock::now();
			time = chrono::duration_cast<chrono::microseconds>(end - start).count();
			iter->SetMatrix(copyMatrix);
			return maxFlow;
		}},
		{"DinicMaxFlowCapacityScaling", [this](int s, int t, long long& time) {
			auto copyMatrix = iter->GetMatrix();
			auto start = chrono::steady_clock::now();
			long long maxFlow = iter->getMaxFlowDinicCapacityScaling(s, t, 10000000000);
			auto end = chrono::steady_clock::now();
			time = chrono::duration_cast<chrono::microseconds>(end - start).count();
			iter->SetMatrix(copyMatrix);
			return maxFlow;
		}},
		{"MaxFlowDinicUnord_map", [this](int s, int t, long long& time) {
			auto copyAdjList = iter->GetAdjList();
			auto start = chrono::steady_clock::now();
			long long maxFlow = iter->getMaxFlowDinicUnord_map(s, t);
			auto end = chrono::steady_clock::now();
			time = chrono::duration_cast<chrono::microseconds>(end - start).count();
			iter->SetAdjList(copyAdjList);
			return maxFlow;
		}},
		{"MaxFlowDinicUnord_mapCapacityScaling", [this](int s, int t, long long& time) {
			auto copyAdjList = iter->GetAdjList();
			auto start = chrono::steady_clock::now();
			long long maxFlow = iter->getMaxFlowDinicUnord_mapCapacityScaling(s, t, 10000000000);
			auto end = chrono::steady_clock::now();
			time = chrono::duration_cast<chrono::microseconds>(end - start).count();
			iter->SetAdjList(copyAdjList);
			return maxFlow;
		}},
		/*{"DinicMaxFlowWithEdges", [this](int s, int t, long long& time) {
			CreateCopyGraph(iter->GetName() + "Copy");
			auto start = chrono::steady_clock::now();
			long long maxFlow = iter->DinicMaxFlow(s, t);
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

	unsigned int countEdges;

	int n = 0;
	while (n < countGraphs) {
		switch (graphType) {
		case 0:
			CreateRandomFlowGraph(countEdges, countNodes, density, maxWeightValue);
			break;
		case 1:
			CreateRandomGraph(countEdges, countNodes, density, maxWeightValue);
			break;
		case 2:
			CreateCompleteGraph(countEdges, countNodes, maxWeightValue);
			break;
		case 3:
			CreateBipartiteGraph(countEdges, countNodes, density, maxWeightValue);
			break;
		default:
			throw string("Неизвестный тип графа");
		}
		
		int currentCountNodes = iter->GetCountOfNodes();
		int s, t;
		if (graphType == 3) {
			s = currentCountNodes - 2;
			t = currentCountNodes - 1;
		}
		else {
			s = 0;
			t = currentCountNodes - 1;
		}
		

		// Проверяем каждый метод
		for (size_t i = 0; i < methodNames.size(); i++) {
			const auto& name = methodNames[i];
			if (methods.find(name) == methods.end()) {
				cout << "Unknown method: " << name << endl;
				continue;
			}

			long long time = 0;
			long long maxFlow = methods[name](s, t, time);
			if (maxFlow == 0) continue; // пропускаем графы без найденных потоков
			totalTime[i] += time;
			maxFlows[n][i] = maxFlow; // Сохраняем последний maxFlow (для проверки)
		}

		system("cls");

		cout << "--------------------------------------------------------------------------------------\n";
		switch (graphType) {
		case 0:
			cout << "Сравнение методов для ориентированных ациклических графов с:\n";
			break;
		case 1:
			cout << "Сравнение методов для случайных графов с:\n";
			break;
		case 2:
			cout << "Сравнение методов для полных графов с:\n";
			break;
		case 3:
			cout << "Сравнение методов для случайных двудольных графов с:\n";
			break;
		default:
			throw string("Неизвестный тип графа");
		}
		cout << countNodes << " вершинами\n";
		if (graphType == 2) {
			cout << "кол-вом дуг около " << countEdges << ")\n";
		}
		else {
			cout << "плотностью " << density << " (кол-во дуг около " << countEdges << ")\n";
		}
		cout << "максимальным значение пропускной способности = " << maxWeightValue << '\n';
		cout << "Количество тестовых графов: " << n + 1 << "\n\n";
		cout << "Среднее время выполнения (микросекунды):\n";

		for (size_t i = 0; i < methodNames.size(); i++) {
			if (i < totalTime.size()) {
				double avgTime = static_cast<double>(totalTime[i]) / (n + 1);
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

// 
void DataStorage::ComparePushRelabelMethods() {
	CompareMethods(100, 0.05, 10000, 25, 0, { "PushRelabelMatrix", "PushRelabelMatrix_HLF",
		"PushRelabelMatrix_v3",
		"PushRelabelEdges",
		});
}

//"DinicMaxFlowMatrix",
void DataStorage::CompareDinicMethods() {
	CompareMethods(1000, 0.005, 10000, 25, 0, { "DinicMaxFlowMatrix",
		"DinicMaxFlowMatrixOptimized", "DinicMaxFlowCapacityScaling", "MaxFlowDinicUnord_map", "MaxFlowDinicUnord_mapCapacityScaling" });
}

void DataStorage::CompareBestMaxFlowMetods() {
	CompareMethods(1000, 0.005, 10000000000, 25, 3, { "PushRelabelMatrix_v3", "PushRelabelEdges", "DinicMaxFlowMatrixOptimized", "MaxFlowDinicUnord_map",
		"DinicMaxFlowCapacityScaling", "MaxFlowDinicUnord_mapCapacityScaling" });
}
//
//100	0.5
//500	0.05
//500	0.5
//1000	0.005


DataStorage::~DataStorage() {
	cout << "DataStorage: start deleting...\n";
}