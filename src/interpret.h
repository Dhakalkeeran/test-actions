#ifndef OUTPUT_H
#define OUTPUT_H
#include "defines.h"
#include "glog/logging.h"
#include "param.h"
#include "program.h"
#include "timer.h"
#include <cstring>
using namespace std;
class Output {
public:
  OutputMethod
      asparagus; // if STANDARD we generate standard output
                 //    ASPARAGUS we generate output for Asparagus competition
                 //    ASPCOMP2 we generate output for 2d ASP competition
                 //    SILENT we generate no output w.r.t. answer sets but there
                 //    can be statistics printed if requested

  bool out_f_c; // if true we generate output in OUT*
  bool stat;    // if true we print out statistics
  bool timings; // if true we print out times in file TIMINGS
  long numSameBodies;
  long numRules;

  // timers
  Timer timerAll;
  Timer timerTranslation;
  Timer timerCompletion;
  Timer timerClausification;
  Timer timerVerification;
  Timer timerLoopFormula;
  Timer timerSat;

  // for statistics
  long numLoops;
  long numELoops;
  Result sat;
  long numSolutions;
  long numModels;
  long numModelsFirstSol;
  long numSatVerifyCalls;
  Program *program;
  Param *param;

  Output();
  ~Output();

  void PrintStats();
  void print_timings();
  void print();
  void print_assignment(bool *assignment);
  void print_wfm();
  // returns false if unsat
  Result print_relsat_solutions(char *solver_out);
  Result interpret_relsat(char *solver_out, bool *sol);
  Result interpret_assat_zchaff(char *solver_out, bool *sol);
  long findNextAtom(char *buf, long &i);
  inline void start_output();
  inline void disj_output();
  inline void tight_output();
  inline void hcf_output();
  void solver_call();
  inline void end_output();
  inline void false_output();
  inline void unknown_output();
};

inline void Output::start_output() {
  if (asparagus == STANDARD)
    printf("Answer: %ld \n Answer set: ", numSolutions);
  else if (asparagus == ASPARAGUS)
    printf("Answer Set: ");
}
inline void Output::disj_output() {
  if (asparagus == STANDARD)
    if (program->disj)
      VLOG(1) << "Program is Disjunctive." << endl;
}
inline void Output::tight_output() {
  if (asparagus == STANDARD)
    if (!program->tight)
      VLOG(1) << "Program is not tight.";
    else
      VLOG(1) << "Program is tight.";
}
inline void Output::hcf_output() {
  if (asparagus == STANDARD)
    if (!program->hcf)
      VLOG(1) << "Program is not HCF." << endl;
    else
      VLOG(1) << "Program is HCF." << endl;
}
inline void Output::end_output() {
  if (asparagus == STANDARD || asparagus == ASPCOMP2)
    printf("\n");
}
inline void Output::false_output() {
  if (asparagus == STANDARD || asparagus == ASPARAGUS)
    if (!numSolutions)
      printf("No Answer Set");
  if (asparagus == ASPCOMP2)
    if (!numSolutions)
      printf("INCONSISTENT");
  end_output();
}

inline void Output::unknown_output() {
  if (asparagus == STANDARD || asparagus == ASPCOMP2) {
    printf("UNKNOWN");
  }
  end_output();
}

;
#endif
