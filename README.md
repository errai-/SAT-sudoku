A SAT solver to solve a sudoku, inspired by the Project Euler problem 96.
Here, we get a set of 50 sudokus to solve. Some easier, some harder.
This is a more evolved revision of the earlier 'sudoku-' repo.

Since the parsing of general boolean conditions is a huge effort, a simpler methodology
is provided here. The problem is to be fed an array of conditions that
are by default considered as "and" connected. Within the single statements
we can connect "yes" ("y") and "no" ("n") format references to the grid of
(tri)boolean values. Thus, we have replaced the ordinary "not" ("~") operator by "n"
and added a redundant "yes" operator "y". Despite the redundancy, this is extremely
convenient for string parsing and visual debugging. In the future versions, this
method might be refined in some way or another.

The "y" and "n" operators are connected using only the common "or" ("V") operator.
What happened to "and" ("A")? In early versions of the program, also this was included.
However, it turned out that in general we won't need any explicit "and" conditions.
The class Tribools holds an array of tri-boolean (true,false,maybe) values and
an array of conditions. These conditions are implicitly bonded by "and" conditions.
Since we have both the presence of a boolean value ("y") and 'not' its value ("n"),
it turns out that we can make all "ands" implicit, so that the given conditions
only include "or" conditions. Therefore, in a single condition (the "cond" class),
we only operate on lists of boolean values that are implicitly bonded by "or"s.
The or sign ("V") is typically only printed to give the viewer visual pleasure.

The specific issue of solving sudoku's is performed via the interface of the class SudoHold.
The most important thing this class does, is that it sets up a Tribools object
containing 829 triboolean values. Each consecutive set of nine tribooleans tell the boolean
truth of numbers 1-9 residing within the current cell. Neighboring ninelets of tribooleans
describe the neighboring columns  Furthermore, it asserts the necessary conditions
for a sudoku to happen: in each cell we have exactly one number of the nine,
on each row, column and 3x3 square we have all the number 1-9. Moreover, the SudoHold
class asserts all the conditions required by a sudoku.

All the four types of conditions are given in the format "exactly on of nine boolean
values is true". For the sake of an example, let us consider the nine tribooleans 1-9.
The condition that at least one of these is true is easily satisfied: with the syntax
used for this SAT solver, we need to satisfy "y1 V y2 V y3 V y4 V y5 V y6 V y7 V y8 V y9".
It is more complicated to produce the requirement "exactly one" is true. This can be
achieved by asserting "~(yi A yj)" - or in other words "ni V nj" for all pairs i and j in
{1,...,9}. We end up with a total of 39x81x4 = 12636 conditions, of which ~1000 are
redundant, since some of the "ni V nj" conditions of the 3x3 squares overlap with
those of the columns and rows.

The generic "filling in" method of solving gets some 40/50 of the test sudokus solved.
For the remaining ones, we face a "duality" problem. No more values can be eliminated,
but there are compelling triboolean choices for flipping - in the hopes that this
causes a cascade that solves the whole sudoku. With a closer inspection, however, we find
two sub-cases for this kind of issue here. The obvious one is to simply try the two
different alternatives. Nevertheless, some of the alternatives are found by using
purely formal logic.

In this context, we use the specific case of conditions with only two variables present.
We note that for boolean values b1 and b2, "b1 V b2" = "~(~b1 A ~b2)". On the other hand,
if for a third boolean value b3 we have the two conditions "~b1 V b3" and "~b2 V b3", we
can combine these conditions to give "(~b1 A ~b2) V b3". Now however, we already know that
the statement, "~(~b1 A ~b2)" must be true, so "(~b1 A ~b2)" is false. Thus, the boolean
value b3 must be true. By seeking such conditions "b1 V b2" we can look for boolean values
that are connected to both ~b1 and ~b2. This forces the conditions b3 to be true. This
procedure solves additional 4 sudokus.

For the remaining six, we simply make attempts. Such boolean values are chosen as seeds that
have the highest present of both "y" and "n" versions in statements that hold only two unknown
boolean values. By this method we can maximize the effect of both "true" and "false" attempts
of the said boolean within the sudoku grid.
