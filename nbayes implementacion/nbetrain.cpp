/*
 * trainpal.cpp
 *
 * Learn a PAL model from training data using EM, given a schema for its
 * variables and a set of abstractions to use.  Writes out learned file.
 *
 * Author: Daniel Lowd <lowd@cs.washington.edu>
 */

#define REMOVE_CLUSTER_CENTERS
#define SAVE_BEST

#include "AbstractionSet.h"
#include "AbsVarAbstractionSet.h"

#include "VarSchema.h"
#include "VarSet.h"

// HACK
#include <stdio.h>

#include <time.h>
#include <ctype.h>
#include <fstream>
using namespace std;

// Variables that determine what we learn and how we learn it.
char* schemaFilename = NULL;
char* obsFilename = NULL;
char* tuningFilename = NULL;
char* holdoutFilename = NULL;
char* modelFilename = NULL;
bool verbose = true;
int modelType = 3;
bool sampledAbs = true;

// Variables for controlling overfitting
int holdoutNum = 0;
double holdoutFrac = 0;
double minllChangeEM  = 0.0001;
double minllChangeAdd = 0.001;

// NOTE: interface to set this is not currently well exposed.
// But that's because you probably don't want this interface.
// Thresholding on weight change is not the best stopping criterion.
double minWeightChange = 0.0;

// Setting variance of continuous variables
double vardiv = 2.5;

// Incremental abstraction loading
int absToLoad = 50;
int totalAbsLoaded = 0;

// Early stopping
int fixedEMiters = -1;

// Abstraction pruning
double pruneFrac = 0.999;
int pruneFreq = 5;

// Helper functions, for handling command-line arguments
void printUsage();
void badOption(char* arg);
void parseArgs(int argc, char* argv[]);

void printUsage()
{
    cout << 
"Usage: trainpal [-i#] [-r#] [-s <schemafile>] [-q] [-v] [-pw#] [-pf#]\n" <<
"                [-we#] [-wa#] [-wf#]\n" <<
"                <data> [holdout] <model>\n" <<
"\n" <<
" -i#: sets the initial number of clusters to load (default: 50)\n" <<
" -r#: sets the random seed, for repeatable experiments\n" <<
" -s <schemafile>: sets the schema, which describes each attribute\n" <<
" -q: run in quiet mode (turn off verbose logging)\n" <<
" -v: run in verbose mode (turn on verbose logging)\n" <<
" -pw#: set the total weight to keep each time we prune (default: 0.999)\n" <<
" -pf#: set the pruning frequency (default: 5, once every 5 iters)\n" <<
" -we#: set \\delta_{EM} (default: 0.0001)\n" <<
" -wa#: set \\delta_{Add} (default: 0.001)\n" <<
" -wf#: run a fixed number of EM iterations (instead of using hold-out set)\n";
}

void badOption(char* arg)
{
    cout << "Ignoring unrecognized option: " << arg << endl;
}

#define DOUBLEARG(offset) \
((strlen(currArg) > (offset)) ? (atof(currArg + (offset))) : (atof(argv[++argNum])))

#define INTARG(offset) \
((strlen(currArg) > (offset)) ? (atoi(currArg + (offset))) : (atoi(argv[++argNum])))

