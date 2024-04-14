CXX = g++
CXXFLAGS = -std=c++20
INCLUDES = -I../
INCLUDES += -I/usr/include/eigen3

all: c s l

c: client_UDP.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@ -pthread

s: server_UDP.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@ -pthread

l: losspackage.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@ -pthread

clean:
	rm -f c s l
