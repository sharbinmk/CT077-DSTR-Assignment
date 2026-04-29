#ifndef RESIDENTARRAY_H
#define RESIDENTARRAY_H

#include "Resident.h"
#include <iostream>

class ResidentArray {
private:
    Resident* data;
    int capacity;
    int currentSize;

public:
    ResidentArray(int cap = 1000) : capacity(cap), currentSize(0) {
        data = new Resident[capacity];
    }

    ~ResidentArray() {
        delete[] data;
    }

    void add(const Resident& r) {
        if (currentSize < capacity) {
            data[currentSize++] = r;
        }
    }

    int size() const { return currentSize; }
    Resident& get(int index) { return data[index]; }

    // Sorting by Age (Bubble Sort) [cite: 98-99]
    void sortByAge() {
        for (int i = 0; i < currentSize - 1; i++) {
            for (int j = 0; j < currentSize - i - 1; j++) {
                if (data[j].age > data[j + 1].age) {
                    Resident temp = data[j];
                    data[j] = data[j + 1];
                    data[j + 1] = temp;
                }
            }
        }
    }
};

#endif
