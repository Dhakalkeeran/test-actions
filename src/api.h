/*
 * File api.h
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
#ifndef API_H
#define API_H
#include "atomrule.h"
#include "defines.h"
#include "graphscc.h"
#include "timer.h"
#include "SimpSolver.h"
#include <iostream>
#include <map>
#include <string>
#include <unordered_set>

using namespace std;
class Program;
class Atom;
class Tree;
class BasicRule;
class Completion;
class WellFounded;
class Init;

struct listelement;
// typedef map<char*,Atom *> amap;
typedef pair<char *, Atom *> apair;
// Benjamin suggestion: He is right but in our case
// we verify duplicates before insertion and find works properly
// for the map with char*, i.e. no less operator is used on
// address but rather on the value of a string.
struct CmpCString {
  bool operator()(const char *lhs, const char *rhs) const {
    return strcmp(lhs, rhs) < 0;
  }
};

typedef map<char *, Atom *, CmpCString> amap;

class WeightRuleMemory {
public:
  WeightRuleMemory() {}
  ~WeightRuleMemory() {}
  void addAtom(Atom *a, char *key);

  Atom *findAtom(char *key);

  void clearAll();

  amap atomMap;
};

class Api {
public:
  Api(Program *);
  virtual ~Api();
  void begin_rule(RuleType);       // First call begin_rule
  void add_head(Atom *);           // Then call these
  void add_body(Atom *, bool pos); // Add atom to body of rule

  // FIXME Shouldn't this be the default behavior?
  void add_head_repetition(Atom *); // checks that head does not contain this
                                    // atom alread and then adds it, same below
  bool add_body_repetition(Atom *, bool pos,
                           RuleType type); // Add atom to body of rule

  void add_nnbody(Atom *); // Add atom to nnbody (double negation)of rule

  void
  add_Cbody(Atom *,
            bool pos); // Add atom to body of completion with auxilary atoms

  void
  add_body_LEGACY(Atom *, Weight,
                  bool pos); // Add atom with weight and positivness to body
  void add_body(Atom *, bool pos, Weight w); // Add atom with weight to body
  void pop_body(bool pos); // pop last added atom in nbody or pbody

  void change_body(long i, bool pos, Weight); // Change weight in body
  void set_atleast_weight(Weight);            // Weight rule

  void set_atleast_body(long); // Constraint rule
  void set_atleast_head(long); // Generate rule
  void end_rule();             // Finally, end with end_rule

  virtual Atom *new_atom(int = -1);             // Create new atom
  void set_compute(Atom *, bool, bool = false); // Compute statement
  void set_name(Atom *, const char *);
  void rule_reset();            // resets list pbody, head, nbody
  void rule_reset_repetition(); // resets list pbody, head, nbody
  void head_reset();            // resets list pbody, head, nbody
  void body_reset();            // resets list pbody, head, nbody
  void comp_reset();            // resets list compNbody compPbody
  // void checkRepretitions();

  Program *const program;
  bool headAtomInBody();

  long sizePbody();
  long sizeNbody();
  long sizeBody();
  long sizeNNbody(); // for the body with double negation
  long sizeHead();

  long sizeCompNbody();
  long sizeCompPbody();

  Atom *headAtom(long pos);
  Atom *pbodyAtom(long pos);
  Atom *nbodyAtom(long pos);
  Atom *nnbodyAtom(long pos);
  Atom *compPbodyAtom(long pos);
  Atom *compNbodyAtom(long pos);

  class listApi {
  public:
    listApi();
    ~listApi();
    void push(Atom *a, Weight w = 0);
    void print();
    void pop();
    void push(Atom *a, Weight w, bool pos);
    void reset();
    void reset_repetition();
    void reset2();
    void grow();
    void grow(long sz);

    bool find(Atom *a);
    Atom **atoms;
    Weight *weights;
    bool *np; // is true if in pbody false if in nbody
              // used for weights atoms
    int top;
    int size;
  };

  listApi pbody;
  listApi nbody;
  listApi nnbody; // body with double negation
  listApi head;

  //  listApi temp; //for sorting
  list<Rule *> tempRules;             // for sorting
  list<NestedRule *> tempNestedRules; // for sorting
  listApi compPbody;                  // completion bodies
  listApi compNbody;

public:
  listApi getPbody() { return pbody; };
  listApi getNbody() { return nbody; };
  long getPbodyWeights(int pos) { return pbody.weights[pos]; };
  long getNbodyWeights(int pos) { return nbody.weights[pos]; };

  Weight totalweight(listApi &l);
  WeightRuleMemory wrmem;

  RuleType type;
  // p/nbody positive neg body, and head are of this types

  char mchaff_time[256];
  long size(listApi &);
  void sort(listApi &);
  void inplace_merge(listApi &l, int first, int middle, int last);
  void copyListElement(listApi &l1, listApi &l2, int p1, int p2);
  void sortRuleList(vector<Rule *> &rules, long numRules);
  void sortBodyRuleList(vector<NestedRule *> &rules, long numRules);
  void inplace_mergeBodyRuleList(vector<NestedRule *> &rules, long first,
                                 long middle, long last);
  void inplace_mergeRuleList(vector<Rule *> &rules, long first, long middle,
                             long last);

  void traverse2cmp(vector<Rule *> &rules, Rule *&rcmp, long from, long to);

  CompareValues cmpRules(Rule *r1, Rule *r2);
  Weight atleast_weight;
  long atleast_body;
  long atleast_head;
  bool maximize;
  Init *init;

  void set_init();
};

#endif
