#ifndef LOOK_UP_TABLE_1D_H
#define LOOK_UP_TABLE_1D_H

#include <iostream>
#include "look_up_table_2d.h"

class LookUpTable1D : protected LookUpTable2D {
    public:
        LookUpTable1D(std::vector<double> x_breakpoints);

        ~LookUpTable1D();

        void loadDatum(double x, double val);

        void loadData(double *data);

        double getValue(double x) const;

        void print() const;
    private:
        std::vector<double> getVectorOfZeros(int size); 
};

#endif