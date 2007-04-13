#ifndef CONTDISTRIBUTION_H
#define CONTDISTRIBUTION_H

#ifndef PI
#define PI 3.14159265
#endif

#include <list>
#include "GenDistribution.h"

using namespace std;

/*
 * ConstDistribution is a wrapper for a mixture of Gaussians, stored
 * as a list of arrays of doubles.  Each element in the list contains three
 * values: weight, mean, and variance, for each component Gaussian.
 *
 * The purpose for this class (and GenDistribution, for that matter) is
 * to make it easy to return a marginal distribution, where some of the
 * distributions are multinomials and some are mixtures of Gaussians.
 * The multinomials are handled by the class Distribution, which is the
 * only one with a sensical name, since it predates this horrid mess.
 *
 * The reason that we want to return marginal distributions is for doing
 * inference experiments with naive Bayes.  See
 * ../gibbs-c/NaiveBayesInference.h for more on that.
 */
typedef struct {
    double weight;
    double mean;
    double var;
} triplet;

class ContDistribution : public GenDistribution
{
    list<triplet> data;

public:
    ContDistribution(list<triplet> initdata) 
        : data(initdata)
    { /* NOP */ }

    ContDistribution()
    { /* NOP */ }

    virtual ~ContDistribution() { /* NOP */ }

    void addComponent(double weight, double mean, double var)
    {
        triplet component = {weight, mean, var};
        data.push_back(component);
    }

    void normalize()
    {
        double totalWeight = 0.0;
        list<triplet>::iterator i;

        // Compute total weight of all components
        for (i = data.begin(); i != data.end(); i++) {
            totalWeight += i->weight;
        }

        // Renormalize
        for (i = data.begin(); i != data.end(); i++) {
            i->weight /= totalWeight;
        }
    }

    virtual double getProb(double state) const {

        double p = 0.0;
        list<triplet>::const_iterator i;
        for (i = data.begin(); i != data.end(); i++) {
            double w = i->weight;
            double mean = i->mean;
            double var = i->var;
            p += w * exp(-0.5 * (state - mean) * (state - mean) / var)
                / sqrt(2 * PI * var);
        }

        return p;
    }

};

#endif // ndef CONTDISTRIBUTION_H
