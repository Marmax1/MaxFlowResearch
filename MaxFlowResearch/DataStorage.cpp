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

void DataStorage::TransformGraphToDirected() {
	iter->TransformToDirected();
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
	cout << iter->getMaxFlowPushRelabel_v2(0, iter->GetCountOfNodes() - 1) << '\n';
	return iter->getMaxFlowPushRelabel_v3(0, iter->GetCountOfNodes() - 1);
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
	cout << iter->getMaxFlowDinic(0, iter->GetCountOfNodes() - 1) << '\n';
	DeleteGraph();
	iter = --graphs.end();

	CreateCopyGraph(iter->GetName() + "Copy");
	maxFlow = iter->getMaxFlowDinicWithEdges(0, iter->GetCountOfNodes() - 1);
	DeleteGraph();
	iter = --graphs.end();
	return maxFlow;
}

long long DataStorage::MaxFlowWO(string s, string t) {
	if (!iter->IsOriented() || !iter->IsWeighted())
		throw string("Граф должен быть ориентированным и взвешенным");

	if (!(iter->IsNodeExist(s) && iter->IsNodeExist(t)))
		throw string("Какой-то из вершин не существует");

	return iter->MaxFlowWO(s, t);
}

long long DataStorage::gargKonemannMaxFlow() {
	if (!iter->IsOriented() || !iter->IsWeighted())
		throw string("Граф должен быть ориентированным и взвешенным");

	if (!(iter->IsNodeExist("source") && iter->IsNodeExist("sink")))
		throw string("Какой-то из вершин не существует");

	return iter->gargKonemannMaxFlow(0, iter->GetCountOfNodes() - 1);
}

