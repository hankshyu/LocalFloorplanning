CXX = g++
FLAGS = -std=c++11 
CFLAGS = -c
DEBUGFLAGS = -g

SRCPATH = ./src
BOOSTPATH = boost_1_82_0

all: lfrun.out
debug: lfrun_debug.out

# LINKFLAGS = -pedantic -Wall -fomit-frame-pointer -funroll-all-loops -O3
LINKFLAGS = 

lfrun.out: main.o Tile.o $(SRCPATH)/LFUnits.h Tessera.o LFLegaliser.o parser.o ppmodule.o ppsolver.o \
maxflow.o DFSLegalizer.o monitor.o rgparser.o rgmodule.o rgsolver.o
	$(CXX) -I $(BOOSTPATH) $(LINKFLAGS) $^ -o $@


main.o: $(SRCPATH)/main.cpp 
	$(CXX) -I $(BOOSTPATH) $(CFLAGS) -DCOMPILETIME="\"`date`\"" $^ -o $@

%.o: $(SRCPATH)/%.cpp $(SRCPATH)/%.h
	$(CXX) -I $(BOOSTPATH) $(CFLAGS) $< -o $@



lfrun_debug.out: main_db.o Tile_db.o $(SRCPATH)/LFUnits.h Tessera_db.o LFLegaliser_db.o parser_db.o ppmodule_db.o ppsolver_db.o \
maxflow_db.o DFSLegalizer_db.o monitor_db.o rgparser_db.o rgmodule_db.o rgsolver_db.o
	$(CXX) $(DEBUGFLAGS) -I $(BOOSTPATH) $(LINKFLAGS) $^ -o $@

main_db.o: $(SRCPATH)/main.cpp 
	$(CXX) $(DEBUGFLAGS) -I $(BOOSTPATH) $(CFLAGS) -DCOMPILETIME="\"`date`\"" $^ -o $@

%_db.o: $(SRCPATH)/%.cpp $(SRCPATH)/%.h
	$(CXX) $(DEBUGFLAGS) -I $(BOOSTPATH) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o *.gch *.out
