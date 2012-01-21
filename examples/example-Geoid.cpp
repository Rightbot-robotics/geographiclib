// Example of using the GeographicLib::Geoid class
// $Id: 2ba4534ebb4c4853dc111932fe44b4bf85ac8149 $

#include <iostream>
#include <exception>
#include <GeographicLib/Geoid.hpp>

using namespace std;
using namespace GeographicLib;

int main() {
  try {
    Geoid egm96("egm96-5");
    // Convert height above egm96 to height above the ellipsoid
    double lat = 42, lon = -75, height_above_geoid = 20;
    double
      geoid_height = egm96(lat, lon),
      height_above_ellipsoid = (height_above_geoid +
                                Geoid::GEOIDTOELLIPSOID * geoid_height);
    cout << height_above_ellipsoid << "\n";
  }
  catch (const exception& e) {
    cerr << "Caught exception: " << e.what() << "\n";
    return 1;
  }
  return 0;
}
