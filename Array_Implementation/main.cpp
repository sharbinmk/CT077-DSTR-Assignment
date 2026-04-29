#ifndef DATALOADER_H
#define DATALOADER_H

#include "Resident.h"
#include "ResidentArray.h"
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <iomanip>
#include <string>
#include <limits>
#include "Resident.h"
#include "ResidentArray.h"
using namespace std;

// ---- Helper: strip leading/trailing whitespace and \r from string ----
static string trimStr(const string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// ---- Load a CSV file into a ResidentArray -------------------------
//  filename : path to CSV file
//  cityName : label to tag each resident with (e.g., "City A")
//  arr      : ResidentArray to populate (existing records are kept)
//  Returns  : true if at least one record was loaded
bool loadCSV(const string& filename, const string& cityName, ResidentArray& arr) {
    ifstream file(filename.c_str());
    if (!file.is_open()) {
        cerr << "  [ERROR] Cannot open file: " << filename << "\n";
        return false;
    }

    string line;
    getline(file, line); // skip header row

    int loaded  = 0;
    int skipped = 0;

    while (getline(file, line)) {
        line = trimStr(line);
        if (line.empty()) continue; // skip blank lines

        stringstream ss(line);
        string tok;
        Resident r;
        r.city = cityName;

        try {
            getline(ss, tok, ',');  r.residentID     = trimStr(tok);
            getline(ss, tok, ',');  r.age             = stoi(trimStr(tok));
            getline(ss, tok, ',');  r.transportMode   = trimStr(tok);
            getline(ss, tok, ',');  r.dailyDistance   = stod(trimStr(tok));
            getline(ss, tok, ',');  r.emissionFactor  = stod(trimStr(tok));
            getline(ss, tok, ',');  r.daysPerMonth     = stoi(trimStr(tok));
        } catch (...) {
            cerr << "  [SKIP] Malformed line: " << line << "\n";
            skipped++;
            continue;
        }

        arr.add(r);
        loaded++;
    }

    file.close();
    cout << "  >> " << filename
         << ":  loaded=" << loaded;
    if (skipped > 0) cout << ", skipped=" << skipped;
    cout << "\n";
    return (loaded > 0);
}

#endif // DATALOADER_H


// =============================================================
//  GLOBAL CONSTANTS
// =============================================================
static const int    NUM_DATASETS    = 3;
static const int    NUM_AGE_GROUPS  = 5;
static const int    MAX_MODES       = 12;   // max unique transport modes expected
 
static const string CITY_NAMES[NUM_DATASETS]   = { "City A", "City B", "City C" };
static const string CSV_FILES [NUM_DATASETS]   = { "cityA.csv", "cityB.csv", "cityC.csv" };
 
static const string AGE_GROUP_NAMES[NUM_AGE_GROUPS] = {
    "Children & Teenagers (6-17)",
    "University Students/Young Adults (18-25)",
    "Working Adults - Early Career (26-45)",
    "Working Adults - Late Career (46-60)",
    "Senior Citizens/Retirees (61-100)"
};
 
// =============================================================
//  UTILITY HELPERS
// =============================================================
 
// Print a horizontal separator line of width w using character c
void sep(int w = 80, char c = '-') {
    cout << string(w, c) << "\n";
}
 
// Print a prominent section banner
void banner(const string& title) {
    sep(80, '=');
    cout << "  " << title << "\n";
    sep(80, '=');
}
 
// Combine all three city datasets into one ResidentArray
ResidentArray combineAll(const ResidentArray ds[]) {
    ResidentArray all;
    for (int d = 0; d < NUM_DATASETS; d++)
        for (int i = 0; i < ds[d].getSize(); i++)
            all.add(ds[d].get(i));
    return all;
}
 
// Print a compact 10-row preview table (used after sorting)
void printPreview(const ResidentArray& arr, const string& title) {
    int rows = arr.getSize() < 10 ? arr.getSize() : 10;
    cout << "\n  [Preview – first " << rows << " of " << arr.getSize()
         << " records: " << title << "]\n";
    cout << left
         << setw(10) << "ID"
         << setw(5)  << "Age"
         << setw(14) << "Transport"
         << setw(10) << "Dist(km)"
         << setw(9)  << "Days/Mo"
         << "CO2/Month (kg)\n";
    sep(60);
    cout << fixed << setprecision(2);
    for (int i = 0; i < rows; i++) {
        const Resident& r = arr.get(i);
        cout << left
             << setw(10) << r.residentID
             << setw(5)  << r.age
             << setw(14) << r.transportMode
             << setw(10) << r.dailyDistance
             << setw(9)  << r.daysPerMonth
             << r.monthlyEmission() << "\n";
    }
    sep(60);
}
 
// =============================================================
//  TASK 4 – Age Group Categorization
//
//  For each of the 5 age groups (across ALL cities):
//    - Most preferred mode of transport
//    - Total carbon emissions
//    - Average carbon emission per resident
// =============================================================
void task4(const ResidentArray ds[]) {
    banner("TASK 4: AGE GROUP CATEGORIZATION");
 
    ResidentArray all = combineAll(ds);
 
    for (int g = 0; g < NUM_AGE_GROUPS; g++) {
 
        // Collect residents in this age group via linear search
        long long dummy;
        ResidentArray grp = all.linearSearchByAgeGroup(g, dummy);
        int n = grp.getSize();
        if (n == 0) continue;
 
        cout << "\nAge Group: " << AGE_GROUP_NAMES[g] << "\n";
        sep(74);
 
        // Tally transport modes using parallel arrays (no STL map)
        string modeNames[MAX_MODES];
        int    modeCnt [MAX_MODES] = {};
        double modeEmis[MAX_MODES] = {};
        int    nm = 0;
        double totalEmis = 0.0;
 
        for (int i = 0; i < n; i++) {
            const Resident& r = grp.get(i);
            double em = r.monthlyEmission();
            totalEmis += em;
 
            // Find existing entry or create new one
            int idx = -1;
            for (int m = 0; m < nm; m++)
                if (modeNames[m] == r.transportMode) { idx = m; break; }
            if (idx == -1 && nm < MAX_MODES)
                { modeNames[nm] = r.transportMode; idx = nm++; }
            if (idx != -1) { modeCnt[idx]++; modeEmis[idx] += em; }
        }
 
        // Find most-preferred mode (highest count)
        int bestIdx = 0;
        for (int m = 1; m < nm; m++)
            if (modeCnt[m] > modeCnt[bestIdx]) bestIdx = m;
 
        // Print mode breakdown table
        cout << left
             << setw(18) << "Mode of Transport"
             << setw(8)  << "Count"
             << setw(26) << "Total Emission (kg CO2)"
             << "Avg per Resident\n";
        sep(74);
        cout << fixed << setprecision(2);
        for (int m = 0; m < nm; m++) {
            double avg = (modeCnt[m] > 0) ? modeEmis[m] / modeCnt[m] : 0.0;
            cout << left
                 << setw(18) << modeNames[m]
                 << setw(8)  << modeCnt[m]
                 << setw(26) << modeEmis[m]
                 << avg << "\n";
        }
        sep(74);
        cout << "Total Emission for Age Group : " << totalEmis << " kg CO2\n";
        cout << "Average Emission per Resident: " << totalEmis / n << " kg CO2\n";
        cout << "Most Preferred Transport     : " << modeNames[bestIdx]
             << " (" << modeCnt[bestIdx] << " residents)\n";
    }
}
 
// =============================================================
//  TASK 5 – Carbon Emission Analysis
//
//  a. Total carbon emissions per dataset (city)
//  b. Carbon emissions per mode of transport
//  c. Comparison across datasets and age groups
// =============================================================
void task5(const ResidentArray ds[]) {
    banner("TASK 5: CARBON EMISSION ANALYSIS");
 
    // ----------------------------------------------------------
    //  [A] Total monthly emissions per city
    // ----------------------------------------------------------
    cout << "\n[A] Total Monthly Carbon Emissions per City\n";
    sep(58);
    cout << left << setw(10) << "City"
         << setw(12) << "Records"
         << "Total Emission (kg CO2)\n";
    sep(58);
    double grandTotal = 0.0;
    cout << fixed << setprecision(2);
    for (int d = 0; d < NUM_DATASETS; d++) {
        double tot = 0.0;
        for (int i = 0; i < ds[d].getSize(); i++)
            tot += ds[d].get(i).monthlyEmission();
        grandTotal += tot;
        cout << left << setw(10) << CITY_NAMES[d]
             << setw(12) << ds[d].getSize()
             << tot << "\n";
    }
    sep(58);
    cout << left << setw(10) << "TOTAL" << setw(12) << "" << grandTotal << "\n\n";
 
    // ----------------------------------------------------------
    //  [B] Emissions per transport mode (all cities combined)
    // ----------------------------------------------------------
    ResidentArray all = combineAll(ds);
 
    cout << "[B] Carbon Emissions per Mode of Transport (All Cities)\n";
    sep(70);
 
    string mn[MAX_MODES]; int mc[MAX_MODES] = {}; double me[MAX_MODES] = {}; int nm = 0;
    for (int i = 0; i < all.getSize(); i++) {
        const Resident& r = all.get(i);
        int idx = -1;
        for (int m = 0; m < nm; m++) if (mn[m] == r.transportMode) { idx = m; break; }
        if (idx == -1 && nm < MAX_MODES) { mn[nm] = r.transportMode; idx = nm++; }
        if (idx != -1) { mc[idx]++; me[idx] += r.monthlyEmission(); }
    }
 
    cout << left << setw(16) << "Transport Mode"
         << setw(10) << "Count"
         << setw(26) << "Total Emission (kg CO2)"
         << "Avg per Resident\n";
    sep(70);
    for (int m = 0; m < nm; m++) {
        cout << left << setw(16) << mn[m]
             << setw(10) << mc[m]
             << setw(26) << me[m]
             << (mc[m] > 0 ? me[m] / mc[m] : 0.0) << "\n";
    }
    sep(70);
 
    // ----------------------------------------------------------
    //  [C] Emissions by City AND Age Group cross-table
    // ----------------------------------------------------------
    cout << "\n[C] Emissions by City and Age Group\n";
    sep(88);
    cout << left << setw(10) << "City"
         << setw(44) << "Age Group"
         << setw(10) << "Count"
         << "Total CO2 (kg)\n";
    sep(88);
    for (int d = 0; d < NUM_DATASETS; d++) {
        for (int g = 0; g < NUM_AGE_GROUPS; g++) {
            double tot = 0.0; int cnt = 0;
            for (int i = 0; i < ds[d].getSize(); i++) {
                if (ds[d].get(i).ageGroupIndex() == g) {
                    tot += ds[d].get(i).monthlyEmission();
                    cnt++;
                }
            }
            if (cnt == 0) continue;
            cout << left << setw(10) << CITY_NAMES[d]
                 << setw(44) << AGE_GROUP_NAMES[g]
                 << setw(10) << cnt
                 << tot << "\n";
        }
        sep(88, '.');
    }
}
 
// =============================================================
//  TASK 6 – Sorting Experiments
//
//  Apply three sorting algorithms to each dataset (on copies).
//  Record and display execution time and memory usage.
// =============================================================
void task6(const ResidentArray ds[]) {
    banner("TASK 6: SORTING EXPERIMENTS");
 
    cout << "\nThree sorting algorithms are applied to independent copies of each dataset.\n";
    cout << "Original datasets are NOT modified.\n\n";
 
    // Performance summary header
    cout << left
         << setw(10) << "Dataset"
         << setw(20) << "Algorithm"
         << setw(18) << "Sort Key"
         << setw(10) << "Records"
         << setw(14) << "Time (us)"
         << "Memory (bytes)\n";
    sep(90);
 
    for (int d = 0; d < NUM_DATASETS; d++) {
        // Three separate deep copies – sorting one does not affect others
        ResidentArray c1(ds[d]); // for Bubble Sort    (by Age)
        ResidentArray c2(ds[d]); // for Selection Sort (by Distance)
        ResidentArray c3(ds[d]); // for Insertion Sort (by CO2)
 
        long long t1 = c1.bubbleSortByAge();
        cout << left << setw(10) << CITY_NAMES[d]
             << setw(20) << "Bubble Sort"
             << setw(18) << "Age"
             << setw(10) << c1.getSize()
             << setw(14) << t1
             << c1.memoryUsage() << "\n";
 
        long long t2 = c2.selectionSortByDistance();
        cout << left << setw(10) << CITY_NAMES[d]
             << setw(20) << "Selection Sort"
             << setw(18) << "Daily Distance"
             << setw(10) << c2.getSize()
             << setw(14) << t2
             << c2.memoryUsage() << "\n";
 
        long long t3 = c3.insertionSortByEmission();
        cout << left << setw(10) << CITY_NAMES[d]
             << setw(20) << "Insertion Sort"
             << setw(18) << "Monthly CO2"
             << setw(10) << c3.getSize()
             << setw(14) << t3
             << c3.memoryUsage() << "\n";
 
        sep(90, '.');
 
        // Show preview of each sorted result
        printPreview(c1, "Bubble Sort – sorted by Age");
        printPreview(c2, "Selection Sort – sorted by Daily Distance");
        printPreview(c3, "Insertion Sort – sorted by Monthly CO2");
        cout << "\n";
    }
 
    // ----------------------------------------------------------
    //  Time & Space Complexity Reference Table
    // ----------------------------------------------------------
    cout << "[Time & Space Complexity of Sorting Algorithms]\n";
    cout << left
         << setw(18) << "Algorithm"
         << setw(14) << "Best"
         << setw(14) << "Average"
         << setw(14) << "Worst"
         << "Space\n";
    sep(75);
    cout << left
         << setw(18) << "Bubble Sort"
         << setw(14) << "O(n)"
         << setw(14) << "O(n^2)"
         << setw(14) << "O(n^2)"
         << "O(1) in-place\n";
    cout << left
         << setw(18) << "Selection Sort"
         << setw(14) << "O(n^2)"
         << setw(14) << "O(n^2)"
         << setw(14) << "O(n^2)"
         << "O(1) in-place\n";
    cout << left
         << setw(18) << "Insertion Sort"
         << setw(14) << "O(n)"
         << setw(14) << "O(n^2)"
         << setw(14) << "O(n^2)"
         << "O(1) in-place\n";
    sep(75);
    cout << "Note: Bubble Sort and Insertion Sort reach O(n) on already-sorted data.\n";
    cout << "      Selection Sort always performs O(n^2) comparisons.\n";
}
 
// =============================================================
//  TASK 7 – Searching Experiments
//
//  Demonstrate and compare:
//    Linear Search (unsorted data) vs Binary Search (sorted data)
//  Criteria tested:
//    a. Age group  b. Transport mode  c. Distance threshold  d. Exact age
// =============================================================
void task7(const ResidentArray ds[]) {
    banner("TASK 7: SEARCHING EXPERIMENTS");
 
    ResidentArray all = combineAll(ds);
 
    // ----------------------------------------------------------
    //  [A] Linear Search – by Age Group: Working Adults Early Career (26-45)
    // ----------------------------------------------------------
    cout << "\n[A] Linear Search | Age Group: Working Adults - Early Career (26-45)\n";
    sep(80);
    long long tA;
    ResidentArray rA = all.linearSearchByAgeGroup(2, tA);
    cout << "  Matched: " << rA.getSize() << " records | Search Time: " << tA << " us\n";
    rA.displayTable(10);
 
    // ----------------------------------------------------------
    //  [B] Linear Search – by Transport Mode: Car
    // ----------------------------------------------------------
    cout << "\n[B] Linear Search | Transport Mode: Car\n";
    sep(80);
    long long tB;
    ResidentArray rB = all.linearSearchByTransport("Car", tB);
    cout << "  Matched: " << rB.getSize() << " records | Search Time: " << tB << " us\n";
    rB.displayTable(10);
 
    // ----------------------------------------------------------
    //  [C] Linear Search – Daily Distance > 15 km
    // ----------------------------------------------------------
    cout << "\n[C] Linear Search | Daily Distance > 15 km\n";
    sep(80);
    long long tC;
    ResidentArray rC = all.linearSearchByDistance(15.0, tC);
    cout << "  Matched: " << rC.getSize() << " records | Search Time: " << tC << " us\n";
    rC.displayTable(10);
 
    // ----------------------------------------------------------
    //  [D] Binary Search – Age = 30  (requires sorted array)
    // ----------------------------------------------------------
    cout << "\n[D] Binary Search | Age = 30  (array first sorted by Bubble Sort)\n";
    sep(80);
    // Sort a copy by age first – Binary Search requires sorted input
    ResidentArray sorted(all);
    sorted.bubbleSortByAge();
 
    long long tD;
    ResidentArray rD = sorted.binarySearchAllByAge(30, tD);
    cout << "  Matched: " << rD.getSize() << " records | Search Time: " << tD << " us\n";
    rD.displayTable(10);
 
    // ----------------------------------------------------------
    //  [E] Performance Comparison Table
    // ----------------------------------------------------------
    cout << "\n[E] Search Performance Comparison\n";
    sep(92);
    cout << left
         << setw(20) << "Algorithm"
         << setw(30) << "Criteria"
         << setw(16) << "Records Found"
         << setw(14) << "Time (us)"
         << "Data State\n";
    sep(92);
 
    // Rerun searches for fresh timing measurement
    long long t1, t2, t3, t4;
    ResidentArray r1 = all.linearSearchByAgeGroup(2,     t1);
    ResidentArray r2 = all.linearSearchByTransport("Car", t2);
    ResidentArray r3 = all.linearSearchByDistance(15.0,  t3);
    ResidentArray r4 = sorted.binarySearchAllByAge(30,   t4);
 
    cout << left << setw(20) << "Linear Search"
         << setw(30) << "Age Group 26-45"
         << setw(16) << r1.getSize()
         << setw(14) << t1
         << "Unsorted\n";
    cout << left << setw(20) << "Linear Search"
         << setw(30) << "Transport = Car"
         << setw(16) << r2.getSize()
         << setw(14) << t2
         << "Unsorted\n";
    cout << left << setw(20) << "Linear Search"
         << setw(30) << "Distance > 15 km"
         << setw(16) << r3.getSize()
         << setw(14) << t3
         << "Unsorted\n";
    cout << left << setw(20) << "Binary Search"
         << setw(30) << "Age = 30"
         << setw(16) << r4.getSize()
         << setw(14) << t4
         << "Sorted by Age\n";
    sep(92);
 
    // ----------------------------------------------------------
    //  [F] Complexity Reference Table
    // ----------------------------------------------------------
    cout << "\n[F] Time & Space Complexity of Searching Algorithms\n";
    cout << left
         << setw(18) << "Algorithm"
         << setw(16) << "Best Case"
         << setw(16) << "Worst Case"
         << setw(12) << "Space"
         << "Pre-condition\n";
    sep(78);
    cout << left
         << setw(18) << "Linear Search"
         << setw(16) << "O(1)"
         << setw(16) << "O(n)"
         << setw(12) << "O(1)"
         << "None – works on unsorted data\n";
    cout << left
         << setw(18) << "Binary Search"
         << setw(16) << "O(1)"
         << setw(16) << "O(log n)"
         << setw(12) << "O(1)"
         << "Array MUST be sorted\n";
    sep(78);
    cout << "Note: Binary Search is faster on large datasets (O(log n) vs O(n)).\n";
    cout << "      The sorting cost must be considered if data is not yet sorted.\n";
}
 
// =============================================================
//  TASK 8 – Performance Analysis
//
//  Summarises execution time and memory observations.
//  Compares arrays vs singly linked lists theoretically.
// =============================================================
void task8(const ResidentArray ds[]) {
    banner("TASK 8: PERFORMANCE ANALYSIS");
 
    // ----------------------------------------------------------
    //  Memory footprint per dataset
    // ----------------------------------------------------------
    cout << "\n[A] Array Memory Usage per Dataset\n";
    sep(65);
    cout << left << setw(12) << "Dataset"
         << setw(12) << "Records"
         << setw(24) << "Array Memory (bytes)"
         << "Bytes per Record\n";
    sep(65);
    for (int d = 0; d < NUM_DATASETS; d++) {
        size_t mem = ds[d].memoryUsage();
        int    rec = ds[d].getSize();
        int    bpr = (rec > 0) ? static_cast<int>(mem / rec) : 0;
        cout << left << setw(12) << CITY_NAMES[d]
             << setw(12) << rec
             << setw(24) << mem
             << bpr << "\n";
    }
    sep(65);
    cout << "Note: Allocated capacity may exceed current size (growth buffer).\n\n";
 
    // ----------------------------------------------------------
    //  Sorting benchmark across all datasets
    // ----------------------------------------------------------
    cout << "[B] Sorting Performance Benchmark (all datasets combined)\n";
    sep(72);
    ResidentArray all = combineAll(ds);
    cout << "  Total records: " << all.getSize() << "\n\n";
 
    // Run each sort on an independent copy and record time
    ResidentArray s1(all), s2(all), s3(all);
    long long tb = s1.bubbleSortByAge();
    long long ts = s2.selectionSortByDistance();
    long long ti = s3.insertionSortByEmission();
 
    cout << left
         << setw(20) << "Algorithm"
         << setw(14) << "Sort Key"
         << "Time (us)\n";
    sep(45);
    cout << left << setw(20) << "Bubble Sort"
         << setw(14) << "Age"        << tb << "\n";
    cout << left << setw(20) << "Selection Sort"
         << setw(14) << "Distance"   << ts << "\n";
    cout << left << setw(20) << "Insertion Sort"
         << setw(14) << "CO2/Month"  << ti << "\n";
    sep(45);
 
    // ----------------------------------------------------------
    //  Arrays vs Singly Linked Lists – characteristic comparison
    // ----------------------------------------------------------
    cout << "\n[C] Arrays vs Singly Linked Lists – Comparative Analysis\n";
    sep(82);
    cout << left
         << setw(30) << "Property"
         << setw(26) << "Array (this program)"
         << "Singly Linked List\n";
    sep(82);
    cout << left << setw(30) << "Random Access (index)"
         << setw(26) << "O(1) direct"
         << "O(n) traversal\n";
    cout << left << setw(30) << "Append (end)"
         << setw(26) << "O(1) amortized"
         << "O(1) with tail pointer\n";
    cout << left << setw(30) << "Insert (middle)"
         << setw(26) << "O(n) shift required"
         << "O(1) with node pointer\n";
    cout << left << setw(30) << "Delete (middle)"
         << setw(26) << "O(n) shift required"
         << "O(1) with prev pointer\n";
    cout << left << setw(30) << "Sorting"
         << setw(26) << "Cache-friendly"
         << "Pointer chasing (slow)\n";
    cout << left << setw(30) << "Binary Search"
         << setw(26) << "Supported O(log n)"
         << "Not supported O(n)\n";
    cout << left << setw(30) << "Memory overhead"
         << setw(26) << "Low (contiguous block)"
         << "High (node + next ptr)\n";
    cout << left << setw(30) << "Dynamic resize"
         << setw(26) << "Realloc + copy O(n)"
         << "Natural O(1) append\n";
    cout << left << setw(30) << "Cache performance"
         << setw(26) << "Excellent (locality)"
         << "Poor (scattered nodes)\n";
    sep(82);
 
    cout << "\n[D] Conclusion\n";
    sep(70);
    cout << "For this read-heavy, sort-heavy carbon emission workload:\n";
    cout << "  ARRAYS are preferred when:\n";
    cout << "    - Random access by index is needed (age lookup, row display)\n";
    cout << "    - Binary search is required (array must be sorted first)\n";
    cout << "    - Sorting speed matters (cache-friendly comparisons)\n";
    cout << "  LINKED LISTS are preferred when:\n";
    cout << "    - Frequent insertions/deletions in the middle occur\n";
    cout << "    - Dataset size is highly unpredictable at runtime\n";
    cout << "    - Stable ordering after partial insertion is needed\n";
    sep(70);
}
