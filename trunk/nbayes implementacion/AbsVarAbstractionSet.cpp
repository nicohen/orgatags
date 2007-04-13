#include "AbsVarAbstractionSet.h"
#include <ctype.h>
#include <math.h>

#define M_PRIOR 0.1
#define INITIAL_WEIGHT 1.0

// HACK
#define TEST_HACKS

class AbsData {
public:
    double** varValProbs;
    double** savedVarValProbs;
    double** varValCounts;
    double*  totalVarCounts;
    bool*    relevant;
    VarSchema schema;

    AbsData(VarSchema s) :schema(s) { alloc(); }
    AbsData(const AbsData& other) 
        :varValProbs(0), savedVarValProbs(0), varValCounts(0), 
         totalVarCounts(0), relevant(0)
     { (*this) = other; }
    ~AbsData() { clear(); }

    void alloc()
    {
        int numVars = schema.getNumVars();
        varValProbs = new double*[numVars];
        varValCounts = new double*[numVars];
        savedVarValProbs = new double*[numVars];
        totalVarCounts = new double[numVars];
        relevant = new bool[numVars];

        // Initialize the new data
        for (int var = 0; var < numVars; var++) {

            int numVals = schema.getNumVals(var);
            varValProbs[var] = new double[numVals];
            varValCounts[var] = new double[numVals];
            savedVarValProbs[var] = new double[numVals];

            if (schema.isContinuous(var)) {
                // FIXME: not properly defining all values here...
                varValCounts[var][0] = 0.0;
                varValCounts[var][1] = INITIAL_WEIGHT;
                varValCounts[var][2] = 0.0;
                totalVarCounts[var] = INITIAL_WEIGHT;
            } else {
                for (int val = 0; val < numVals; val++) {
                    varValProbs[var][val] = 1.0/numVals;
                    varValCounts[var][val] = 1.0/numVals;
                    savedVarValProbs[var][val] = 0.0;
                }
                totalVarCounts[var] = 1.0;
            }
            relevant[var] = true;
        }
    }

    void clear()
    {
        int numVars = schema.getNumVars();
        for (int var = 0; var < numVars; var++) {
            delete [] varValProbs[var];
            delete [] varValCounts[var];
            delete [] savedVarValProbs[var];
        }
        delete [] varValProbs;
        delete [] varValCounts;
        delete [] savedVarValProbs;
        delete [] totalVarCounts;
        delete [] relevant;
    }

    const AbsData& operator=(const AbsData& other)
    {
        clear();
        schema = other.schema;
        alloc();

        for (int var = 0; var < schema.getNumVars(); var++) {
            for (int val = 0; val < schema.getNumVals(var); val++) {
                varValProbs[var][val] = other.varValProbs[var][val];
                varValCounts[var][val] = other.varValCounts[var][val];
                savedVarValProbs[var][val] = other.savedVarValProbs[var][val];
            }
            totalVarCounts[var] = other.totalVarCounts[var];
            relevant[var] = other.relevant[var];
        }
        return (*this);
    }
};

AbsVarAbstractionSet::AbsVarAbstractionSet(const AbsVarAbstractionSet& other)
    :AbstractionSet(), globalVarValProbs(0)
{
    this->operator=(other);
}

