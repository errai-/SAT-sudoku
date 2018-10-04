#include "satsolv.h"

//////////////////
// Cond functions:
//////////////////

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

//////////////////////
// Tribools functions:
//////////////////////

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
            if (despairLvl==0) {
                ++despairLvl;
                // Start with cleaning up the "extra" conditions
                EraseDuplicates();
                conds = mConds.size();
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

//////////////////////
// SudoHold functions:
//////////////////////

bool SudoHold::ReadSudos(string fname) {
    std::ifstream ifile(fname);
    regex grd("^Grid ([0-9]+)");
    regex lne("^([0-9]{9})");
    std::smatch match;
    if (ifile.is_open()) {
        string line;
        // Loop over potential "starter lines" in file
        while (getline(ifile,line)) {
            // Does the "starter line" match our expectations?
            if (std::regex_search(line,match,grd)) {
                // Add a skeleton tribool structure and the index of the current sudoku to the corresponding vectors
                mSudos.push_back(Tribools());
                mTags.push_back(std::stoi(match[1]));

                // Loop over nine rows to add a single sudoku to the boolean grid
                for (int l = 0; l<9; ++l) {
                    if (getline(ifile,line) and std::regex_search(line,match,lne)) {
                        // Loop over columns
                        for (char &c : line) {
                            int r = c-'0';
                            // Loop over values
                            if (r==0) {
                                for (int dmy = 1; dmy<=9; ++dmy) mSudos.back().mVals.push_back(mbe);
                            } else {
                                for (int dmy = 1; dmy<=9; ++dmy) mSudos.back().mVals.push_back(dmy==r ? tru : fls);
                            }
                        }
                    } else {
                        mSudos.pop_back();
                        mTags.pop_back();
                        return false;
                    }
                }

                // This is a sudoku, so in the next few moments we add the conditions necessary to a sudoku:
                //
                //{ 1. Loop over all cells: set conditions so that exactly one number is true in one cell
                for (int row = 0; row < 9; ++row) {
                    for (int col = 0; col < 9; ++col) {
                        int currpos = 81*row+9*col;
                        // Add "at least one is true" condition
                        string exists = "y" + to_string(currpos);
                        // Loop over remaining numbers
                        for (int no = 1; no < 9; ++no)
                            exists += " V y" + to_string(currpos+no);
                        mSudos.back().AddCond(exists);
                        // Add "no other truths" conditions (loop over number pairs)
                        for (int no1 = 0; no1 < 8; ++no1) {
                            for (int no2 = no1+1; no2 < 9; ++no2) {
                                string only1 = "n" + to_string(currpos+no1) + " V n" + to_string(currpos+no2);
                                mSudos.back().AddCond(only1);
                            }
                        }
                    }
                }
                //} 1. Cell loop
                //
                //{ 2. Loop over all rows: set conditions so that each number is present exactly once in a row
                for (int row = 0; row < 9; ++row) {
                    // Loop over all numbers
                    for (int no = 0; no < 9; ++no) {
                        int currpos = 81*row + no;
                        // Add "at least one is true" condition
                        string exists = "y" + to_string(currpos);
                        // Loop over remaining cols
                        for (int col = 1; col < 9; ++col)
                            exists += " V y" + to_string(currpos+9*col);
                        mSudos.back().AddCond(exists);
                        // Add "no other truths" conditions (loop over column pairs)
                        for (int col1 = 0; col1 < 8; ++col1) {
                            for (int col2 = col1+1; col2 < 9; ++col2) {
                                string only1 = "n" + to_string(currpos+9*col1) + " V n" + to_string(currpos+9*col2);
                                mSudos.back().AddCond(only1);
                            }
                        }
                    }
                }
                //} 2. Row loop
                //
                //{ 3. Loop over all cols: set conditions so that each number is present exactly once in a column
                for (int col = 0; col < 9; ++col) {
                    // Loop over all numbers
                    for (int no = 0; no < 9; ++no) {
                        int currpos = 9*col + no;
                        // Add "at least one is true" condition
                        string exists = "y" + to_string(currpos);
                        // Loop over remaining rows
                        for (int row = 1; row < 9; ++row)
                            exists += " V y" + to_string(currpos+81*row);
                        mSudos.back().AddCond(exists);
                        // Add "no other truths" conditions (loop over row pairs)
                        for (int row1 = 0; row1 < 8; ++row1) {
                            for (int row2 = row1+1; row2 < 9; ++row2) {
                                string only1 = "n" + to_string(currpos+81*row1) + " V n" + to_string(currpos+81*row2);
                                mSudos.back().AddCond(only1);
                            }
                        }
                    }
                }
                //} 3. Column loop
                //
                //{ 4. This is the best part of the trip: loop over the 9 squares so that each number is present exactly once in a square
                for (int cpos = 0; cpos < 3; ++cpos) {
                for (int rpos = 0; rpos < 3; ++rpos) {
                    int lcorner = 9*3*cpos+81*3*rpos;
                    vector<int> vertices;
                    // Looping over the nine locations within the square
                    for (int row = 0; row < 3; ++row) {
                    for (int col = 0; col < 3; ++col) {
                        vertices.push_back(lcorner+9*col+81*row);
                    }}
                    // Loop over all numbers
                    for (int no = 0; no < 9; ++no) {
                        // Add "at least one is true" condition
                        string exists = "y" + to_string(vertices[0]+no);
                        for (int vtx = 1; vtx < 9; ++vtx)
                            exists += " V y" + to_string(vertices[vtx]+no);
                        mSudos.back().AddCond(exists);
                        // Add "no other truths" conditions (loop over vtx pairs)
                        for (int vtx1 = 0; vtx1 < 8; ++vtx1) {
                            for (int vtx2 = vtx1+1; vtx2 < 9; ++vtx2) {
                                string only1 = "n" + to_string(vertices[vtx1]+no) + " V n" + to_string(vertices[vtx2]+no);
                                mSudos.back().AddCond(only1);
                            }
                        }
                    }
                }}
                //} 4. Square loop
            } // Does the "starter line" match our expectations?
        } // Loop over potential "starter lines" in file
    } else {
        return false;
    }

    return true;
}

bool SudoHold::SolveSudos() {
    if (mTags.size()!=mSudos.size()) return false;

    unsigned success = 0;
    for (auto idx = 0u; idx < mSudos.size(); ++idx) {
        auto &sudo = mSudos[idx];

        if (sudo.Solve()) {
            PrintSudo(idx);
            if (sudo.mConds.size()==0) ++success;
            //else sudo.Print2Conds();
        } else {
            cout << "At idx " << idx << endl;
        }
    }
    cout << "Out of the " << mSudos.size() << " sudokus, " << success << " were solved successfully!" << endl;

    return true;
}

void SudoHold::PrintSudo(int idx) {
    if (idx < mSudos.size() and idx < mTags.size()) {
        cout << endl << "Printing grid " << mTags[idx] << ":" << endl;
        cout << "=================" << endl << endl;
        int counter = 0;
        int currval = 0;
        int rowidx = 0;
        int rowcount = 0;
        for (auto &val : mSudos[idx].mVals) {
            // Go through the nine boolean values per cell
            if (++counter>9) {
                // Go through the nine cells on a row
                if (++rowidx>9) {
                    // Induce a row change
                    cout << endl;
                    rowidx = 1;
                    if (++rowcount%3==0)
                        cout << "===============" << endl;
                }
                if (rowidx>1 and rowidx%3==1) cout << " | ";
                if (currval!=0) cout << currval;
                else cout << " ";
                counter = 1;
                currval = 0;
            }
            if (val==tru) {
                currval = counter;
            }
        }
        if (currval!=0) cout << currval << endl;
        else cout << " " << endl;
    }
}

////////
// main:
////////

int main(void) {
    try {
        SudoHold sudoHolder;
        if (!sudoHolder.ReadSudos("p096_sudoku.txt")) return 1;
        if (!sudoHolder.SolveSudos()) return 1;
    } catch (std::regex_error &e) {
        cout << e.what() << " " << e.code() << endl;
    } catch (std::exception &e) {
        cout << e.what() << endl;
    }

    return 0;
}
