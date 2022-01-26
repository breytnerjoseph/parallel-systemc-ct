#include "look_up_table_1d.h"

LookUpTable1D::LookUpTable1D(std::vector<double> x_breakpoints)
    : LookUpTable2D(x_breakpoints, getVectorOfZeros(1)) {
}

LookUpTable1D::~LookUpTable1D(){

}

void LookUpTable1D::loadDatum(double x, double val){
    LookUpTable2D::loadDatum(x, 0.0, val);
}

double LookUpTable1D::getValue(double x) const{
    return LookUpTable2D::getValue(x, 0.0);
}

void LookUpTable1D::loadData(double *data){
    int i = 0;
    for (auto x_break :  LookUpTable2D::_x_breakpoints) {
        loadDatum(x_break, data[i++]);
    }
}

std::vector<double> LookUpTable1D::getVectorOfZeros(int size){
    std::vector<double> result(size);
    return result;
}