CXX = g++
FLAGS = -std=c++17
CFLAGS = -c
DEBUGFLAGS = -g

SRCPATH = ./src
BOOSTPATH = "" # ! Remember to specify the path

all: fprun
debug: fprun_debug
verify: ./utils/verify

# LINKFLAGS = -pedantic -Wall -fomit-frame-pointer -funroll-all-loops -O3
LINKFLAGS = -O3

fprun: main.o Tile.o LFUnits.o Tessera.o LFLegaliser.o parser.o ppmodule.o ppsolver.o \
maxflow.o maxflowLegaliser.o monitor.o rgparser.o rgmodule.o rgsolver.o paletteKnife.o cake.o \
tensor.o DFSLegalizer.o
	$(CXX) $(FLAGS) -I $(BOOSTPATH) $(LINKFLAGS) $^ -o $@


main.o: $(SRCPATH)/main.cpp 
	$(CXX) $(FLAGS) -I $(BOOSTPATH) $(CFLAGS) $(LINKFLAGS) -DCOMPILETIME="\"`date`\"" $^ -o $@

%.o: $(SRCPATH)/%.cpp $(SRCPATH)/%.h
	$(CXX) $(FLAGS) -I $(BOOSTPATH) $(CFLAGS) $(LINKFLAGS) $< -o $@



fprun_debug: main_db.o Tile_db.o LFUnits_db.o Tessera_db.o LFLegaliser_db.o parser_db.o ppmodule_db.o ppsolver_db.o \
maxflow_db.o maxflowLegaliser_db.o monitor_db.o rgparser_db.o rgmodule_db.o rgsolver_db.o  paletteKnife_db.o cake_db.o \
tensor_db.o DFSLegalizer_db.o
	$(CXX) $(DEBUGFLAGS) -I $(BOOSTPATH) $(LINKFLAGS) $^ -o $@

main_db.o: $(SRCPATH)/main.cpp 
	$(CXX) $(DEBUGFLAGS) -I $(BOOSTPATH) $(CFLAGS) -DCOMPILETIME="\"`date`\"" $^ -o $@

%_db.o: $(SRCPATH)/%.cpp $(SRCPATH)/%.h
	$(CXX) $(DEBUGFLAGS) -I $(BOOSTPATH) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o *.gch *.out fprun ./utils/verify



./utils/verify: $(SRCPATH)/verifier.cpp
	$(CXX) $(FLAGS) -I $(BOOSTPATH) $(LINKFLAGS) $^ -o $@
