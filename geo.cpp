#define _USE_MATH_DEFINES
#include "geo.h"

#include <cmath>

const int EARTH_RAD = 6371000;

namespace geo {

double ComputeDistance(Coordinates from, Coordinates to) {
    using namespace std;
    if (from == to) {
        return 0;
    }
    static const double dr = M_PI / 180.;
    return acos(sin(from.lat * dr) * sin(to.lat * dr)
                + cos(from.lat * dr) * cos(to.lat * dr) * cos(abs(from.lng - to.lng) * dr))
        * EARTH_RAD;
}

bool Coordinates::operator==(const Coordinates& other) const {
    return lat == other.lat && lng == other.lng;
}
bool Coordinates::operator!=(const Coordinates& other) const {
    return !(*this == other);
}

}