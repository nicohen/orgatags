/*
 * AbstractionSet.h
 *
 * Represents a PAL model as a set of abstractions.  The PAL model
 * facilitates learning and inference on (discrete) probability
 * distributions.  This particular implementation features methods
 * for learning, sampling, and adding random abstractions.
 * Input and output are done with C++ stream operators >> and <<.
 *
 * Author: Daniel Lowd <lowd@cs.washington.edu>
 */

#ifndef ABSTRACTIONSET_H
#define ABSTRACTIONSET_H
#include "GrowArray.h"
#include "VarSchema.h"
#include "VarSet.h"
#include "Prob.h"
#include <stdlib.h>
#include <iostream>

// HACK (for margtest)
#include <vector>
#include <list>
#include "GenDistribution.h"
#include "Distribution.h"

using namespace std;

#if 0
// For now, an Abstraction is just a VarSet.
typedef VarSet Abstraction;
#else
#include "Abstraction.h"
#endif

class AbstractionSet
{
protected:
    GrowArray<Abstraction*> abstractions;
    GrowArray<double> weights;
    int numAbs;
    VarSchema schema;
    int numVars;

public:
    AbstractionSet() :numAbs(0), numVars(0) {};
    AbstractionSet(const VarSchema& s, int initialSize = 16);
    virtual ~AbstractionSet();

    // NOTE: AbstractionSet takes ownership of a.
    void addAbstraction(Abstraction* a);
    void addNullAbstraction();
    void removeLastAbstraction();

    int findAbstraction(Abstraction* abs) const;
#if 1
    virtual VarSet getSample() const;
    virtual VarSet getSample(int a) const = 0;
#endif

    int getNumAbstractions() const { return numAbs; }
    double getWeight(int i) const { return weights[i]; }
    const Abstraction& getAbstraction(int i) const {return *abstractions[i];}

    double getLogProb(const VarSet& x) const { return getLogLikelihood(x); }
    double getLogLikelihood(const VarSet& x) const;
    double getLogLikelihood(const VarSet* obs, int numObs) const;
    virtual Prob getProb(const VarSet& x, int a) const;
    virtual double getProb(int varIndex, double varVal, int a) const;

    // HACK (for margtest) -- Implemented only in AbsVarAbstractionSet
    virtual vector<GenDistribution*> createMarginals(const VarSet& x) const = 0;
    // HACK (for jointtest) -- Implemented only in AbsVarAbstractionSet
    virtual AbstractionSet* createJoint(list<int> queryVars, 
            const VarSet& evidence) const = 0;
    virtual int getRange(int i) const { return schema.getRange(i); }

    // Weight all abstractions equally.
    void resetWeights();

    // Take a single step towards learning the proper abstraction weights
    // using the EM algorithm.
    double EMStep(VarSet* obs, int numObs);

    // Keep only a set fraction of abstractions (by total weight),
    // or those with minimum weight. 
    void prune(double weightFraction);
    void threshold(double threshold);

    // EM hooks for use in subclasses.
    virtual void initParameters(VarSet* obs, int numObs) {}
    virtual void extendParameters() {}
    virtual void assignObservation(const VarSet& obs, int a, double p) {}
    virtual void updateParameters() {}
    virtual void saveParameters() {}
    virtual void restoreParameters() {}
    virtual void resetParameters() {}


    // I/O
    istream& readAbstractions(istream& in);
    ostream& writeAbstractions(ostream& out) const;
    istream& readSchema(istream& in);
    ostream& writeSchema(ostream& out) const;
    istream& readWeights(istream& in);
    ostream& writeWeights(ostream& out) const;
    virtual istream& readParameters(istream& in) { return in; }
    virtual ostream& writeParameters(ostream& out) const { return out; }
    
    friend istream& operator>>(istream& in, AbstractionSet& as);
    friend ostream& operator<<(ostream& out, const AbstractionSet& as);

private:
    double frand() const { return random()/(double)RAND_MAX; }

    // Disallow copy constructor and assignment op
    AbstractionSet(const AbstractionSet& other);
    AbstractionSet& operator=(const AbstractionSet& other);
};

istream& operator>>(istream& in, AbstractionSet& model);
ostream& operator<<(ostream& out, const AbstractionSet& model);

#endif // ndef ABSTRACTIONSET_H
