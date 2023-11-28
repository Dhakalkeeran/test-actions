#include "graphscc.h"
#include "iostream"
#include <algorithm>

void Graph::addEdge(long sourceName, long destName) {
  Vertex *v = getVertex(sourceName);
  Vertex *w = getVertex(destName);
  v->adj.push_back(w);
  w->revAdj.push_back(v);
}

// If vertexName is not present, add it to vertexMap
// In either case, return the Vertex
Vertex *Graph::getVertex(long vertexName) {
  vmap::iterator itr = vertexMap.find(vertexName);

  if (itr == vertexMap.end()) {
    Vertex *newv = new Vertex(vertexName);
    allVertices.push_back(newv);
    vertexMap.insert(vpair(vertexName, newv));
    return newv;
  }
  return (*itr).second;
}

void Graph::clearAll() { vertexMap.clear(); }

void Graph::deleteGraph() {
  vertexMap.clear();
  for (unsigned long i = 0; i < allVertices.size(); i++)
    if (allVertices[i] != NULL) {
      delete allVertices[i];
      allVertices[i] = NULL;
    }
}

void Graph::DFSClassic() {
  long time = 0;
  for (unsigned long i = 0; i < allVertices.size(); i++) {
    if (allVertices[i]->color == 0)
      DFS_visitClassic(allVertices[i], time);
  }
  return;
}
void Graph::DFS_visitClassic(Vertex *v, long &time) {
  v->color = 1; // gray
  //  time++;

  for (unsigned long j = 0; j < v->adj.size(); j++) {
    if (v->adj[j]->color == 0) {
      DFS_visitClassic(v->adj[j], time);
    }
  }
  v->color = 2;
  v->f = time;
  time++;
  return;
}

void Graph::DFSreverseCC(long *atomCC, long &numSCC) {
  numSCC = 0;
  long size = allVertices.size();
  long *corrsepondingVertex = new long[size];
  long *final = new long[size];
  for (long i = 0; i < size; i++) {
    corrsepondingVertex[allVertices[i]->f] = i;
    final[i] = allVertices[i]->f;
    allVertices[i]->color = 0;
  }
  sort(final, final + size);

  for (long i = size - 1; i >= 0; i--) {
    if (allVertices[corrsepondingVertex[final[i]]]->color == 0) {
      allVertices[corrsepondingVertex[final[i]]]->cc = numSCC;
      DFS_visitCC(allVertices[corrsepondingVertex[final[i]]]);
      numSCC++;
    }
  }
  int *elsInScc = new int[numSCC];
  // init to 0;
  for (int i = 0; i < numSCC; i++) {
    elsInScc[i] = 0;
  }
  for (unsigned long i = 0; i < allVertices.size(); i++) {
    if (allVertices[i]->cc >= 0 && allVertices[i]->cc < numSCC)
      elsInScc[allVertices[i]->cc]++;
  }
  long idSCC = 0;
  // here elsInScc will be reassigned to contain new ids for SCC
  for (int i = 0; i < numSCC; i++) {
    if (elsInScc[i] < 2)
      elsInScc[i] = -1;
    else {
      elsInScc[i] = idSCC;
      idSCC++;
    }
  }
  numSCC = idSCC;
  for (unsigned long i = 0; i < allVertices.size(); i++) {
    atomCC[allVertices[i]->name] = elsInScc[allVertices[i]->cc];
  }
  delete[] corrsepondingVertex;
  delete[] final;
  delete[] elsInScc;
}
void Graph::SCC(long *atomCC, long &numSCC) {
  DFSClassic();
  DFSreverseCC(atomCC, numSCC);
}

void Graph::DFS_visitCC(Vertex *v) {
  v->color = 1;
  for (unsigned long j = 0; j < v->revAdj.size(); j++) {
    if (v->revAdj[j]->color == 0) {
      v->revAdj[j]->cc = v->cc;
      DFS_visitCC(v->revAdj[j]);
    }
  }
  v->color = 2;
  return;
}
