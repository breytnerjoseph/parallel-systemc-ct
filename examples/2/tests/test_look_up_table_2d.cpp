#include<iostream>
#include "look_up_table_2d.h"


int main(int argc, char *argv[]){
   
    /* // Test bilinear interpolation 
    double q11 = 91, q12 = 162, q21 = 210, q22 = 95;
    double x1 = 14, y1 = 20, x2 = 15, y2 =21;
    double x = 14.5, y = 20.2;

    std::cout << "Expected result: 146.1" << std::endl;
    std::cout << "Result: " << bilinearInterpolation(q11, q12, q21, q22, x1, x2, y1, y2, x, y) << std::endl;  
    */


   // Test LookUpTable2D
   std::vector<double> x_breakpoints = {13, 14, 15, 16};
   std::vector<double> y_breakpoints = {19, 20, 21, 22};
   double data[4][4] = {
       {1, 2, 3, 4},
       {80, 91, 210, 300},
       {100, 162, 95, 30},
       {4, 3, 2, 1}
   };

    LookUpTable2D lut(x_breakpoints, y_breakpoints);

    int i = 0, j = 0;
    for (auto y_break : y_breakpoints) {
        for (auto x_break : x_breakpoints) {
            lut.loadDatum(x_break, y_break, data[i][j] );
            j++;
        }
        i++;
        j = 0;
    }

    std::cout << "Look up value (14.5, 20.2): " << lut.getValue(14.5, 20.2) << std::endl;
    std::cout << "Look up value (14, 20): " << lut.getValue(14, 20) << std::endl;
    std::cout << "Look up value (14, 20) - extrapolation both senses: " << lut.getValue(10, 15) << std::endl;
    std::cout << "Look up value (18, 30) - extrapolation both senses: " << lut.getValue(18, 30) << std::endl;
    std::cout << "Look up value (14, 20.2) - linear interpolation, expected 105.2 : " << lut.getValue(14, 20.2) << std::endl;
    std::cout << "Look up value (10, 20.2) - x extrapolation, expected 84 : " << lut.getValue(10, 20.2) << std::endl;
    std::cout << "Look up value (14.5, 20) - linear interpolation, expected 150.5 : " << lut.getValue(14.5, 20) << std::endl;
    std::cout << "Look up value (14.5, 24) - y extrapolation, expected 2.5 : " << lut.getValue(14.5, 24) << std::endl;


    return 0;
}