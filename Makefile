CXX = g++
FLAGS = -std=c++11 
CFLAGS = -c
DEBUGFLAGS = -g 
all: pprun
debug: pprun_debug

# LINKFLAGS = -pedantic -Wall -fomit-frame-pointer -funroll-all-loops -O3
LINKFLAGS = 

pprun: main.o ppsolver.o ppmodule.o
	$(CXX) $(LINKFLAGS) $^ -o $@
		
main.o: main.cpp 
	$(CXX) $(LINKFLAGS) $(CFLAGS) -DCOMPILETIME="\"`date`\"" $^ -o $@

ppsolver.o: ppsolver.cpp ppsolver.h
	$(CXX) $(LINKFLAGS) $(CFLAGS) ppsolver.cpp -o $@
			
ppmodule.o: ppmodule.cpp ppmodule.h
	$(CXX) $(LINKFLAGS) $(CFLAGS) ppmodule.cpp -o $@



pprun_debug: main_debug.o ppsolver_debug.o ppmodule_debug.o
	$(CXX) $(DEBUGFLAGS) $^ -o $@
		
main_debug.o: main.cpp 
	$(CXX) $(DEBUGFLAGS) $(CFLAGS) -DCOMPILETIME="\"`date`\"" $^ -o $@

ppsolver_debug.o: ppsolver.cpp ppsolver.h
	$(CXX) $(DEBUGFLAGS) $(CFLAGS) ppsolver.cpp -o $@
			
ppmodule_debug.o: ppmodule.cpp ppmodule.h
	$(CXX) $(DEBUGFLAGS) $(CFLAGS) ppmodule.cpp -o $@


clean:
	rm -rf *.o *.gch pprun pprun_debug
