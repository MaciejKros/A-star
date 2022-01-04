// 
// Autor: Maciej Krosman
// Album: 257341
//


#pragma once

#include <vector>
#include <time.h>
#include <cstdlib>
#include <map>
#include <utility>
#include <fstream>
#include <iostream>

using namespace std;

typedef pair<int, int> Pair;

//reprezentacja danych wyjsciowych z funkcji random_terrain
struct rt {
    vector<vector<char>> terrain;
    Pair start;
    Pair dest;
};

//mapa zwracajaca koszt akcji w zaleznosci od symbolu komorki
map<char, double> actionCost = { {'f',1.0},{'o',2.0},{'S',1.0},{'D',1.0} };

//
void outputTerrain(const vector<vector<char>>& terrain, ofstream& output) {
    int s = terrain[0].size();
    for (int i = 0; i < s; i++) {
        for (int j = 0; j < s; j++) {
            output << terrain[i][j];
            cout<< terrain[i][j];
        }
        output << endl;
        cout << endl;
    }
}

//
void outputTerrainWithPath(const vector<vector<char>>& terrain, vector<Pair> path, ofstream& output) {
    vector<vector<char>> terrainWithPath = terrain;
    int s = path.size();
    for (int i = 1; i < s-1; i++) {
        terrainWithPath[path[i].first][path[i].second] = '#';
    }
    outputTerrain(terrainWithPath, output);
}

//funkcja generujaca losowa tablice symboli reprezentujaca losowe przesztrzen komorek oraz losujaca wezly start i dest(cel)
rt random_terrain(const int& N, const int& P, ofstream& oLog) {

    //generowanie losowej tablicy symboli
    vector<vector<char>> terrain(N, vector<char>(N));
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (rand() % 101 > P) {
                terrain[i][j] = 'f';
            }
            else {
                terrain[i][j] = 'o';
            }
        }
    }
    oLog << "Wylosowano tablice komorek " << N << "x" << N << '.' << endl;

    int x, y;
    Pair start, dest;
    
    //generowanie losowego wezla startu na losowej krawedzi tablicy symboli
    int k = rand() % 4;
    switch (k) {
    case 0:
        y = rand() % N;
        start = { 0,y };
        terrain[0][y] = 'S';
        break;
    case 1:
        x = rand() % N;
        start = { x,0 };
        terrain[x][0] = 'S';
        break;
    case 2:
        y = rand() % N;
        start = { N - 1,y };
        terrain[N - 1][y] = 'S';
        break;
    default:
        x = rand() % N;
        start = { x,N - 1 };
        terrain[x][N - 1] = 'S';
        break;
    }
    oLog << "Wylosowano wezel start: (" << start.first << ',' << start.second << ')' << endl;

    //generowanie losowego wezla dest(cel) na losowej krawedzi tablicy symboli
    k = rand() % 4;
    switch (k) {
    case 0:
        y = rand() % N;
        dest = { 0,y };
        terrain[0][y] = 'D';
        break;
    case 1:
        x = rand() % N;
        dest = { x,0 };
        terrain[x][0] = 'D';
        break;
    case 2:
        y = rand() % N;
        dest = { N - 1,y };
        terrain[N - 1][y] = 'D';
        break;
    default:
        x = rand() % N;
        dest = { x,N - 1 };
        terrain[x][N - 1] = 'D';
        break;
    }
    oLog << "Wylosowano wezel dest(cel): (" << dest.first << ',' << dest.second << ')' << endl;
    
    
    return { terrain, start, dest };
}