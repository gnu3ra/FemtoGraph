CPP=g++
CPPFLAGS=-std=c++11 -g

graph: src/graph.cpp
	$(CPP) $(CPPFLAGS) src/graph.cpp -o bin/graph

test: graph
	bin/graph

clean: 
	rm -rf bin/*
