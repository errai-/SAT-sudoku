// The class to hold sudokus

#ifndef SUDOHOLD
#define SUDOHOLD

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

#endif // SUDOHOLD
