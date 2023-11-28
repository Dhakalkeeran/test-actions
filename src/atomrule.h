/*
 * File atomrule.h
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
#ifndef ATOMRULE_H
#define ATOMRULE_H
#include "defines.h"
#include <assert.h>
#include <iostream>
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <unordered_set>
#include <vector>

class Api;
class Rule;
class DisjunctionRule;
class NestedRule;
class FunctionalityRule;
class Atom;
class Program;
using namespace std;

struct Auxiliary {
  Auxiliary(bool p = false) {
    positive = p;
    in_loop = p;
  };
  Weight weight;     // Local weight
  bool positive : 1; // Is the literal an atom?
  bool in_loop : 1;  // Is the atom in a strongly connected component?
  Atom **a;          // Position in rule body
};
struct Init {
  long hsize;
  long psize;
  long nsize;
  Atom **head;
  Atom **pbody;
  Atom **nbody;
  Weight *pweight;
  Weight *nweight;
  Weight atleast_weight;
  long atleast_body;
  long atleast_head;
  bool maximize;
};

class Atom {
public:
  Atom(Program *p);
  ~Atom();

  static bool conflict;
  static bool change;

  const char *atom_name();
  inline int get_lparse_id() { return original_id; }
  inline void set_lparse_id(int id) { original_id = id; }
  void checkDoubleCycle();
  void printsmt(FILE *file);
  void setBTrue();
  void setComputeTrue();
  void setComputeTrue0();
  void setComputeFalse();
  void setBFalse();
  void setConsTrue();

  bool allAtomsBpos();
  void propagateInBodies(bool value); // go thru list of all rules
                                      // and sets Counters
  void propagateConsInBodies();       // go thru list of bPosRules
                                      // and sets posCounter
  void propagateComputeTrueInNegBodies();

  bool found();
  static void setChangeFalse() { Atom::change = false; }
  static void setChangeTrue() { Atom::change = true; }
  static void setConflictTrue() { Atom::conflict = true; }

  bool isLevelRankingConstraint();
  string getSmtName();

  // (allocated in head)
  bool Bpos : 1;         // True if the atom is in B+
  bool Bneg : 1;         // True if the atom is in B- or computeFalse
  bool computeTrue : 1;  // True if the atom is in computeTrue
  bool computeTrue0 : 1; // True if the atom is in computeTrue
  bool computeFalse : 1;

  bool inUpper; // used in computing upperclosure for WFM

  //  long headActive;  // The number of rules in the head array whose
  // inactive counter is zero

  long headof; // The number of rules in the head array whose
               // inactive counter is zero

  long
      headofDR; // The number of rules in which atom appears in disjunctive head

  void print();
  void printClean(FILE * = stdout);
  void printRules();
  void printNestedRules();
  void printBCircuit(FILE *file);
  void printCompletionBCircuit(FILE *file, char *gatename);

  // TODO Remove. This is mixing state across several layers of the app.
  list<NestedRule *> nestedRules; // list which contains pointers to the rules
                                  // in which this atom is in the head
                                  // within this list we push disjunctive rules
                                  // upfront while nondisjunctive rules
                                  // go to the end of the list

  // TODO Remove. This is mixing state across several layers of the app.
  // for which this atom is a head
  // made for making completion faster
  list<NestedRule *> pBodyRules; // list which contains pointers to the rules
                                 // in which this atom is in the B+

  // TODO Remove. This is mixing state across several layers of the app.
  list<Rule *> posBodyRules; // list which contains pointers to the (Nonnested)
                             // rules in which this atom is in the Body+

  // TODO Remove. This is mixing state across several layers of the app.
  list<Rule *> negBodyRules; // list which contains pointers to the (Nonested)
                             // rules in which this atom is in the Body-

  // TODO Remove. This is mixing state across several layers of the app.
  vector<Rule *> headRules; // list which contains pointers to the
                            // Disjuntionrules
                            // in which this atom is in the Head

  // takes care of headOf counters + adds the rule to a list
  void addToRuleList(NestedRule *rule);
  void addPBodyList(NestedRule *rule) { pBodyRules.push_front(rule); };
  int id; // identifier for an atom
  // the field below is needed for proper implementation of
  // void Ctable::addDenial (int* constaint_lits, int num_lits);
  // void Ctable::Solve(int* answerset_lits, int& num_atoms)
  // which will refer to this original id rather than to the new id after
  // modification
  int original_id; // identifier for an atom that was originally assigned
  // to this atom by gringo or lparse

  char *name; // The name of the atom

  // FIXME Remove. This is a circular dependency
  Program *const p; // The program in which the atom appears

  bool choiceruleSpecified; // when we create graph for checcking tightness
  // graph might become simpler and als some nontughtness may be eliminated
  //  (replacing {p}. p occurences by not not p everywhere
  bool theoryStatement = false; // indicates if this is a theory statement
  bool inM;
  bool inMminus;
  int inLoopId;
  bool cons;
  // bool scopeNegAsFail;

  int inLoop;

  InClause inClause; // used while building clause thru multiplication
                     // to avoid duplications of atoms in clause
                     // 0 - not in clause NOTIN
                     // 1 - positively in clause POS
                     // 2 - neg in clause  NEG
  int ruleId;        // used while building clause thru multiplication
                     // -1 undef

  // when global SCC are calculated for modular unfounded free check
  bool act;
  list<Atom *> exp;
  int root;
  InRule inRule; // while reading we control that atom does not repeat in rule

  bool external; // true if externally defined (EZCSP type of use), false
                 // default

  bool showInOutputAnswerSet = false;
};
class FunctionalityRule {
public:
  RuleType type;
  bool erase;
  CompareValues equal(Atom **l1, Atom **l1end, Atom **l2, Atom **l2end);
  CompareValues bodyEq(Atom **nbody1, Atom **pbody1, Atom **nnbody1,
                       Atom **end1, Atom **nbody2, Atom **pbody2,
                       Atom **nnbody2, Atom **end2);
};

class NestedRule : public FunctionalityRule {
public:
  NestedRule();
  virtual ~NestedRule();
  void print();
  long sizeHead();
  long sizePbody();
  long sizeNbody();
  long sizeNNbody();
  long sizeBody();

  bool support(Atom *h);
  Atom **getHead();
  Atom **getPbody();
  Atom **getNbody();
  Atom **getNNbody();

  void addHead(long pos, Atom *atom);
  void addNbody(long pos, Atom *atom);
  void addPbody(long pos, Atom *atom);
  void addNNbody(long pos, Atom *atom);
  void addBody(long pos, Atom *atom);
  void setType(RuleType ruleType);
  RuleType getType();
  void allocateHead(long size);
  void allocateRule(long sizeH, long sizeN, long sizeP, long = 0);
  void initRuleFromApi(Api *api, RuleType ruletype);
  bool bodySatisfied();

  void finishRule(); // everytime rule is created we verify that
                     // all its atoms are initialized
  void printBCircuit(FILE *file, Atom *a);
  bool erased; // used in building reduct, rule is erased whenever it is not
               // satisfied by the model

  Atom **head;   // The heads of the rule
  Atom **hend;   // Sentinel of the head array
  Atom **nbody;  // The negative literals in the body (allocated
                 // in head)
  Atom **pbody;  // The positive literals in the body (allocated
                 // in head)
  Atom **nnbody; // The double negated literals in the body
                 // Alloctaed in head)
  Atom **nend;   // Sentinel of the nbody array
  Atom **pend;   // Sentinel of the pbody array
  Atom **nnend;  // Sentinel of the nnbody array

  Atom **end; // The end of the head-body array

  long numHead;   // number of literals in head
  long numPbody;  // number of literals in posbody
  long numNbody;  // number of literals in neg body
  long numNNbody; // number of literals in neg neg body

  int pbodyCount;

  Atom *reprComp;  // atom which represents body of the rule in the completion
  Atom *reprComp2; // atom which represents the conjunction in level ranking
                   // formula
  InClause signReprComp; // is repr atom
                         //  not assgined =0 NOT_DEF
                         //  positive =1  POS
                         //  or negative =2 NEG
  bool isRR;             // true if the rule is recursive rule.

  bool markedSCC; // true if this rule was already once explored in
                  // buildProgramSCC

  bool bodyACl;            // true if Body implies A clause already exists
  int bodyAClVerification; //>0 if Body implies A clause already
  // exists in verification file

  bool addedInLoop; // when we find rules relevant to loop formulas
                    // for disjunctive rules we store SCC ids
                    // so that for each SCC we add this rule only once
  CompareValues cmpBody(NestedRule *rcmp);
};
class Rule : public FunctionalityRule {
public:
  Rule() {
    type = ENDRULE;
    ruleId = 0;
  };
  virtual void propUpper(Atom *a = 0) = 0;
  virtual void initUpper() = 0;

  virtual CompareValues cmpRule(Rule *rcmp) = 0;
  CompareValues subsumes(Atom **l1, Atom **l1end, Atom **l2, Atom **l2end);

  CompareValues subsumesGr(Atom **sup, Atom **supend, Atom **sub,
                           Atom **subend);
  CompareValues bodySubsumed(Atom **nbody1, Atom **pbody1, Atom **nnbody1,
                             Atom **end1, Atom **nbody2, Atom **pbody2,
                             Atom **nnbody2, Atom **end2);
  bool atomInList(Atom *a, Atom **la,
                  Atom **laend); // traverses list and returns true
                                 //  if an atom is in the list
  //  virtual ~Rule () {};

  virtual void print(){};
  // virtual void print_internal () {};

  long ruleId;
  void setRuleId(long id) { ruleId = id; };
  long getRuleId() { return ruleId; };

  Result satUUn;
  // for WFM
  int posBodyCount;
  int upper;
  int negBodyCount;
  virtual void propagateHeadFalse() = 0;
  //  virtual bool posbody() = 0;
  virtual Atom **posbody() = 0;
  virtual Atom **posendbody() = 0;

  virtual bool allAtomsBpos() = 0;

  virtual void HeadAtom(bool value, Atom *at) = 0;
  virtual void HeadInOneRule(Atom *at) = 0;

  virtual void BodyAtom(bool posBody, bool posAtom, Atom *at) = 0;
  virtual bool pt() = 0;

  virtual Result satUnsatUnknown() = 0;
  virtual void init(Init *, int pos = -1) = 0;
  virtual void initRuleLists4WF() = 0;
  virtual void initHeadRuleLists4Sort() = 0;
};

class HeadRule : public Rule {
public:
  HeadRule() { head = 0; };
  virtual ~HeadRule(){};
  CompareValues cmpHead(HeadRule *r);
  Atom *head; // The head of the rule
};

class BasicRule : public HeadRule {
public:
  BasicRule();
  virtual ~BasicRule();
  void propagateHeadFalse();
  void propUpper(Atom *a = 0);
  void initUpper();

  CompareValues cmpRule(Rule *r);
  Atom **posbody();
  Atom **posendbody();

  bool allAtomsBpos();

  void propagateHeadTrue();
  void BodyAtom(bool posBody, bool posAtom, Atom *at);
  void HeadAtom(bool posAtom, Atom *at);
  void HeadInOneRule(Atom *at);
  bool pt();
  Result satUnsatUnknown();
  //  Result satUUn;

  void init(Init *, int pos = -1);
  void initRuleLists4WF();
  void initHeadRuleLists4Sort();
  void print();

  Atom **nbody; // The negative literals in the body
  Atom **pbody; // The positive literals in the body (allocated
                // in nbody)
  Atom **nend;  // Sentinel of the nbody array
  Atom **pend;  // Sentinel of the pbody array
  Atom **end;   // The end of the body array
};

class ConstraintRule : public HeadRule {
public:
  ConstraintRule();
  virtual ~ConstraintRule();
  CompareValues cmpRule(Rule *r);
  void propagateHeadFalse();
  Atom **posbody();
  Atom **posendbody();
  void propUpper(Atom *a = 0);
  void initUpper();

  bool allAtomsBpos();

  void BodyAtom(bool posBody, bool posAtom, Atom *at);
  void HeadAtom(bool posAtom, Atom *at);
  void HeadInOneRule(Atom *at);
  bool pt();

  Result satUnsatUnknown();
  //  Result satUUn;

  void init(Init *, int pos = -1);
  void initRuleLists4WF();
  void initHeadRuleLists4Sort();
  void print();

  Atom **nbody; // The negative literals in the body
  Atom **pbody; // The positive literals in the body (allocated
                // in nbody)
  Atom **nend;  // Sentinel of the nbody array
  Atom **pend;  // Sentinel of the pbody array
  Atom **end;   // The end of the body array
  long atleast; // If atleast literals are in B then fire

  long atleastCount;
  long lower;
};

class ChoiceRule : public HeadRule {
public:
  ChoiceRule();
  virtual ~ChoiceRule();
  void propagateHeadFalse();
  CompareValues cmpRule(Rule *r);
  Atom **posbody();
  Atom **posendbody();
  void propUpper(Atom *a = 0);
  void initUpper();

  bool allAtomsBpos();

  void BodyAtom(bool posBody, bool posAtom, Atom *at);
  void HeadAtom(bool posAtom, Atom *at);
  void HeadInOneRule(Atom *at);
  bool pt();

  Result satUnsatUnknown();
  //  Result satUUn;

  void init(Init *, int pos = -1);
  void initRuleLists4WF();
  void initHeadRuleLists4Sort();
  void print();

  //  Atom **head;       // The heads of the rule
  Atom **hend;  // Sentinel of the head array
  Atom **nbody; // The negative literals in the body (allocated
                // in head)
  Atom **pbody; // The positive literals in the body (allocated
                // in head)
  Atom **nend;  // Sentinel of the nbody array
  Atom **pend;  // Sentinel of the pbody array
  Atom **end;   // The end of the body array
};

class DisjunctionRule : public Rule {
public:
  DisjunctionRule();
  virtual ~DisjunctionRule();
  CompareValues cmpRule(Rule *r);
  void propagateHeadFalse();
  Atom **posbody();
  Atom **posendbody();
  bool allAtomsBpos();
  void propUpper(Atom *a = 0);
  void initUpper();

  void BodyAtom(bool posBody, bool posAtom, Atom *at);
  bool pt();
  void HeadAtom(bool value, Atom *at);
  void HeadInOneRule(Atom *at);
  Result satUnsatUnknown(); // gives the value of the body
                            // whether it  is SAT Unsat or Unknown
  //  Result satUUn;  //stores value if it is SAT or UNknown
  // when rule is UNSAT it is then deleted

  void init(Init *, int pos = -1);
  void initRuleLists4WF();
  void initHeadRuleLists4Sort();
  void print();

  Atom **head;  // Start  of the head array
  Atom **hend;  // Sentinel of the head array
  Atom **nbody; // The negative literals in the body (allocated
                // in head)
  Atom **pbody; // The positive literals in the body (allocated
                // in head)
  Atom **nend;  // Sentinel of the nbody array
  Atom **pend;  // Sentinel of the pbody array
  Atom **end;   // The end of the body array

  // for WFM
  int negHeadCount;
};

//
// WEIGHT RULE
//
class WeightRule : public HeadRule {
public:
  WeightRule();
  virtual ~WeightRule();
  Weight findAtWeightBody(Atom *at, bool posBody);
  CompareValues cmpRule(Rule *r);
  void propagateHeadFalse();
  Atom **posbody();
  Atom **posendbody();
  bool allAtomsBpos();
  void propUpper(Atom *a = 0);
  void initUpper();

  void BodyAtom(bool posBody, bool posAtom, Atom *at);
  void HeadAtom(bool posAtom, Atom *at);
  void HeadInOneRule(Atom *at);
  bool pt();
  CompareValues bodyEqWr(WeightRule *wr);

  Result satUnsatUnknown();
  // Result satUUn;

  void init(Init *, int pos = -1);
  void initRuleLists4WF();
  void initHeadRuleLists4Sort();
  void print();

  Atom **bend;    // Sentinel of the body array
  Atom **body;    // The literals in the body
  Atom **end;     // The end of the array
  Auxiliary *aux; // Holds among other things the local weights of
                  // the atoms
  Weight maxweight;

  Weight upper_c; // Used in upper closure
  Weight lower;   // Used in upper closure

  Weight atleast;
  Atom **max; // Position of the literal of largest absolute
  Atom **min; // value of weight not in B, used in backchain
  Atom **max_shadow;
  Atom **min_shadow;

  // for WFM
  Weight atleastCount;
  Weight maxweightCount;
};

class Completion {
public:
  Completion();
  ~Completion();
  void allocateNbody(long size);
  void initCompletionNbodyFromApi(Api *api);
  void initCompletionNbodyFromCompApi(Api *api);
  void print();
  void printBCircuit(FILE *file, char *gateName);

  Atom *head;          // The head of the rule
  Atom **headDisj;     // The head of the rule if it is Disj
  Atom **endheadDisj;  // End of The head of the rule if it is Disj
  Connector connector; // is OR when v is a connective
                       //       AND when & is a connective

  Atom **nbody; // The negative literals in the body
  Atom **pbody; // The positive literals in the body (allocated
                // in nbody)
  Atom **nend;  // Sentinel of the nbody array
  Atom **pend;  // Sentinel of the pbody array
  Atom **end;   // The end of the body array

  Connector eq; // is EQUIV if this is a comletion of the from atom == expr
                //    IMPL if completion of the form atom => expr

  long numNbody;
  long numPbody;
};

class Clause {
public:
  void print(bool useAtomNames = false);
  void printcnf(FILE *file);
  void printsmtcnf(FILE *file);
  void printBCircuit(FILE *file, char *gateName);

  void translateToZchaffClause(int *clause, int &size);
  void translateToMinisatClause(int *clause, int &size);
  void translateToSimoReason(int *reason, int &reasonSize, const int &sizeCl);
  void allocateClause(long negBody, long posBody);
  void allocate(long size);
  void initClauseFromApi(Api *api);

  Clause();
  ~Clause();
  void addNbody(long pos, Atom *atom);
  void addPbody(long pos, Atom *atom);
  void addBody(long pos, Atom *atom);
  void finishClause();
  Atom **getNbody();
  Atom **getPbody();
  long sizeNbody();
  long sizePbody();
  long sizeCl();

  // void sat_print ();

  Atom **nbody; // The negative literals in the body
  Atom **pbody; // The positive literals in the body (allocated
                // in nbody)
  Atom **nend;  // Sentinel of the nbody array
  Atom **pend;  // Sentinel of the pbody array
  Atom **end;   // The end of the body array

  long numNbody;
  long numPbody;
};

#endif
