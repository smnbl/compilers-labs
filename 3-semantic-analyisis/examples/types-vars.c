// RUN-WITH-ARGS: -dump-type-table

int variables_typecheck(int i, float f, string s)
{
    i;
    f;
    s;

    float foo[2];

    foo[0];
}
