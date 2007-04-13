/*
 * querypal.cpp
 *
 * Compute the probabilities of certain observations, given a PAL.
 * This will only compute P(A|M), where M is a PAL model.  However,
 * with a little work you can compute more complicated relationships.
 * For example: P(A|B,M) = P(A,B|M)/P(B|M).
 *
 * Author: Daniel Lowd <lowd@cs.washington.edu>
 */

#include "AbsVarAbstractionSet.h"
#include <fstream>
using namespace std;

int main(int argc, char* argv[])
{
    // Check arguments
    if (argc < 3) {
        cerr << "Usage: ./querypal <model> <querydata>\n";
        return 1;
    }

    // Load model
    ifstream modelin(argv[1]);
    if (!modelin) {
        cerr << "Error: could not open model file \"" << argv[1] << "\".\n";
        return 1;
    }

    AbsVarAbstractionSet model;
    modelin >> model;

    // Open queries file
    ifstream queryin(argv[2]);
    if (!queryin) {
        cerr << "Error: could not open queries file \"" << argv[2] << "\".\n";
        return -1;
    }

    // Read in each query.
    VarSet q;
    while (queryin >> q) {
        double p = model.getLogLikelihood(q);
        cout << "log(P" << q << ") = " << p << endl;
    }

    return 0;
}