AbsVarAbstractionSet& AbsVarAbstractionSet::operator=(
        const AbsVarAbstractionSet& other)
{
    // Delete global distribution data
    if (globalVarValProbs != 0) {
        for (int i = 0; i < numVars; i++) {
            delete [] globalVarValProbs[i];
        }
        delete [] globalVarValProbs;
    }

    // Delete all abstractions
    while (numAbs > 0) {
        numAbs--;
        delete (AbsData*)(abstractions[numAbs]->getData());
        delete abstractions[numAbs];
        abstractions.deleteLast();
        weights.deleteLast();
    }

    // Set new schema
    schema = other.schema;
    numVars = other.numVars;

    // Copy other parameters
    fuzzyAbs = other.fuzzyAbs;
    vardiv = other.vardiv;
    numObs = other.numObs;

    // Copy global distributions
    if (other.globalVarValProbs != NULL) {
        globalVarValProbs = new double*[numVars];
        for (int i = 0; i < numVars; i++) {
            globalVarValProbs[i] = new double[schema.getNumVals(i)];
            for (int j = 0; j < schema.getNumVals(i); j++) {
                globalVarValProbs[i][j] = other.globalVarValProbs[i][j];
            }
        }
    }

    // Add all abstractions (and their data)
    for (int a = 0; a < other.numAbs; a++) {
        AbsData* otherData = (AbsData*)other.abstractions[a]->getData();
        // HACK: assume they're all null abstractions.  Probably true.
        addAbstraction(new Abstraction(schema));
        delete (AbsData*)(abstractions[a]->getData());
        abstractions[a]->setData(new AbsData(*otherData));
    }

    return (*this);
}

AbsVarAbstractionSet::~AbsVarAbstractionSet()
{
    // Delete added data in each abstraction
    for (int a = 0; a < numAbs; a++) {
        delete (AbsData*)(abstractions[a]->getData());
    }

    if (globalVarValProbs != NULL) {
        for (int i = 0; i < numVars; i++) {
            delete [] globalVarValProbs[i];
        }
        delete [] globalVarValProbs;
    }
}

#ifndef PI
#define PI 3.1415927
#endif

Prob AbsVarAbstractionSet::getProb(const VarSet& x, int a) const
{
    Prob p = 1.0;
    AbsData* data;
    for (int i = 0; i < numVars; i++) {
        //if (fuzzyAbs || (x.isObserved(i) && !abs.isObserved(i))) {
#ifdef TEST_HACKS
        if (x.isTested(i)) {
#endif
            data = (AbsData*)abstractions[a]->getData();
            if (schema.isContinuous(i)) {
                double z = (x[i] - data->varValProbs[i][0]);
                double s2 = data->varValProbs[i][1];

                if (!x.isObserved(i)) {
                    p *= data->varValProbs[i][2];
                } else {
                    /* DEBUG */
                    if (p.isZero()) {
                        cout << "P is zero WAY before\n";
                        return p;
                    }
                    p *= (1.0 - data->varValProbs[i][2]);

                    if (s2 > 0) {
#if 0
                        /* DEBUG */
                        if (p.isZero()) {
                            cout << "P is zero before\n";
                            cout << data->varValProbs[i][2]<< endl;
                        }
#endif
                        p.logplus(-0.5*z*z/s2);
                        p *= 1.0/sqrt(2.0*PI*s2);

                        /* DEBUG */
                        if (p.isZero()) {
#if 0
                            cout << "p became zero!\n";
                            cout << "abs = " << a << "; var = " << i << endl;
                            cout << "z = " << z << "; s2 = " << s2 << endl;
                            cout << "exp = " << exp(-0.5*z*z/s2)/sqrt(2.0*PI*s2) << endl;
#endif
                            return p;
                        }
                    // What's the right way to deal with zero variance?
                    } else if (z != 0.0) {
                        // DEBUG
                        cout << "abs = " << a << "; var = " << i << endl;
                        return Prob(0.0);
                    }
                }
            } else if (x.isObserved(i)) {
                p *= data->varValProbs[i][(int)x[i]];
            }

            // DEBUG
            if (p.isZero()) {
#if 0
                cout << "p became zero!\n";
                cout << "abs = " << a << "; var = " << i << endl;
                cout << "val = " << x[i] << endl;
#endif
                return Prob(0.0);
            }
#ifdef TEST_HACKS
        }
#endif
        //}
    }

    return p;
}

double AbsVarAbstractionSet::getProb(int varIndex, double varVal, int a) const
{
    // DEBUG
    cout << "Using wrong getProb!!\n";

    AbsData* data = (AbsData*)abstractions[a]->getData();
    if (schema.isContinuous(varIndex)) {
        if (varVal == VarSet::UNKNOWN) {
            return Prob(data->varValProbs[varIndex][3]);
        } else {
            double coeff = 1.0/sqrt(2.0*PI*data->varValProbs[varIndex][1]);
            double z = (varVal - data->varValProbs[varIndex][0]);
            return Prob(coeff*exp(-0.5*z*z/(data->varValProbs[varIndex][1])));
        }
    } else {
        return Prob(data->varValProbs[varIndex][(int)varVal]);
    }
}

