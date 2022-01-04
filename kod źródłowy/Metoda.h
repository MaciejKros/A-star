// 
// Autor: Maciej Krosman
// Album: 257341
//


#pragma once

#include <utility>
#include <fstream>
#include <iostream>
#include <list>
#include <algorithm>


using namespace std;

typedef pair<int, int> Pair;
typedef tuple<double, int, int> Tuple;

//wezel reprezentujacy komorki odwiedzone i nastepnikow
struct cell {
	double g, h, f;
	Pair parent;
	cell() :
		parent(-1, -1),
		f(-1),
		g(-1),
		h(-1)
	{}
};

//reprezentacja danych wyjsciowych z funkcji algorytmu A*
struct aStarValues {
	vector<Pair> Path;
	double f;
	int openListSize;
	int closedListSize;
};

//funkcja sprawdza czy wezel nalezy do tablicy komorek 
bool isValid(const int& N, const Pair& current) {
	if ( N > 0 )
		return (current.first >= 0) && (current.first < N) && (current.second >= 0) && (current.second < N);
	return false;
}

//funkcja sprawdza osiagniecie celu
bool isDest(const Pair& current, const Pair& dest) {
	return (current.first == dest.first && current.second == dest.second);
}

//pierwsza funkcja heurystyczna
double calcH1(const Pair& current, const Pair& dest, map<char, double> actionCost) {
	return actionCost['f'] * sqrt(pow(abs(current.first - dest.first), 2) + pow(abs(current.second - dest.second), 2));
}

//druga funkcja heurystyczna
double calcH2(const Pair& current, const Pair& dest, map<char, double> actionCost) {
	return actionCost['f'] * max(abs(current.first - dest.first), abs(current.second - dest.second));
}

//funkcja buduje sciezke
vector<Pair> makePath(const Pair& dest, const vector<vector<cell>>& cells) {
	
	vector<Pair> Path;
	
	Pair temp1 = dest;
	Pair nextP = cells[temp1.first][temp1.second].parent;
	
	//poczawszy od konca umieszcza wezly w tablicy sciezki
	while (temp1 != nextP){
		Path.insert(Path.begin(), temp1);
		temp1 = nextP;
		nextP = cells[temp1.first][temp1.second].parent;
	} 
	Path.insert(Path.begin(), temp1);

	return Path;
}

//funckja wylicza ilosc elementow w zbiorze closed
int countClosed(const vector<vector<bool>>& closed) {
	int n = closed[0].size();
	int k = 0;
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			if (closed[i][j])
				k++;
		}
	}
	return k;
}

//funkcja wypisuje do pliku wynikowego sciezke wyliczona przez algorytm A*
void outputPath(ofstream& output, vector<Pair> path) {
	int s = path.size();	
	for (int i = 0; i < s;i++) {
		output << '(' << path[i].first << ',' << path[i].second << ") ";
		if (i % 10 == 9)
			output << endl;
	}
	if (s % 10 != 0) 
		output << endl;
}

