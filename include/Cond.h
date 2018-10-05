// The class of conditions

#ifndef CONDS
#define CONDS

#include "helper.h"

class Cond {
public:
    ~Cond() {}
    // Intended constructor use
    Cond(const string &c) { mCond = c; mStat = mbe; mUnk = 100; if (!Parse()) mStat = fls; }

    void PrintCond() const { cout << mCond << " s:" << Status() << " u:" << Unknowns() << endl; }
    string &GetCond() { return mCond; }
    vector<string> &GetTruths() { return mTruths; }
    bool Update(vector<tribool> &vals);
    const tribool Status() const { return mStat; }
    const int Unknowns() const { return mUnk; }
private:
    // Suppress default constructor
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

#endif
