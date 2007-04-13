#ifndef ABSTRACTION_H
#define ABSTRACTION_H

#include "VarSet.h"

class Abstraction : public VarSet
{
public:
    Abstraction() :VarSet(), data(NULL), weight(0) {}

    Abstraction(const NumSet<double>& v)
        :VarSet(v), data(NULL), weight(0) {}

    Abstraction(VarSchema s)
        :VarSet(s.getNumVars()), data(NULL), weight(0) {}


    double getWeight() const { return weight; }
    void   setWeight(double w) { weight = w; }

    void* getData() { return data; }
    void setData(void* p) { data = p; }

private:
    void* data;
    double weight;
};





// Read an Abstraction from a stream.
inline istream& operator>>(istream& in, Abstraction& abs)
{
    VarSet v;
    in >> v;

    abs = Abstraction(v);
	return in;
}
#endif
