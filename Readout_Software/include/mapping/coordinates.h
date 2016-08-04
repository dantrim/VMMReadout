#ifndef COORDINATES_H
#define COORDINATES_H

#include <string>

class Coordinates {
    public :
        Coordinates();
        void set(double x, double y, double z);

        double x() { return x_; }
        double y() { return y_; }
        double z() { return z_; }

    private :
        double x_;
        double y_;
        double z_;
};

#endif
