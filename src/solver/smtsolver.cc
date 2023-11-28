#include "smtsolver.h"
#include "program.h"
#include "symbolicexpressionparser.h"
#include "defines.h"
#include "theory.h"
#include "timer.h"
#include <algorithm>
#include <boost/process.hpp>
#include <chrono>
#include <cstring>
#include <fstream>
#include <glog/logging.h>
#include <iostream>
#include <iterator>
#include <map>
#include <ostream>
#include <ratio>
#include <regex>
#include <sstream>
#include <unistd.h>
#include "solver/smtprocess.h"
#include "smtstringhelpers.h"

using namespace chrono;

void logIterationResults(int roundNumber, SolverResult& results, milliseconds totalRoundDuration) {
  if (VLOG_IS_ON(1)) {
    cout << "Answer " << roundNumber << ": ";
  }

  int displayedAtoms = 0;
  for (auto assignment : results.atomAssignments) {
    auto atom = assignment.first;
    bool atomSign = assignment.second;
    if (atomSign && (atom->showInOutputAnswerSet || VLOG_IS_ON(3))) {
      cout << atom->atom_name() << " ";
      displayedAtoms++;
    }
  }

  if (displayedAtoms == 0) {
    cout << "{}";
  }

  cout << endl;

  if (!results.constraintVariableAssignments.empty()) {
    for (auto assignment : results.constraintVariableAssignments) {
      cout << assignment.first->name << "=" << assignment.second << " ";
    }
    cout << endl;
  }

  if (VLOG_IS_ON(1) && !results.minimizationAtomAssignments.empty()) {
    cout << "Minimization: ";
  }

  if (!results.minimizationAtomAssignments.empty()) {
    for (auto minimization : results.minimizationAtomAssignments) {
      cout << minimization.second;
    }
    cout << endl;
  }

  VLOG(1) << "Finished round " << roundNumber << " in " << totalRoundDuration.count() << "ms";
  VLOG(1) << "  " << results.solveDuration.count() << "ms SMT Check Satisfiability";
  VLOG(1) << "  " << results.getValuesDuration.count() << "ms SMT Get Values";

  // Add newline between iterations
  cout << endl;
}

void Solver::SolveProgram(Param &params, Program &program) {
  list<TheoryStatement*> theoryStatements;
  for (auto pair : program.theoryStatements) {
    theoryStatements.push_back(pair.second);
  }

  this->logic = new QF_LIA_logic();
  this->logic->processTheoryStatements(theoryStatements);

  string programBody = getProgramBodyString(program);
  string command = "echo \"" + programBody + "\" > " + "temp.smt2";
  system(command.c_str());

  // Override -s option with --solver-command if provided
  auto solverProcess = !params.smtSolverCommand.empty() ? SMTProcess(params.smtSolverCommand) : SMTProcess(params.SMTsolver);

  solverProcess.Send(programBody);

  auto constraintVariables = logic->getConstraintVariables();
  list<Atom*> answerSetAtoms;
  for (auto a : program.atoms) {
    if (a->showInOutputAnswerSet) {
      answerSetAtoms.push_back(a);
    }
  }

  auto roundStart = high_resolution_clock::now();

  int i = 1;
  for (;; i++) {
    auto result = solverProcess.CheckSatAndGetAssignments(answerSetAtoms, constraintVariables, program.minimizations, params);

    if (i == 1) {
      LOG(INFO) << (result->isSatisfiable ? "SAT" : "UNSAT");
    }

    if (!result->isSatisfiable) {
      break;
    }

    // Only log timer results if SAT
    auto roundEnd = high_resolution_clock::now();
    logIterationResults(i, *result, duration_cast<milliseconds>(roundEnd - roundStart));
    roundStart = high_resolution_clock::now();

    if (params.answerSetsToEnumerate != 0 &&
        params.answerSetsToEnumerate == i) {
      break;
    }

    if (!result->minimizationAtomAssignments.empty()) {
      auto minimizationAssertion = getMinimizationAssertionString(result->minimizationAtomAssignments);
      solverProcess.Send(minimizationAssertion);
    }

    if (result->atomAssignments.empty()) {
      break;
    }

    auto answerSetNegation = getAnswerNegationString(*result, params.includeConstraintsInEnumeration);
    solverProcess.Send(answerSetNegation);
  }
}

string Solver::getAnswerNegationString(SolverResult& result, bool includeConstraintVariables) {
  if (result.atomAssignments.size() == 0) {
    LOG(FATAL) << "Cannot create answer negation for empty set";
  }

  list<string> answerSetValueExpressions;

  for (pair<Atom*, bool> atomAssignment : result.atomAssignments) {
    if (atomAssignment.second) {
      answerSetValueExpressions.push_back(SMT::Var(atomAssignment.first));
    } else {
      answerSetValueExpressions.push_back(SMT::Not(SMT::Var(atomAssignment.first)));
    }
  }

  if (includeConstraintVariables && result.constraintVariableAssignments.size() > 0) {
    for (pair<SymbolicTerm*, string> variableAssignment : result.constraintVariableAssignments) {
      answerSetValueExpressions.push_back(SMT::Expr("=", {SMT::Var(variableAssignment.first), variableAssignment.second}));
    }
  }

  return SMT::Assert(SMT::Not(SMT::And(answerSetValueExpressions)));
}

string Solver::getMinimizationAssertionString(map<MinimizationStatement*,string> &minimizationResults) {
  ostringstream output;
  output << "(assert (or ";
  for (auto minimization : minimizationResults) {
    output << "(< " << SMT::Var(minimization.first) << " " << minimization.second << ") ";
  }
  output << "))" << endl;

  return output.str();
}

string Solver::getProgramBodyString(Program &program) {
  ostringstream output;

  output << "(set-info :smt-lib-version 2.6)" << endl;
  output << "(set-option :produce-assignments true)" << endl;
  output << "(set-logic " << logic->SMT_LOGIC_NAME() << ")" << endl;

  for (Atom *a : program.atoms) {
    if (!a->isLevelRankingConstraint()) {
      output << SMT::DeclareConst(a->getSmtName(), "Bool");
    }
  }

  for (LevelRankingVariable levelRankingVar : program.levelRankingVariables) {
    string lrVar = SMT::Var(levelRankingVar);
    output << SMT::DeclareConst(lrVar, "Int");
    output << SMT::Assert(SMT::Expr("<=", {
            to_string(levelRankingVar.lowerBound),
            lrVar,
            to_string(levelRankingVar.upperBound)
          }));
  }

  logic->getDeclarationStatements(output);

  for (Clause *c : program.clauses) {
    output << "(assert " << toSMTString(c) << ")" << endl;
  }
  logic->getAssertionStatements(output);

  for (MinimizationStatement *m : program.minimizations) {
    output << "(declare-const " << SMT::Var(m) << " Int)" << endl;
    output << "(assert (= " << SMT::Var(m) << " (+";
    for (MinimizationAtom *a : m->atoms) {
      output << " (ite " << SMT::Var(&a->atom) << " " << a->weight << " 0)";
    }
    output << ")))" << endl;
  }

  return output.str();
}

string Solver::toSMTString(Clause *clause) {
  ostringstream expression;
  Atom **a;

  list<string> expressions;
  for (a = clause->nbody; a != clause->nend; a++) {
    expressions.push_back(SMT::Not(SMT::Var(*a)));
  }
  for (a = clause->pbody; a != clause->pend; a++) {
    expressions.push_back(SMT::Var(*a));
  }

  return SMT::Or(expressions);
}
