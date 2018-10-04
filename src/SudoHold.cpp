#include "SudoHold.h"

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
