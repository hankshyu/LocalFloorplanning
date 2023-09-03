CXX = g++
FLAGS = -std=c++11 
CFLAGS = -c
DEBUGFLAGS = -g

SRCPATH = ./src
BOOSTPATH = boost_1_72_0

all: cadd0007
debug: cadd0007_debug.out
verify: ./utils/verify.out

# LINKFLAGS = -pedantic -Wall -fomit-frame-pointer -funroll-all-loops -O3
LINKFLAGS = -O3

cadd0007: main.o Tile.o LFUnits.o Tessera.o LFLegaliser.o parser.o ppmodule.o ppsolver.o \
maxflow.o maxflowLegaliser.o monitor.o rgparser.o rgmodule.o rgsolver.o paletteKnife.o cake.o \
tensor.o DFSLegalizer.o
	$(CXX) $(FLAGS) -I $(BOOSTPATH) $(LINKFLAGS) $^ -o $@


main.o: $(SRCPATH)/main.cpp 
	$(CXX) $(FLAGS) -I $(BOOSTPATH) $(CFLAGS) $(LINKFLAGS) -DCOMPILETIME="\"`date`\"" $^ -o $@

%.o: $(SRCPATH)/%.cpp $(SRCPATH)/%.h
	$(CXX) $(FLAGS) -I $(BOOSTPATH) $(CFLAGS) $(LINKFLAGS) $< -o $@



cadd0007_debug.out: main_db.o Tile_db.o LFUnits_db.o Tessera_db.o LFLegaliser_db.o parser_db.o ppmodule_db.o ppsolver_db.o \
maxflow_db.o maxflowLegaliser_db.o monitor_db.o rgparser_db.o rgmodule_db.o rgsolver_db.o  paletteKnife_db.o cake_db.o \
tensor_db.o DFSLegalizer_db.o
	$(CXX) $(DEBUGFLAGS) -I $(BOOSTPATH) $(LINKFLAGS) $^ -o $@

main_db.o: $(SRCPATH)/main.cpp 
	$(CXX) $(DEBUGFLAGS) -I $(BOOSTPATH) $(CFLAGS) -DCOMPILETIME="\"`date`\"" $^ -o $@

%_db.o: $(SRCPATH)/%.cpp $(SRCPATH)/%.h
	$(CXX) $(DEBUGFLAGS) -I $(BOOSTPATH) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o *.gch *.out cadd0007



./utils/verify.out: $(SRCPATH)/verifier.cpp
	$(CXX) -I $(BOOSTPATH) $(LINKFLAGS) $^ -o $@
