/*
 * testpal.cpp
 *
 * Test a PAL model by computing the log likelihood of the observations
 * in a user-specified test set.
 *
 * Author: Daniel Lowd <lowd@cs.washington.edu>
 */

#include "AbsVarAbstractionSet.h"

#include <math.h>
#include <fstream>
using namespace std;

int main(int argc, char* argv[])
{
    // Check arguments
    if (argc < 3) {
        cerr << "Usage: ./testpal [-v] <model> <testdata>\n";
        return 1;
    }

    int nextArg = 1;
    bool reportfull = false;

    if (!strcmp(argv[nextArg], "-v")) {
        reportfull = true;
        nextArg++;
    }

    // Load model
    ifstream modelin(argv[nextArg]);
    if (!modelin) {
        cerr << "Error: could not open model file \"" 
            << argv[nextArg] << "\".\n";
        return 1;
    }
    nextArg++;

    AbsVarAbstractionSet model;
    modelin >> model;

    // DEBUG
    //cout << model;

    // Open test file
    ifstream testin(argv[nextArg]);
    if (!testin) {
        cerr << "Error: could not open test file \"" 
            << argv[nextArg] << "\".\n";
        return 1;
    }
    nextArg++;

    // Read in each observation and compute its log likelihood
    double loglikelihood = 0.0;
    double sq_loglikelihood = 0.0;
    int observations = 0;
    VarSet t;
    while (testin >> t) {
        double l = model.getLogLikelihood(t);
        loglikelihood += l;
        sq_loglikelihood += l*l;
        observations++;
        // DEBUG
        //cout << loglikelihood/observations << endl;
    }

    // Output results (both log likelihood and stdev)
    cout << "log likelihood  = " << loglikelihood/observations << endl;
    double sigma_sq = (sq_loglikelihood 
            - (loglikelihood*loglikelihood)/observations)
            /(observations*observations);
    cout << "sigma = " << sqrt(sigma_sq) << endl;

    if (reportfull) {
        cout << "sum = " << loglikelihood << endl;
        cout << "sumsq = " << sq_loglikelihood << endl;
        cout << "n = " << observations << endl;
    }
    return 0;
}
