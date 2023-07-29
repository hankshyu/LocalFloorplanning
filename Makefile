CXX = g++
FLAGS = -std=c++11 
BOOSTPATH = boost_1_82_0
CFLAGS = -c
DEBUGFLAGS = -g

SRCPATH = ./src

all: lfrun.out
debug: lfrun_debug

# LINKFLAGS = -pedantic -Wall -fomit-frame-pointer -funroll-all-loops -O3
LINKFLAGS = 

lfrun.out: main.o Tile.o $(SRCPATH)/LFUnits.h Tessera.o LFLegaliser.o
	$(CXX) -I $(BOOSTPATH) $(LINKFLAGS) $^ -o $@


main.o: $(SRCPATH)/main.cpp 
	$(CXX) -I $(BOOSTPATH) $(CFLAGS) -DCOMPILETIME="\"`date`\"" $^ -o $@

Tessera.o: $(SRCPATH)/Tessera.cpp $(SRCPATH)/Tessera.h
	$(CXX) -I $(BOOSTPATH) $(CFLAGS) $< -o $@

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
