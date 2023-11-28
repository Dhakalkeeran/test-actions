/*
 * File ctable.h
 * Last modified on Feb 8, 2010
 * By Yuliya Lierler
 */
// /*============================================================
//
// This is the header for using the "incremental" cmodels
//  A typical flow is
//
// 1. calling read(FILE *f) where f is the file containing
//    grounded program (by Lparse or Gringo)
//    Will load grounded program into Cmodels
//    if retunred value of read is different from 0
//    then there is an error in input
//
// 2. calling setSolver(SolverType st)
//    SolverType is defined in defines.h
//    for incremental solving valid types are MINISAT and ZCHAFF and MINISAT1
//    feedback solving is available only with MINISAT1
//    By default it will be MINISAT
// 2.1 calling void setExecutionArgs(char *args)
//    passes on string of possible arguments that are then used in solving
//    e.g. --forget=50 forget 50% learnt clauses at each denial added externally
//
// 3.  Calling  Initialize(int* answerset_lits, int& num_atoms)
//    that allows Cmodels to perform all the necessery preprocessing on the
//    progarm including initiate SAT_manager. The memory for answerset_lits
//    should be allocated before calling Solve. The length of an array should be
//    the number of atoms in the grounded
//     program located in FILE* f passed to read.
//    (int getNumberGroundedAtoms() returns this number.)
//    Cmodels might establish that there is no or that there
//    is a unique answer set at the time of preproccessing.
//    After the execution of Initialize, if num_atoms is a positive integer then
//    the unique answer set was found by preprocessing step and it is returned
//    in answerset_lits array contains an array of integers where each integer
//    stands for the id (assigned by lparse or gringo) for every atom that
//    belongs to the answer set found by Cmodels. If no answer set is found then
//    num_atoms is assigned -1. If neither answer set if found nor it is
//    eastablished that there is a unique answer set then -2 is returned.

//
// 4. Calling   void Solve(int* answerset_lits, int& num_atoms, char **
// answerset_lits_names);
//   int* answerset_lits:
//     The memory for answerset_lits should be allocated before calling Solve.
//    The length of an array should be the number of atoms in the grounded
//     program located in FILE* f passed to read.
//    (int getNumberGroundedAtoms() returns this number.)
//    After the execution of Solve answerset_lits contains an array of integers
//    where each integer stands for
//    the id (assigned by lparse or gringo) for every atom that belongs to
//    the answer set found by Cmodels.
// char ** answerset_lits_names:
//    by default is NULL
//    if the memory to answerset_lits_names is allocated in the following
//    manner: The length of an array should be the number of atoms as in
//    answerset_lits array case
//        and each char* should be of at least length 1024 (deafult number of
//        characters for atom_name)
//    then the array will contain names of atoms that belong to found answer set
//    (i.e. names of atoms that correspond to ids in int* answerset_lits.
// int& num_atoms:
//    If no answer set is found then num_atoms is assigned -1.
//    If answer set if found then num_atoms contains the number of atoms in
//    returned array. If answer set solver were not able to establish anything
//    (UNKNOWN) then num_atoms =-2

// 5. Add denial (constraint, i.e., :-a,b, not c) by calling
//    addDenial (int* constaint_lits, int num_lits)
//    Constraint is represented by an array of integers.
//
//    For instance, for constraint :-a,b, not c, such that
//    Lparse or (Gringo) assigned ID's 1 to a, 2 to b, and 3 to c
//
//    Each literal in constraint, e.g., a..c is represented  by
//    2 * Lparse_id + Sign. The Sign is 0 for positive literals,
//    and 1 for literals under negation as failure.
//    So in the example above constraint :-a,b, not c (:- 1,2, not 3)
//    should be represneted by (2,4,7)
//    The assumption is that literals are given in sorted ascending order
//    addDenial returns false if it was established that the denial makes the
//    problem inconsistent; otherwise it returns true (i.e., nothing is
//    established and denial is added) Note: if a variable occures in both
//    positively and negatively, the constrainet is dropped.

// 6. Mark atoms that are constrained externally so that additional propagation
// rule is set up
//    (i.e. CSP constraints as far as EZCSP is concerned)
//    constained_atoms -- contains a list of atoms that external solver would
//    like to see values of trueExternal -- suggests which of the
//    constained_atoms are really external atoms e.g., trueExternal[1]== true
//    says that  constained_atoms[1] is an external atom e.g., trueExternal[2]==
//    false says that  constained_atoms[2] is an asp atom that external solver
//    wants to see a value of (no external propagators are associated with it
//    num_atoms is an int saying how many members are in constained_atoms (and
//    hence trueExternal) void markExternallyConstrainedAtoms (int*
//    constained_atoms, int num_atoms, bool* trueExternal); Should be performed
//    after Initialize

//    constained_atoms are represented  by an array of integers.

//    For instance, for atoms a b c
//    Lparse or (Gringo) assigned ID's 1 to a, 2 to b, and 3 to c
//    then constained_atoms will be an array [1,2,3] and num_atoms will be 3
//    The assumption is that atoms are given in sorted ascending order
//    This functionality is only available with Minisat1 -ms1 option so
//    Warning will be issued and solver flag will be changed to Minisat1
//    automatically

#ifndef CTABLE_H
#define CTABLE_H

#include "cmodels.h"
#include "read.h"
// #include "api.h"
#include "defines.h"
#include "solver/smtsolver.h"
#include "timer.h"

class Ctable {
public:
  Read &reader;
  Api &api;
  Cmodels &cmodels;

  Ctable(Cmodels &, Api &, Read &);
  virtual ~Ctable();

  void calculate();
  bool solved;

  // incremental (EZCSP) related
  int read();
  void numberExpected(char *str);
  void setSolver(SolverType st);
  void usage(void);
  void setExecutionArgs(char args[]);
  int setSingleExecutionArgument(char *arg, char *option);
  char *cmodels_version(void) { return (CMODELS_VERSION); }

  bool addDenial(int *constaint_lits, int num_lits);
  int getNumberGroundedAtoms();

  void Initialize(int *answerset_lits, int &num_atoms,
                  const char **&symbolTable, int &symbolTableEntries);
  void Initialize(int *answerset_lits, int &num_atoms);

  void Solve(int *answerset_lits, int &num_atoms);
  void print_lits(int *answerset_lits, int num_lits, bool denial);
  // end of incremental (EZCSP) related
};

#endif
