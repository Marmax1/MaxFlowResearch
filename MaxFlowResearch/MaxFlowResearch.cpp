#include <iostream>
#include "DataStorage.h"
#include <windows.h>
#include <atomic>
#include <vector>
#include <omp.h>

using namespace std;


DataStorage myBase;

class MenuBase {
public:
	virtual int Menu() = 0;
	static bool isExit;

protected:
	static bool AskYesNo(string question, bool& ans) {
		cout << question;
		cout << "\n0. нет\n1. да\n2. отмена\n";

		int choice;
		(cin >> choice).get();			// что делать, если введут не число?
		if (choice == 0) {
			ans = false;
		}
		else if (choice == 1) {
			ans = true;
		}
		else if (choice == 2) {
			return false;
		}
		else {
			cout << "Ошибка. Попробуйте заново\n";
			return false;
		}
		return true;
	};
	int countPages;
	int curPage;
	static int comm;
	static int lastMenu;
};

int MenuBase::comm = -1;
bool MenuBase::isExit = false;
int MenuBase::lastMenu = -1;

class ActionSet0 : public MenuBase { // добавление графов
public:
	bool isWeighted;
	bool isOriented;

	virtual int Menu() override {
		cout << "\nВозможные команды:\n";
		cout << "1. Создать случайный сетевой граф\n";
		cout << "2. Сравнить скорости всех алгоритмов MaxFlow\n";
		if (lastMenu != -1) {
			cout << "9. Вернуться ко всем графам\n";
		}
		cout << "0. Выход\n";
		string name;
		(cin >> comm).get();			// get для удаления символа \n
		system("cls");
		switch (comm) {
		case 0:
			isExit = true;
			return 0;
		case 1:
			try {
				myBase.CreateRandomFlowGraph();
			}
			catch (string errorMessage) {
				cout << errorMessage << '\n';
				return 0;
			}
			lastMenu = 0;
			return 1;
		case 2:
			myBase.CompareMaxFlowMetods();
			return 0;
		case 9:
			if (lastMenu == -1)
				return 0;
			lastMenu = 0;
			return 1;
		default:
			return 0;
		}
	}
};

class ActionSet1 : public MenuBase { // работа с графами
public:
	virtual int Menu() override {
		cout << "\nТекущий граф:\n";
		myBase.WriteToConsole();
		cout << "Возможные команды:\n";
		cout << "1. Вывести все графы\n";
		cout << "2. Добавить граф\n";
		cout << "3. Добавить копию текущего графа\n";
		cout << "4. Выбрать граф\n";
		cout << "5. Изменить текущий граф\n";
		cout << "6. Удалить текущий граф\n";
		cout << "7. Выбрать специфичные методы\n";
		cout << "0. Выход\n";
		lastMenu = 1;
		string nameGraph;
		(cin >> comm).get();
		system("cls");
		switch (comm) {
		case 0:
			isExit = true;
			return 0;
		case 1:
			myBase.WriteToConsoleAll();
			return 1;
		case 2:
			return 0;
		case 3:
			cout << "Введите название нового графа: ";
			getline(cin, nameGraph);
			try {
				myBase.CreateCopyGraph(nameGraph);
			}
			catch (string errorMessage) {
				cout << errorMessage << '\n';
				return 1;
			}
			return 2;
		case 4:
			cout << "Введите название графа: ";
			getline(cin, nameGraph);
			try {
				myBase.SetGraph(nameGraph);
			}
			catch (string errorMessage) {
				cout << errorMessage << '\n';
			}
			return 1;
		case 5:
			return 2;
		case 6:
			myBase.DeleteGraph();
			if (myBase.Size() == 0) {
				cout << "В базе больше нет графов\n";
				lastMenu = -1;
				return 0;
			}
			return 1;
		case 7:
			return 3;
		default:
			return 1;
		}
	}
};

