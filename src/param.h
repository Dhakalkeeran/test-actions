// yuliya@cs.utexas.edu
#include <string.h>
#include <string>
#include "defines.h"
#ifndef PARAM_H
#define PARAM_H
class Param {
public:
  bool loopFormula;  // true if loop formula is built
                     // false is reason is built
  bool loopFormula1; // true if only one loop formula is built
                     // false otherwise
  bool temp;         // true if added clauses for loop formula or
  // reason are only temporary learned (relevent to minisat v.1)
  bool eloop;       // elementary loop
  bool eloopSwitch; // elementary loop
  //_________________
  // parameters that can be passed at command line
  // or set by functions
  char config[100];
  std::string file;
  std::string debug_file;
  char dirName[256];
  char cmodelsname[256];
  char dimacsFileName[512];
  char solverOutputFileName[512];
  int many;
  bool manySet;
  char files[10][512];
  int numOfFiles;
  int extmany;
  bool sort;
  SolverType sys;
  bool reducedCompletion;
  bool minimalUpperBound;
  SMTSolverCommand SMTsolver;
  bool PrintExtAS;
  bool wf;
  bool nowellfounded; // skip well founded computation
  int verifyMethod;
  bool param;
  bool cm_wfm; // default 0 stops at prinitng WFM
  bool dir;
  int timeout; // default is 0 and this meens no timeout
               // otherwise the value meens that in timeout
               // seconds the process will timeout
  int heur;

  bool hf; // first false  heuristic for Simo
  bool keep;
  bool bt;
  bool bj;
  bool le;
  bool bmodes;
  bool shortr;
  int numLFClauses;
  bool printCycle;
  int forgetPercent; // percentage of learnt clauses to be forgottem when denial
                     // comes back from external solver. Default is 0. No
                     // clauses forgot.
  //_________________

  int verboseLogLevel;
  int answerSetsToEnumerate;
  bool includeConstraintsInEnumeration;
  std::string grounderCommand;
  std::string smtSolverCommand;

  Param() {
    forgetPercent = 0;
    printCycle = false;
    verifyMethod = MINSCC;
    sys = MINISAT;
    numLFClauses = 1;
    dir = false;
    timeout = 0;
    heur = 0; // default is 0 and this meens the default heuristics for
    // simo to do search, all other numbers are specified within simo
    // for different heuristics
    keep = false;
    bt = false;
    bj = false;
    le = true;
    bmodes = false;
    hf = false;
    shortr = false;
    many = 1;
    manySet = false;
    extmany = 1;
    loopFormula = true;
    loopFormula1 = false;
    temp = false;
    numOfFiles = 0;
    strcpy(config, "\0");
    for (int a = 0; a < 10; a++)
      strcpy(files[a], "\0");
    strcpy(dirName, "\0");
    strcpy(cmodelsname, "\0");
    strcpy(dimacsFileName, "\0");
    strcpy(solverOutputFileName, "\0");
    eloop = false;
    wf = true;
    cm_wfm = false;
    sort = false;
    nowellfounded = false;
  }
  void finish() {
    if (sys == SIMO)
      numLFClauses = 1;
    if (numLFClauses < 0)
      numLFClauses = 256;
    if (many < 0)
      many = 1;
    if (eloop) {
      //	  loopFormula = true;
      eloopSwitch = false;
    }
  }
}

;
#endif
