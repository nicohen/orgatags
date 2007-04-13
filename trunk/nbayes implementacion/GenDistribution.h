#ifndef GENDISTRIBUTION_H
#define GENDISTRIBUTION_H

using namespace std;

class GenDistribution
{
    public:
    virtual double getProb(double state) const = 0;
};

#endif // ndef GENDISTRIBUTION_H
