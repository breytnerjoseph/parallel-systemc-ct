#ifndef LOOK_UP_TABLE_2D_H
#define LOOK_UP_TABLE_2D_H

#include <vector>
#include <map>
#include <tuple>

class LookUpTable2D {
    public:
        LookUpTable2D(std::vector<double> x_breakpoints, std::vector<double> y_breakpoints);

        ~LookUpTable2D();

        void loadDatum(double x, double y, double val);

        void loadData(std::vector<std::vector<double> > data);

        double getValue(double x, double y) const;

    protected:
        std::vector<double> _x_breakpoints;
        std::vector<double> _y_breakpoints;

    private:
        typedef std::tuple<double, double> Index2D;
        typedef std::map<Index2D, double>::const_iterator _data_iterator;

        std::map<Index2D, double> _data;

        // Assume that elements is and ordered vector
        std::tuple<double, double> get_enclosing(double val, std::vector<double> const &elements) const;

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
        double bilinearInterpolation(
            double q11, double q12, double q21, double q22, 
            double x1, double x2, double y1, double y2, 
            double x, double y) const;

        double linearInterpolation(double q1, double q2, double x1, double x2, double x) const;

};

#endif