void DataStorage::WriteSpeedExecutionMaxFlowMethodsFor(unsigned int countNodes, float density, unsigned int maxWeightValue, unsigned int countGraphs) {
	string s = "source";
	string t = "sink";

	long long maxFlow;
	auto start = chrono::steady_clock::now();
	auto end = chrono::steady_clock::now();

	int n = 0;

	vector<long long> totalTime (9);

	vector<string> methodNames = {
		"FordFulkerson (матрица)",
		"PushRelabelMatrix",
		"PushRelabelMatrix_v2",
		"PushRelabelMatrix_v3",
		"PushRelabelParallel",
		//"ApproximatorMaxFlowSherman",
		"DinicMaxFlow unordered_map",
		"DinicMaxFlowMatrix",
		"DinicMaxFlowWithEdges",
		"gargKonemannMaxFlow"
	};
	
	while (n++ < countGraphs) {
		CreateRandomFlowGraph(countNodes, density, maxWeightValue);

		int currentCountNodes = iter->GetCountOfNodes();

		// алгоритмы FordFulkerson

		start = chrono::steady_clock::now();
		maxFlow = iter->FordFulkersonMatrix(s, t);
		end = chrono::steady_clock::now();
		totalTime[0] += chrono::duration_cast<chrono::microseconds>(end - start).count();

		// алгоритмы PushRelabel
		start = chrono::steady_clock::now();
		maxFlow = iter->getMaxFlowPushRelabel(0, currentCountNodes - 1);
		end = chrono::steady_clock::now();
		totalTime[1] += chrono::duration_cast<chrono::microseconds>(end - start).count();

		start = chrono::steady_clock::now();
		maxFlow = iter->getMaxFlowPushRelabel_v2(0, currentCountNodes - 1);
		end = chrono::steady_clock::now();
		totalTime[2] += chrono::duration_cast<chrono::microseconds>(end - start).count();

		start = chrono::steady_clock::now();
		maxFlow = iter->getMaxFlowPushRelabel_v3(0, currentCountNodes - 1);
		end = chrono::steady_clock::now();
		totalTime[3] += chrono::duration_cast<chrono::microseconds>(end - start).count();

		start = chrono::steady_clock::now();
		maxFlow = iter->PushRelabelParallel(s, t);
		end = chrono::steady_clock::now();
		totalTime[4] += chrono::duration_cast<chrono::microseconds>(end - start).count();


		//// ApproximatorMaxFlowSherman
		//start = chrono::steady_clock::now();
		////maxFlow = iter->approximatorMaxflow(0, currentCountNodes - 1);
		//end = chrono::steady_clock::now();
		//totalTime[6] += chrono::duration_cast<chrono::microseconds>(end - start).count();

		// алгоритмы Dinic
		/*start = chrono::steady_clock::now();
		maxFlow = iter->DinicMaxFlow(s, t);
		end = chrono::steady_clock::now();
		totalTime[6] += chrono::duration_cast<chrono::microseconds>(end - start).count();*/


		CreateCopyGraph(iter->GetName() + "Copy");
		start = chrono::steady_clock::now();
		maxFlow = iter->DinicMaxFlowMatrix(s, t);
		end = chrono::steady_clock::now();
		totalTime[5] += chrono::duration_cast<chrono::microseconds>(end - start).count();
		DeleteGraph();
		iter = --graphs.end();

		CreateCopyGraph(iter->GetName() + "Copy");
		start = chrono::steady_clock::now();
		maxFlow = iter->getMaxFlowDinic(0, currentCountNodes - 1);
		end = chrono::steady_clock::now();
		totalTime[6] += chrono::duration_cast<chrono::microseconds>(end - start).count();
		DeleteGraph();
		iter = --graphs.end();

		CreateCopyGraph(iter->GetName() + "Copy");
		start = chrono::steady_clock::now();
		maxFlow = iter->getMaxFlowDinicWithEdges(0, currentCountNodes - 1);
		end = chrono::steady_clock::now();
		totalTime[7] += chrono::duration_cast<chrono::microseconds>(end - start).count();
		DeleteGraph();
		iter = --graphs.end();

		CreateCopyGraph(iter->GetName() + "Copy");
		start = chrono::steady_clock::now();
		maxFlow = iter->gargKonemannMaxFlow(0, currentCountNodes - 1);
		end = chrono::steady_clock::now();
		totalTime[8] += chrono::duration_cast<chrono::microseconds>(end - start).count();
		DeleteGraph();
		iter = --graphs.end();

		system("cls");

		cout << "--------------------------------------------------------------------------------------\n";
		cout << "Сравнение методов для графов с " << countNodes << " вершинами и плотностью " << density << '\n\n';
		cout << "Количество тестовых графов: " << n << "\n\n";
		cout << "Среднее время выполнения (микросекунды):\n";

		for (size_t i = 0; i < methodNames.size(); i++) {
			if (i < totalTime.size()) {
				double avgTime = static_cast<double>(totalTime[i]) / n;
				cout << methodNames[i] << ": " << avgTime / 1000 << " мс\n";
			}
		}
		cout << "--------------------------------------------------------------------------------------\n";
	}
}

void DataStorage::CompareMaxFlowMetods() {
	WriteSpeedExecutionMaxFlowMethodsFor(200, 0.5, 10000, 20);
	//WriteSpeedExecutionMaxFlowMethodsFor(300, 0.3, 100);
	//WriteSpeedExecutionMaxFlowMethodsFor(300, 0.8, 100);
}


