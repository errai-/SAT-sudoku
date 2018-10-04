#include "Tribools.h"

bool Tribools::CheckConds() {
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

bool Tribools::EraseDuplicates() {
    for (auto i = 0u; i<mConds.size()-1; ++i) {
        string &first = mConds[i].GetCond();
        for (auto j = i+1; j<mConds.size();) {
            string &second = mConds[j].GetCond();
            if (first==second) { mConds.erase(mConds.begin()+j); }
            else ++j;
        }
    }
    return true;
}

bool Tribools::FindAttempts() {
    map<int,pair<int,int>> numbers;
    for (auto &c : mConds) {
        if (c.Unknowns()!=2) continue;
        auto &ts = c.GetTruths();
        if (ts.size()!=2) {
            cout << "Problems in the condition class!" << endl;
            continue;
        }
        for (int i = 0; i<2; ++i) {
            string &ti = ts[i];
            int idx = stoi(ti.substr(1));
            if (numbers.find(idx)==numbers.end()) numbers[idx] = std::make_pair(0,0);
            if (ti[0]=='y') numbers[idx].first += 1;
            else numbers[idx].second += 1;
        }
    }
    mPotentials.clear();
    for (auto &val : numbers) {
        if (val.second.first == 0 or val.second.second == 0) continue;
        mPotentials.emplace_back(val.first,val.second.first,val.second.second);
    }

    std::sort(mPotentials.begin(),mPotentials.end(),std::greater<Triplet>());
    return true;
}

bool Tribools::FindComplements() {
    for (auto &c : mConds) {
        if (c.Unknowns()!=2) continue;
        auto &ts = c.GetTruths();
        if (ts.size()!=2) {
            cout << "Problems in the condition class!" << endl;
            continue;
        }
        // The original two conditions
        string &t1 = ts[0];
        string &t2 = ts[1];
        // We build complementary conditions
        string ct1 = (t1[0]=='y' ? "n" : "y");
        string ct2 = (t2[0]=='y' ? "n" : "y");
        ct1 += t1.substr(1);
        ct2 += t2.substr(1);
        // We go through the found pairings in search of the two conditions
        for (auto &ass : mAssoc) {
            auto &cands = ass.second;
            if (std::find(cands.begin(),cands.end(),ct1)==cands.end()) continue;
            if (std::find(cands.begin(),cands.end(),ct2)==cands.end()) continue;
            int idx = stoi(ass.first.substr(1));
            if (ass.first[0]=='y') mVals[idx] = tru;
            else mVals[idx] = fls;
        }
    }
    return true;
}

bool Tribools::FindPairings() {
    mAssoc.clear();
    for (auto &c : mConds) {
        if (c.Unknowns()!=2) continue;
        auto &ts = c.GetTruths();
        if (ts.size()!=2) {
            cout << "Problems in the condition class!" << endl;
            continue;
        }
        for (auto j = 0u; j<2; ++j) {
            auto jc = (j==0 ? 1 : 0);
            if (mAssoc.find(ts[j])==mAssoc.end()) mAssoc[ts[j]] = vector<string>();
            mAssoc[ts[j]].push_back(ts[jc]);
        }
    }
    return true;
}

bool Tribools::LoopAttempts() {
    // We need to do this, until we find some triggering value
    for (auto &p : mPotentials) {
        int idx = p.v1;
        Tribools try1(*this);
        Tribools try2(*this);
        try1.mVals[idx] = tru;
        try2.mVals[idx] = fls;

        bool g1 = try1.Solve();
        bool g2 = try2.Solve();

        // This was undecisive
        cout << endl << g1 << " " << g2 << endl << endl;
        if (g1 and g2) {
            continue;
        } else if (g1) {
            *this = try1;
            break;
        } else if (g2) {
            *this = try2;
            break;
        } else {
            return false;
        }
    }
    return true;
}

bool Tribools::RemoveNonDual() {
    auto iter = mAssoc.begin();
    auto endIter = mAssoc.end();
    for(; iter != endIter; ) {
        if (iter->second.size()<2) mAssoc.erase(iter++);
        else ++iter;
    }
    return true;
}

bool Tribools::Solve() {
    int despairLvl = 0;
    unsigned prevconds = mConds.size();
    cout << endl << "s " << prevconds << endl;
    EraseDuplicates();
    prevconds = mConds.size();
    bool goodness = true;
    for (int iter = 0; iter<100; ++iter) {
        if (!Updates()) {
            goodness = false;
            break;
        }
        cout << "i " << prevconds << endl;
        unsigned conds = mConds.size();
        // Stop when there are no more conditions to monitor
        if (conds==0) break;
        // If there is a redundant step, we try to use special measures
        if (conds==prevconds) {
            // Start with cleaning up the "extra" conditions
            EraseDuplicates();
            conds = mConds.size();
            if (despairLvl==0) {
                ++despairLvl;
                // Method 1 : we eliminate "violating triplet conditions"
                cout << "Retrying method 1: " << endl;
                if (!Method1()) {
                    goodness = false;
                    break;
                }
            } else if (despairLvl==1) {
                ++despairLvl;
                // Method 2 : make an educated guess
                cout << "Retrying method 2: " << endl;
                if (!Method2()) {
                    goodness = false;
                    break;
                }
                conds = mConds.size();
            } else {
                // Getting too desperate, this needs to stop
                // TODO: we could implement a method that uses
                // general guessing (i.e. not only conditions of
                // variables). This could be useful for non-sudoku
                // purposes.
                goodness = false;
                break;
            }
        } else if (despairLvl>0) {
            // Despair levels normalizing
            despairLvl = 0;
        }
        prevconds = conds;
    }
    return goodness;
}
