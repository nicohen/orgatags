#include "AbstractionSet.h"
#include <ctype.h>
#include <math.h>

//#define BIGNUM 1000000000
#define BIGNUM 1

AbstractionSet::AbstractionSet(const VarSchema& s, int initialSize)
        : abstractions(initialSize), weights(initialSize), numAbs(0),
              schema(s), numVars(s.getNumVars())
{
    /* NOP */
}

AbstractionSet::~AbstractionSet()
{
    for (int i = 0; i < numAbs; i++) {
        delete abstractions[i];
    }
}

void AbstractionSet::addAbstraction(Abstraction* abs)
{
    abstractions.append(abs);
    numAbs++;

    // Ensure that every abstraction has some weight.
    if (weights.size() < numAbs) {
        double scaling = (numAbs - 1.0)/numAbs;
        for (int a = 0; a < numAbs; a++) {
            weights[a] *= scaling;
        }
        weights.append(1.0/numAbs);
    }

    // Add any parameters necessary to model variable probabilities,
    // conditioned on this abstraction.
    extendParameters();
}

void AbstractionSet::addNullAbstraction()
{
    Abstraction* abs = new Abstraction(VarSet(schema.getNumVars()));
    addAbstraction(abs);
}

void AbstractionSet::removeLastAbstraction()
{
    numAbs--;
    weights[numAbs] = 0.0;
    delete abstractions[numAbs];
}

int AbstractionSet::findAbstraction(Abstraction* abs) const
{
    for (int a = 0; a < numAbs; a++) {
        bool good = true;
        for (int v = 0; v < numVars; v++) {
            if ((*abstractions[a])[v] != (*abs)[v]) {
                good = false;
                break;
            }
        }
        if (good) {
            return a;
        }
    }
    return -1;
}

double AbstractionSet::getLogLikelihood(const VarSet* obs, int numObs) const
{
    double ll = 0.0;

    for (int i = 0; i < numObs; i++) {
        ll += getLogLikelihood(obs[i]);
    }
    return ll/numObs;
}

double AbstractionSet::getLogLikelihood(const VarSet& x) const
{
    Prob p = 0.0;
    for (int a = 0; a < numAbs; a++) {
        p += weights[a] * getProb(x, a);
    }

    return p.ln();
}

Prob AbstractionSet::getProb(const VarSet& x, int a) const
{
    Abstraction& abs = *abstractions[a];
    if (!x.isSubsetOf(abs)) {
        return 0;
    }

    Prob p = 1.0;
    for (int i = 0; i < numVars; i++) {
        if (x.isObserved(i) && !abs.isObserved(i)) {
            p *= getProb(i, x[i], a);
        }
    }

    return p;
}

double AbstractionSet::getProb(int varIndex, double varVal, int a) const
{
    return 1.0/schema.getRange(varIndex);
}

void AbstractionSet::resetWeights()
{
    double w = 1.0/numAbs;

    for (int a = 0; a < numAbs; a++) {
        weights[a] = w;
    }
}


double AbstractionSet::EMStep(VarSet* obs, int numObs)
{
    // HACK: make sure our newWeights are actually big enough...
    // since we could be adding more abstractions partway through.
    double newWeights[numAbs + numObs];
    for (int a = 0; a < numAbs + numObs; a++) {
        newWeights[a] = 0.0;
    }

    for (int i = 0; i < numObs; i++) {
        Prob tempWeights[numAbs+1];
        Prob totalWeight = 0.0;

        // E step; calculate the probability that each observation
        // belongs to each abstraction.
        for (int a = 0; a < numAbs; a++) {

            if (weights[a] == 0.0) {
                tempWeights[a] = 0.0;
            } else {
                // P(a|x) = P(x|a)*P(a) (ignoring P(x), since it's constant)
                tempWeights[a] = getProb(obs[i], a) * weights[a];

                totalWeight += tempWeights[a];
            }
        }

#if 0
        // The following hack isn't as helpful I thought it would be.
        if (totalWeight.isZero()) {
            // HACK HACK HACK
            Abstraction abs = obs[i];
            addAbstraction(new Abstraction(abs));
            tempWeights[numAbs] = 1;
            totalWeight += tempWeights[numAbs];

            // DEBUG
            //cout << "Totalweight is zero!\n";
            cout << "Added zero-weight observation\n";
        }
#endif

#if 0
        // DEBUG
        for (int a = 0; a < numAbs; a++) {
            if (tempWeights[a].ln() > totalWeight.ln()){
                Prob secondTotal = 0.0;
                Prob thirdTotal = 0.0;
                for (int a = 0; a < numAbs; a++) {
                    thirdTotal = secondTotal;
                    secondTotal += tempWeights[a];
                    cout << "tempWeights[" << a << "] = " << tempWeights[a].ln() << endl;
                    cout << "totalWeight = " << secondTotal.ln() << endl;
                    if (secondTotal.ln() < thirdTotal.ln()) {
                        cout << "EEEE prevWeight = " << thirdTotal.ln() << endl;
                        cout << "...sum... " << (thirdTotal + tempWeights[a]).ln() << endl;
                    }
                }
                break;
            }
        }
#endif

        // DEBUG
        double totalFrac = 0.0;

        // Normalize, so probabilities for all abstractions sum to one.
        for (int a = 0; a < numAbs; a++) {

            // P(a) = P(a|x)/P(x)
            double fracAssign = exp(tempWeights[a].ln() - totalWeight.ln());
            newWeights[a] += fracAssign;
			if (fracAssign > 0) {
                assignObservation(obs[i], a, fracAssign);
			}

            totalFrac += fracAssign;
        }

        // DEBUG
        if (fabs(totalFrac - 1.0) > 0.0001) {
            cout << "totalFrac = " << totalFrac << " (i=" << i << ")\n";
        }
    }

    /* M step: choose maximally probably hypothesis.
     * (Assign new weights based on the probable membership of
     * all observations.)  Keep track of change in weights, so
     * we know when to stop.
     */
    double weightChange = 0.0;
    for (int a = 0; a < numAbs; a++) {
        weightChange += fabs(newWeights[a]/numObs - weights[a]);
        weights[a] = newWeights[a]/numObs;
    }

    // Compute maximally likely parameters.
    updateParameters();


    /* DEBUG: print out an error message if our weights don't sum to 1.  This
     * would probably indicate some floating point underflow.
     */
    double finalTotalWeight = 0.0;
    for (int a = 0; a < numAbs; a++) {
        finalTotalWeight += weights[a];
    }
    if (fabs(finalTotalWeight - 1.0) > 0.0001) {
        cout << "ERROR: total weight = " << finalTotalWeight << endl;
        cout << "numAbs = " << numAbs << endl;
    }

    return weightChange;
}

