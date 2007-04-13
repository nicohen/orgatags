/*
 * samplepal.cpp
 *
 * Sample a PAL a specified number of times, writing the observations to 
 * stdout.
 *
 * Author: Daniel Lowd <lowd@cs.washington.edu>
 */

#include "AbsVarAbstractionSet.h"
#include <fstream>
#include <stdlib.h>
#include <time.h>

int main(int argc, char* argv[])
{
    // Check arguments
    if (argc < 3) {
        cerr << "Usage: ./samplepal <modelfile> <numsamples>\n";
        return 1;
    }

    // Initiate random number generator
    srand(time(0));

    // Get number of samples
    int numsamples = atoi(argv[2]);

    // Load model file
    ifstream modelin(argv[1]);
    if (!modelin) {
        cerr << "Error loading model file \"" << argv[1] << "\".\n";
        return 1;
    }

    AbsVarAbstractionSet model;
    modelin >> model;
    //cout << model;

    // Output requested number of samples to stdout
    while (numsamples--) {
        cout << ((AbstractionSet&)model).getSample() << endl;
        //cout << model.getSample() << endl;
    }
    return 0;
}
