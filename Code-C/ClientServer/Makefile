CXX = g++
CXXFLAGS = -std=c++20
INCLUDES = -I../
INCLUDES += -I/usr/include/eigen3

all: c s l

c: client_UDP.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@

s: server_UDP.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@

l: losspackage.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@

clean:
	rm -f c s l
