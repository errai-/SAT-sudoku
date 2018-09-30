// g++ -std=c++14 -O2 -march=native -o play satsolv.cpp

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <regex>

using std::vector;
using std::cout;
using std::endl;
using std::string;
using std::regex;
using std::to_string;
using std::stoi;

enum tribool { tru, fls, mbe };

class Cond {
public:
    ~Cond() {}
    Cond(const string &c, vector<tribool> *v) { mCond = c; mVals = v; mStat = mbe; mUnk = 100; if (!Parse()) mStat = fls; }
    void PrintCond() const {
        if (mStat != mbe) {
            if (mStat==tru) cout << "t" << endl;
            else cout << "f" << endl;
            return;
        }
        cout << mTruths[0];
        for (auto i = 0u; i < mDelims.size(); ++i) {
            cout << " " << mDelims[i];
            cout << " " << mTruths[i+1];
        }
        cout << " s:" << Status() << " u:" << Unknowns() << endl;
    }
    bool Update();
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
    vector<tribool> *mVals;
    tribool mStat;
    int mUnk;
    vector<string> mTruths;
    vector<string> mDelims;
};

struct Tribools {
    vector<tribool> mVals;
    vector<Cond> mConds;
    bool AddCond(string cond) {
        mConds.push_back(Cond(cond,&mVals));
        return true;
    }
    void PrintConds() {
        for (auto &c : mConds)
            c.PrintCond();
    }
    bool Updates() {
        for (auto &c : mConds)
            c.Update();
        return CheckConds();
    }
    bool CheckConds() {
        bool success = true;
        for (auto i = 0u; i<mConds.size();) {
            if (mConds[i].Status()==tru) {
                mConds.erase(mConds.begin()+i);
            } else if (mConds[i].Status()==fls) {
                mConds.erase(mConds.begin()+i);
                success = false;
            } else {
                i += 1;
            }
        }
        return success;
    }
};

class SudoHold {
public:
    SudoHold() {}
    ~SudoHold() {}

    bool ReadSudo(string fname);
    void PrintSudo(int idx);
private:
    vector<Tribools> mSudos;
    vector<int> mTags;
};
