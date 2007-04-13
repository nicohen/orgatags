/*
 * VarSchema.h
 *
 * Defines the range for each of a set of variables.
 *
 * Author: Daniel Lowd <lowd@cs.washington.edu>
 */

#ifndef VARSCHEMA_H
#define VARSCHEMA_H

#include "NumSet.h"
using namespace std;

class VarSchema : public NumSet<int>
{
public:
    // Construct a schema for 0 variables.
    VarSchema() :NumSet<int>() {}

    // Construct a schema with ranges specified by the given IntSet.
    VarSchema(const NumSet<int>& other) 
        : NumSet<int>(other) { /* NOP */ }

    // Construct a schema with nV variables,
    // where the ith variable has range r[i].
    VarSchema(int* r, int nV) 
        : NumSet<int>(r, nV) { /* NOP */ }

    // Construct a schema with nV variables,
    // each having the range of states [0, range-1].
    VarSchema(int range, int nV)
        : NumSet<int>(nV)
    {
        for (int i = 0; i < nV; i++) {
            setValue(i, range);
        }
    }

    // Copy constructor
    VarSchema(const VarSchema& other) 
        : NumSet<int>(other) { /* NOP */ }

    // Assignment operator
    VarSchema& operator=(const VarSchema& other)
    {
        NumSet<int>::operator=(other);
        return *this;
    }

    // Destructor
    ~VarSchema() { /* NOP */ }

    int getNumVals(int i) const { return (getValue(i) < 0) ? 3 : getValue(i); }

    // Returns the number of states of this variable.
    int getRange(int i) const { return getValue(i); }

    bool isContinuous(int i) const { return (getValue(i) < 0); }

    // Returns the number of variables described by this schema.
    int getNumVars() const { return getNumNums(); }
};

// Read a VarSchema from a stream.
inline istream& operator>>(istream& in, VarSchema& schema)
{
    NumSet<int> v;
    in >> v;
    schema = VarSchema(v);
	return in;
}  

// (operator<< is inherited from NumSet<int>.)

#endif // ndef VARSCHEMA_H