typedef struct {
    double weight;
    Abstraction* abs;
} absWeight;

int cmpWeights(absWeight a, absWeight b)
{
    if (a.weight > b.weight) return 1;
    if (a.weight < b.weight) return -1;
    return 0;
}

int cmpAbs(const void* a1, const void* a2)
{
    const Abstraction* abs1 = *(const Abstraction**)a1;
    const Abstraction* abs2 = *(const Abstraction**)a2;

    if (abs1->getWeight() < abs2->getWeight()) {
        return 1;
    } else if (abs1->getWeight() > abs2->getWeight()) {
        return -1;
    } else {
        return 0;
    }
}

void AbstractionSet::prune(double percentWeight)
{
    abstractions[0]->setWeight(1.0);
    for (int a = 1; a < numAbs; a++) {
        abstractions[a]->setWeight(weights[a]);
    }

    abstractions.sort(cmpAbs);

    // Define replacement arrays
    GrowArray<Abstraction*> newAbstractions;
    GrowArray<double> newWeights;

    // Keep the zeroth abstraction
    newAbstractions.append(abstractions[0]);
    newWeights.append(weights[0]);

    // Keep all abstractions that comprise a certain fraction
    // of the total weight.
    double totalWeight = weights[0];
    for (int a = 1; a < numAbs; a++) {
        if (totalWeight < percentWeight) {
            totalWeight += abstractions[a]->getWeight();
            newAbstractions.append(abstractions[a]);
            newWeights.append(abstractions[a]->getWeight());
        } else {
            delete abstractions[a];
        }
    }

    // Update our actual parameters
    abstractions = newAbstractions;
    weights = newWeights;
    numAbs = abstractions.length();
}

istream& AbstractionSet::readAbstractions(istream& in)
{
    char c;
    while (in.get(c) && isspace(c)) {}

    if (c != '{') {
        in.putback(c);
        in.clear(ios::badbit);
        return in;
    }

    Abstraction abs;
    while (in >> abs) {
        addAbstraction(new Abstraction(abs));

        while (in.get(c) && isspace(c)) {}
        if (c == '}') {
            break;
        } else if (c == '(') {
            in.putback(c);
        } else {
            in.putback(c);
            in.clear(ios::badbit);
            return in;
        }
    }

    return in;
}


ostream& AbstractionSet::writeAbstractions(ostream& out) const
{
    out << "{\n";
    for (int a = 0; a < numAbs; a++) {
        /* HACK...
        if (weights[a] == 0) {
            continue;
        }
        */

        out << *abstractions[a] << endl;
    }
    out << "}\n";
    return out;
}

istream& AbstractionSet::readSchema(istream& in)
{
    in >> schema;
    numVars = schema.getNumVars();
    return in;
}

ostream& AbstractionSet::writeSchema(ostream& out) const
{
    out << schema << endl;
    return out;
}

istream& AbstractionSet::readWeights(istream& in)
{
    char c;
    while (in.get(c) && isspace(c)) {}

    if (c != '[') {
        in.putback(c);
        in.clear(ios::badbit);
        return in;
    }

    double w;
    int index = 0;
    while (in >> w) {
        if (index >= weights.length()) {
            weights.append(w);
        } else {
            weights[index] = w;
        }

        index++;

        while (in.get(c) && isspace(c)) {}
        if (c == ']') {
            break;
        } else if (isdigit(c)) {
            in.putback(c);
        } else {
            in.putback(c);
            in.clear(ios::badbit);
            return in;
        }
    }

    return in;
}

ostream& AbstractionSet::writeWeights(ostream& out) const
{
    bool firstWeight = true;

    out << "[";
    for (int a = 0; a < numAbs; a++) {
        if (firstWeight) {
            firstWeight = false;
        } else {
            out << " ";
        }
        out << weights[a];
    }
    out << "]\n";
    return out;
}

istream& operator>>(istream& in, AbstractionSet& model)
{
    model.readSchema(in);
    //model.readAbstractions(in);
    model.readWeights(in);
    model.readParameters(in);
    return in;
}

ostream& operator<<(ostream& out, const AbstractionSet& model)
{
    model.writeSchema(out);
    //model.writeAbstractions(out);
    model.writeWeights(out);
    model.writeParameters(out);
    return out;
}


VarSet AbstractionSet::getSample() const
{
    // Select a random abstraction
    double p = frand();
    int a = 0;
    for (int i = 0; i < numAbs; i++) {
        if (weights[i] > p) {
            a = i;
            break;
        }
        p -= weights[i];
    }

    return getSample(a);
}
