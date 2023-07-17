CXX = g++
FLAGS = -std=c++11 
CFLAGS = -c
DEBUGFLAGS = -g

SRCPATH = ./src

all: lfrun.out
debug: lfrun_debug

# LINKFLAGS = -pedantic -Wall -fomit-frame-pointer -funroll-all-loops -O3
LINKFLAGS = 

lfrun.out: main.o Tile.o $(SRCPATH)/LFUnits.h Tessera.o LFLegaliser.o
	$(CXX) $(LINKFLAGS) $^ -o $@


main.o: $(SRCPATH)/main.cpp 
	$(CXX) $(CFLAGS) -DCOMPILETIME="\"`date`\"" $^ -o $@

%.o: $(SRCPATH)/%.cpp $(SRCPATH)/%.h
	$(CXX) $(CFLAGS) $< -o $@



# pprun_debug: main_debug.o ppsolver_debug.o ppmodule_debug.o
# 	$(CXX) $(DEBUGFLAGS) $^ -o $@
		
# main_debug.o: main.cpp 
# 	$(CXX) $(DEBUGFLAGS) $(CFLAGS) -DCOMPILETIME="\"`date`\"" $^ -o $@

# ppsolver_debug.o: ppsolver.cpp ppsolver.h
# 	$(CXX) $(DEBUGFLAGS) $(CFLAGS) ppsolver.cpp -o $@
			
# ppmodule_debug.o: ppmodule.cpp ppmodule.h
# 	$(CXX) $(DEBUGFLAGS) $(CFLAGS) ppmodule.cpp -o $@

clean:
	rm -rf *.o *.gch lfrun.out
