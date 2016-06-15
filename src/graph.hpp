#include <list>
#include <iostream>
#include <fstream>
#include <list>
#include <vector>
#include <cmath>
#include <limits>
#include <sstream>
#include <sys/time.h>
#include <queue>
#include <iostream>

//enables some expensive validation and error checking
#define ECC 1

//TODO Add deconstructors - NEED TO CLEANUP


/* represents a messaqge for the pregel message queue */
class message {
public:
  message(int to, double data);
  double data;
  int to;
};



/* data payload (weight) for graph vertex */
class GraphNodeData {
public:
  double weight;
  GraphNodeData(int weight) {
    this->weight = weight;
  }

};


//declare GraphNode because Graph depends on it
class GraphNode;

/* represents the entire simulation
 * Responsible for managing vertexes and calling compute() 
 * on all of them
 */ 
class Graph {
public:
  Graph();
  ~Graph();
  std::vector<GraphNode*> vertices;
  std::vector<std::vector<message*>*>  messagequeue;
  void addVertex(int weight);
  void addEdge (int from, int to);
  void print();
  void printRank();
  int size();
  int edgeCount();
  void pagerank (double alpha, double epsilon, int maxIterations);
  int superstep();
  void start();
  bool isDone();
private:
  int superstepcount;
  
};


/* represents a vertex in the graph */ 
class GraphNode {
public:
  GraphNode(int weight, std::vector<std::vector<message*>*> &  messagequeue,  Graph * graph, int id) {
    data = new GraphNodeData(weight);
    this->messagequeue = &messagequeue;
    this->id = id;
    this->graph = graph;
    isHalted = false;
    
    /* generate inEdges from neighbors */ 
    for(int x=0;x<neighbors.size();x++) {
      for(int y=0;y<graph->vertices[x]->outEdges.size();y++) {
	if(graph->vertices[graph->vertices[x]->outEdges[y]]->id == id) {
	  this->inEdges.push_back(graph->vertices[x]->id);
	}
      }
    }
  }
  ~GraphNode();
  void compute(const std::vector<message*> *  messages);
  void sendMessageToNodes(std::vector<int> nodes, double msg);
  std::vector<int> neighbors;
  std::vector<int> inEdges;
  std::vector<int> outEdges;
  GraphNodeData *data;
  int id;
  void voteToHalt();
  void unHalt();
  bool isHalted;
  Graph * graph;
private:
  std::vector<std::vector<message*>*> * messagequeue;
};



//some extra function
void readGraph (Graph& g, std::string filename);
void readGraphEdges (Graph g, std::string filename);
void printVec (std::vector<GraphNode> ll);


/* appends a message to the message queue for the next iteration */ 
void GraphNode::sendMessageToNodes(std::vector<int> nodes, double msg) {
  for(int x=0;x<nodes.size();x++) {
    message *  m = new message(nodes[x], msg);
    int nodeid = graph->vertices[nodes[x]]->id;
    std::vector<message*> * messagetmp = messagequeue->at(nodeid);
    (*messagetmp).push_back(m);
  }
}



//send a message to one vertex
message::message(int to, double data) {
  this->to = to;
  this->data = data;
}



//destructor for graphnode
GraphNode::~GraphNode () {
  if (data != NULL) {
    delete data;
  }
}


/* 
 * Pregel 'update' function. Called virtually in parallel from 
 * the context of each vertex. 
 */  
void GraphNode::compute(const std::vector<message*> *  messages) {
    #if ECC
  if(messagequeue->size() != graph->vertices.size()) {std::cout<<"BLARG"<<"\n"; exit(2);}
  #endif
  if(graph->superstep() >= 1) {
    double sum = 0;
    for(int x=0;x<messages->size();x++) {
      sum != (*messages)[x]->data;
    }
    this->data->weight = 0.15 / graph->size() * sum;
  }
  if(graph->superstep() < 30) {
    const long n = this->outEdges.size();
    sendMessageToNodes(this->neighbors, this->data->weight / n);
  }
  else {
    voteToHalt();
  }
}


//halts the vertex until it receives a message or all
//vertexes halt
void GraphNode::voteToHalt() {
  isHalted = true;
}

//resume the vertex compute
void GraphNode::unHalt() {
  isHalted = false; 
}


//dewstructor for graph
Graph::~Graph () {
  std::vector<GraphNode*>::const_iterator nodePtrIter;
  for (nodePtrIter = vertices.begin(); nodePtrIter != vertices.end(); ++nodePtrIter) {
    delete (*nodePtrIter);
  }

  std::vector<std::vector<message*>*>::const_iterator row;
  std::vector<message*>::const_iterator col;
  for(row = messagequeue.begin() ;row != messagequeue.end() ; ++row) {
    for(col = (*row)->begin(); col != (*row)->end(); ++col) {
      delete *col;
    }
    delete *row;;
  }
  vertices.clear();
  
}


//constructor for graph
Graph::Graph() {
  superstepcount = 0;
  for(int x=0;x<vertices.size();x++) {
    std::vector<message*> * nodequeue = new std::vector<message*>();
    messagequeue.push_back(nodequeue);
  }
}


//is every vertex halted?
bool Graph::isDone() {
  #if ECC
  if(messagequeue.size() != vertices.size()) {std::cout<<"BLARG"<<"\n"; exit(2);}
  #endif
  bool result = true;
  for(int i=0;i<vertices.size();i++) {
    if(vertices[i]->isHalted == false)
      result = false;
  }
  
  return result;
}