void parseArgs(int argc, char* argv[])
{
    // Handle command-line args.
    for (int argNum = 1; argNum < argc; argNum++) {
        char* currArg = argv[argNum];

        if (currArg[0] == '-') {

            // Consider abstraction pruning options
            if (currArg[1] == 'p') {
                if (currArg[2] == 'w') {
                    pruneFrac = DOUBLEARG(3);
                } else if (currArg[2] == 'f') {
                    pruneFreq = INTARG(3);
                } else {
                    badOption(currArg);
                }

            // Then consider stopping criteria
            } else if (currArg[1] == 'w') {
                switch (currArg[2]) {
                    case 'c': minWeightChange = DOUBLEARG(3);
                              break;
                    case 'e': minllChangeEM = DOUBLEARG(3);
                              break;
                    case 'a': minllChangeAdd = DOUBLEARG(3);
                              break;
                    case 'f': fixedEMiters = INTARG(3);
                              break;
                    default:  badOption(currArg);
                              break;
                }

            } else {

                // Weed out multi-lettered arguments, since we don't
                // have any of those, except the ones handled above.
                if (strlen(currArg) > 2 && !isdigit(currArg[2])
                        && currArg[2] != '.') {
                    badOption(currArg);
                    continue;
                }

                switch(currArg[1]) {
                    case 's': schemaFilename = argv[++argNum];
                              break;

                    case 'q': verbose = false;
                              break;

                    case 'v': verbose = true;
                              break;

                    case 'd': vardiv = DOUBLEARG(2);
                              break;

                    case 'h': {
                                  double holdout = DOUBLEARG(2);
                                  if (holdout > 1) {
                                      holdoutNum = (int)holdout;
                                  } else if (holdout > 0) {
                                      holdoutFrac = holdout;
                                  }
                                  break;
                              }

                    case 'i': absToLoad = INTARG(2);
                              break;

                    case 'r': srand(INTARG(2));
                              break;

                    default: badOption(currArg);
                             break;
                }
            }
        } else if (obsFilename == NULL) {
            obsFilename = currArg;
        } else if (modelFilename == NULL) {
            modelFilename = currArg;
        } else if (holdoutFilename == NULL) {
            /* If three filenames are specified, they must be 
             * tuning, validation, and model filenames.  Tuning and
             * validation together comprise the training data.
             * If only two filenames are specified, then it is 
             * presumed that they are the training and model filenames.
             */
            tuningFilename = obsFilename;
            holdoutFilename = modelFilename;
            modelFilename = currArg;
            // obsFilename = NULL;
        } else {
            cout << "Extra argument \"" << currArg << "\" ignored.\n";
        }
    }

    if (obsFilename == NULL || modelFilename == NULL) {
        printUsage();
        exit(-1);
    }
}


