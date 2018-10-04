// g++ -std=c++14 -O2 -march=native -o play satsolv.cpp

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

// The class of conditions
class Cond {
public:
    Cond(const string &c) { mCond = c; mStat = mbe; mUnk = 100; if (!Parse()) mStat = fls; }
    Cond(const Cond &c) { mCond = c.mCond; mStat = c.mStat; mUnk = c.mUnk; mTruths = c.mTruths; }
    ~Cond() {}

    void PrintCond() const { cout << mCond << " s:" << Status() << " u:" << Unknowns() << endl; }
    string &GetCond() { return mCond; }
    vector<string> &GetTruths() { return mTruths; }
    bool Update(vector<tribool> &vals);
    const tribool Status() const { return mStat; }
    const int Unknowns() const { return mUnk; }
private:
    Cond() {}
    bool Parse();
    tribool Eval(tribool c1, tribool c2, string delim) {
        if (delim=="V") {
            if (c1==tru or c2==tru) return tru;
            if (c2==fls) return c1;
            if (c1==fls) return c2;
            return fls;
        } else if (delim=="A") {
            if (c1==fls or c2==fls) return fls;
            if (c2==tru) return c1;
            if (c1==tru) return c2;
            return tru;
        } else {
            return mbe;
        }
    }
    string mCond;
    tribool mStat;
    int mUnk;
    vector<string> mTruths;
};

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

// The class holding a single set of triboolean values and their conditions
class Tribools {
public:
    Tribools() {};
    Tribools(const Tribools &tbs) {
        mVals = tbs.mVals;
        mConds = tbs.mConds;
    }
    ~Tribools() {};
    Tribools& operator=(const Tribools& tbs) {
        mVals = tbs.mVals;
        mConds = tbs.mConds;
        return *this;
    }

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

class SudoHold {
public:
    SudoHold() {}
    ~SudoHold() {}

    bool ReadSudos(string fname);
    bool SolveSudos();
    void PrintSudo(int idx);
private:
    vector<Tribools> mSudos;
    vector<int> mTags;
};