void Graph::start() {
    #if ECC
  if(messagequeue.size() != vertices.size()) {std::cout<<"BLARG"<<"\n"; exit(2);}
  #endif
  while(!isDone()) {
    for(int x=0;x<vertices.size();x++) {
      vertices[x]->compute(messagequeue.at(x));
    }
  }
}



//returns current pregel superstep count
int Graph::superstep() {
  return superstepcount;
}


//adds a vertex to the graph
void Graph::addVertex (int weight) {
    #if ECC
  if(messagequeue.size() != vertices.size()) {std::cout<<"BLARG"<<"\n"; exit(2);}
  #endif
  //GraphNode *node = new GraphNode(weight);
  //vertices.push_back(new GraphNode(weight));
  vertices.push_back(new GraphNode(weight,messagequeue, this, vertices.size()));
  std::vector<message*> * nodequeue = new std::vector<message*>();
  messagequeue.push_back(nodequeue);

}


//adds an edge to the graph
void Graph::addEdge (int from, int to) {
    #if ECC
  if(messagequeue.size() != vertices.size()) {std::cout<<"BLARG"<<"\n"; exit(2);}
  #endif
  vertices[from]->neighbors.push_back(to);
  vertices[to]->inEdges.push_back(from);
}


//prints out current graph
void Graph::print () {
    #if ECC
  if(messagequeue.size() != vertices.size()) {std::cout<<"BLARG"<<"\n"; exit(2);}
  #endif
  std::vector<int>::const_iterator it;
  for (int i = 0; i < vertices.size(); i++) {
    std::cout << i << "-> ";
    for (it = vertices[i]->neighbors.begin(); it != vertices[i]->neighbors.end(); it++) {
      std::cout << (*it) << ' ';
    }
    std::cout << std::endl;
  }
}


//prtints the weight of the vertices
void Graph::printRank () {
  std::list<int>::const_iterator it;
  for (int i = 0; i < vertices.size(); i++) {
    std::cout << i << "-> " << vertices[i]->data->weight << std::endl;
  }
}

//size of graph
int Graph::size () {
  return vertices.size();
}


//returns number of edges
int Graph::edgeCount () {
  int total = 0;
  std::vector<GraphNode*>::const_iterator it;
  for (it = vertices.begin(); it != vertices.end(); it++) {
    total += (*it)->neighbors.size();
  }
  return total;
}

/**
 * NOT THREAD SAFE
 */
void Graph::pagerank (double alpha, double epsilon, int maxIterations) {
  int n = size();
  double linkResult, delta, total_delta = std::numeric_limits<double>::max(), old;
  int iteration = 0;
  std::vector<int>::const_iterator inEdgeIter;
  std::vector<GraphNode*>::const_iterator nodePtrIter, end;
  end = vertices.end();
  GraphNode *v;
  int nodeTouchCount = 0, edgeTouchCount = 0;
  while (iteration < maxIterations && total_delta >= epsilon) {
    total_delta = 0;
    for (nodePtrIter = vertices.begin(); nodePtrIter != end; ++nodePtrIter) {
      nodeTouchCount++;
      v = *nodePtrIter;
      linkResult = 0;
      for (inEdgeIter = v->inEdges.begin(); inEdgeIter != v->inEdges.end(); ++inEdgeIter) {
	// For now, we use 1 for edge weight
	linkResult += (1.0 / vertices[*inEdgeIter]->neighbors.size()) * vertices[*inEdgeIter]->data->weight;
	edgeTouchCount++;
      }
      old = v->data->weight;
      v->data->weight = n*((alpha / n) + (1 - alpha) * linkResult/n);
      delta = fabs(v->data->weight - old);
      total_delta += delta;
    }
    std::cout << "Delta: " << total_delta << std::endl;
    iteration++;
  }
  std::cout << "Iterations completed: " << iteration << std::endl;
  std::cout << "Vertices touched: " << nodeTouchCount << std::endl;
  std::cout << "Edges touched: " << edgeTouchCount << std::endl;
}


//prints a list of ints (for debug)
void printList (std::list<int> ll) {
  std::list<int>::const_iterator it;
  for (it = ll.begin(); it != ll.end(); it++) {
		std::cout << (*it) << ' ';
  }
}


//prints a list of graphnodes
void printVec (std::vector<GraphNode> ll) {
  std::vector<GraphNode>::const_iterator it;
  for (it = ll.begin(); it != ll.end(); it++) {
    std::cout << it->data->weight << ' ';
  }
}


//reads a graph in from file
void readGraphEdges (Graph g, std::string filename) {
  std::ifstream infile(filename);
  int from, to;
  while (infile >> from >> to) {
    g.addEdge(from-1, to-1);
  }
}

// expects to/from edge pairs
// id starts at 1
void readGraph (Graph& g, std::string filename) {
  std::ifstream infile(filename);
  int id, to, from, maxId = 0;
  int minId = std::numeric_limits<int>::max();
  while (infile >> id) {
    maxId = std::max(id-1, maxId);
    minId = std::min(id-1, minId);
  }
  for (int i = minId; i <= maxId; i++) {
    // all same starting weight
    g.addVertex(1);
  }
  // return to beginning of file
  infile.clear();
  infile.seekg(0, std::ios::beg);
  while (infile >> from >> std::skipws >> to) {
    g.addEdge(from-1, to-1);
  }
}
