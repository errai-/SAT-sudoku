#include "satsolv.h"

bool Cond::Parse() {
    mTruths.clear();
    mDelims.clear();
    string curr;
    for (char &c : mCond) {
        if (c != ' ')
            curr += c;
        else {
            if (curr=="V" or curr=="A")
                mDelims.push_back(curr);
            else {
                mTruths.push_back(curr);
            }
            curr = "";
        }
    }
    mTruths.push_back(curr);
    return (mTruths.size()==mDelims.size()+1);
}

bool Cond::Update() {
    if (mStat!=mbe) return false;
    if (mTruths.size()!=mDelims.size()+1) return false;
    tribool valprev = mbe;

    {
        string &currs = mTruths[0];
        if (currs=="t") {
            valprev = tru;
        } else if (currs=="f") {
            valprev = fls;
        } else {
            unsigned pos = stoi(currs.substr(1));
            bool reverse = (currs[0]=='n');
            valprev = mVals->at(pos);

            if (valprev == tru) { if (reverse) valprev = fls; }
            else if (valprev == fls) { if (reverse) valprev = tru; }
            if (valprev==tru) currs = "t";
            else if (valprev==fls) currs = "f";
        }
    }

    // We proceed from left to right, eliminating "from left"
    for (auto i = 0u; i < mDelims.size();) {
        string &d = mDelims[i];
        auto idx = i+1;
        tribool val = mbe;

        string &curris = mTruths[idx];
        if (curris=="t") {
            val = tru;
        } else if (curris=="f") {
            val = fls;
        } else {
            unsigned pos = stoi(curris.substr(1));
            bool reverse = (curris[0]=='n');
            val = mVals->at(pos);

            if (val == tru) { if (reverse) val = fls; }
            else if (val == fls) { if (reverse) val = tru; }
            if (val==tru) curris = "t";
            else if (val==fls) curris = "f";
        }

        // The previous value is exact, so we can retract
        if (valprev != mbe) {
            // One element removed, nothing changes for i
            tribool method = Eval(valprev,val,d);
            if (method==tru) { mTruths[idx] = "t"; }
            else if (method==fls) { mTruths[idx] = "f"; }
            // We remove the current delimeter and the previous position
            mTruths.erase(mTruths.begin()+i);
            mDelims.erase(mDelims.begin()+i);
            valprev = method;
        } else {
            // Both values are maybe, nothing to be done
            i += 1;
            valprev = val;
        }
    }

    if (mTruths.size()!=mDelims.size()+1) return false;

    {
        string &currs = mTruths.back();
        if (currs=="t") {
            valprev = tru;
        } else if (currs=="f") {
            valprev = fls;
        } else {
            valprev = mbe;
        }
    }

    // We proceed from left to right, eliminating "from left"
    for (auto i = mDelims.size(); i > 0;) {
        int idx = i-1;
        string &d = mDelims[idx];
        tribool val = mbe;

        string &curris = mTruths[idx];
        if (curris=="t") {
            val = tru;
        } else if (curris=="f") {
            val = fls;
        } else {
            val = mbe;
        }

        // The previous value is exact, so we can retract
        if (valprev != mbe) {
            // One element removed, so we need to make i smaller
            tribool method = Eval(valprev,val,d);
            if (method==tru) { mTruths[idx] = "t"; }
            else if (method==fls) { mTruths[idx] = "f"; }
            // We remove the current delimeter and the previous position
            mTruths.erase(mTruths.begin()+i);
            mDelims.erase(mDelims.begin()+idx);
            valprev = method;
        } else {
            // Both values are maybe, nothing to be done
            valprev = val;
        }
        i -= 1;
    }

    // Count the unknowns
    mUnk = 0;
    for (auto &t : mTruths)
        if (t != "t" and t != "f") ++mUnk;

    if (mUnk==0) {
        if (mTruths[0]=="t") {
            mStat = tru;
        } else {
            mStat = fls;
            return false;
        }
    } else if (mUnk==1) {
        unsigned pos = stoi(mTruths[0].substr(1));
        bool reverse = (mTruths[0][0]=='n');
        if (reverse) mVals->at(pos) = fls;
        else mVals->at(pos) = tru;
        mStat = tru;
    }

    return true;
}

bool SudoHold::ReadSudo(string fname) {
    std::ifstream ifile(fname);
    regex grd("^Grid ([0-9]+)");
    regex lne("^([0-9]{9})");
    std::smatch match;
    if (ifile.is_open()) {
        string line;
        while (getline(ifile,line)) {
            if (std::regex_search(line,match,grd)) {
                mSudos.push_back(Tribools());
                mTags.push_back(std::stoi(match[1]));
                // Loop over rows
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
                // Loop over all cells: set conditions so that exactly one number is true in one cell
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
                // Loop over all rows: set conditions so that each number is present exactly once in a row
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
                // Loopover all cols: set conditions so that each number is present exactly once in a column
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
                // This is the best part of the trip: loop over the 9 squares
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
                cout << endl;
                for (int iter = 0; iter<100; ++iter) {
                    cout << mSudos.back().Updates() << " " << mSudos.back().mConds.size() << endl;
                    if (mSudos.back().mConds.size()==0) break;
                }
                mSudos.back().PrintConds();
                PrintSudo(mSudos.size()-1);
            }
        }
    } else {
        return false;
    }

    return true;
}

void SudoHold::PrintSudo(int idx) {
    if (idx < mSudos.size() and idx < mTags.size()) {
        cout << endl << "Printing grid " << mTags[idx] << endl;
        int counter = 0;
        int currval = 0;
        int rowidx = 0;
        for (auto &val : mSudos[idx].mVals) {
            if (++counter>9) {
                if (++rowidx>9) {
                    cout << endl;
                    rowidx = 1;
                }
                cout << currval;
                counter = 1;
                currval = 0;
            }
            if (val==tru) {
                currval = counter;
            }
        }
        cout << currval << endl;
    }
}

int main(void) {
    try {
        SudoHold sudoHolder;
        if (!sudoHolder.ReadSudo("p096_sudoku.txt")) return 1;
    } catch (std::regex_error &e) {
        cout << e.what() << " " << e.code() << endl;
    } catch (std::exception &e) {
        cout << e.what() << endl;
    }

    return 0;
}
