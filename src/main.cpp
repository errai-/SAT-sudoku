#include "SudoHold.h"

int main(void) {
    try {
        SudoHold sudoHolder;
        if (!sudoHolder.ReadSudos("txt/p096_sudoku.txt")) return 1;
        if (!sudoHolder.SolveSudos()) return 1;
    } catch (std::regex_error &e) {
        cout << e.what() << " " << e.code() << endl;
    } catch (std::exception &e) {
        cout << e.what() << endl;
    }

    return 0;
}
