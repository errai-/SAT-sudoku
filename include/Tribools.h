// The class holding a single set of triboolean values and their conditions

#ifndef TRIBOOLS
#define TRIBOOLS

#include "helper.h"
#include "Cond.h"

class Tribools {
public:
    Tribools() {}

    vector<tribool> mVals;
    vector<Cond> mConds;

    bool AddCond(string cond) {
        mConds.push_back(Cond(cond));
        return true;
    }
    void PrintConds() {
        for (auto &c : mConds)
            c.PrintCond();
    }
    void Print2Conds() {
        for (auto &c : mConds)
            if (c.Unknowns()<=2) c.PrintCond();
    }
    bool Solve();
private:
    map<string,vector<string> > mAssoc;
    vector<Triplet> mPotentials;

    bool CheckConds();
    bool EraseDuplicates();
    bool FindAttempts();
    bool FindComplements();
    bool FindPairings();
    bool LoopAttempts();
    bool RemoveNonDual();

    bool Method1() {
        if (!FindPairings()) return false;
        if (!RemoveNonDual()) return false;
        if (!FindComplements()) return false;
        return true;
    }
    bool Method2() {
        if (!FindAttempts()) return false;
        if (!LoopAttempts()) return false;
        return true;
    }
    bool Updates() {
        for (auto &c : mConds)
            if (!c.Update(mVals)) return false;
        return CheckConds();
    }
};

#endif // TRIBOOLS
