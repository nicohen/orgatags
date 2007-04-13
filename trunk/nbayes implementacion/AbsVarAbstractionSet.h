#ifndef ABSVARABSTRACTIONSET_H
#define ABSVARABSTRACTIONSET_H
#include "AbstractionSet.h"

class AbsVarAbstractionSet : public AbstractionSet
{
    double**  globalVarValProbs;

#if 0
    GrowArray<double**> absVarValProbs;
    GrowArray<double**> absVarValCounts;
    GrowArray<double*>  totalAbsVarCounts;

    GrowArray<double**> savedAbsVarValProbs;
#endif

    bool fuzzyAbs;
    int  numObs;
    double vardiv;

public:
    AbsVarAbstractionSet(bool fuzzy = false, double div = 3.0)
        :AbstractionSet(), globalVarValProbs(0), fuzzyAbs(fuzzy), vardiv(div)
            {}

    AbsVarAbstractionSet(const VarSchema& s, bool fuzzy = false, 
            double div = 3.0) 
        :AbstractionSet(s), globalVarValProbs(0), fuzzyAbs(fuzzy), vardiv(div)
            {}

    AbsVarAbstractionSet(const AbsVarAbstractionSet& other);
    AbsVarAbstractionSet& operator=(const AbsVarAbstractionSet& other);

    virtual ~AbsVarAbstractionSet();

    // TODO: add support for continuous vars in this
    virtual VarSet getSample(int a) const;

    virtual Prob getProb(const VarSet& x, int a) const;
    virtual double getProb(int varIndex, double varVal, int a) const;
#if 0
    virtual vector<GenDistribution*> computeMarginals(
            const VarSet& evidence) const;
    virtual Distribution computeJoint(list<int> queryVars, 
            const VarSet& evidence) const;
#endif
    virtual vector<GenDistribution*> createMarginals(const VarSet& x) const;
    virtual AbstractionSet* createJoint(list<int> queryVars, 
            const VarSet& evidence) const;

    void allocParameters();
    virtual void initParameters(VarSet* obs, int numObs);
    virtual void extendParameters();
    virtual void assignObservation(const VarSet& obs, int a, double p);
    virtual void updateParameters();
    virtual void saveParameters();
    virtual void restoreParameters();
    virtual void resetParameters();

    virtual istream& readParameters(istream& in);
    virtual ostream& writeParameters(ostream& out) const;

protected:
    void checkRelevance(int a, int var);
};
#endif // ndef VARABSTRACTIONSET_H
