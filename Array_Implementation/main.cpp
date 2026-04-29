#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include "ResidentArray.h"

using namespace std;

void loadCSV(string filename, ResidentArray& resArray) {
    ifstream file(filename);
    string line, word;
    getline(file, line); // Skip header

    while (getline(file, line)) {
        stringstream s(line);
        Resident r;
        getline(s, r.residentID, ',');
        string temp;
        getline(s, temp, ','); r.age = stoi(temp);
        getline(s, r.mode, ',');
        getline(s, temp, ','); r.dailyDistance = stod(temp);
        getline(s, temp, ','); r.emissionFactor = stod(temp);
        getline(s, temp, ','); r.daysPerMonth = stoi(temp);
        resArray.add(r);
    }
}

void displayResults(ResidentArray& resArray) {
    cout << left << setw(10) << "ID" << setw(5) << "Age" << setw(12) << "Mode" 
         << "Emission (kg CO2)" << endl;
    cout << "---------------------------------------------" << endl;
    for (int i = 0; i < resArray.size(); i++) {
        Resident& r = resArray.get(i);
        cout << left << setw(10) << r.residentID 
             << setw(5) << r.age 
             << setw(12) << r.mode 
             << r.calculateMonthlyEmission() << endl;
    }
}

int main() {
    ResidentArray cityA(1000);

    // Track execution time [cite: 111]
    auto start = chrono::high_resolution_clock::now();
    
    loadCSV("dataset1-cityA.csv", cityA);
    cityA.sortByAge();

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end - start;

    displayResults(cityA);
    cout << "\nOperation completed in: " << elapsed.count() << " seconds." << endl;

    return 0;
}
