/*
 * NumSet.h
 *
 * Contains a set of n numbers with indices 0 to (n-1).
 *
 * Author: Daniel Lowd <lowd@cs.washington.edu>
 */

#ifndef NUMSET_H
#define NUMSET_H

#include <string.h>   // Included for memcpy function.
#include <iostream>
#include "GrowArray.h"

using namespace std;

template <class T>
class NumSet
{
protected:
    int numNums;
    T*  values;

public:
    NumSet() : numNums(0), values(0)    { /* NOP */ }

    // Create a new NumSet of the specified dimension.
    explicit NumSet(int numberOfVariables)
        : numNums(numberOfVariables), values(new T[numberOfVariables])
    { memset(values, numNums*sizeof(T), 0); }

    // Construct an NumSet using an array of T for values.
    NumSet(T* v, int numberOfVars)  
        : numNums(numberOfVars), values(new T[numberOfVars])
    { memcpy(values, v, sizeof(T) * numNums); }

    // Copy constructor
    NumSet(const NumSet& other)     
        : numNums(other.numNums), values(new T[other.numNums])
    { memcpy(values, other.values, sizeof(T) * numNums); }

    // Destructor
    ~NumSet()                       { delete [] values; }

    // Assignment operator
    const NumSet& operator=(const NumSet& other);

    // Equality comparator
    bool operator==(const NumSet& other) const;

    // Get the number of ints in this set.
    int getNumNums() const          { return numNums; }

    // Access values in the NumSet
    T getValue(int i) const       { return values[i]; }
    void setValue(int i, T val)   { values[i] = val; }
    const T& operator[](int i) const { return values[i]; }
    T& operator[](int i)          { return values[i]; }

    // HACK
    T* getArray() { return values; }
};

template <class T>
const NumSet<T>& NumSet<T>::operator=(const NumSet<T>& other)
{
    // If the two NumSets are distinct, copy the values.
    if (this->values != other.values) {
        delete [] values;
        numNums = other.numNums;
        values = new T[numNums];
        memcpy(values, other.values, sizeof(T) * numNums);
    }
    return *this;
}

template <class T>
bool NumSet<T>::operator==(const NumSet<T>& other) const
{
    if (numNums != other.numNums) {
        return false;
    }
    for (int i = 0; i < numNums; i++) {
        if (values[i] != other.values[i]) {
            return false;
        }
    }

    return true;
}

template <class T>
ostream& operator<<(ostream& out, const NumSet<T>& x)
{
    int n = x.getNumNums();
    out << "(";
    out << x[0];
    for (int i = 1; i < n; i++) {
        out << " " << x[i];
    }
    out << ")";

    return out;
}

#include <ctype.h>

template <class T>
istream& operator>>(istream& in, NumSet<T>& x)
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
    T v;
    GrowArray<T> values;
    while (in.get(c)) {
        // Skip spaces
        while (isspace(c)) {
            in.get(c);
        }
        if (!isdigit(c) && c != '-') {
            break;
        }
        in.putback(c);
        in >> v;
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
        x = NumSet<T>(values.getArray(), values.size());
    }

    return in;
}

#endif // ndef NUMSET_H