class ActionSet2 : public MenuBase { // изменение конкретного графа
public:
	virtual int Menu() override {
		cout << "\nТекущий граф:\n";
		myBase.WriteToConsole();
		cout << "Возможные команды:\n";
		cout << "1. Добавить вершину\n";
		if (myBase.IsOriented())
			cout << "2. Добавить дугу\n";
		else
			cout << "2. Добавить ребро\n";
		cout << "3. Удалить вершину\n";
		if (myBase.IsOriented())
			cout << "4. Удалить дугу\n";
		else
			cout << "4. Удалить ребро\n";
		cout << "9. Вернуться ко всем графам\n";
		cout << "0. Выход\n";

		string name1, name2;
		int num = 0;
		(cin >> comm).get();
		system("cls");
		switch (comm) {
		case 0:
			isExit = true;
			return 0;
		case 1:
			cout << "\nТекущий граф:\n";
			myBase.WriteToConsole();
			cout << "Введите название вершины: ";
			getline(cin, name1);
			try {
				myBase.AddNode(name1);
			}
			catch (string errorMessage) {
				cout << errorMessage << '\n';
			}
			return 2;
		case 2:
			cout << "\nТекущий граф:\n";
			myBase.WriteToConsole();

			AskAboutWay(name1, name2, num);

			try {
				myBase.AddEdge(name1, name2, num);
			}
			catch (int error) {
				bool ans;
				if (!AskYesNo("Какой-то из вершин нет. Добавить их в граф?", ans)) {
					return 2;
				}

				if (ans) {
					if (error == 1) {
						myBase.AddNode(name1);
					}
					else if (error == 2) {
						myBase.AddNode(name2);
					}
					else if (error == 3) {
						myBase.AddNode(name1);
						myBase.AddNode(name2);
					}
				}
				else {
					return 2;
				}

				myBase.AddEdge(name1, name2, num);
			}
			catch (string errorMessage) {
				cout << errorMessage << '\n';

				if (errorMessage[0] == 'Н') {
					return 2;
				}

				if (!myBase.IsWeighted()) {
					return 2;
				}

				bool ans;
				if (!AskYesNo("Хотите заменить вес дуги?", ans)) {
					return 2;
				}

				if (ans) {
					myBase.ChangeValueWay(name1, name2, num);
				}
				else {
					return 2;
				}
			}
			return 2;
		case 3:
			cout << "\nТекущий граф:\n";
			myBase.WriteToConsole();

			cout << "Введите название вершины: ";
			getline(cin, name1);
			try {
				myBase.DeleteNode(name1);
			}
			catch (string errorMessage) {
				cout << errorMessage << '\n';
			}
			return 2;
		case 4:
			cout << "\nТекущий граф:\n";
			myBase.WriteToConsole();

			int value;
			AskAboutWay(name1, name2, value, false);
			try {
				myBase.DeleteEdge(name1, name2);
			}
			catch (string errorMessage) {
				cout << errorMessage << '\n';
			}
			return 2;
		case 9:
			return 1;
		default:
			return 2;
		}
	}
private:
	void AskAboutWay(string& from, string& to, int& value, bool isForAdd = true) {

		int type = 0;
		if (myBase.IsWeighted() && isForAdd) {
			type += 1;
		}
		if (myBase.IsOriented()) {
			type += 2;
		}

		switch (type) {
		case 0:
			cout << "Введите ребро в формате: A B\n";
			cout << "A и B - название вершин\n";
			(cin >> from >> to).get();
			value = 1;
			break;
		case 1:
			cout << "Введите ребро в формате: A B N\n";
			cout << "A и B - вершины, между которыми будет ребро, N - его вес\n";
			(cin >> from >> to >> value).get();
			break;
		case 2:
			cout << "Введите дугу в формате: A B\n";
			cout << "A - вершина, откуда идёт дуга, B - куда идёт\n";
			(cin >> from >> to).get();
			value = 1;
			break;
		case 3:
			cout << "Введите дугу в формате: A B N\n";
			cout << "A - вершина, откуда идёт дуга, B - куда идёт, N - его вес\n";
			(cin >> from >> to >> value).get();
			break;
		default:
			break;
		}
	}
};

class ActionSet3 : public MenuBase { // специфичные задания для конктретного графа
public:

	virtual int Menu() override {
		cout << "\nТекущий граф:\n";
		myBase.WriteToConsole();
		cout << "Возможные команды:\n";
		cout << "1. Найти максимальный поток из вершины s в t FordFulkerson\n";
		cout << "2. Найти максимальный поток из вершины s в t PushRelabel\n";
		cout << "3. Найти максимальный поток из вершины s в t Dinic\n";
		cout << "4. Найти приблизительный максимальный поток из вершины s в t gargKonemann\n";
		cout << "5. Сравнить скорости всех алгоритмов MaxFlow\n";
		cout << "8. Переход в неориентированный граф\n";
		cout << "9. Вернуться ко всем графам\n";
		cout << "0. Выход\n";
		lastMenu = 3;
		(cin >> comm).get();
		system("cls");
		int num;
		try {
			switch (comm) {
			case 0:
				isExit = true;
				return 0;
			case 1:
				cout << "\nТекущий граф:\n";
				myBase.WriteToConsole();
				cout << "Введите s и t через пробел\n";
				cout << "Максимальный поток равен " << myBase.GetMaxFlowFordFulkerson("source", "sink") << '\n';
				return 3;
			case 2:
				cout << "\nТекущий граф:\n";
				myBase.WriteToConsole();
				cout << "Максимальный поток равен " << myBase.GetMaxFlowPushRelabel() << '\n';
				return 3;
			case 3:
				cout << "\nТекущий граф:\n";
				myBase.WriteToConsole();
				cout << "Максимальный поток равен " << myBase.GetMaxFlowDinic("source", "sink") << '\n';
				return 3;
			case 4:
				cout << "\nТекущий граф:\n";
				myBase.WriteToConsole();
				cout << "Максимальный поток равен " << myBase.gargKonemannMaxFlow() << '\n';
				return 3;
			case 5:
				cout << "\nТекущий граф:\n";
				myBase.WriteToConsole();
				myBase.CompareMaxFlowMetods();
				return 3;
			case 8:
				myBase.TransformGraphToDirected();
				cout << "\nТекущий граф:\n";
				myBase.WriteToConsole();
				return 3;
			case 9:
				return 1;
			default:
				return 3;
			}
		}
		catch (string errorMessage) {
			cout << errorMessage << '\n';
			return 3;
		}
	}
};

int main()
{
	/*setlocale(LC_ALL, "Rus");*/
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	srand(time(NULL));

	ActionSet0 as0;
	ActionSet1 as1;
	ActionSet2 as2;
	ActionSet3 as3;
	MenuBase* menus[] = { &as0, &as1, &as2, &as3 };
	omp_set_num_threads(4);

	int currentMenu = 0;
	while (!menus[currentMenu]->isExit) {
		currentMenu = menus[currentMenu]->Menu();
	}

	return 0;
}