/*
 * File program.h
 * Last modified on 2 19:34 2002
 * By Yuliya Babovich
 *
 */

// Copyright 1998 by Patrik Simons
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston,
// MA 02111-1307, USA.
//
// Patrik.Simons@hut.fi
#ifndef PROGRAM_H
#define PROGRAM_H

#include "atomrule.h"
#include "theory.h"
#include <map>
#include <queue>
#include <vector>
#include <sstream>
using namespace std;

class OptimizeRule;
struct sortedLE;

class MinimizationAtom {
public:
  MinimizationAtom(Atom &atom, double weight) : atom(atom) {
    this->weight = weight;
  };
  Atom &atom;
  double weight;
};

const string NEVER_ATOM = "EZSMTPLUS_NEVER";
const string MINIMIZATION_SMT_PREFIX = "EZSMTPLUS_MINIMIZATION";
const string THEORY_ATOM_PREFIX = "EZSMTPLUS_THEORY";
const string LEVEL_RANKING_ATOM_PREFIX = "EZSMTPLUX_LR";

class MinimizationStatement {

public:
    MinimizationStatement(int id, int priority) {
      this->id = id;
      this->priority = priority;
    };

  int id;
  int priority;
  list<MinimizationAtom *> atoms;

  string getAtomName()
  {
    stringstream name;
    name << MINIMIZATION_SMT_PREFIX << "(" << id << "," << priority << ")";
    return name.str();
  }
};

class LevelRankingVariable {
public:
    LevelRankingVariable(int id, int lowerBound, int upperBound) {
      this->id = id;
      this->lowerBound = lowerBound;
      this->upperBound = upperBound;
    }

    int id;
    int lowerBound;
    int upperBound;

    string GetSmtName() {
      return "lr" + to_string(id);
    }
};

class Program {
public:
  Program();
  ~Program();
  void init();
  void print();
  void print_completion();
  void print_clauses(bool useAtomNames = false);
  void print_atoms();
  void print_atoms_wf();

  queue<Atom *> q;
  vector<Atom *> atoms;

  list<Rule *> rules;
  list<MinimizationStatement *> minimizations;
  map<int, ITheoryTerm *> theoryTerms;
  map<int, TheoryAtomElement *> theoryAtomElements;
  map<int, TheoryStatement *> theoryStatements;
  list<LevelRankingVariable> levelRankingVariables;


  // Vector which will have all the completions
  vector<Completion *> completions;

  // Vector which will have all the clauses
  vector<Clause *> clauses;

  // this number will contain a number of atoms contained in grounded program
  // passed to cmodels It is needed for the correct implementation of
  // Ctable::getNumberGroundedAtoms() and Ctable::Solve(int* answerset_lits,
  // int& num_atoms) (since number_of_atoms will contain a different number
  // after a number of simplifications done on a program
  long original_number_of_atoms;

  long number_of_atoms;
  int number_of_rules;
  int number_of_complitions;
  long number_of_clauses;
  long number_of_nestedRules;
  long cmodelsAtomsFromThisId;

  Atom *false_atom; // since we need to work with atom which stands for false
                    // differently creating it's completions false_atom is a
                    // pointer
                    // to false atom

  bool conflict;

  bool basic; // true if program is translated into basic
              // false if program is translated into nested basic program (if it
              // has choice rules)
  bool tight; // true if the program is tight
  bool hcf;   // true if the program is hcf
  bool disj;  // true if the program is disjunctive

  bool disjProgramLparse; // true if the program is disjunctive and lparse is
                          // used

  void clearInM();
  void clearInLoop();
  void clearInMminus();
  void clearInCons();
  bool wellfounded();

  void atleast();
  void atmost();
};

#endif
