#include "Cond.h"

bool Cond::Parse() {
    mTruths.clear();
    string curr;
    for (char &c : mCond) {
        if (c != ' ')
            curr += c;
        else {
            if (curr!="V" and curr!="A") mTruths.push_back(curr);
            curr = "";
        }
    }
    mTruths.push_back(curr);

    return true;
}

bool Cond::Update(vector<tribool> &vals) {
    if (mStat!=mbe) return false;

    // We proceed from left to right, eliminating "from left".
    // In the process, possible updates are taken into account
    tribool valprev = mbe;
    for (auto idx = 0u; idx < mTruths.size();) {
        tribool val = mbe;

        string &currs = mTruths[idx];
        if (currs=="t") {
            val = tru;
        } else if (currs=="f") {
            val = fls;
        } else {
            unsigned pos = stoi(currs.substr(1));
            bool reverse = (currs[0]=='n');
            val = vals.at(pos);

            if (val == tru) { if (reverse) val = fls; }
            else if (val == fls) { if (reverse) val = tru; }
            if (val==tru) currs = "t";
            else if (val==fls) currs = "f";
        }

        if (valprev == mbe) {
            // Previous value not exact, so nothing to retract
            ++idx;
            valprev = val;
        } else {
            // The previous value is exact, so we can retract
            tribool method = Eval(valprev,val,"V");
            if (method==tru) { mTruths[idx] = "t"; }
            else if (method==fls) { mTruths[idx] = "f"; }
            // We remove the previous position, so idx remains static
            mTruths.erase(mTruths.begin()+idx-1);
            valprev = method;
        }
    }

    // We proceed from left to right, eliminating "from left".
    // No need to update the cell values this time.
    valprev = mbe;
    for (int idx = mTruths.size()-1; idx >= 0; --idx) {
        tribool val = mbe;

        string &currs = mTruths[idx];
        if (currs=="t") {
            val = tru;
        } else if (currs=="f") {
            val = fls;
        } else {
            val = mbe;
        }

        if (valprev == mbe) {
            // Previous value not exact, so nothing to retract
            valprev = val;
        } else {
            // The previous value is exact, so we can retract
            tribool method = Eval(valprev,val,"V");
            if (method==tru) { mTruths[idx] = "t"; }
            else if (method==fls) { mTruths[idx] = "f"; }
            // We remove the previous position
            mTruths.erase(mTruths.begin()+idx+1);
            valprev = method;
        }
    }

    // Count the unknowns
    mUnk = 0;
    for (auto &t : mTruths)
        if (t != "t" and t != "f") ++mUnk;

    if (mUnk==0) {
        if (mTruths[0]=="t") {
            mStat = tru;
            mCond = "t";
        } else {
            mStat = fls;
            mCond = "f";
            return false;
        }
    } else if (mUnk==1) {
        unsigned pos = stoi(mTruths[0].substr(1));
        bool reverse = (mTruths[0][0]=='n');
        if (reverse) vals.at(pos) = fls;
        else vals.at(pos) = tru;
        mStat = tru;
        mCond = "t";
    } else {
        mCond = mTruths[0];
        for (auto i = 1u; i < mTruths.size(); ++i) {
            mCond += " V ";
            mCond += mTruths[i];
        }
    }

    return true;
}
