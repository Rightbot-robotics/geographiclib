// Example of using the GeographicLib::DMS class
// $Id: c12828a51e75bb4a2a28f67768acdb405f63a197 $

#include <iostream>
#include <exception>
#include <GeographicLib/DMS.hpp>

using namespace std;
using namespace GeographicLib;

int main() {
  try {
    {
      string dms = "30d14'45.6\"S";
      DMS::flag type;
      double ang = DMS::Decode(dms, type);
      cout << type << " " << ang << "\n";
    }
    {
      double ang = -30.245715;
      string dms = DMS::Encode(ang, 6, DMS::LATITUDE);
      cout << dms << "\n";
    }
  }
  catch (const exception& e) {
    cerr << "Caught exception: " << e.what() << "\n";
    return 1;
  }
  return 0;
}
