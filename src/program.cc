/*
 * File program.cc
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
#include "program.h"
#include "atomrule.h"
#include "ctable.h"
#include "defines.h"
#include <ctype.h>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <vector>

Program::Program() {
  //  optimize = 0;
  number_of_atoms = 0;
  number_of_nestedRules = 0;
  number_of_rules = 0;
  number_of_complitions = 0;
  number_of_clauses = 0;
  cmodelsAtomsFromThisId = 0;

  basic = true;
  false_atom = 0;
  disj = false;
  disjProgramLparse = false;
  tight = false;
  hcf = false;
  rules.clear();
  completions.clear();
  atoms.clear();
  clauses.clear();
}

Program::~Program() {}

void Program::print() {
  for (list<Rule *>::iterator itrRule = rules.begin(); itrRule != rules.end();
       ++itrRule)
    (*itrRule)->print();

  cout << "compute { ";
  bool comma = false;

  for (vector<Atom *>::iterator itrAtom = atoms.begin(); itrAtom != atoms.end();
       ++itrAtom) {

    if ((*itrAtom)->computeTrue) {
      if (comma)
        cout << ", ";
      (*itrAtom)->print();
      comma = true;
    }
    if ((*itrAtom)->Bneg) {
      if (comma)
        cout << ", ";
      cout << "not ";
      (*itrAtom)->print();
      comma = true;
    }
  }
  cout << " }" << endl;
}

void Program::print_completion() {
  cout << "COMPLETION OF A PROGRAM" << endl;
  for (vector<Completion *>::iterator itrComp = completions.begin();
       itrComp != completions.end(); ++itrComp)
    (*itrComp)->print();
}

void Program::print_clauses(bool useAtomNames) {
  cout << "CLAUSES OF A PROGRAM" << endl;
  long size = 0;
  for (vector<Clause *>::iterator itrCl = clauses.begin();
       itrCl != clauses.end(); itrCl++) {
    size++;
    (*itrCl)->print(useAtomNames);
  }
}
void Program::print_atoms() {
  cout << "ATOMS OF A PROGRAM" << endl;
  long size = 0;
  for (vector<Atom *>::iterator itrCl = atoms.begin(); itrCl != atoms.end();
       itrCl++) {
    size++;
    (*itrCl)->print();
    cout << " ";
  }
  cout << endl;
}
void Program::print_atoms_wf() {
  cout << "WF ATOMS OF A PROGRAM" << endl;
  long size = 0;
  for (vector<Atom *>::iterator itrCl = atoms.begin(); itrCl != atoms.end();
       itrCl++) {
    size++;
    if ((*itrCl)->Bpos || (*itrCl)->computeTrue) {
      if ((*itrCl)->Bpos)
        cout << "+";
      (*itrCl)->print();
      cout << " ";
    }
    if ((*itrCl)->Bneg) {
      cout << "-";
      (*itrCl)->print();
      cout << " ";
    }
  }
  cout << endl;
}

void Program::clearInM() {
  for (long indA = 0; indA < atoms.size(); indA++)
    atoms[indA]->inM = false;
}

void Program::clearInLoop() {

  for (long indA = 0; indA < atoms.size(); indA++)
    atoms[indA]->inLoop = -1;
}
void Program::clearInMminus() {

  for (long indA = 0; indA < atoms.size(); indA++) {
    atoms[indA]->inMminus = false;
  }
}
void Program::clearInCons() {
  for (long indA = 0; indA < atoms.size(); indA++)
    atoms[indA]->cons = false;
};