//// Finding the shortest paths
//
//void DataStorage::WriteShortestPathByBFSFor01Graph(string from, string to) {
//	if (!(iter->IsNodeExist(from) && iter->IsNodeExist(to)))
//		throw string("Какой-то из вершин не существует");
//
//	auto start = chrono::steady_clock::now();
//	auto shortestWay = iter->GetShortestPathByBFSDeque(from, to);
//	auto end = chrono::steady_clock::now();
//	cout << "Elapsed time in microseconds: "
//		<< chrono::duration_cast<chrono::microseconds>(end - start).count()
//		<< " µs" << endl;
//
//	if (shortestWay.first.empty()) {
//		cout << "Пути из " << from << " в " << to << " не существует\n";
//		return;
//	}
//
//	cout << "Кратчайший путь из " << from << " в " << to << ":\n";
//
//	for (int i = 0; i < shortestWay.first.size() - 1; i++) {
//		cout << shortestWay.first[i] << " -> ";
//	}
//	cout << shortestWay.first.back() << '\t' << "Длина: " << shortestWay.second << '\n';
//}
//
//void DataStorage::WriteShortestPathByBFSFor12Graph(string from, string to) {
//	if (!(iter->IsNodeExist(from) && iter->IsNodeExist(to)))
//		throw string("Какой-то из вершин не существует");
//
//	Graph withoutWeight2 = iter->SplitEdgesWithWeightN(2);
//
//	auto start = chrono::steady_clock::now();
//	auto shortestWay = withoutWeight2.GetShortestPathByBFS(from, to);
//	auto end = chrono::steady_clock::now();
//	cout << "Elapsed time in microseconds: "
//		<< chrono::duration_cast<chrono::microseconds>(end - start).count()
//		<< " µs" << endl;
//
//	if (shortestWay.first.empty()) {
//		cout << "Пути из " << from << " в " << to << " не существует\n";
//		return;
//	}
//
//	cout << "Кратчайший путь из " << from << " в " << to << ":\n";
//
//	for (int i = 0; i < shortestWay.first.size() - 1; i++) {
//		if (!iter->IsNodeExist(shortestWay.first[i]))
//			continue;
//		cout << shortestWay.first[i] << " -> ";
//	}
//	cout << shortestWay.first.back() << '\t' << "Длина: " << shortestWay.second << '\n';
//}
//
//void DataStorage::WriteSpeedExecutionOfDFSAndDijkstraMethods() {
//	string name = "RandomGraph";
//	int secondPartName = 1;
//	while (IsGraphExist(name + to_string(secondPartName))) {
//		secondPartName++;
//	}
//	name = name + to_string(secondPartName);
//	bool weighted = true;
//	bool oriented = false;
//	vector<long long> weightValues = { 0, 1 };
//
//	// Compare method for different graphs
//
//	cout << "\nFor small (4-24 nodes) sparse undirected 0-1 graph:\n";
//	CompareMethodsFindingShortestPath([]() -> int {return rand() % 21 + 4; }, [](int countNodes) -> int {return rand() % (countNodes * (countNodes - 1) / 4); },
//		name, weighted, oriented, weightValues);
//
//	cout << "\nFor small (4-24 nodes) dense undirected 0-1 graph:\n";
//	CompareMethodsFindingShortestPath([]() -> int {return rand() % 21 + 4; }, [](int countNodes) -> int {int countEdges = (countNodes * (countNodes - 1) / 2); return rand() % (countEdges / 2) + (countEdges - countEdges / 2) + 1; },
//		name, weighted, oriented, weightValues);
//
//	cout << "\nFor large (40-80 nodes) sparse undirected 0-1 graph:\n";
//	CompareMethodsFindingShortestPath([]() -> int {return rand() % 41 + 40; }, [](int countNodes) -> int {return rand() % (countNodes * (countNodes - 1) / 4); },
//		name, weighted, oriented, weightValues);
//
//	cout << "\nFor large (40-80 nodes) dense undirected 0-1 graph:\n";
//	CompareMethodsFindingShortestPath([]() -> int {return rand() % 41 + 40; }, [](int countNodes) -> int {int countEdges = (countNodes * (countNodes - 1) / 2); return rand() % (countEdges / 2) + (countEdges - countEdges / 2) + 1; },
//		name, weighted, oriented, weightValues);
//
//	oriented = true;
//
//	cout << "\nFor small (4-24 nodes) sparse directed 0-1 graph:\n";
//	CompareMethodsFindingShortestPath([]() -> int {return rand() % 21 + 4; }, [](int countNodes) -> int {return rand() % (countNodes * countNodes / 2); },
//		name, weighted, oriented, weightValues);
//
//	cout << "\nFor small (4-24 nodes) dense directed 0-1 graph:\n";
//	CompareMethodsFindingShortestPath([]() -> int {return rand() % 21 + 4; }, [](int countNodes) -> int {int countEdges = countNodes * countNodes; return rand() % (countEdges / 2) + (countEdges - countEdges / 2) + 1; },
//		name, weighted, oriented, weightValues);
//
//	cout << "\nFor large (40-80 nodes) sparse directed 0-1 graph:\n";
//	CompareMethodsFindingShortestPath([]() -> int {return rand() % 41 + 40; }, [](int countNodes) -> int {return rand() % (countNodes * (countNodes - 1) / 2); },
//		name, weighted, oriented, weightValues);
//
//	cout << "\nFor large (40-80 nodes) dense directed 0-1 graph:\n";
//	CompareMethodsFindingShortestPath([]() -> int {return rand() % 41 + 40; }, [](int countNodes) -> int {int countEdges = countNodes * countNodes; return rand() % (countEdges / 2) + (countEdges - countEdges / 2) + 1; },
//		name, weighted, oriented, weightValues);
//
//	oriented = false;
//	weightValues = { 1, 2 };
//
//	cout << "\nFor small (4-24 nodes) sparse undirected 1-2 graph:\n";
//	CompareMethodsFindingShortestPath([]() -> int {return rand() % 21 + 4; }, [](int countNodes) -> int {return rand() % (countNodes * (countNodes - 1) / 4); },
//		name, weighted, oriented, weightValues);
//
//	cout << "\nFor small (4-24 nodes) dense undirected 1-2 graph:\n";
//	CompareMethodsFindingShortestPath([]() -> int {return rand() % 21 + 4; }, [](int countNodes) -> int {int countEdges = (countNodes * (countNodes - 1) / 2); return rand() % (countEdges / 2) + (countEdges - countEdges / 2) + 1; },
//		name, weighted, oriented, weightValues);
//
//	cout << "\nFor large (40-80 nodes) sparse undirected 1-2 graph:\n";
//	CompareMethodsFindingShortestPath([]() -> int {return rand() % 41 + 40; }, [](int countNodes) -> int {return rand() % (countNodes * (countNodes - 1) / 4); },
//		name, weighted, oriented, weightValues);
//
//	cout << "\nFor large (40-80 nodes) dense undirected 1-2 graph:\n";
//	CompareMethodsFindingShortestPath([]() -> int {return rand() % 41 + 40; }, [](int countNodes) -> int {int countEdges = (countNodes * (countNodes - 1) / 2); return rand() % (countEdges / 2) + (countEdges - countEdges / 2) + 1; },
//		name, weighted, oriented, weightValues);
//
//	oriented = true;
//
//	cout << "\nFor small (4-24 nodes) sparse directed 1-2 graph:\n";
//	CompareMethodsFindingShortestPath([]() -> int {return rand() % 21 + 4; }, [](int countNodes) -> int {return rand() % (countNodes * countNodes / 2); },
//		name, weighted, oriented, weightValues);
//
//	cout << "\nFor small (4-24 nodes) dense directed 1-2 graph:\n";
//	CompareMethodsFindingShortestPath([]() -> int {return rand() % 21 + 4; }, [](int countNodes) -> int {int countEdges = countNodes * countNodes; return rand() % (countEdges / 2) + (countEdges - countEdges / 2) + 1; },
//		name, weighted, oriented, weightValues);
//
//	cout << "\nFor large (40-80 nodes) sparse directed 1-2 graph:\n";
//	CompareMethodsFindingShortestPath([]() -> int {return rand() % 41 + 40; }, [](int countNodes) -> int {return rand() % (countNodes * (countNodes - 1) / 2); },
//		name, weighted, oriented, weightValues);
//
//	cout << "\nFor large (40-80 nodes) dense directed 1-2 graph:\n";
//	CompareMethodsFindingShortestPath([]() -> int {return rand() % 41 + 40; }, [](int countNodes) -> int {int countEdges = countNodes * countNodes; return rand() % (countEdges / 2) + (countEdges - countEdges / 2) + 1; },
//		name, weighted, oriented, weightValues);
//}

DataStorage::~DataStorage() {
	cout << "DataStorage: start deleting...\n";
}