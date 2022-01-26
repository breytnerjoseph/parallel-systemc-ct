#include<iostream>
#include "look_up_table_1d.h"


int main(int argc, char *argv[]){

    // Test LookUpTable1D
    std::vector<double> x_breakpoints = {1.0, 2.0, 3.0, 4.0};
    double gears[4] = {2.34, 1.450, 1.000, 0.68};
    LookUpTable1D lut(x_breakpoints);

    int i = 0;
    for (auto x : x_breakpoints) {
        lut.loadDatum(x, gears[i++]);
    }

    std::cout << "Look up value (1.0) - expected result 2.34 : " << lut.getValue(1.0) << std::endl;
    std::cout << "Look up value (2.5) - expected result  1.225 : " << lut.getValue(2.5) << std::endl;
    std::cout << "Look up value (5) - expected result  0.68 : " << lut.getValue(5) << std::endl;
    std::cout << "Look up value (0) - expected result  2.34 : " << lut.getValue(0) << std::endl;

    return 0;
}