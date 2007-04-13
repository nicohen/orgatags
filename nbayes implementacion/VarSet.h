/*
 * VarSet.h
 *
 * Contains a set of variables, not all of which necessarily
 * need to be observed.
 *
 * Daniel Lowd <lowd@cs.washington.edu>
 */

#ifndef VARSET_H
#define VARSET_H

#include "NumSet.h"
using namespace std;

class VarSet : public NumSet<double>
{
public:
    static const int UNKNOWN  = -99999999; 
    static const int UNTESTED =  -9999999;

    VarSet() : NumSet<double>() { /* NOP */ }
    VarSet(const NumSet<double>& other) : NumSet<double>(other) { /* NOP */ }

    // Create a new VarSet with the specified dimension.
    explicit VarSet(int numberOfVariables)
        : NumSet<double>(numberOfVariables)
    {
        for (int i = 0; i < getNumNums(); i++) {
            setValue(i, UNKNOWN);
        }
    }

    // Destructor
    ~VarSet()                       { /* NOP */ }

    // Assignment operator
    const VarSet& operator=(const VarSet& other)
    { NumSet<double>::operator=(other); return *this; }


    // True if our variable assignment is strictly more specific
    // than that of our argument.
    bool isSubsetOf(const VarSet& other) const;

    int  getNumVars() const         { return getNumNums(); }
    //bool isObserved(int i) const    { return getValue(i) != UNKNOWN; }
    bool isObserved(int i) const    { return values[i] != UNKNOWN; }
    bool isTested(int i) const      { return values[i] != UNTESTED; }

    bool operator==(const VarSet& other) const
    {
        if (getNumVars() != other.getNumVars()) {
            return false;
        }
        for (int i = 0; i < getNumVars(); i++) {
            if (values[i] != other[i]) {
                return false;
            }
        }
        return true;
    }

    bool operator!=(const VarSet& other) const
    {
        return !(*this == other);
    }
};

ostream& operator<<(ostream& out, const VarSet& x);
istream& operator>>(istream& in, VarSet& x);

#endif // ndef VARSET_H
