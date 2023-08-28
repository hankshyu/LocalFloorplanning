#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cassert>
#include <cmath>
#include <unordered_map>
#include <boost/polygon/polygon.hpp>

namespace gtl = boost::polygon;

typedef gtl::polygon_90_data<int> Polygon;
typedef gtl::rectangle_data<int>  Rectangle;
typedef gtl::point_data<int>      Point;
typedef std::vector<Polygon>      PolygonSet;

using namespace gtl::operators;

struct SoftModInfo {
    std::string modName;
    int minArea;
    Polygon rectiPolygon;
};

struct FixedModInfo {
    std::string modName;
    Rectangle box;
};

struct ConnectionInfo {
    std::string mod1Name;
    std::string mod2Name;
    int connection;
};

int main(int argc, char *argv[]) {
    assert(argc == 3);

    // case spec
    int chipWidth, chipHeight;
    int softModuleCount, fixedModuleCount, connectionCount;
    std::vector<SoftModInfo> softModVec;
    std::vector<FixedModInfo> fixedModVec;
    std::vector<ConnectionInfo> connectionVec;

    // floorplan info
    double HPWL;
    int givenSoftModCount;
    std::unordered_map< std::string, std::pair<double, double> > modName2center;


    std::ifstream cfile(argv[1]);  // case file
    std::ifstream ifile(argv[2]);  // floorplan file
    std::string trash;

    // ----------------------------------------------
    //               Read Case Info
    // ----------------------------------------------

    cfile >> trash >> chipWidth >> chipHeight;

    cfile >> trash >> softModuleCount;
    softModVec.resize(softModuleCount);
    for ( int i = 0; i < softModuleCount; ++i ) {
        cfile >> softModVec[i].modName >> softModVec[i].minArea;
    }

    cfile >> trash >> fixedModuleCount;
    fixedModVec.resize(fixedModuleCount);
    for ( int i = 0; i < fixedModuleCount; ++i ) {
        int x, y, w, h;
        cfile >> fixedModVec[i].modName >> x >> y >> w >> h;
        fixedModVec[i].box = Rectangle(x, y, x + w, y + h);
        modName2center[fixedModVec[i].modName] = std::make_pair(x + w / 2., y + h / 2.);
    }

    cfile >> trash >> connectionCount;
    connectionVec.resize(connectionCount);
    for ( int i = 0; i < connectionCount; ++i ) {
        cfile >> connectionVec[i].mod1Name >> connectionVec[i].mod2Name >> connectionVec[i].connection;
    }

    // ----------------------------------------------
    //              Read Floorplan Info
    // ----------------------------------------------

    ifile >> trash >> HPWL;
    ifile >> trash >> givenSoftModCount;

    int pointCount;
    std::string givenSoftModName;
    for ( int i = 0; i < softModuleCount; ++i ) {
        ifile >> givenSoftModName >> pointCount;
        int curID;
        for ( curID = 0; curID < softModuleCount; ++curID ) {
            if ( givenSoftModName == softModVec[curID].modName ) {
                break;
            }
        }
        std::vector<Point> points;
        int x, y;
        for ( int p = 0; p < pointCount; ++p ) {
            ifile >> x >> y;
            points.push_back(Point(x, y));
        }
        gtl::set_points(softModVec[curID].rectiPolygon, points.begin(), points.end());
    }

    // ----------------------------------------------
    //            Verify Area Constraint
    // ----------------------------------------------

    bool areaPassed = true;
    for ( auto &softMod : softModVec ) {
        if ( gtl::area(softMod.rectiPolygon) < softMod.minArea ) {
            std::cout << "VIOLATION: " << softMod.modName << " is smaller than given minimum area." << std::endl;
            areaPassed = false;
        }
    }
    if ( areaPassed ) {
        std::cout << "All modules pass area constraint.\n";
    }

    // ----------------------------------------------
    //          Verify Rectiliner Constrint
    // ----------------------------------------------

    bool rectiPassed = true;
    for ( auto &softMod : softModVec ) {
        Rectangle extentRect;
        gtl::extents(extentRect, softMod.rectiPolygon);
        int extentWidth = gtl::xh(extentRect) - gtl::xl(extentRect);
        int extentHeight = gtl::yh(extentRect) - gtl::yl(extentRect);
        modName2center[softMod.modName] = std::make_pair(gtl::xl(extentRect) + extentWidth / 2., gtl::yl(extentRect) + extentHeight / 2.);
        double extentAspectRatio = (double) extentHeight / extentWidth;
        // std::cout << extentAspectRatio << " " << (double) gtl::area(softMod.rectiPolygon) / gtl::area(extentRect) << "\n";
        if ( extentAspectRatio > 2. || extentAspectRatio < 0.5 ) {
            std::cout << "VIOLATION: " << softMod.modName << " is out of aspect ratio constraint (0.5~2)." << std::endl;
            rectiPassed = false;
        }
        if ( (double) gtl::area(softMod.rectiPolygon) / gtl::area(extentRect) < 0.8 ) {
            std::cout << "VIOLATION: " << softMod.modName << " violates rectangle ratio constraint (>80%)." << std::endl;
            rectiPassed = false;
        }
    }
    if ( rectiPassed ) {
        std::cout << "All modules pass rectiliner constraint.\n";
    }

    // ----------------------------------------------
    //               Verify Overlaps
    // ----------------------------------------------

    bool overlapPassed = true;
    for ( auto &softMod : softModVec ) {
        for ( auto &tarSoftMod : softModVec ) {
            if ( tarSoftMod.modName == softMod.modName ) {
                continue;
            }
            PolygonSet intersection;
            intersection += softMod.rectiPolygon & tarSoftMod.rectiPolygon;
            if ( !intersection.empty() ) {
                std::cout << "VIOLATION: " << softMod.modName << " and " << tarSoftMod.modName << " overlap together.\n";
                overlapPassed = false;
            }
        }
        for ( auto &tarfixedMod : fixedModVec ) {
            PolygonSet intersection;
            intersection += softMod.rectiPolygon & tarfixedMod.box;
            if ( !intersection.empty() ) {
                std::cout << "VIOLATION: " << softMod.modName << " and " << tarfixedMod.modName << " overlap together.\n";
                overlapPassed = false;
            }
        }
    }
    if ( overlapPassed ) {
        std::cout << "All modules do not overlap with each other.\n";
    }

    // ----------------------------------------------
    //                 Verify HPWL
    // ----------------------------------------------

    double goldenHPWL = 0;
    for ( auto &connection : connectionVec ) {
        double mod1cx = modName2center[connection.mod1Name].first;
        double mod1cy = modName2center[connection.mod1Name].second;
        double mod2cx = modName2center[connection.mod2Name].first;
        double mod2cy = modName2center[connection.mod2Name].second;
        goldenHPWL += connection.connection * ( std::abs(mod1cx - mod2cx) + std::abs(mod1cy - mod2cy) );
    }

    goldenHPWL = std::round(goldenHPWL * 10.0) / 10.0;

    std::cout << "Golden HPWL = " << goldenHPWL << std::endl;
    if ( goldenHPWL != HPWL ) {
        std::cout << "VIOLATION: HPWL calculation wrong.\n";
    }
    else {
        std::cout << "HPWL calculation is correct.\n";
    }

    cfile.close();
    ifile.close();
    return 0;
}

/*

for ( auto &softMod : softModVec ) {
    std::cout << softMod.modName << "\n";
    for ( auto it = softMod.rectiPolygon.begin(); it != softMod.rectiPolygon.end(); ++it ) {
        std::cout << "(" << ( *it ).x() << ", " << ( *it ).y() << ")" << std::endl;
    }
}

*/