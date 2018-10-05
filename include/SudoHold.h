// g++ -std=c++14 -O2 -march=native -o play satsolv.cpp

#include "helper.h"
#include "Cond.h"
#include "Tribools.h"

class SudoHold {
public:
    SudoHold() {}

    bool ReadSudos(string fname);
    bool SolveSudos();
    void PrintSudo(int idx);
private:
    vector<Tribools> mSudos;
    vector<int> mTags;
};
