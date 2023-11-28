/*
 * File main.cc
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

// 3.53 version Aug 24, 05 removed hend1,pend1,nend1 from the rules
// that used to be copies of hen,pend,nend
//
// 3.54 version Aug 25, 05:
//(1) bug at buildingLoopFormulas fixed so it could cause
// problems for nontight programs
//(2) bug with rules ...v A v...:-..A... not being remove is fixed
// 3.55 version Sep 21, 05:
//(1) Zchaff version 2004.11.15 is incorporated in place of Zchaff 2003
//(2) Cmodels is made compatable with gcc3.4/4.* thanks too Wolfgang Faber
// 3.57 Includes
//         HCF check
//         modularity in model checking
//         loopformula by default
// 3.58 fixes the bug with loopformula from now on for disj programs only part
// of loop formula is added where head atom is sat
//
// 3.59 includes -eloop flag that performs elementary set computation
//  and adds loop formulas of elementary loops when needed
//
// 3.60 tightness bug fix through away SCC of 1 element
//      now all the unsat loops are added
//
// 3.6x    Ilkka's circuit component's
//         where implemented -bc flag is used plus bcircuit program
//         must be present
//         impl is preliminary hence output is not readable
//         no positive time results were gained
//         as Cmodels CNF representation is more compact and seem to
//         be better suit
// 3.63 Datastructures added to support linear time WFM, and Reduct
//
// 3.64-65 Bug fixes at WFM
//
// 3.66 Bug fixes Zchaff to be portable into 64 bit archetecture
//  Thanks to Yinlei Yu:
//  zchaff_dbase.cpp, line 324. Replacing "int displacement" with
//"long long displacement"
// 3.67 Bug fixes Zchaff to be portable into 64 bit archetecture
//  Thanks to Andre Neumann: on 64 bit archetecture UNSAT when SAT, due to no
//  return true; in the end of createClauses
//  zchaff_dbase.cpp, line 324. Replacing "int displacement" with
//"long long displacement"
//
//  3.68 Esra's bug fix that could occur on constraint rules translation
//  where new_atom was created instead of a false atom
//  + improved simplification
//  + Added flag -ms that allows calling MINISAT
//     Minisat is now incorporated into Cmodels,
//     its code has been added as part of Cmodels
//     two functions added to SimpSolver to allow
//     adding clauses from cmodels, and getting feedback from a solver
//     once solution is found
//     Minisat and Zchaff communication is alike
//  + bug fixes for choice rules as {k}:-k.
//  + bug fix with Weights that were not allowed to drop below negative value
//  version 3.69
//  + replaced zChaff version from 2004 with zChaff 2007.3.12
//  + added check on what addClause from Minisat returns, since when
//    it returns false the instance is UNSAT.
//  + -zca flag added only for comparison reasons of incremental SAT solving and
//    assat method
//  cmodels 3.70
//  Esra's bug Fix
//  WFM bug
// void ConstraintRule::HeadInOneRule(Atom* at)
// void     WeightRule::HeadInOneRule(Atom* at){
// some of the atoms could have been assigned values by now
// for instance a:-1{falseassigned1, notassigned}
// in such case only notassigned should be assigned computetrue
// Such bug could not appear in Basic, choice or disj rules
//
//
// 3.71 improvement of clausification by not inroducing aux atom
// when -a occurs in the body i.e, avoiding intoruding clauses for aux_1==-a
//+fixing intro two aux atoms for the same room
//
// 3.73 atmost is computed (wfs--complete); sorting is introduced to disregard
//   same bodies in rules, and also the rules that are subsummed by others
//
// 3.74 bug fixes due to Benjamin Kaufmann
//  (i) weightrule Atmost computation
//  (ii) completionchoice
//  (iii) weight rule elimination inefficiency in translation is removed
//       (compare worked unproperly) cmodels worked correctly but inefficiently
// 3.75 bug fix due to  Christian Drescher drescher@cs.uni-potsdam.de
//(occured in 3.73 not previously and only for the case of
// disjunctive programs i.e.
//  a v b:-c would be represented as two clauses: -c v a; -c v b
//  in place of -c v a v b
//
//+ following Benjamin suggestion changed minisatSolver->solve(true,false))
//   to minisatSolver->solve(true,true)) that disables farther than first time
//   preprocessing in minisat
//
// 3.76 bug fixed in loop formula construction due to not cleanscc
// 3.77 bug fixed in loop formula construction due to not cleanscc
//
// Minisat version 1 is added -ms1
//-loopformula1 flag (just one loopformula at a time is added
//  instead of all
//-temp flag in case of Minisat v 1(-ms1) learns and forgets loop formulas
//
//
// 3.78 bug fixed in read.cc
// int Read::addWeightRule (FILE *f)
//  now reading is done with add_head()
//  in place of add_head_repetition
//  bug was reported by Roland Kaminski 26 Feb Fri
//(it could occur only on programs with weight rules)
// 3.79
//  1) bug fixed in cmodels.cc
//  bool Cmodels::rec_weight_rule(Weight totalweight, int sizeC, Atom* headC,
//		     unsigned long atleast, int counter_body)
//
// weightrules translation had a bug by assigning the same atom to
// two different expressions (see comments in the code that specify what was
// changed)
//
//  bug was reported by  Raphael Finkel  10 Mar 2009
//(it could occur only on programs with weight rules)
//  2) in zchaff_dbase.cc displacement given a new type
//     to support both 32 and 64 bit archetectures in uniform manner.
//     The change is suggested by  Gurucharan Huchachar 11 Mar 2009
//     Line 329 zchaff_dbase.cc :
//     ptrdiff_t displacement = _lit_pool_start - old_start;
//     For 64 bit archetecture
// 3.80
//  minisat is now default instead of zchaff
//
// 3.81
//  capability for incremental ASP is added; please see ctable.h file
//  that contains all the details of the interface
// 3.82
//  under construction
//  idea to implement an additional propagation of minisat 1 on cmodels side
//  so that non-lazy approach were possible with EZ-CSP or other system if it
//  needs to 3.82g -> h bug fix due to Benjamin Kaufmann clearLoop added at
//  cmodels.cc 4847 3.83 per Marcello's request change in partial assignment
//  return to external solver. Now it is more flexible which atoms external
//  solver wants to see. See ctable.h for Docs. 3.84 bug fix reported by Peter
//  Schueller. Occurred in addDenial cmodels.cc 3.86 introduced -cdimacs flag to
//  allow ezsmt
//       to use cmodles capability compute completion
//  3.86.1 fixed declarations in Minisat so that gcc compiler version 5+
//  compiles the code (due to Marcello Balduccini)
//       introduced -file option to pass filename as a parameter for reading
//       (due to Ben Susman)

#include "boost/program_options/options_description.hpp"
#include "boost/program_options/parsers.hpp"
#include "boost/program_options/value_semantic.hpp"
#include "boost/program_options/variables_map.hpp"
#include "boost/throw_exception.hpp"
#include "ctable.h"
#include "defines.h"
#include "interpret.h"
#include "timer.h"
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <ctype.h>
#include <exception>
#include <fstream>
#include <glog/logging.h>
#include <iostream>
#include <new>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

using namespace std;
using namespace boost::algorithm;
namespace popts = boost::program_options;

void a_new_handler() {
  cerr << "operator new failed: out of memory" << endl;
  exit(23);
}
static void timeOutHandler(int sig) {
  // ctable.cmodels.output.PrintStats();
  // switch (sig) {

  //   ctable.cmodels.output.PrintStats();
  // #ifndef _WIN32	// marcy
  //   case SIGXCPU:
  // #ifndef SHORT_OUT
  //     ctable.cmodels.output.PrintStats();
  // #else
  //     ctable.cmodels.output.PrintStats();
  // #endif
  //     break;
  // #endif
  // default:
  //   fprintf(stderr, "Unknown event\n");
  //   break;
  // }

  exit(22);
}

void SetupLogging(char *executableName, int verbosity) {
  FLAGS_logtostderr = 1;
  FLAGS_colorlogtostderr = 1;
  FLAGS_log_prefix = false;
  FLAGS_v = verbosity;
  google::InitGoogleLogging(executableName);
}

int ParseArguments(int argc, char *argv[], Param &params) {
  bool showHelpMenu = false;
  popts::variables_map vm;

  // Empty comments are added after each line to prevent clang-format
  // from poorly formatting them.
  popts::options_description genericOptions("Generic Options");
  genericOptions.add_options()
    ("help,h", "Show this help menu") //
    ("verbose,v", popts::value<int>()->default_value(1)->implicit_value(2), "Verbose logging level:\n0 = Minimal output,\n1 = Default output,\n2 = Debug output,\n3 = Verbose debug output") //
    // ("file,f", popts::value<string>(), "Input file") // for single input file
    ("file,f", popts::value<vector<string>>()->multitoken(), "Input file") // for multiple input files
    ("debug-file,d", popts::value<string>()->default_value(""), "File that will be output for debugging or testing against other system");

  popts::options_description cmodelsOptions("CModels Options");
  cmodelsOptions.add_options()
    ("grounder-command", popts::value<string>(), //
                               "Override the grounder command used. The "
                               "command will be passed the input "
                               "file as an argument. It must output the "
                               "grounded program in ASPIF (Gringo "
                               "5) format to stdout.")
    ("level-ranking,l", popts::value<string>()->default_value("SCCLevelRanking"), //
     "[levelRanking | levelRankingStrong | SCCLevelRanking | SCCLevelRankingStrong]\n"
     "The type of level ranking formulas produced for non-tight programs.")
    ("reduced-completion", popts::value<bool>()->default_value(false)->implicit_value(true), //
     "Remove the part of Clark's completion that is captured by a level ranking formula.")
    ("minimal-upper-bound", popts::value<bool>()->default_value(false)->implicit_value(true), //
     "Sets the upper bound of level ranking variables to the number of atoms inside the "
     "Strongly Connected Component containing that variable. A bigger upper bound "
     "(the total number of atoms) would be selected by default.");

  popts::options_description solverOptions("Solver Options");
  solverOptions.add_options()
    ("solver,s", popts::value<string>()->default_value("cvc4"), "[z3|cvc4|cvc5|yices]\nSMT Solver to use.") //
    ("enumerate,e", popts::value<int>()->default_value(1)->implicit_value(0), //
      "Enumerate X answer sets. 0 or -e will enumerate all answer sets.") //
    ("enumerate-extended,E", popts::value<bool>()->default_value(false)->implicit_value(true), //
      "Enumerate extended answer sets (Include constraint values). Must be used with -e <number>.") //
    ("solver-command", popts::value<string>(), //
      "Override the SMT solver command used. This will override the --solver "
      "option. The command must support reading the SMT-LIB program from "
      "stdin.");

  popts::positional_options_description positionalArgs;
  positionalArgs.add("file", -1);

  popts::options_description allOptions;
  allOptions.add(genericOptions).add(cmodelsOptions).add(solverOptions);

  if (argc > 1) {
    try {
      store(popts::command_line_parser(argc, argv)
                .options(allOptions)
                .positional(positionalArgs)
                .run(),
            vm);
      notify(vm);

      params.verboseLogLevel = vm["verbose"].as<int>();

      // for single input file
      // params.file = vm["file"].as<string>().c_str();
      // params.numOfFiles = 1;
      
      // for multiple input files
      params.numOfFiles = 0;
      for (auto a : vm["file"].as<vector<string>>()){
        params.file += a + " ";
        params.numOfFiles++;
      }
      params.file = trim_copy(params.file);
      params.debug_file = vm["debug-file"].as<string>().c_str();

      params.grounderCommand = vm["grounder-command"].empty()
                                  ? "gringo"
                                  : vm["grounder-command"].as<string>();

      params.smtSolverCommand =
          vm["solver-command"].empty() ? "" : vm["solver-command"].as<string>();

      auto solver = vm["solver"].as<string>();
      if (solver == "z3")
        params.SMTsolver = Z3;
      if (solver == "cvc4")
        params.SMTsolver = CVC4;
      if (solver == "cvc5")
        params.SMTsolver = CVC5;
      if (solver == "yices")
        params.SMTsolver = YICES;

      auto levelRanking = vm["level-ranking"].as<string>();
      if (levelRanking == "levelRanking") {
        params.sys = LEVEL_RANKING;
      } else if (levelRanking == "levelRankingStrong") {
        params.sys = LEVEL_RANKING_STRONG;
      } else if (levelRanking == "SCCLevelRanking") {
        params.sys = SCC_LEVEL_RANKING;
      } else if (levelRanking == "SCCLevelRankingStrong") {
        params.sys = SCC_LEVEL_RANKING_STRONG;
      } else {
        throw new std::runtime_error("Could not find level ranking configuration for '" + levelRanking + "'");
      }

      params.reducedCompletion = vm["reduced-completion"].as<bool>();
      params.minimalUpperBound = vm["minimal-upper-bound"].as<bool>();

      params.answerSetsToEnumerate = vm["enumerate"].as<int>();
      params.includeConstraintsInEnumeration = vm["enumerate-extended"].as<bool>();
    } catch (const std::exception &e) {
      cout << "Error parsing arguments: " << e.what() << endl;
      showHelpMenu = true;
    }
  }

  if (argc == 1 || showHelpMenu || vm.count("help")) {
    cout << "The EZSMT Constraint Answer Set Solver" << endl;
    cout << "Version 3.0" << endl << endl;
    cout << "Usage: " << argv[0] << " [options] <file>" << endl;
    cout << allOptions << endl;
    return 1;
  }

  return 0;
}

int main(int argc, char *argv[]) {
  Param params;
  int exit = ParseArguments(argc, argv, params);
  if (exit == 1) {
    return 1;
  }

  SetupLogging(argv[0], params.verboseLogLevel);

  Timer mainTimer;

  set_new_handler(&a_new_handler);
  bool error = false;

  Solver smtSolver;
  Cmodels cmodels(smtSolver, params);
  Api api(&cmodels.program);
  Read reader(&cmodels.program, &api, &params);
  Ctable ctable(cmodels, api, reader);

  strcpy(params.cmodelsname, &argv[0][0]);

  strcpy(params.config, "CONFIG");

  // if the timeout was set then we will set the function for timeout
  if (params.timeout != 0)
    mainTimer.SetTimeout(params.timeout, timeOutHandler);

  // This is not possble at the moment unless we include
  // clausification of loop formulas using many clauses
  //   if(ctable.cmodels.sys == 3 & ctable.cmodels.le)
  //     ctable.cmodels.sys =6; //here we force zchaff
  // with incremental learning to be our choice of a sat solver

  // we find the path to cmodels and assume that config file is located there
  if (strcmp(params.config, "\0") == 0) {
    char path_to_config[100];
    int k = 0;
    path_to_config[k] = '\0';
    int length = strlen(params.cmodelsname);
    int l = length - 1;
    while (l != -1 && params.cmodelsname[l] != '/') {
      l--;
    }
    if (l >= 0) {
      for (k = 0; k <= l; k++)
        path_to_config[k] = params.cmodelsname[k];
      path_to_config[k] = '\0';
    }
    strcat(path_to_config, "CONFIG");
    strcpy(params.config, path_to_config);
  }

  if (error) {
    return 1;
  }

  if (params.numOfFiles == 0) {
    return 1;
  }

  vector<string> split_names;
  split(split_names, params.file, is_space());
  string grounded_file = split_names[0];
  for (int i=1; i < split_names.size(); i++){
    string temp = split_names[i];
    temp = temp.substr(temp.find_last_of("/\\") + 1);
    grounded_file += "_" + temp;
  }

  string groundingCommand = params.grounderCommand + " " + params.file +
                            " > " + grounded_file + ".grounded";

  VLOG(2) << "Running grounding command: " << groundingCommand;
  system(groundingCommand.c_str());

  params.file = grounded_file + ".grounded";

  // Check errors from grounding output
  ifstream groundedProgram(params.file);
  for (string line; getline(groundedProgram, line);) {
    if (line.find("error: ") != string::npos ||
        line.find("ERROR: (gringo): ") != string::npos) {
      LOG(FATAL) << "Error during grounding. See output file " << params.file;
    }
  }
  groundedProgram.close();

  VLOG(3) << "Grounded program:";
  if (VLOG_IS_ON(3)) {
    system((string("cat ") + params.file).c_str());
    cout << endl;
  }

  ctable.cmodels.output.timerAll.start();

  if (params.file.length() > 0) {
    freopen(params.file.c_str(), "r", stdin);
  }

  // std::ifstream groundedFile(params.file);

  int bad = ctable.read();
  if (ctable.cmodels.output.asparagus == STANDARD)
    if (bad) {
      cerr << "Error in input" << endl;
      return 1;
    }
  // removes some setting that might be not fitting
  // for some specific SAT solver
  params.finish();

  ctable.calculate();

  ctable.cmodels.output.timerAll.stop();
  ctable.cmodels.output.print();

  return 0;
}
