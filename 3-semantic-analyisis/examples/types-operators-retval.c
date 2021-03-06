// RUN-WITH-ARGS: -dump-type-table

int operators() {
    // arithmetic binary operators: integers
    1 ^ 2 * 3 / 4 % 5 + 6 - 7;

    // arithmetic binary operators: floats
    1.0 ^ 2.0 * 3.0 / 4.0 % 5.0 + 6.0 - 7.0;

    // comparison operators
    1 == 2;
    1.0 < 3.0;

    // unary operators
    +1;
    -2.0;

    // assignment
    // note that our semantic analyser does not check whether the left hand side
    // of an assignment is an lvalue.
    "foo" = "bar";
}
