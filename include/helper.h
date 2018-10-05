#ifndef HELPER
#define HELPER

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <regex>
#include <algorithm>

using std::pair;
using std::vector;
using std::map;
using std::cout;
using std::endl;
using std::string;
using std::regex;
using std::to_string;
using std::stoi;

enum tribool { tru, fls, mbe };

struct Triplet {
    Triplet() {
        v1 = 0; v2 = 0; v3 = 0;
    }
    Triplet(int _v1, int _v2, int _v3) {
        v1 = _v1; v2 = _v2; v3 = _v3;
    }
    bool operator > (const Triplet& tri) const {
        int m1 = std::min(v2,v3);
        int m2 = std::min(tri.v2,tri.v3);
        if (m1==m2) {
            return std::max(v2,v3)>std::max(tri.v2,tri.v3);
        } else {
            return (m1>m2);
        }
    }
    int v1;
    int v2;
    int v3;
};

#endif // HELPER
