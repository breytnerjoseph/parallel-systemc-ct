#include "look_up_table_2d.h"

#include<iostream>

LookUpTable2D::LookUpTable2D(std::vector<double> x_breakpoints, std::vector<double> y_breakpoints){
    _x_breakpoints = x_breakpoints;
    _y_breakpoints = y_breakpoints;
}

LookUpTable2D::~LookUpTable2D(){
}


void LookUpTable2D::loadDatum(double x, double y, double val){ 
    _data[std::make_tuple(x,y)] = val;
}

double LookUpTable2D::getValue(double x, double y) const {
    double x1, x2, y1, y2;

    // Avoid using exceptions ! They are slow

    // // Try to directly get the value
    // try{
    //     double val = _data.at(std::make_tuple(x, y));
    //     return val;
    // }
    // // Interpolate
    // catch (std::out_of_range) { 


        // try{
            std::tie(x1, x2) = get_enclosing(x, _x_breakpoints);
            std::tie(y1, y2) = get_enclosing(y, _y_breakpoints);

            // Extrapolation both dimensions case
            if (x1 == x2 && y1 == y2) {
                return _data.at(std::make_tuple(x1, y1));
            }

            // Extrapolation in the X dimension
            // Linear interpolation in the Y dimension
            if (x1 == x2 && y1 != y2) {
                double q1 = _data.at(std::make_tuple(x1, y1));
                double q2 = _data.at(std::make_tuple(x1, y2));
                return linearInterpolation(q1, q2, y1, y2, y);
            }

            // Extrapolation in the Y dimension
            // Linear interpolation in the X dimension
            if (y1 == y2 && x1 != x2) {
                double q1 = _data.at(std::make_tuple(x1, y1));
                double q2 = _data.at(std::make_tuple(x2, y1));
                return linearInterpolation(q1, q2, x1, x2, x);
            }

            // x1 != x2 and y1 != y2 => bilinear interpolation
            double q11, q12, q21, q22;
            q11 = _data.at(std::make_tuple(x1, y1));
            q12 = _data.at(std::make_tuple(x1, y2));
            q21 = _data.at(std::make_tuple(x2, y1));
            q22 = _data.at(std::make_tuple(x2, y2));

            return bilinearInterpolation(q11, q12, q21, q22, x1, x2, y1, y2, x, y);
        // }
        // catch(...) {
        //     std::cout << "Except in look up table\n";
        //     std::cout << "x is " << x << " and y " << y << std::endl;
        // }

    // }


}

// Assume that elements is and ordered vector
std::tuple<double, double> LookUpTable2D::get_enclosing(double val, std::vector<double> const &elements) const {
    double lower, upper;
    
    // Make sure that no empty vector is passed
    if (elements.size() < 1) {
        throw std::invalid_argument("The size of the given vector must be greater than 0");
    }

    // If the value is less than the smallest element, 
    // return a 2-tuple where both places contain the smallest element.
    if( val < elements[0] ){
        lower = upper = elements[0];
        return std::make_tuple(lower, upper);
    }

    // If the value is greater than the greatest element, 
    // return a 2-tuple where both places contain the greatest element.
    upper = *(elements.end()-1);
    if(val > upper){
        lower = upper;
        return std::make_tuple(lower, upper);
    }

    // The value is between the smallest and greatest element, 
    // look for and return the two closest elements that surround it. 
    for (std::vector<double>::const_iterator it = elements.begin(); it != elements.end(); it++) {
        
        // An exact match is found
        if (*it == val){
            lower = upper = val;
            return std::make_tuple(lower, upper);
        }

        // An enclosing is found
        if (*it > val) {
            upper = *it;
            if( it != elements.begin() ){
                lower = *(it-1);
            }
            else{
                lower = upper;
            }
            return std::make_tuple(lower, upper);
        }
    }

    return std::make_tuple(lower, upper);
}

void LookUpTable2D::loadData(std::vector<std::vector<double> > data){
    int i = 0, j = 0;
    for (auto y_break : _y_breakpoints) {
        for (auto x_break : _x_breakpoints) {
            loadDatum(x_break, y_break, data[i][j] );
            j++;
        }
        i++;
        j = 0;
    }
}

/**************************************************************
 *  BILINEAR INTERPOLATION:
 * 
 *      (x1,y1)->q11              x              (x2,y1)->q21
 *                                | 
 *                                v
 *           y -----------------> ?                      
 * 
 *      (x1,y2)->q12                            (x2,y2)->q22 
 *************************************************************/
double LookUpTable2D::bilinearInterpolation(
    double q11, double q12, double q21, double q22, 
    double x1, double x2, double y1, double y2, 
    double x, double y) const {
    double R1 = ((x2 - x)/(x2 - x1))*q11 + ((x - x1)/(x2 - x1))*q21;
    double R2 = ((x2 - x)/(x2 - x1))*q12 + ((x - x1)/(x2 - x1))*q22;
    double P = ((y2 - y)/(y2 - y1))*R1 + ((y - y1)/(y2 - y1))*R2;
    return P;
}


double LookUpTable2D::linearInterpolation(double q1, double q2, double x1, double x2, double x) const {
    // Avoid division by zero
    if (x1 == x2) {
        throw std::invalid_argument("For a linear interpolation x1 and x2 must have different values.");
    }
    return q1 + (x - x1)*(q2-q1)/(x2-x1);
}