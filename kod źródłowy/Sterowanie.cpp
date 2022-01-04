// 
// Autor: Maciej Krosman
// Album: 257341
//


#include "Problem.h"
#include "Metoda.h"
#include <iostream>

using namespace std;

int main()
{   
    //pobranie daty i godziny poczatku pracy programu
    struct tm newtime;
    time_t aclock;
    char buffer[32];
    errno_t errNum;
    time(&aclock);
    localtime_s(&newtime, &aclock);
    errNum = asctime_s(buffer, 32, &newtime);

    string date = buffer;
    date.erase(date.end() - 1);
    replace(date.begin(), date.end(), ' ', '_');
    replace(date.begin(), date.end(), ':', '-');
    string txt = ".txt";
    string logFileName = date + txt; // nazwa pliku monitorujacego

    ofstream oLog;
    oLog.open(logFileName.c_str());
    oLog << "Utworzono plik monitorujacy." << endl;

    //wczytywanie rozmiaru tablicy komorek, prawdopodobienstwa przeszkody, kosztu akcji na wolna komorke i przeszkode, ilosc wygenerowan losowej tablicy komorek
    int N, P, K;
    double F, O;
    ifstream ifile;
    ifile.open("config.txt");
    if(!ifile.good()) {
        oLog << "Blad wczytywania pliku konfiguracyjnego. Ustawiono parametry domyslne." << endl;
        cout << "Blad wczytywania pliku konfiguracyjnego. Ustawiono parametry domyslne." << endl;
        N = 10;
        P = 50;
        F = 1.0;
        O = 2.0;
        K = 1;
    }
    else {
        ifile >> N >> P >> F >> O >> K;
            oLog << "Wczytano parametry z pliku konfiguracujnego." << endl;

        if (N < 1) {
            oLog << "Podano za maly rozmiar tablicy komorek N(min 1). Domyslnie zostanie przypisane 10." << endl;
            N = 10;
        }
        if (P > 100 || P < 0) {
            oLog << "Podano wartosc prawdopodobienstwa przeszkod P spoza zakresu [0,100]. P zostanie domyslnie przypisana watosc 50." << endl;
            P = 50;
        }
        if (F <= 0.0) {
            oLog << "Podano za mala wartosc kosztu akcji na wolna komorke, musi ona by wieksza od zera. Zostanie domyslnie przypisana watosc 1.0." << endl;
            F = 1.0;
        }
        if (O <= 0.0 || O < F) {
            oLog << "Podano za mala wartosc kosztu akcji na przeszkode, musi ona by wieksza od zera oraz wieksza od kosztu akcji na wolna komorke. Zostanie domyslnie przypisana watosc 2.0." << endl;
            O = 2.0*F;
        }
        if (K <= 0) {
            oLog << "Podano za mala ilosc tablic komorek do wygenerowania, musi on by niemniejsza niz 1. Zostanie domyslnie przypisana watosc 1." << endl;
            K = 1;
        }

        //przypisanie kosztow akcji z pliku konfiguracyjnego
        actionCost['f'] = F;
        actionCost['o'] = O;
        oLog << "Przypisano nowe koszty akcji." << endl;
    }
    ifile.close();

    srand(time(NULL));
    
    double avgOpenCount1 = 0.0, avgOpenCount2 = 0.0, avgClosedCount1 = 0.0, avgClosedCount2 = 0.0;
    double avgPathSize1 = 0.0, avgPathSize2 = 0.0, avgPathCost1 = 0.0, avgPathCost2 = 0.0;
    double avgHeuristicCost1 = 0.0, avgHeuristicCost2 = 0.0;
    float avgTime1 = 0.0, avgTime2 = 0.0;

    for (int z = 0; z < K; z++) {

        oLog << "/////////////////////////////////////////////////////////////////////" << endl << endl;
        cout << "/////////////////////////////////////////////////////////////////////" << endl << endl;

        //wywolanie funkcji generowania losowej tablicy komorek, wezla startu i dest(cel)
        rt random_terrainResaults = random_terrain(N, P, oLog);

        vector<vector<char>> terrain = random_terrainResaults.terrain;
        Pair start = random_terrainResaults.start;
        Pair dest = random_terrainResaults.dest;

        //wpisanie wylosowanej tablicy komorek do pliku monitorujacego
        oLog << "Wylosowana tablica komorek:" << endl << endl;
        cout << "Wylosowana tablica komorek:" << endl << endl;
        outputTerrain(terrain, oLog);
        oLog << endl;
        cout << endl;

        oLog << "Start: (" << start.first << ',' << start.second << "), Symbol: " << terrain[start.first][start.second] << endl;
        oLog << "Dest: (" << dest.first << ',' << dest.second << "), Symbol: " << terrain[dest.first][dest.second] << endl << endl;
        cout << "Start: (" << start.first << ',' << start.second << "), Symbol: " << terrain[start.first][start.second] << endl;
        cout << "Dest: (" << dest.first << ',' << dest.second << "), Symbol: " << terrain[dest.first][dest.second] << endl << endl;
        double H1 = calcH1(start, dest, actionCost);
        oLog << "Dystans start->dest(cel) w heurystyce 1 = " << H1 << endl;
        cout << "Dystans start->dest(cel) w heurystyce 1 = " << H1 << endl;
        avgHeuristicCost1 += H1;
        double H2 = calcH2(start, dest, actionCost);
        oLog << "Dystans start->dest(cel) w heurystyce 2 = " << H2 << endl << endl;
        cout << "Dystans start->dest(cel) w heurystyce 2 = " << H2 << endl << endl;
        avgHeuristicCost2 += H2;
        
        //wywolanie funkcji algorytmu A* dla funkcji heurystycznej 1 wraz z liczeniem czasu wykonania
        clock_t timer;
        timer = clock();
        aStarValues temp1 = aStar(terrain, start, dest, actionCost, 1, oLog);
        timer = clock() - timer;
        
        //sprawdzenie poprawnosci rozwiazania
        if (temp1.f == -2) {
            oLog << "Wezel start byl niepoprawny, przerwano wykonywanie algorytmu.";
            cout << "Wezel start byl niepoprawny, przerwano wykonywanie algorytmu.";
        }
        else if (temp1.f == -3) {
            oLog << "Wezel dest(cel) byl niepoprawny, przerwano wykonywanie algorytmu.";
            cout << "Wezel dest(cel) byl niepoprawny, przerwano wykonywanie algorytmu.";
        }
        else if (temp1.f == -1) {
            oLog << "Nie znaleziono rozwiazania dla heurystyki 1.";
            cout << "Nie znaleziono rozwiazania dla heurystyki 1.";
        }
        else {

            //wpisanie tablicy komorek z sciezka rozwiazania do pliku monitorujacego
            oLog << "Rozwiazanie dla heurystyki 1:" << endl << endl;
            oLog << "Tablica komorek z sciezka rozwiazania(symbol #):" << endl << endl;
            cout << "Rozwiazanie dla heurystyki 1:" << endl << endl;
            cout << "Tablica komorek z sciezka rozwiazania(symbol #):" << endl << endl;
            outputTerrainWithPath(terrain, temp1.Path, oLog);

            //wpisanie sciezki rozwiazania do pliku monitorujacego
            oLog << endl << "Sciezka od wezla start do wezla dest(cel):" << endl;
            cout << endl << "Sciezka od wezla start do wezla dest(cel):" << endl;
            outputPath(oLog, temp1.Path);
            oLog << endl;
            cout << endl;

            avgPathCost1 += temp1.f;
            avgOpenCount1 += temp1.openListSize;
            avgClosedCount1 += temp1.closedListSize;
            int ps1 = temp1.Path.size();
            avgPathSize1 += ps1;

            float tempTime1 = (float)timer / CLOCKS_PER_SEC * 1000;
            avgTime1 += tempTime1;

            oLog << "Koszt sciezki(S->D): " << temp1.f << endl;
            oLog << "Ilosc wezlow w sciezce(S->D): " << ps1 << endl;
            oLog << "Ilosc wezlow w zbiorze open: " << temp1.openListSize << endl;
            oLog << "Ilosc wezlow w zbiorze closed: " << temp1.closedListSize << endl;
            oLog << "Czas wykonania algorytmu A*: " << tempTime1 << "ms" << endl << endl;

            cout << "Koszt sciezki(S->D): " << temp1.f << endl;
            cout << "Ilosc wezlow w sciezce(S->D): " << ps1 << endl;
            cout << "Ilosc wezlow w zbiorze open: " << temp1.openListSize << endl;
            cout << "Ilosc wezlow w zbiorze closed: " << temp1.closedListSize << endl;
            cout << "Czas wykonania algorytmu A*: " << tempTime1 << "ms" << endl << endl;
        }

        //wywolanie funkcji algorytmu A* dla funkcji heurystycznej 2 wraz z liczeniem czasu wykonania
        timer = clock();
        aStarValues temp2 = aStar(terrain, start, dest, actionCost, 2, oLog);
        timer = clock() - timer;

        //sprawdzenie poprawnosci rozwiazania
        if (temp1.f == -2) {
            oLog << "Wezel start byl niepoprawny, przerwano wykonywanie algorytmu.";
            cout << "Wezel start byl niepoprawny, przerwano wykonywanie algorytmu.";
        }
        else if (temp1.f == -3) {
            oLog << "Wezel dest(cel) jest niepoprawny, przerwano wykonywanie algorytmu.";
            cout << "Wezel dest(cel) jest niepoprawny, przerwano wykonywanie algorytmu.";
        }
        else if (temp1.f == -1) {
            oLog << "Nie znaleziono rozwiazania dla heurystyki 2.";
            cout << "Nie znaleziono rozwiazania dla heurystyki 2.";
        }
        else {

            //wpisanie tablicy komorek z sciezka rozwiazania do pliku monitorujacego
            oLog << "Rozwiazanie dla heurystyki 2:" << endl << endl;
            oLog << "Tablica komorek z sciezka rozwiazania(symbol #):" << endl << endl;
            cout << "Rozwiazanie dla heurystyki 2:" << endl << endl;
            cout << "Tablica komorek z sciezka rozwiazania(symbol #):" << endl << endl;
            outputTerrainWithPath(terrain, temp2.Path, oLog);

            //wpisanie sciezki rozwiazania do pliku monitorujacego
            oLog << endl << "Sciezka od wezla start do wezla dest(cel):" << endl;
            cout << endl << "Sciezka od wezla start do wezla dest(cel):" << endl;
            outputPath(oLog, temp2.Path);
            oLog << endl;
            cout << endl;

            avgPathCost2 += temp2.f;
            avgOpenCount2 += temp2.openListSize;
            avgClosedCount2 += temp2.closedListSize;
            int ps2 = temp2.Path.size();
            avgPathSize2 += ps2;

            float tempTime2 = (float)timer / CLOCKS_PER_SEC * 1000;
            avgTime2 += tempTime2;

            oLog << "Koszt sciezki(S->D): " << temp2.f << endl;
            oLog << "Ilosc wezlow w sciezce(S->D): " << ps2 << endl;
            oLog << "Ilosc wezlow w zbiorze open: " << temp2.openListSize << endl;
            oLog << "Ilosc wezlow w zbiorze closed: " << temp2.closedListSize << endl;
            oLog << "Czas wykonania algorytmu A*: " << tempTime2 << "ms" << endl << endl;

            cout << "Koszt sciezki(S->D): " << temp2.f << endl;
            cout << "Ilosc wezlow w sciezce(S->D): " << ps2 << endl;
            cout << "Ilosc wezlow w zbiorze open: " << temp2.openListSize << endl;
            cout << "Ilosc wezlow w zbiorze closed: " << temp2.closedListSize << endl;
            cout << "Czas wykonania algorytmu A*: " << tempTime2 << "ms" << endl << endl;
        }
    }

    avgHeuristicCost1 = avgHeuristicCost1 / K;
    avgHeuristicCost2 = avgHeuristicCost2 / K;
    avgOpenCount1 = avgOpenCount1 / K;
    avgOpenCount2 = avgOpenCount2 / K;
    avgClosedCount1 = avgClosedCount1 / K;
    avgClosedCount2 = avgClosedCount2 / K;
    avgPathSize1 = avgPathSize1 / K;
    avgPathSize2 = avgPathSize2 / K;
    avgPathCost1 = avgPathCost1 / K;
    avgPathCost2 = avgPathCost2 / K;
    avgTime1 = avgTime1 / K;
    avgTime2 = avgTime2 / K;

    oLog << "Dla zadanego rozmiaru tablicy komorek, prawdopodobienstwa przeszkod oraz kosztow akcji, przy wygenerowaniu losowych "
        << K << " tablic komorek wyliczono:" << endl;
    oLog << "Sredni dystans start->cel w heurystyce 1: " << avgHeuristicCost1 << endl;
    oLog << "Sredni dystans start->cel w heurystyce 2: " << avgHeuristicCost2 << endl;
    oLog << "Sredni koszt sciezki rozwiazania dla funkcji heurystycznej 1: " << avgPathCost1 << endl;
    oLog << "Sredni koszt sciezki rozwiazania dla funkcji heurystycznej 2: " << avgPathCost2 << endl;
    oLog << "Srednia ilosc wezlow w sciezce rozwiazania dla funkcji heurystycznej 1: " << avgPathSize1 << endl;
    oLog << "Srednia ilosc wezlow w sciezce rozwiazania dla funkcji heurystycznej 2: " << avgPathSize2 << endl;
    oLog << "Sredni czas wykonania algorytmu dla funkcji heurystycznej 1: " << avgTime1 << "ms" << endl;
    oLog << "Sredni czas wykonania algorytmu dla funkcji heurystycznej 2: " << avgTime2 << "ms" << endl;
    oLog << "Srednia ilosc wezlow w zbiorze open dla funkcji heurystycznej 1: " << avgOpenCount1 << endl;
    oLog << "Srednia ilosc wezlow w zbiorze open dla funkcji heurystycznej 2: " << avgOpenCount2 << endl;
    oLog << "Srednia ilosc wezlow w zbiorze closed dla funkcji heurystycznej 1: " << avgClosedCount1 << endl;
    oLog << "Srednia ilosc wezlow w zbiorze closed dla funkcji heurystycznej 2: " << avgClosedCount2 << endl;

    cout << "Dla zadanego rozmiaru tablicy komorek, prawdopodobienstwa przeszkod oraz kosztow akcji, przy wygenerowaniu losowych "
        << K << " tablic komorek wyliczono:" << endl;
    cout << "Sredni dystans start->cel w heurystyce 1: " << avgHeuristicCost1 << endl;
    cout << "Sredni dystans start->cel w heurystyce 2: " << avgHeuristicCost2 << endl;
    cout << "Sredni koszt sciezki rozwiazania dla funkcji heurystycznej 1: " << avgPathCost1 << endl;
    cout << "Sredni koszt sciezki rozwiazania dla funkcji heurystycznej 2: " << avgPathCost2 << endl;
    cout << "Srednia ilosc wezlow w sciezce rozwiazania dla funkcji heurystycznej 1: " << avgPathSize1 << endl;
    cout << "Srednia ilosc wezlow w sciezce rozwiazania dla funkcji heurystycznej 2: " << avgPathSize2 << endl;
    cout << "Sredni czas wykonania algorytmu dla funkcji heurystycznej 1: " << avgTime1 << "ms" << endl;
    cout << "Sredni czas wykonania algorytmu dla funkcji heurystycznej 2: " << avgTime2 << "ms" << endl;
    cout << "Srednia ilosc wezlow w zbiorze open dla funkcji heurystycznej 1: " << avgOpenCount1 << endl;
    cout << "Srednia ilosc wezlow w zbiorze open dla funkcji heurystycznej 2: " << avgOpenCount2 << endl;
    cout << "Srednia ilosc wezlow w zbiorze closed dla funkcji heurystycznej 1: " << avgClosedCount1 << endl;
    cout << "Srednia ilosc wezlow w zbiorze closed dla funkcji heurystycznej 2: " << avgClosedCount2 << endl;

    //zamkniecie pliku monitorujacego
    oLog.close();

    cout << endl << "By zakonczyc dzialanie programu nacisnij przycisk enter.";
    getchar();
}