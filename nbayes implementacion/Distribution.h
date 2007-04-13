#ifndef DISTRIBUTION_H
#define DISTRIBUTION_H
#include <vector>
#include "GenDistribution.h"

using namespace std;

class Distribution : public GenDistribution
{
private:
    vector<double> probs;

public:
    Distribution() { /* NOP */ }

    // Create marginal multinomial distribution over i values
    Distribution(int dimension) 
        :probs(dimension, 1.0) 
    { /* NOP */ }

    Distribution(int dimension, double val) 
        :probs(dimension, val) 
    { /* NOP */ }

    Distribution(const double* p, int numVars) 
        :probs(numVars)
    {
        for (int i = 0; i < numVars; i++) {
            probs[i] = p[i];
        }
    }

    Distribution(const vector<double>& p)
        :probs(p) { /* NOP */ }

    Distribution(const Distribution& other)
        :probs(other.probs) { /* NOP */ }

    virtual ~Distribution() { /* NOP */ }

    // Implementation of GenDistribution interface
    virtual double getProb(double state) const {

        if (state < 0.0 || state >= dim()) {
            return 0;
        } else {
            return probs[(int)state];
        }
    }

    int dim() const { return probs.size(); }

    // Normalize the distribution, so all probs sum to one
    void normalize() 
    { 
        double totalSum = 0.0;
        for (int i = 0; i < dim(); i++) {
            totalSum += probs[i];
        }
        if (totalSum > 0.0) {
            for (int i = 0; i < dim(); i++) {
                probs[i] /= totalSum;
            }
        }
    }

#if 0 // A bad idea
    void pseudoNormalize()
    {
        bool renorm = false;
        double totalSum = 0.0;
        for (int i = 0; i < dim(); i++) {
            totalSum += probs[i];
            if (probs[i] < 1.0e-10) {
                renorm = true;
            }
        }
        if (renorm && totalSum > 0.0) {
            for (int i = 0; i < dim(); i++) {
                probs[i] /= totalSum;
            }
        }
    }
#endif

    // Array-like get/set methods
    double  operator[](int index) const  { return probs[index]; }
    double& operator[](int index)        { return probs[index]; }
    double  get(int index) const         { return probs[index]; }
    void    set(int index, double value) { probs[index] = value; }


    Distribution& operator*=(const Distribution& other)
    {
        if (dim() != other.dim()) {
            // TODO: throw an exception?
            return *this;
        }

        for (int i = 0; i < dim(); i++) {
            probs[i] *= other[i];
        }
        return *this;
    }

    Distribution operator*(const Distribution& other) const
    {
        Distribution ret(*this);
        ret *= other;
        return ret;
    }

    Distribution& operator/=(const Distribution& other)
    {
        if (dim() != other.dim()) {
            // TODO: throw an exception?
            return *this;
        }

        for (int i = 0; i < dim(); i++) {
            // HACK: avoid divide by zero
            if (other[i] > 0.0) {
                probs[i] /= other[i];
            }
        }
        return *this;
    }
    
    Distribution operator/(const Distribution& other) const
    {
        Distribution ret(*this);
        ret /= other;
        return ret;
    }

    Distribution& operator=(const Distribution& other)
    {
        probs = other.probs;
        return *this;
    }
};

ostream& operator<<(ostream& out, const Distribution& d);

#endif // DISTRIBUTION_H
