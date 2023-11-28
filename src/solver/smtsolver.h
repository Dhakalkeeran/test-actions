#ifndef SMTSOLVER_H_
#define SMTSOLVER_H_

#include "defines.h"
#include "glog/logging.h"
#include "param.h"
#include "program.h"
#include "logics/logic.h"
#include "logics/QF_LIA_logic.h"
#include <chrono>
#include <memory>
#include <regex>


class SolverResult {
  public:
    bool isSatisfiable;
    map<Atom*, bool> atomAssignments;
    map<SymbolicTerm*, string> constraintVariableAssignments;
    map<MinimizationStatement*, string> minimizationAtomAssignments;

    chrono::milliseconds solveDuration;
    chrono::milliseconds getValuesDuration;
};

class Solver {
public:
  void SolveProgram(Param &params, Program &program);

private:
  string getProgramBodyString(Program &program);
  string getAnswerNegationString(SolverResult& result, bool includeConstraintVariables);
  string getMinimizationAssertionString(map<MinimizationStatement*,string> &minimizationResults);

  string toSMTString(Clause *clause);

  ILogic* logic;
};

#endif // SMTSOLVER_H_
