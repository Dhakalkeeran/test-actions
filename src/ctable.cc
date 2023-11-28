/*
 * File ctable.cc
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
#include <iomanip>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined _BSD_SOURCE || defined _SVID_SOURCE
#include <sys/resource.h>
#include <sys/time.h>
#endif
#include "ctable.h"
#include "time.h"

Ctable::Ctable(Cmodels &cmodels, Api &api, Read &reader)
    : cmodels(cmodels), api(api), reader(reader) {
  cmodels.api = &api;
}

Ctable::~Ctable() {}

int Ctable::read() {

  int result = reader.read(cmodels.param.file);

  // the program has just been read; no modifications on the program were
  // performed so we can safely set the value of
  // program.original_number_of_atoms
  cmodels.program.original_number_of_atoms = cmodels.program.number_of_atoms;
  return result;
}
void Ctable::setSolver(SolverType st) {
  cmodels.param.sys = st;
  // this function is assumed to be used only for incremental solving and
  // Minisat and ZChaff and Minisat1 are the only solvers that will support this
  if (st != MINISAT && st != ZCHAFF && st != MINISAT1)
    cmodels.param.sys = MINISAT1;
}
void Ctable::setExecutionArgs(char args[]) {
  char str[256];
  strcpy(str, args); //"- This, a sample string.";
  char *arg = strtok(str, " ");
  char *option;
  while (arg != NULL) {

    option = strtok(NULL, " ");
    setSingleExecutionArgument(arg, option);
    arg = option;
  }
}

// DEPRECATED This has been replaced by Boost program_options.
// I'm keeping this here for reference.
int Ctable::setSingleExecutionArgument(char *arg, char *option) {
  int ret = 0;

  if (isdigit(arg[0])) {
    if (cmodels.param.manySet == false) {
      cmodels.param.many = atoi(arg);
      cmodels.param.manySet = true;
    } else
      cmodels.param.extmany = atoi(arg);
  } else if (arg[0] == '-') {
    if (strcmp(&arg[1], "rs") == 0) {
      cmodels.param.sys = RELSAT;
      //			cmodels.param.loopFormula = true;

    } else if (strcmp(&arg[1], "file") == 0) {
      if (option == NULL) {
        usage();
        exit(1);
      }
      // if (cmodels.param.numOfFiles == 0)
      // strcpy(cmodels.param.file, &option[0]);
      strcpy(cmodels.param.files[cmodels.param.numOfFiles], &option[0]);
      cmodels.param.numOfFiles++;
      ret = 1;
    } else if (strcmp(&arg[1], "dimacs") == 0)
      cmodels.param.sys = DIMACS_PRODUCE;
    else if (strcmp(&arg[1], "SCClevelRanking") == 0)
      cmodels.param.sys = SCC_LEVEL_RANKING;
    else if (strcmp(&arg[1], "levelRanking") == 0)
      cmodels.param.sys = LEVEL_RANKING;
    else if (strcmp(&arg[1], "SCClevelRankingStrong") == 0)
      cmodels.param.sys = SCC_LEVEL_RANKING_STRONG;
    else if (strcmp(&arg[1], "levelRankingStrong") == 0)
      cmodels.param.sys = LEVEL_RANKING_STRONG;
    else if (strcmp(&arg[1], "reducedCompletion") == 0)
      cmodels.param.reducedCompletion = true;
    else if (strcmp(&arg[1], "minimalUpperBound") == 0)
      cmodels.param.minimalUpperBound = true;
    else if (strcmp(&arg[1], "cvc4") == 0)
      cmodels.param.SMTsolver = CVC4;
    else if (strcmp(&arg[1], "z3") == 0)
      cmodels.param.SMTsolver = Z3;
    else if (strcmp(&arg[1], "yices") == 0)
      cmodels.param.SMTsolver = YICES;
    else if (strcmp(&arg[1], "PrintExtAS") == 0)
      cmodels.param.PrintExtAS = true;
    else if (strcmp(&arg[1], "cdimacs") == 0)
      cmodels.param.sys = CASP_DIMACS_PRODUCE;
    else if (strcmp(&arg[1], "zc") == 0)
      cmodels.param.sys = ZCHAFF;
    else if (strcmp(&arg[1], "zca") == 0)
      cmodels.param.sys = ASSAT_ZCHAFF;
    else if (strcmp(&arg[1], "ms") == 0)
      cmodels.param.sys = MINISAT;
    else if (strcmp(&arg[1], "ms1") == 0)
      cmodels.param.sys = MINISAT1;
    else if (strcmp(&arg[1], "bc") == 0)
      cmodels.param.sys = BCIRCUIT;

    else if (strcmp(&arg[1], "si") == 0) {
      cmodels.param.sys = SIMO;
    } else if (strcmp(&arg[1], "sb") == 0) {
      cmodels.param.sort = true;
    } else if (strcmp(&arg[1], "atomreason") == 0) {
      cmodels.param.loopFormula = false;
    } else if (strcmp(&arg[1], "loopformula1") == 0) {
      cmodels.param.loopFormula1 = true;
    } else if (strcmp(&arg[1], "temp") == 0) {
      cmodels.param.temp = true;
    } else if (strcmp(&arg[1], "version") == 0) {
      cout << "Version " << CMODELS_VERSION << "\n";
      exit(0);
    } else if (strcmp(&arg[1], "eloop") == 0) {
      cmodels.param.eloop = true;
    } else if (strcmp(&arg[1], "statistics") == 0)
      cmodels.output.stat = true;
    else if (strcmp(&arg[1], "w") == 0)
      cmodels.param.cm_wfm = true;

    else if (strcmp(&arg[1], "dlp") == 0 | strcmp(&arg[1], "-dlp") == 0) {
      cmodels.program.disj = true;
      cmodels.program.disjProgramLparse = true;
    } else if (strcmp(&arg[1], "verify") == 0) {
      numberExpected(option);
      int method = atoi(&option[0]);
      if (method == 0)
        cmodels.param.verifyMethod = MIN;
      if (method == 1)
        cmodels.param.verifyMethod = MINSCC;
      if (method > 1 | method < 0) // default then
        cmodels.param.verifyMethod = MINSCC;
      ret = 1;
    }
    //	  else if(strcmp (&arg[1], "s") == 0)
    //	cmodels.param.wf= false;
    else if (strcmp(&arg[1], "printCycle") == 0)
      cmodels.param.printCycle = true;
    else if (strcmp(&arg[1], "out_f_c") == 0) {
      cmodels.output.out_f_c = true;
    } else if (strcmp(&arg[1], "timings") == 0)
      cmodels.output.timings = true;
    else if (strcmp(&arg[1], "silent") == 0) {
      cmodels.output.asparagus =
          SILENT; // program should not output res onlytime

    } else if (strcmp(&arg[1], "asparagus") == 0) {
      cmodels.output.asparagus =
          ASPARAGUS; // program should output in asparagus format
    } else if (strcmp(&arg[1], "asp2") == 0) {
      cmodels.output.asparagus =
          ASPCOMP2; // program should  output in 3d asp competition format

    }

    else if (strcmp(&arg[1], "onlytime") == 0) {
      cmodels.output.asparagus =
          SILENT; // program should not output res onlytime

    } else if (strcmp(&arg[1], "ext") == 0) {
      if (option == NULL) {
        usage();
        exit(1);
      }
      cmodels.param.dir = true;
      strcpy(cmodels.param.dirName, &option[0]);
      ret = 1;
    } else if (strcmp(&arg[1], "forget-percent") == 0) {
      numberExpected(option);
      cmodels.param.forgetPercent = atoi(&option[0]);
      if (cmodels.param.forgetPercent < 0 ||
          cmodels.param.forgetPercent > 100) {
        cout << "Warning: -forget-percent is set to default 0 as its specified "
                "value should range between 0 and 100 whereas it is sat to "
             << &option[0] << endl;
        cmodels.param.forgetPercent = 0;
      }
      ret = 1;
    } else if (strcmp(&arg[1], "timeout") == 0) {
      numberExpected(option);
      cmodels.param.timeout = atoi(&option[0]);
      // strcpy(cmodels.dirName,&argv[c+1][0]);
      ret = 1;
    } else if (strcmp(&arg[1], "heur") == 0) {
      numberExpected(option);
      cmodels.param.heur = atoi(&option[0]);
      // strcpy(cmodels.dirName,&argv[c+1][0]);
      ret = 1;
    } else if (strcmp(&arg[1], "config") == 0) {
      if (option == NULL) {
        usage();
        exit(1);
      }
      strcpy(cmodels.param.config, &option[0]);
      ret = 1;
    } else if (strcmp(&arg[1], "numlf") == 0) {
      numberExpected(option);
      cmodels.param.numLFClauses = atoi(&option[0]);
      // strcpy(cmodels.dirName,&argv[c+1][0]);
      ret = 1;
    } else if (strcmp(&arg[1], "keep") == 0) {
      cmodels.param.keep = true;

    } else if (strcmp(&arg[1], "bt") == 0) {
      cmodels.param.bt = true;
      cmodels.param.le = false;
    } else if (strcmp(&arg[1], "bj") == 0) {
      cmodels.param.bj = true;
      cmodels.param.le = false;
    } else if (strcmp(&arg[1], "le") == 0) {
      cmodels.param.le = true;
    } else if (strcmp(&arg[1], "modes") == 0) {
      cmodels.param.bmodes = true;
    } else if (strcmp(&arg[1], "shortr") == 0) {
      cmodels.param.shortr = true;
    } else if (strcmp(&arg[1], "hf") == 0) {
      cmodels.param.hf = true;
    } else {
      usage();
      exit(1);
    }
  } else {
    usage();
    exit(1);
  }

  return ret;
}

void Ctable::numberExpected(char *option) {
  if (option == NULL || !isdigit(option[0])) {
    usage();
    exit(1);
  }
}

int Ctable::getNumberGroundedAtoms() {
  return cmodels.program.original_number_of_atoms;
}

void Ctable::Initialize(int *answerset_lits, int &num_atoms,
                        const char **&symbolTable, int &symbolTableEntries) {
  cmodels.output.asparagus = SILENT;
  cmodels.init(answerset_lits, num_atoms, symbolTable, symbolTableEntries);
  //  cout<<"After initialize: num_atoms: "<<num_atoms<<endl;
  if (num_atoms >= -1)
    solved = true;
  else
    solved = false;
}

void Ctable::Initialize(int *answerset_lits, int &num_atoms) {
  const char **symbolTable;
  int symbolTableEntries;

  Initialize(answerset_lits, num_atoms, symbolTable, symbolTableEntries);
}

bool Ctable::addDenial(int *constraint_lits, int num_lits) {
  //  print_lits(constraint_lits, num_lits,true);
  return cmodels.addDenial(constraint_lits, num_lits);
}

void Ctable::print_lits(int *constraint_lits, int num_lits, bool denial) {
  int cur;
  bool cur_sign;
  int inner_count = 0;
  long indA = 0;

  if (!denial) {
    cout << "Solution: ";
  } else
    cout << "Denial: :- ";

  for (int i = 0; i < num_lits; i++) {
    if (!denial) {
      cur = constraint_lits[i];
      cur_sign = true;
    } else {
      if (constraint_lits[i] % 2) { // if it is odd
        cur = (constraint_lits[i] - 1) / 2;
        cur_sign = true;
      } else {
        cur = constraint_lits[i] / 2;
        cur_sign = false;
      }
    }

    for (indA = inner_count; indA < cmodels.program.cmodelsAtomsFromThisId;
         indA++) {

      if (cur == cmodels.program.atoms[indA]->get_lparse_id()) {

        if (cur_sign)
          cmodels.program.atoms[indA]->print();
        else {
          cout << "-";
          cmodels.program.atoms[indA]->print();
        }
        cout << ", ";

        break;
      }
    }
    if (indA == cmodels.program.cmodelsAtomsFromThisId - 1 &&
        i != num_lits - 1) {
      cerr << "Cmodels: Error with denial print ";
      exit(20);
    }
  }
  cout << " done." << endl;
}

void Ctable::calculate() { cmodels.cmodels(); }


// DEPRECATED This has been replaced by Boost program_options in main.cc
// I'm leaving this here for reference.
void Ctable::usage() {
  cerr << "Usage: ./ezsmtPlus -file <path> [-file <path>] [-file <path>] "
          "<num1> <num2> [-PrintExtAS] "
          "[-levelRanking|-levelRankingStrong|-SCClevelRanking|-"
          "SCClevelRankingStrong] [-reducedCompletion] [-minimalUpperBound] "
          "[-cvc4|-yices|-z3] [-non-linear]"
       << endl
       << "<path> is the path to the input program (file should be in ezcsp "
          "language)"
       << endl
       << "<num1> is the number of answer sets to be computed. 0 stands for "
          "all and 1 is default."
       << endl
       << "<num2> is the number of extended answer sets to be computed. 0 "
          "stands for all and 1 is default."
       << endl
       << "[-cvc4|-z3|-yices] instructs EZSMTPLUS to call the corresponding "
          "SMT solver. By default, -z3 is chosen."
       << endl
       << "[-PrintExtAS] instructs EZSMT+ to print out extended answer sets. "
          "By default, only answer sets will be printed."
       << endl
       << "[-levelRanking|-levelRankingStrong|-SCClevelRanking|-"
          "SCClevelRankingStrong] selects the typy of level ranking formulas "
          "produced for non-tight programs. By default, -SCClevelRanking is "
          "chosen."
       << endl
       << "[-reducedCompletion] will instruct EZSMTPLUS to remove the part of "
          "Clark's completion that is captured by a level ranking formula."
       << endl
       << "[-minimalUpperBound] sets the upper bound of level ranking "
          "variables to the number of atoms inside the Strongly Connected "
          "Component containing that variable. A bigger upper bound (the total "
          "number of atoms) would be selected by default."
       << endl
       << "[-non-linear] selects non-linear logics for SMT solvers." << endl;
};