#include "ContDistribution.h"

vector<GenDistribution*> AbsVarAbstractionSet::createMarginals(
        const VarSet& evidence) const
{
    vector<GenDistribution*> dMarginals(numVars);
    vector<Distribution*> marginals(numVars);
    vector<ContDistribution*> mixtures(numVars);
    Prob margweights[numAbs];

    // Initialize all marginals
    for (int v = 0; v < numVars; v++) {

        dMarginals[v] = 0;
        marginals[v] = 0;
        mixtures[v] = 0;

        // Skip evidence vars
        if (evidence.isTested(v)) {
            continue;
        }

        // Create the actual distributions.  Caller will take ownership
        // of this memory.
        if (schema.isContinuous(v)) {
            dMarginals[v] = mixtures[v] = new ContDistribution();
        } else {
            dMarginals[v] = marginals[v] 
                = new Distribution(schema.getRange(v), 0.0);
        }
    }


    // Compute probability of each abstraction, given the evidence
    // (Keep track of total weight, for normalization later.)
    Prob totalWeight = 0.0;
    for (int a = 0; a < numAbs; a++) {
        margweights[a] = getProb(evidence, a) * weights[a];
        totalWeight += margweights[a];
    }


    // Add the contribution of each abstraction to each distribution 
    for (int v = 0; v < numVars; v++) {

        // Skip known vars
        if (evidence.isTested(v)) {
            continue;
        }

        for (int a = 0; a < numAbs; a++) {

            double w = margweights[a]/totalWeight;
            AbsData* data = (AbsData*)abstractions[a]->getData();

            if (schema.isContinuous(v)) {
                // Add another Gaussian to the mixture, with weight w.
                mixtures[v]->addComponent(w, data->varValProbs[v][0],
                        data->varValProbs[v][1]);
            } else {
                // Add this abstraction into the marginals
                for (int val = 0; val < schema.getRange(v); val++) {
                    Prob probVal = data->varValProbs[v][val];
                    (*marginals[v])[val] += w * probVal;
                }
            }
        }
    }

    return dMarginals;
}

// TODO: why are we copying the list?  Why not use a const ref?
AbstractionSet* AbsVarAbstractionSet::createJoint(list<int> queryVars, 
            const VarSet& evidence) const
{
    Prob newWeights[numAbs];
    Prob totalWeight = 0.0;

    // Sum over all abstractions
    for (int a = 0; a < numAbs; a++) {

        // Compute probability of abstraction, using the evidence
        newWeights[a] = getProb(evidence, a) * weights[a];
        totalWeight += newWeights[a];
    }

    // Create a copy of this mixture model, but with new weights...
    AbsVarAbstractionSet* ret = new AbsVarAbstractionSet(*this);

    for (int a = 0; a < numAbs; a++) {
        ret->weights[a] = newWeights[a]/totalWeight;
    }

    return ret;
}

void AbsVarAbstractionSet::allocParameters()
{
    // All of this is taken care of by extendParameters().
}

