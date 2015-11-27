CXXFLAGS = -I../common -std=c++11 -W -Wall -O2 -DNDEBUG
LDFLAGS = -L../common
LDLIBS = -lcommon
precomp: main.o
	$(CXX) $(LDFLAGS) -o $@ $^ $(LOADLIBES) $(LDLIBS)