//funkcja wlasciwa implementujaca algorytm A*
aStarValues aStar(const vector<vector<char>>& terrain, const Pair& start, const Pair& dest, map<char, double> actionCost, const int& heuristic, ofstream& oLog) {

	oLog << "Poczatek wykonywania algorytmu A* dla funkcji heurystycznej "<< heuristic << "." << endl;
	vector<Pair> path;// sciezka wyjsciowa
	int N = terrain[0].size();

	//weryfikacjia wezlow start i dest(cel) oraz czy start jest dest(cel)
	oLog << "Sprawdzenie poprawnosci wezlow start i dest(cel)." << endl;
	if (!isValid(N, start)) {
		path.insert(path.begin(), { -2,-2 });
		oLog << "Wezel start jest niepoprawny." << endl;
		return { path,-2.0,-2,-2 };
	}

	if (!isValid(N, dest)) {
		path.insert(path.begin(), { -3,-3 });
		oLog << "Wezel dest(cel) jest niepoprawny." << endl;
		return { path,-3.0,-3,-3 };
	}

	oLog << "Sprawdzenie czy wezel start to dest(cel)." << endl;
	if (isDest(start, dest)) {
		path.insert(path.begin(), start);
		oLog << "Wezel start jest taki sam jak wezel dest(cel)." << endl;
		return { path,0.0,0,0 };
	}


	//reprezentacja zbioru closed poprzez tablice bool, gdzie wartosc true jest wezlem znajdujacym sie w zbiorze
	vector<vector<bool>> closed(N, vector<bool>(N));
	for (int i = 0; i < N; i++) {
		fill(closed[i].begin(), closed[i].end(), false);
	}
	oLog << "Inicjalizacja zbioru closed." << endl;

	// tablica pomocnicza przechowywujaca najlepsze wartosci dla wezlow
	vector<vector<cell>> cells(N, vector<cell>(N));
	oLog << "Inicjalizacja tablicy pomocniczej." << endl;

	//dodanie wezla start do tablicy pomocniczej
	cells[start.first][start.second].f = 0.0;
	cells[start.first][start.second].g = 0.0;
	cells[start.first][start.second].h = 0.0;
	cells[start.first][start.second].parent = { start.first,start.second };
	oLog << "Dodanie wezla start do tablicy pomocniczej." << endl;
	
	//inicjalizacja zbioru open i dodanie do niego wezla start
	list<Tuple> open;
	open.push_front(Tuple( cells[start.first][start.second].f, start.first, start.second ));
	oLog << "Inicjalizacja i dodanie wezla start do zbioru open." << endl;
	
	int i, j;

	while (!open.empty()) {
		
		open.sort(less<Tuple>()); //posortowanie open od najmniejszej wartosci f
		oLog << "Posortowanie listy open malejaco wzgledem wartosci f." << endl;
		const Tuple& p = open.front(); //najlepszy wezel ze zbioru open
		i = get<1>(p);
		j = get<2>(p);
		oLog << "Wezel o najmniejszym f: (" << i << ',' << j << "), f=" << get<0>(p) << endl;
		oLog << "Sciezka do wezla (" << i << ',' << j << "): " << endl;
		outputPath(oLog, makePath({ i,j }, cells));

		//sprawdzenie osiagniecia celu
		oLog << "Sprawdzenie czy wezel o najmniejszym f jest dest(cel)." << endl;
		if (isDest({ i,j }, dest)) {
			oLog << "Wezel: (" << i << ',' << j << ") jest dest(cel)." << endl << endl;
			return { makePath(dest, cells),cells[i][j].f ,(int)open.size() ,countClosed(closed) };
		}

		open.pop_front(); //usuniecie najlepszego wezla ze zbioru open
		oLog << "Usuniecie wezla o najmniejszym f z zbioru open." << endl;
		closed[i][j] = true; //dodanie wezla do zbioru closed
		oLog << "Dodanie wezla o najmniejszym f do zbioru closed." << endl;
		
		//zestaw par wspolrzednych wezlow nastepnikow
		vector<Pair> nextPairs = { {i - 1,j - 1}, {i - 1,j}, {i,j - 1}, {i - 1,j + 1}, {i + 1,j - 1}, {i + 1,j}, {i,j + 1}, {i + 1,j + 1} };
		oLog << "Wygenerowano wspolrzedne nastepnikow." << endl;

		for (Pair temp : nextPairs) {

			oLog << "Nastepnik: (" << temp.first << ',' << temp.second << ")";
			//sprawdzenie poprawnosci nastepnika
			if (isValid(N, temp)) {

				oLog << " jest poprawny, ";
				//wyliczenie wartosci g,h,f dla nastepnika
				double newF, newG, newH;
				newG = cells[i][j].g + actionCost[terrain[temp.first][temp.second]];
				if (heuristic == 1) {
					newH = calcH1(temp, dest, actionCost);
				}
				else {
					newH = calcH2(temp, dest, actionCost);
				}
				newF = newG + newH;
				oLog << "g=" << newG << ", h=" << newH << ", f=" << newF << endl;
				
				//sprawdzenie czy nastepnik nie nalezy do zbiorow open i closed
				if (cells[temp.first][temp.second].f == -1){

					oLog << "Nastepnik: (" << temp.first << ',' << temp.second << ") nie nalezy do zbiorow open i closed." << endl;
					//jezeli nie nalezy to jest dodany do open
					cells[temp.first][temp.second].g = newG;
					cells[temp.first][temp.second].h = newH;
					cells[temp.first][temp.second].f = newF;
					cells[temp.first][temp.second].parent = { i,j };
					open.push_front(Tuple(newF, temp.first, temp.second));
					oLog << "Nastepnik: (" << temp.first << ',' << temp.second << ") dodany do zbioru open." << endl;
				}//sprawdzenie czy nowy wezel o tych samych wspolrzednych jest lepszy
				else if (cells[temp.first][temp.second].g > newG) {

					//sprawdzenie czy nalezy do closed czy open
					if (closed[temp.first][temp.second]) {
						oLog << "Nastepnik: (" << temp.first << ',' << temp.second << ") nalezy do zbioru closed oraz ma mniejsza wartosc g." << endl;
						oLog << newG << '<' << cells[temp.first][temp.second].g << endl;

						closed[temp.first][temp.second] = false; //usuniecie gorszego wezla z closed
						oLog << "Nastepnik: (" << temp.first << ',' << temp.second << ") usuniety z zbioru closed" << endl;
					}
					else {

						oLog << "Nastepnik: (" << temp.first << ',' << temp.second << ") nalezy do zbioru open oraz ma mniejsza wartosc g." << endl;
						list<Tuple>::iterator it;
						Tuple tmpTuple = { cells[temp.first][temp.second].f, temp.first, temp.second };
						it = find(open.begin(), open.end(), tmpTuple);
						open.erase(it); //usuniecie gorszego wezla z open
						oLog << "Nastepnik: (" << temp.first << ',' << temp.second << ") usuniety z zbioru open." << endl;
					}
					cells[temp.first][temp.second].g = newG;
					cells[temp.first][temp.second].h = newH;
					cells[temp.first][temp.second].f = newF;
					cells[temp.first][temp.second].parent = { i,j };
					oLog << "Nastepnik: (" << temp.first << ',' << temp.second << ") przypisano lepsze wartosci g, h, f." << endl;
					open.push_front(Tuple(newF, temp.first, temp.second));//dodanie lepszego wezla do open
					oLog << "Nastepnik: (" << temp.first << ',' << temp.second << ") dodany do open" << endl;
				}				
				else {
					oLog << "Nastepnik: (" << temp.first << ',' << temp.second << ") nalezy do zbioru closed lub open ale ma niemniejsza wartosc g." << endl;
				}
			}
			else {
				oLog << " nie jest poprawny." << endl;
			}
		}
	}

	oLog << "Nie znaleziono sciezki S->D." << endl;
	path.insert(path.begin(), { -1,-1 });
	return { path,-1.0,-1,-1 };
}
