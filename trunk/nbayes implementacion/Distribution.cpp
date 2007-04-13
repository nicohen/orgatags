#include "Distribution.h"
#include <iostream>

ostream& operator<<(ostream& out, const Distribution& d)
{
    out << '(';
    for (int i = 0; i < d.dim(); i++) {
        if (i > 0) {
            out << ' ';
        }
        out << d[i];
    }
    out << ')';

    return out;
}
