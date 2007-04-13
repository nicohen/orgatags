#include "Prob.h"

Prob Prob::slowMult(const Prob& other) const
{
    Prob ret;
    ret.vGood = false;
    ret.logvGood = false;

    if (logvGood && other.logvGood) {
        ret.logv = logv + other.logv;
        ret.logvGood = true;
    }

    if (vGood && other.vGood) {
        ret.v = v * other.v;
        if (ret.v > REALLY_SMALL_PROB) {
            ret.vGood = true;
        }
    }

    if (ret.vGood || ret.logvGood) {
        return ret;
    }

    ret.logv = ln() + other.ln();
    ret.logvGood = true;
    return ret;
}

Prob Prob::slowDiv(const Prob& other) const
{
    Prob ret;
    ret.vGood = false;
    ret.logvGood = false;

    if (logvGood && other.logvGood) {
        ret.logv = logv - other.logv;
        ret.logvGood = true;
    }

    if (vGood && other.vGood) {
        ret.v = v / other.v;
        if (ret.v > REALLY_SMALL_PROB) {
            ret.vGood = true;
        }
    }

    if (ret.vGood || ret.logvGood) {
        return ret;
    }

    ret.logv = ln() - other.ln();
    ret.logvGood = true;
    return ret;
}

Prob& Prob::operator*=(const Prob& other) 
{
    // TODO: make this more efficient...
    (*this) = (*this) * other;
    return (*this);
}

Prob& Prob::operator/=(const Prob& other) 
{
    // TODO: make this more efficient...
    (*this) = (*this) / other;
    return (*this);
}

Prob Prob::operator+(const Prob& other) const
{
    if (isReallySmall() || other.isReallySmall()) {
        if (ln() - other.ln() > 25) {
            return *this;
        } else if (other.ln() - ln() > 25) {
            return other;
        } else {
            Prob ret;
            ret.logv = log(exp(ln() - other.ln()) + 1) + other.ln();
            ret.vGood = false;
            ret.logvGood = true;
            return ret;
        }
    } else {
        return Prob(val() + other.val());
    }
}

#include <iostream>
Prob& Prob::operator+=(const Prob& other)
{
    if (isReallySmall() || other.isReallySmall()) {
        if (ln() - other.ln() > 25 || other.isZero()) {
            // Add nothing
        } else if (other.ln() - ln() > 25 || isZero()) {
            // Use the argument
            *this = other;
        } else {
#if 0
            double prevLog = logv;
#endif
            // Do sum of logs
            logv = log(exp(ln() - other.ln()) + 1) + other.ln();
            logvGood = true;
            vGood = false;

#if 0
            // DEBUG
            if (logv < prevLog || logv < other.ln()) {
                cout << "\nERROR: decrease in logv in +=\n";
                cout << "our log: " << prevLog << endl;
                cout << "oth log: " << other.ln() << endl;
                cout << "new log: " << logv << endl << endl;
                cout << "asdf: " << (exp(ln() - other.ln()) + 1) << endl;
            }
#endif
        }
    } else {
        v = val() + other.val();
        vGood = true;
        logvGood = false;
    }

    return *this;
}

Prob& Prob::operator=(const Prob& other)
{
    v = other.v;
    logv = other.logv;
    vGood = other.vGood;
    logvGood = other.logvGood;
    return (*this);
}
