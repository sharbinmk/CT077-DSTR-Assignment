#ifndef RESIDENT_H
#define RESIDENT_H

#include <string>

struct Resident {
    std::string residentID;
    int age;
    std::string mode;
    double dailyDistance;
    double emissionFactor;
    int daysPerMonth;

    // Logic: Distance * Factor * Days [cite: 26-27]
    double calculateMonthlyEmission() const {
        return dailyDistance * emissionFactor * daysPerMonth;
    }

    // Helper for Age Group categorization [cite: 58-60]
    std::string getAgeGroup() const {
        if (age >= 6 && age <= 17) return "6-17: Children & Teenagers";
        if (age >= 18 && age <= 25) return "18-25: University Students / Young Adults";
        if (age >= 26 && age <= 45) return "26-45: Working Adults (Early Career)";
        if (age >= 46 && age <= 60) return "46-60: Working Adults (Late Career)";
        if (age >= 61 && age <= 100) return "61-100: Senior Citizens / Retirees";
        return "Unknown";
    }
};

#endif
