CXXFLAGS = -I../common -std=c++11 -W -Wall -O2 -DNDEBUG
LDFLAGS = -L../common
LDLIBS = -lcommon
solver: key_recovery.o  main.o  r123solver.o  r4lookup.o
	$(CXX) $(LDFLAGS) -o $@ $^ $(LOADLIBES) $(LDLIBS)