void AbsVarAbstractionSet::extendParameters()
{
    for (int a = 0; a < numAbs; a++) {
        AbsData* data = (AbsData*)(abstractions[a]->getData());

        if (data == NULL) {
            data = new AbsData(schema);
            abstractions[a]->setData(data);

            for (int i = 0; i < numVars; i++) {
                if (schema.isContinuous(i)) {
                    if (!abstractions[a]->isObserved(i)) {
                        if (!globalVarValProbs) {
                            data->varValProbs[i][0] = 0.0;
                            data->varValProbs[i][1] = INITIAL_WEIGHT;
                            data->varValProbs[i][2] = 1.0;
                        } else {
                            data->varValProbs[i][0] = globalVarValProbs[i][0];
                            data->varValProbs[i][1] = globalVarValProbs[i][1];
                            data->varValProbs[i][2] = M_PRIOR;
                        }
                    } else {
                        // Estimate variance from global variance...
                        data->varValProbs[i][1] 
                            = globalVarValProbs[i][1]/vardiv;

                        data->varValProbs[i][0] = (*abstractions[a])[i]
                        // Add a little bit of noise...
                            + 0.00 * data->varValProbs[i][1]
                               * (double)rand()/RAND_MAX;
                        data->varValProbs[i][2] = M_PRIOR;
                    }
                } else if (abstractions[a]->isObserved(i)) {
                    for (int j = 0; j < schema.getRange(i); j++) {
                        if (!globalVarValProbs) {
                            data->varValProbs[i][j]
                                = M_PRIOR/schema.getRange(i)
                                   /(1.0 + M_PRIOR);
                        } else {
                            data->varValProbs[i][j]
                                = M_PRIOR * globalVarValProbs[i][j]
                                   /(1.0 + M_PRIOR);
                        }
                    }
                    data->varValProbs[i][(int)(*abstractions[a])[i]] 
                        += 1.0/(1.0 + M_PRIOR);
                }
                (*abstractions[a])[i] = Abstraction::UNKNOWN;
            }
        }
    }
}

void AbsVarAbstractionSet::initParameters(VarSet* obs, int numObs)
{
    // Initialize data
    double* totalCounts = new double[numVars];
    globalVarValProbs = new double*[numVars];
    for (int i = 0; i < numVars; i++) {
        if (schema.isContinuous(i)) {
            globalVarValProbs[i] = new double[3];
            globalVarValProbs[i][0] = 0.0;
            globalVarValProbs[i][1] = INITIAL_WEIGHT;
            globalVarValProbs[i][2] = 0.0;
            totalCounts[i] = INITIAL_WEIGHT;
        } else {
            globalVarValProbs[i] = new double[schema.getRange(i)];
            for (int v = 0; v < schema.getRange(i); v++) {
                globalVarValProbs[i][v] = 1.0/schema.getRange(i);
            }
            totalCounts[i] = 1.0;
        }
    }

    // Count occurrences of each value
    for (int o = 0; o < numObs; o++) {
        for (int i = 0; i < numVars; i++) {
            if (schema.isContinuous(i)) {
                if (obs[o].isObserved(i)) {
                    //cout << "obs[" << o << "][" << i << "] = " << obs[o][i] << endl;
                    globalVarValProbs[i][0] += obs[o][i];
                    globalVarValProbs[i][1] += obs[o][i] * obs[o][i];
                } else {
                    globalVarValProbs[i][2]++;
                }
                totalCounts[i]++;
            } else if (obs[o].isObserved(i)) {
                if ((int)obs[o][i] >= schema.getRange(i)) {
                    cout << "Error: observation " << o << " has value " << 
                        obs[o][i] << " > " << (schema.getRange(i)-1) << 
                        " for variable " << i << endl;
                    continue;
                }
                globalVarValProbs[i][(int)obs[o][i]]++;
                totalCounts[i]++;
            }
        }
    }

    // Normalize
    for (int i = 0; i < numVars; i++) {
        if (schema.isContinuous(i)) {
            double x = globalVarValProbs[i][0];
            double x2 = globalVarValProbs[i][1];
            double n = totalCounts[i] - globalVarValProbs[i][2];

            globalVarValProbs[i][0] = x/n;
            globalVarValProbs[i][1] = (x2 - x*x/n)/(n-1);
            globalVarValProbs[i][2] /= totalCounts[i];
            /* DEBUG
            cout << i << ": mean = " << globalVarValProbs[i][0] << "; "
                << " var = " << globalVarValProbs[i][1] << "; " 
                << "p(missing) = " << globalVarValProbs[i][2] << endl;
                */
        } else {
            for (int v = 0; v < schema.getRange(i); v++) {
                globalVarValProbs[i][v] /= totalCounts[i];
#if 0
                // DEBUG
                if (v > 0)
                    cout << " "
                cout << globalVarValProbs[i][v];
#endif
            }
        }
    }

    delete [] totalCounts;
    numObs = 0;
}

