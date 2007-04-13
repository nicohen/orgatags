CC = g++
# Debug flags
#CCFLAGS = -g -Wall
# Profiling flags
#CCFLAGS = -pg -O2 -Wall
# Optimized flags
CCFLAGS = -O2 -Wall

ABS_H_FILES   = NumSet.h AbstractionSet.h VarSet.h VarSchema.h Abstraction.h \
                AbsVarAbstractionSet.h Prob.h GrowArray.h \
                Distribution.h GenDistribution.h ContDistribution.h
ABS_CPP_FILES = AbstractionSet.cpp VarSet.cpp \
                AbsVarAbstractionSet.cpp Prob.cpp Distribution.cpp
ABS_O_FILES   = AbstractionSet.o VarSet.o \
               AbsVarAbstractionSet.o Prob.o Distribution.o


# Basic targets: all, dist, clean, tags

all: nbetrain nbetest nbequery nbesample

dist:
	rm -rf nbe
	mkdir nbe
	cp *.h *.cpp iris-1.* README Makefile nbe
	tar -cvf nbe.tar nbe
	gzip nbe.tar
	rm -rf nbe

clean:
	rm ${ABS_O_FILES} nbetrain nbetest nbequery nbesample

tags:
	ctags ${ABS_H_FILES} ${ABS_CPP_FILES}

depend:
	makedepend -- ${CCFLAGS} -- ${ABS_CPP_FILES} nbetrain.cpp nbetest.cpp nbequery.cpp 


# Program targets: nbetrain, nbetest, nbesample, nberand, nbequery

nbetrain: nbetrain.cpp ${ABS_O_FILES} ${ABS_H_FILES}
	$(CC) $(CCFLAGS) -o nbetrain nbetrain.cpp ${ABS_O_FILES}

nbetest: nbetest.cpp ${ABS_O_FILES} ${ABS_H_FILES}
	$(CC) $(CCFLAGS) -o nbetest nbetest.cpp ${ABS_O_FILES}

nbesample: nbesample.cpp ${ABS_O_FILES} ${ABS_H_FILES}
	$(CC) $(CCFLAGS) -o nbesample nbesample.cpp ${ABS_O_FILES}

nbequery: nbequery.cpp ${ABS_O_FILES} ${ABS_H_FILES}
	$(CC) $(CCFLAGS) -o nbequery nbequery.cpp ${ABS_O_FILES}

# Object file dependencies

#VarSet.o: VarSet.cpp VarSet.h NumSet.h

#AbstractionSet.o: AbstractionSet.h VarSet.h NumSet.h VarSchema.h 

#AbsVarAbstractionSet.o: AbsVarAbstractionSet.h AbstractionSet.h

#Prob.o: Prob.cpp Prob.h

.cpp.o:
	$(CC) $(CCFLAGS) -c $<
        
# DO NOT DELETE

