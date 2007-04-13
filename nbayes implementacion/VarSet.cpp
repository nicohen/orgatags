#include "VarSet.h"

/* DEPRECATED! */
bool VarSet::isSubsetOf(const VarSet& other) const
{
    // HACK: breaking inheritence abstraction here, for speed.
    for (int i = 0; i < numNums; i++) {
        if (other.values[i] != UNKNOWN
                && values[i] != UNKNOWN
                && values[i] != other.values[i]) {
            return false;
        }
    }

    return true;
}

ostream& operator<<(ostream& out, const VarSet& x)
{
    out << "(";
    for (int i = 0; i < x.getNumNums(); i++) {
        if (i > 0) {
            out << " ";
        }
        if (!x.isObserved(i)) {
            out << "?";
        } else if (!x.isTested(i)) {
            out << "*";
        } else {
            out << x[i];
        }
    }
    out << ")";
    return out;
}

#include <ctype.h>
istream& operator>>(istream& in, VarSet& x)
{
    char c;

    // Skip leading spaces
    while (in.get(c) && isspace(c)) {}

    if (!in) {
        return in;
    }

    // Must begin with '('
    if (c != '(') {
        // Report formating error
        in.clear(ios::badbit);
        return in;
    }

    // Read array of values
    double v;
    GrowArray<double> values;
    while (in.get(c)) {
        // Skip spaces
        while (isspace(c)) {
            in.get(c);
        }
        if (c == '?') {
            v = VarSet::UNKNOWN;
        } else if (c == '*') {
            v = VarSet::UNTESTED;
        } else if (!isdigit(c) && c != '-') {
            break;
        } else {
            in.putback(c);
            in >> v;
        }
        values.append(v);
    }

    // Must end with ')'
    if (c != ')') {
        // Report formating error
        in.clear(ios::badbit);
        return in;
    }

    // Test stream state before assigning to NumSet
    if (in) {
        x = VarSet(NumSet<double>(values.getArray(), values.size()));
    }

    return in;
}
