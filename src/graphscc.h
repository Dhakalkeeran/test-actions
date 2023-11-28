#ifndef GRAPHSCC_H
#define GRAPHSCC_H
#include <iostream>
#include <map>
#include <vector>
using namespace std;
class Vertex;

class Vertex {
public:
  long name;               // Vertex name
  vector<Vertex *> adj;    // Adjacent vertices
  vector<Vertex *> revAdj; // Adjacent vertices
  long color;              // Can be 3 values 0-white
                           // 1 -gray
                           // 2 -black

  long cc; // number specifying which connected component
           // the vertex belongs to
  // long              ccDouble;  // number specifying which connected component
  //                    // the vertex belongs to

  long f; // finishing time needed for Strongly connected components

  Vertex(long nm) : name(nm) {
    color = 0;
    cc = -1;
    //    ccDouble=-1;
  }

  void reset() {
    color = 0;
    cc = -1;
    // ccDouble=-1;
  }
};

typedef map<long, Vertex *> vmap;
typedef pair<long, Vertex *> vpair;

class Graph {
public:
  Graph() {}
  ~Graph() { deleteGraph(); }
  void addEdge(long sourceName, long destName);

  void DFS_visitCC(Vertex *v);
  void DFSreverseCC(long *atomCC, long &numSCC);
  void DFSClassic();
  void DFS_visitClassic(Vertex *v, long &time);
  void SCC(long *atomCC, long &numSCC);
  long numberOfVertex() { return allVertices.size(); }
  Vertex *getVertex(long vertexName);

  void clearAll();
  void deleteGraph();

  vmap vertexMap;
  vector<Vertex *> allVertices;
};
#endif