void AbsVarAbstractionSet::resetParameters()
{
    /* Ignore this for now...

    for (int a = 0; a < absVarValProbs.size(); a++) {
        for (int var = 0; var < numVars; var++) {
            delete [] absVarValProbs[a][var];
            delete [] absVarValCounts[a][var];
        }
        delete [] absVarValProbs[a];
        delete [] absVarValCounts[a];
        delete [] totalAbsVarCounts[a];
    }

    absVarValProbs  = GrowArray<double**>(numAbs);
    absVarValCounts = GrowArray<double**>(numAbs);
    totalAbsVarCounts = GrowArray<double*>(numAbs);

    extendParameters();
    */
}

void AbsVarAbstractionSet::assignObservation(const VarSet& obs, int a, double p)
{
    // Record effect of the observation in our counts.
    Abstraction& abs = *abstractions[a];
    AbsData* data = (AbsData*)(abs.getData());
    for (int var = 0; var < numVars; var++) {
        if (!abs.isObserved(var)) {
            if (schema.isContinuous(var)) {
                if (obs.isObserved(var)) {
                    data->varValCounts[var][0] += p*obs[var];
                    data->varValCounts[var][1] += p*obs[var]*obs[var];
                } else {
                    data->varValCounts[var][2] += p;
                }
                data->totalVarCounts[var] += p;
            } else if (obs.isObserved(var)) {
                data->varValCounts[var][(int)obs[var]] += p;
                data->totalVarCounts[var] += p;
            }
        }
    }
    numObs++;
}

void AbsVarAbstractionSet::updateParameters()
{
    AbsData* prevData = NULL;
    int lastA = 0;

    for (int a = 0; a < numAbs; a++) {
        bool isDuplicate = true;
        AbsData* data = (AbsData*)(abstractions[a]->getData());

        if (prevData == NULL) {
            isDuplicate = false;
        }

        for (int var = 0; var < numVars; var++) {
            if (schema.isContinuous(var)) {
                double x = data->varValCounts[var][0];
                double x2 = data->varValCounts[var][1];
                double n = data->totalVarCounts[var] 
                            - data->varValCounts[var][2];

                if (n == 0.0 || isinf(n) || isnan(n)) {
                    data->varValProbs[var][0] = globalVarValProbs[var][0];
                    data->varValProbs[var][1] = globalVarValProbs[var][1];
                    data->varValProbs[var][2] = globalVarValProbs[var][2];
                } else {
                    data->varValProbs[var][0] = x/n;
                    data->varValProbs[var][1] = (x2 - x*x/n)/n;
                    if (data->varValProbs[var][1] <= 0) {
                        weights[a] = 0.0;
                        // DEBUG
                        cout << "ERROR: abs = " << a << "; var = " << var << endl;
                        cout << "n = " << n << "; x = " << x << "; x2 = " << x2 << endl;
                    }
                    data->varValProbs[var][2] /= data->totalVarCounts[var];
                }

                if (isDuplicate && 
                       (data->varValProbs[var][0] 
                           != prevData->varValProbs[var][0]
                        || data->varValProbs[var][1] 
                           != prevData->varValProbs[var][1]
                        || data->varValProbs[var][2] 
                           != prevData->varValProbs[var][2])) {
                    isDuplicate = false;
                }

                data->varValCounts[var][0] = 0.0;
                data->varValCounts[var][1] = INITIAL_WEIGHT;
                data->varValCounts[var][2] = 0.0;
                data->totalVarCounts[var]  = INITIAL_WEIGHT;
            } else {
                for (int val = 0; val < schema.getRange(var); val++) {
                    data->varValProbs[var][val] =
                        data->varValCounts[var][val]/data->totalVarCounts[var];
                    data->varValCounts[var][val] = 1.0/schema.getRange(var);
                }
                data->totalVarCounts[var] = 1.0;

                for (int val = 0; 
                        isDuplicate && val < schema.getRange(var); val++) {
                    if (data->varValProbs[var][val] 
                            != prevData->varValProbs[var][val]) {
                        isDuplicate = false;
                    }
                }
            }
        }

        // Prune duplicate abstractions
        if (isDuplicate) {
            weights[lastA] += weights[a];
            weights[a] = 0;
        } else {
            prevData = data;
            lastA = a;
        }
    }
    numObs = 0;
}