int main(int argc, char* argv[])
{
    srand(time(NULL));

    /*
     * Handle all arguments
     */
    parseArgs(argc, argv);

    /*
     * Set-up vout
     */
    ofstream devNullObj("/dev/null");
    ostream* pvout = &devNullObj;
    if (verbose) {
        pvout = &cout;
    }
    ostream& vout = *pvout;

    /*
     * Load observations
     */
    ifstream obsin(obsFilename);
    if (!obsin) {
        cout << "Error reading evidence file \"" << obsFilename << "\".\n";
        return 1;
    }

    vout << "Loading training data...";
    vout.flush();
    VarSet o;
    GrowArray<VarSet> observations;
    while (obsin >> o) {
        observations.append(o);
    }
    obsin.close();
    vout << "done.\n";
    vout << "  # of observations: " << observations.size() << endl;

    if (holdoutNum > 0) {
        holdoutFrac = (double)holdoutNum/observations.size();
    }

    if (observations.size() < 1) {
        cout << "ERROR: no observations loaded!\n";
        return 1;
    }

    /*
     * Select a default overfitting strategy: 
     * hold-out set of some reasonable size.
     */
    if (holdoutFilename == NULL &&
            minWeightChange == 0.0 && fixedEMiters == -1
            && holdoutFrac == 0.0) {
        if (observations.size() < 3000) {
            holdoutFrac = 1.0/3.0;
        } else if (observations.size() > 10000 ) {
            holdoutFrac = 0.10;
        } else {
            holdoutFrac = 1000.0/observations.size(); 
        }
    }

    /*
     * Split data into training and holdout datasets.
     */
    vout << "Forming hold-out set...\n";
    GrowArray<VarSet> trainSet;
    GrowArray<VarSet> holdoutSet;
    if (holdoutFrac > 0.0) {
        for (int i = 0; i < observations.size(); i++) {
            if (rand() < holdoutFrac * RAND_MAX) {
                holdoutSet.append(observations[i]);
            } else {
                trainSet.append(observations[i]);
            }
        }
    } else {
        trainSet = observations;
    }

    if (holdoutFilename != NULL)
    {
        ifstream holdin(holdoutFilename);
        if (!holdin) {
            cout << "Error reading hold-out file \"" << holdoutFilename 
                << "\".\n";
            return 1;
        }
        vout << "Loading holdout data...";
        vout.flush();
        VarSet o;
        while (holdin >> o) {
            holdoutSet.append(o);
            observations.append(o);
        }
        holdin.close();
        vout << "done.\n";
    }

    vout << "  # training examples: " << trainSet.size() << endl;
    vout << "  # hold-out examples: " << holdoutSet.size() << endl;

    /*
     * Load schema
     */
    VarSchema s;
    if (schemaFilename != NULL) {
        ifstream schemain(schemaFilename);
        if (!schemain) {
            cout << "Error loading schema file \"" << schemaFilename << "\".\n";
            return 1;
        }
        schemain >> s;
        schemain.close();
    } else {
        // HACK TMP DEBUG
        // vout << "Guessing schema from training data...\n";

        NumSet<int> tempSchema(observations[0].getNumNums());
        for (int j = 0; j < tempSchema.getNumNums(); j++) {
            tempSchema[j] = 0;
        }

        for (int i = 0; i < observations.length(); i++) {
            for (int j = 0; j < tempSchema.getNumNums(); j++) {
                if (tempSchema[j] >= 0) {
                    if (floor(observations[i][j]) != observations[i][j]) {
                        tempSchema[j] = -1;
                    } else if (observations[i][j] + 1 > tempSchema[j]) {
                        tempSchema[j] = (int)observations[i][j] + 1;
                    }
                }
            }
        }

        s = tempSchema;
    }
    vout << "Loaded schema: " << s.getNumVars() << " vars.\n";

    /*
     * Create model, based on schema
     */
    AbsVarAbstractionSet model(s, true, vardiv);


    /* TODO: Register a signal handler to save the model, should we be
     * interrupted.
     * signal(SIGINT, ...);
     */

    /*
     * Train model
     */
    vout << "Learning model...\n";
    model.resetWeights();
    model.initParameters(trainSet.getArray(), trainSet.size());

    // Start with one intial cluster, representing variable marginals
    model.addNullAbstraction();


    {
        time_t overallStartTime;
        time_t overallEndTime;

        if (holdoutSet.size() == 0) {
            minllChangeEM = -1;
            minllChangeAdd = -1;
        }

        double weightChange = 1.0;
        double bestOverall  = -1e100;
        double bestll       = -1e100;
        double currll       = -1e100;
        AbsVarAbstractionSet bestModel = model;
        GrowArray<VarSet> bestTrainSet = trainSet;
        double bestModelll = bestOverall;
        int bestAbsLoaded = 0;
        int bestEMSteps = 0;
        int totalEMSteps = 0;

        // Random restarts
        AbsVarAbstractionSet origModel = model;
        GrowArray<VarSet> origTrainSet = trainSet;
        int origAbsToLoad = absToLoad;

        int randomRestarts = 0;
        
        time(&overallStartTime);
        do {
            /* When we have a fixed number of EM iterations to run, we
             * run exactly that number, no more no less.  We will only reenter
             * this loop if we're doing random restarts.  We will never extend
             * the model by adding more clusters.
             */
            int EMitersLeft = fixedEMiters;

            if (randomRestarts > 0) {
                randomRestarts--;
            }

            //bestOverall = bestll;
            bestOverall = bestModelll;

            // Do multiple random restarts.
            if (randomRestarts > 0) {
                trainSet = origTrainSet;
                model = origModel;
                absToLoad = origAbsToLoad;
                totalAbsLoaded = 0;
                bestll = -1e100;
                vout << randomRestarts << " random restarts left.\n";
            }

            /*
             * Load additional abstractions incrementally
             */
            int addedAbs = 0;
            while (sampledAbs && addedAbs != absToLoad) {
#ifdef REMOVE_CLUSTER_CENTERS
                int absIndex = rand() % trainSet.size();
                Abstraction abs = trainSet[absIndex];
                trainSet[absIndex] = trainSet[trainSet.size() - 1];
                trainSet.deleteLast();
#else
                int absIndex = rand() % (trainSet.size() - totalAbsLoaded);
                Abstraction abs = trainSet[absIndex];
                trainSet[absIndex] = trainSet[trainSet.size() 
                    - totalAbsLoaded - 1];
                trainSet[trainSet.size() - totalAbsLoaded - 1] = abs;
#endif
                model.addAbstraction(new Abstraction(abs));
                totalAbsLoaded++;
                addedAbs++;

#ifdef REMOVE_CLUSTER_CENTERS
                if (trainSet.size() == 0) {
                    sampledAbs = false;
                }
#else
                if (totalAbsLoaded >= trainSet.size()) {
                    sampledAbs = false;
                }
#endif
            }
            if (addedAbs > 0) {
                absToLoad += addedAbs;
                vout << "  Added " << addedAbs << " clusters.\n";
            } else {
                absToLoad = 0;
            }

            // DEBUG
            //cout << model << endl;

            if (holdoutSet.size() > 0.0) {
                currll = model.getLogLikelihood(holdoutSet.getArray(),
                            holdoutSet.size());
                vout << "    Initial ll: " << currll << endl;

                // HACK  (wait... why is this here?)
                if (currll > bestll) {
                    currll = bestll;
                }
                if (currll > bestModelll) {
                    bestModelll = currll;
                    bestModel = model;
                    bestTrainSet = trainSet;
                    bestAbsLoaded = totalAbsLoaded;
                    bestEMSteps = totalEMSteps;
                }
            }

            // Run EM with these abstractions until
            // we begin to converge (or overfit)
            int iter = 0;
            do {
                time_t startTime, endTime;
                time(&startTime);

                // Run one step of EM
                weightChange = model.EMStep(trainSet.getArray(),
                        trainSet.size());
                time(&endTime);
                totalEMSteps++;

                //vout << "Weight change: " << weightChange << " (" <<
                //    (endTime - startTime) << "s)\n";

                // Compute the log likelihood for the holdout set
                if (holdoutSet.size() > 0) {
                    bestll = currll;
                    currll = model.getLogLikelihood(holdoutSet.getArray(),
                            holdoutSet.size());
                    vout << "    Hold-out ll: " << currll << 
                        " (" << (endTime - startTime) << "s)\n";

                    if (currll > bestModelll) {
                        bestModelll = currll;
                        bestModel = model;
                        bestTrainSet = trainSet;
                        bestAbsLoaded = totalAbsLoaded;
                        bestEMSteps = totalEMSteps;
                    }
                } else {
                    vout << '.';
                }

                // DEBUG
                //cout << model << endl;

                // Prune abstractions with low weight, every so often
                if (pruneFrac < 1.0 && (++iter % pruneFreq == 0)) {
                    vout << "    Pruning model...";
                    model.prune(pruneFrac);
                    vout << model.getNumAbstractions() << " clusters left.\n";
                }

#if 0
                {
                    char filename[1024];
                    sprintf(filename, "%s.%d", modelFilename, iter);
                    ofstream midout(filename);
                    midout << model;
                    midout.close();
                }
#endif
            } while ((--EMitersLeft != 0) 
                    && (currll - bestll)/fabs(currll) >= minllChangeEM
                    && weightChange >= minWeightChange);

#ifdef SAVE_BEST
            // Restore best model
            if (holdoutSet.size() > 0.0) {
                model = bestModel;
                trainSet = bestTrainSet;
                totalEMSteps = bestEMSteps;
            }
#endif

            if (pruneFrac < 1.0) {
                vout << "  Pruning model...";
                model.prune(pruneFrac);
                vout << model.getNumAbstractions() << " clusters left.\n";
            }
        } while (randomRestarts || ((fixedEMiters < 0) && sampledAbs
                && (bestll - bestOverall)/fabs(bestll) >= minllChangeAdd
                && weightChange >= minWeightChange));

        if (holdoutSet.size() > 0.0) {
//#define RERUN
#ifdef RERUN
            // TODO -- this should be an option at runtime, not compile-time
            if (1) {
                trainSet = observations;
                model = origModel;

                vout << "...RERUNNING...\n";
                vout << "Clusters to load: " << bestAbsLoaded << endl;
                vout << "EM steps to run: " << bestEMSteps << endl;
                
                while (bestAbsLoaded--) {
                    int absIndex = rand() % trainSet.size();
                    Abstraction abs = trainSet[absIndex];
                    trainSet[absIndex] = trainSet[trainSet.size() - 1];
                    trainSet.deleteLast();

                    model.addAbstraction(new Abstraction(abs));
                }

                while (bestEMSteps--) {
                    model.EMStep(trainSet.getArray(), trainSet.size());
                }
                vout << "Done.\n";
            } else {
#endif
                vout << "  Running two more EM steps with all the data...";
                vout.flush();
                for (int i = 0; i < holdoutSet.size(); i++) {
                    trainSet.append(holdoutSet[i]);
                }
                model.EMStep(trainSet.getArray(), trainSet.size());
                model.EMStep(trainSet.getArray(), trainSet.size());
                vout << "done\n";
#ifdef RERUN
            }
#endif
        }

        time(&overallEndTime);
        vout << "Total training time: " << 
                (overallEndTime - overallStartTime) << "s\n";
    }
    //vout << "done.\n";

    // Output the model
    ofstream modelout(modelFilename);
    if (!modelout) {
        cout << "Error opening model file \"" << modelFilename << "\" for output.\n";
        return 1;
    }

    //modelout << modelType << endl;
    modelout << model;
    modelout.close();

#if 0
    delete pModel;
#endif
    return 0;
}
