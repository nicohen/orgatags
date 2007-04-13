#ifndef PROB_H
#define PROB_H
#include <math.h>

#define SMALL_PROB 1.0e-50
#define REALLY_SMALL_PROB 1.0e-100
#define SMALL_LOG  -115.12925
#define REALLY_SMALL_LOG  -230.25851

#ifndef INFINITY
#define INFINITY HUGE_VAL
#endif

class Prob
{
public:
    Prob() :v(0), logv(0), vGood(false), logvGood(false) { /* NOP */ }

    Prob(double value)
        :v(value), logv(0), vGood(true), logvGood(false) { /* NOP */ }

    Prob(const Prob& other)
        :v(other.v), logv(other.logv),
         vGood(other.vGood), logvGood(other.logvGood) { /* NOP */ }
    
    double ln() const { if (!logvGood) { logv = log(v); logvGood = true; }
                        return logv; }
    double val() const { if (!vGood && logv < -300) return 0;
                         if (!vGood) { v = exp(logv); vGood = true; }
                         return v; }

    Prob logplus(double d) {
        logv = ln() + d;
        vGood = false;
        return (*this);
    }

    // Basic arithmetical operators
    Prob operator*(const Prob& other) const
    {
        return slowMult(other);
        /*
        Prob ret = val() * other.val();
        if (ret.isReallySmall()) {
            ret.logv = ln() + other.ln();
            ret.logvGood = true;
            ret.vGood = false;
            return ret;
        }
        return ret;
        */
    }
    Prob slowMult(const Prob& other) const;

    Prob operator/(const Prob& other) const
    {
        return slowDiv(other);
    }
    Prob slowDiv(const Prob& other) const;

    Prob& operator*=(const Prob& other);
    Prob& operator/=(const Prob& other);
    Prob operator+(const Prob& other) const;
    Prob& operator+=(const Prob& other);

    Prob operator*(double other) const 
    { return this->operator*(Prob(other)); }

    Prob operator/(double other) const 
    { return this->operator/(Prob(other)); }

    Prob operator+(double other) const
    { return this->operator+(Prob(other)); }

    // Conversion
    operator double() const
    { return val(); }

    // Assignment operator
    Prob& operator=(const Prob& other);

    bool isSmall() const {
        return (!logvGood || v < SMALL_LOG) && (!vGood || v < SMALL_PROB);
    }

    bool isReallySmall() const {
        return (logvGood && logv < REALLY_SMALL_LOG) 
            || (vGood && v < REALLY_SMALL_PROB);
    }

    bool isZero() const {
        return (logvGood && isinf(logv))
            || (vGood && v == 0);
    }


private:
    /* Since we do lazy evaluation, even checking the value of
     * a Prob may alter its state, if it causes a value to be computed
     * and then cached.
     */
    mutable double v;
    mutable double logv;
    mutable bool vGood;
    mutable bool logvGood;
};

inline Prob operator*(double a, const Prob& b)
{
    return b.operator*(a);
}

inline Prob operator+(double a, const Prob& b)
{
    return b.operator+(a);
}

inline Prob operator/(double a, const Prob& b)
{
    return b.operator/(a);
}

#endif // ndef PROB_H
