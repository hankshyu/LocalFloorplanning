CXX = g++
FLAGS = -std=c++11 
BOOSTPATH = boost_1_82_0
CFLAGS = -c
DEBUGFLAGS = -g

SRCPATH = ./src

all: lfrun.out
debug: lfrun_db.out

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


lfrun_db.out: main_db.o Tile_db.o $(SRCPATH)/LFUnits.h Tessera_db.o LFLegaliser_db.o
	$(CXX) $(DEBUGFLAGS) -I $(BOOSTPATH) $(LINKFLAGS) $^ -o $@


main_db.o: $(SRCPATH)/main.cpp 
	$(CXX) $(DEBUGFLAGS) -I $(BOOSTPATH) $(CFLAGS) -DCOMPILETIME="\"`date`\"" $^ -o $@

Tessera_db.o: $(SRCPATH)/Tessera.cpp $(SRCPATH)/Tessera.h
	$(CXX) $(DEBUGFLAGS) -I $(BOOSTPATH) $(CFLAGS) $< -o $@

%_db.o: $(SRCPATH)/%.cpp $(SRCPATH)/%.h
	$(CXX) $(DEBUGFLAGS) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o *.gch lfrun.out