void AbsVarAbstractionSet::saveParameters()
{
    // Save current parameters
    for (int a = 0; a < numAbs; a++) {
        AbsData* data = (AbsData*)(abstractions[a]->getData());
        for (int var = 0; var < numVars; var++) {
            for (int val = 0; val < schema.getNumVals(var); val++) {
                data->savedVarValProbs[var][val] 
                    = data->varValProbs[var][val];
            }
        }
    }
}

void AbsVarAbstractionSet::restoreParameters()
{
    // Restore current parameters
    for (int a = 0; a < numAbs; a++) {
        AbsData* data = (AbsData*)(abstractions[a]->getData());
        for (int var = 0; var < numVars; var++) {
            for (int val = 0; val < schema.getNumVals(var); val++) {
                data->varValProbs[var][val] 
                    = data->savedVarValProbs[var][val];
            }
        }
    }
}

istream& AbsVarAbstractionSet::readParameters(istream& in)
{
    // We must already have a valid schema when this is called;
    // otherwise, it's all bad.

    char c;

    // Read opening [
    while (in.get(c) && isspace(c)) {} 
    if (c != '{') {
        in.putback(c);
        in.clear(ios::badbit);
        return in;
    }

    //for (int a = 0; a < numAbs; a++) 
    int a = 0;
    while(1)
    {
        while (in && in.get(c) && isspace(c)) {}
        if (!in) {
            break;
        }
        if (c != '[') {
            in.putback(c);
            in.clear(ios::badbit);
            return in;
        }

        // Ensure that we have enough abstractions added
        while (a >= numAbs) {
            addAbstraction(new Abstraction(schema));
        }

        AbsData* data = (AbsData*)(abstractions[a]->getData());
        for (int var = 0; var < numVars; var++) {
            for (int val = 0; val < schema.getNumVals(var); val++) {
                in >> data->varValProbs[var][val];
            }

            // Each variable should end with a semicolon, except for the last
            // which should end with a closing ].
            c = ' ';
            while (in.get(c) && isspace(c)) {}
            if ((var < numVars - 1 && c != ';')
                    || (var == numVars - 1 && c != ']')) {
                in.putback(c);
                in.clear(ios::badbit);
                return in;
            }
        }
        a++;
    }

    while (in.get(c) && isspace(c)) {} 
    if (c != '}') {
        in.putback(c);
        in.clear(ios::badbit);
        return in;
    }

    return in;
}


ostream& AbsVarAbstractionSet::writeParameters(ostream& out) const 
{
    out << "{\n";
    for (int a = 0; a < numAbs; a++) {
        /* HACK...
        if (weights[a] == 0.0) {
            continue;
        }
        */
        
        AbsData* data = (AbsData*)(abstractions[a]->getData());
        out << "[";
        for (int var = 0; var < numVars; var++) {
            for (int val = 0; val < schema.getNumVals(var); val++) {
                out << data->varValProbs[var][val] << " ";
            }
            if (var < numVars - 1) {
                out << "; ";
            }
        }
        out << "]\n";
    }
    out << "}\n";
    return out;
}

VarSet AbsVarAbstractionSet::getSample(int a) const
{
    VarSet ret(numVars);
    AbsData* data = (AbsData*)abstractions[a]->getData();

    for (int i = 0; i < numVars; i++) {
        
        double p = (double)rand()/RAND_MAX;

        if (schema.isContinuous(i)) {
            cout << "ERROR: continuous variables not yet supported!\n";
        } else {
            for (int value = 0; value < schema[i]; value++) {
                p -= data->varValProbs[i][value];
                if (p <= 0) {
                    ret[i] = value;
                    break;
                }
            }
        }
    }

    return ret;
}
