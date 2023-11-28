/*
 * File atomrule.cc
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
#include <fstream>
#include <iostream>
//#include <regex>
#include <limits.h>
#include <regex>
#include <sstream>
#include <string>
#ifdef USEDOUBLE
#include <math.h>
#endif
#include "atomrule.h"
#include "defines.h"
#include "program.h"
#include "api.h"

bool Atom::change = false;
bool Atom::conflict = false;

static Auxiliary pos_aux(true);
Atom::Atom(Program *p0) : p(p0) {
  //  change=false;
  // conflict=false;
  //  headActive=0;
  // scopeNegAsFail=false;
  inLoop = -1;
  inRule = UNDEF;
  choiceruleSpecified = false;
  id = p->number_of_atoms;
  headof = 0;
  headofDR = 0;
  name = 0;
  Bpos = false;
  Bneg = false;
  computeTrue = false;
  computeTrue0 = false;
  computeFalse = false;
  inClause = NOT_DEF;
  ruleId = -1;

  inM = false;
  inMminus = false;
  inLoopId = 0;
  act = false;
  root = 0;
  cons = false;
  external = false;
}
Atom::~Atom() { delete[] name; }
void Atom::setComputeTrue() {
  if (computeTrue || Bpos)
    return;
  computeTrue = true;
  if (Bneg || computeFalse) {
    setConflictTrue();
    return;
  }
  setChangeTrue();
  propagateComputeTrueInNegBodies();
}
void Atom::setComputeTrue0() {
  computeTrue0 = true;
  if (Bneg || computeFalse) {

    setConflictTrue();
    return;
  }
}

void Atom::setComputeFalse() {
  if (computeFalse)
    return;
  computeFalse = true;
  if (Bpos || computeTrue || computeTrue0) {

    setConflictTrue();
    return;
  }
}

void Atom::setBTrue() {
  if (Bpos)
    return;
  Bpos = true;
  if (Bneg || computeFalse) {

    setConflictTrue();
    return;
  }
  setChangeTrue();
  propagateInBodies(true);
}
void Atom::setConsTrue() {
  if (cons)
    return;
  cons = true;
  setChangeTrue();
  propagateConsInBodies();
}

void Atom::setBFalse() {
  //  print();
  // printRules();

  if (Bneg)
    return;

  Bneg = true;
  if (Bpos || computeTrue || computeTrue0) {

    setConflictTrue();
    return;
  }

  setChangeTrue();
  propagateInBodies(false);
}

void Atom::printRules() {
  cout << "Nested\n";
  for (list<NestedRule *>::iterator itrR = nestedRules.begin();
       itrR != nestedRules.end(); itrR++) {
    (*itrR)->print();
  }

  cout << "POS\n";
  for (list<Rule *>::iterator itrR = posBodyRules.begin();
       itrR != posBodyRules.end(); itrR++) {
    (*itrR)->print();
  }
  cout << "NEG\n";
  for (list<Rule *>::iterator itrR = negBodyRules.begin();
       itrR != negBodyRules.end(); itrR++) {
    (*itrR)->print();
  }
  cout << "HEAD\n";
  for (vector<Rule *>::iterator itrR = headRules.begin();
       itrR != headRules.end(); itrR++) {
    (*itrR)->print();
  }
  cout << "\n";
}
void Atom::printNestedRules() {
  cout << "headOf\n";
  for (list<NestedRule *>::iterator itrR = nestedRules.begin();
       itrR != nestedRules.end(); itrR++) {
    (*itrR)->print();
  }
  cout << "\n";
}

// TODO delete
// This is no longer there only for debugging purporses
// So do not change: I use it to output to casp-dimacs form
void Atom::printClean(FILE *file) {
  if (strcmp("#noname#", atom_name()))
    fprintf(file, "%s", atom_name());
  else
    fprintf(file, "%d", id);
}

bool Atom::isLevelRankingConstraint() {
  return name && string(name).find(LEVEL_RANKING_ATOM_PREFIX) != string::npos;
}

string Atom::getSmtName() {
  if (!name) {
    return to_string(id);
  }

  // Encode integer level ranking constraints in atom name
  // so that they will be output in SMT assertions
  string smtName = name;
  if (isLevelRankingConstraint()) {
    return smtName.substr(LEVEL_RANKING_ATOM_PREFIX.length());
  }

  return smtName;
}

void Atom::print() {
  if (strcmp("#noname#", atom_name()))
    cout << atom_name() << ":" << id << ":" << original_id;
  else
    cout << id << ":" << original_id;
  return;
  if (inRule != -1)
    cout << ":" << inRule;

  if (inLoop != -1)
    cout << ":" << inLoop;
  if (Bpos)
    cout << ":Bpos";
  if (computeTrue)
    cout << ":CT ";
  if (computeTrue0)
    cout << ":CT0 ";
  if (Bneg)
    cout << ":Bneg ";
  cout << ":hof " << headof;
}

void Atom::printBCircuit(FILE *file) {
  if (strcmp("#noname#", atom_name()))
    fprintf(file, "atom_%d", id);
  else
    fprintf(file, "_%d", id);
}
void Atom::printCompletionBCircuit(FILE *file, char *gatename) {
  if (Bpos) {
    printBCircuit(file);
    fprintf(file, "; \n ASSIGN ");
    printBCircuit(file);
    fprintf(file, "; \n");
    return;
  }
  if (choiceruleSpecified) {
    printBCircuit(file);
    fprintf(file, ";\n");
    return;
  }
  // Bneg and no supporting rules -- we simply through it out
  // it is not false_atom
  if (Bneg && nestedRules.size() == 0) {
    return;
  }

  // head is not in any cycle in POSNEG dependency graph
  if (inLoop == -1) {
    printBCircuit(file);
    fprintf(file, ":= ");
  } else {
    fprintf(file, "%s := ", gatename);
    printBCircuit(file);
    fprintf(file, " == ");
  }
  if (nestedRules.size() == 0) {
    fprintf(file, "F;\n");
    if (inLoop != -1) { // if in some loop
      fprintf(file, "ASSIGN %s;\n", gatename);
    }
    return;
  }
  if (nestedRules.size() > 1) {
    fprintf(file, "OR(");
  }
  bool comma = false;

  for (list<NestedRule *>::iterator itrNRule = nestedRules.begin();
       itrNRule != nestedRules.end(); itrNRule++) {
    if (comma) {
      fprintf(file, ",");
    }
    comma = true;
    (*itrNRule)->printBCircuit(file, this);
  }
  if (nestedRules.size() > 1) {
    fprintf(file, ");\n");
  } else
    fprintf(file, ";\n");
  // if the atom is in some cycle
  // we assign gatename
  if (inLoop != -1) {
    fprintf(file, "ASSIGN %s;\n", gatename);
  }
  if (computeTrue) {
    fprintf(file, "ASSIGN ");
    printBCircuit(file);
    fprintf(file, ";\n ");
  }
  if (Bneg) {
    fprintf(file, "ASSIGN ~");
    printBCircuit(file);
    fprintf(file, ";\n ");
  }
}
bool Atom::found() {
  for (list<NestedRule *>::iterator itrNRule = pBodyRules.begin();
       itrNRule != pBodyRules.end(); itrNRule++) {
    if ((*itrNRule)->support(this))
      return true;
  }
  return false;
}

void Atom::addToRuleList(NestedRule *rule) {
  assert(rule);
  headof++;
  if (rule->getType() == DISJUNCTIONRULE) {
    headofDR++;
    nestedRules.push_front(rule);
  } else
    nestedRules.push_back(rule);
}

// verifyies cycles of the type:
// a,b occur only in the head of a single rule
// s.t.
// ..a..:-..b..
// ..b..:-..a..
void Atom::checkDoubleCycle() { // go thru list of all positive atoms
  // in the single rule and check is some of the atoms also has
  // only one rule depending on the current atom
  int i = 0;
  int iB = 0;

  for (vector<Rule *>::iterator itrR = headRules.begin();
       itrR != headRules.end(); itrR++) {
    if ((*itrR)->satUUn != UNSAT) {
      i++;
      assert(i <= 1);
      Atom **pbody;
      Atom **pend;
      Atom **pbodyB;
      Atom **pendB;
      pbody = (*itrR)->posbody();
      if (!pbody)
        return; // weight or constraint rule then we do nothing
      pend = (*itrR)->posendbody();

      for (Atom **a = pbody; a < pend; a++) {

        if ((*a)->headof == 1) {
          iB = 0;

          for (vector<Rule *>::iterator itrRB = (*a)->headRules.begin();
               itrRB != (*a)->headRules.end(); itrRB++) {
            if ((*itrRB)->satUUn != UNSAT) {
              iB++;
              assert(iB <= 1);
              pbodyB = (*itrRB)->posbody();
              if (pbodyB) {
                pendB = (*itrRB)->posendbody();
                for (Atom **aB = pbodyB; aB < pendB; aB++) {
                  if ((*aB) == this) {
                    setBFalse();
                    return;
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}
void Atom::propagateInBodies(bool value) { // go thru list of all rules
  // and sets Counters

  if (!this->computeTrue) { // if this is a computeTrue then we
                            // have already substracted these values
    for (list<Rule *>::iterator itrR = posBodyRules.begin();
         itrR != posBodyRules.end(); itrR++) {
      (*itrR)->BodyAtom(true, value, this);
    }

    // in negative bodies we propogate even cons
    for (list<Rule *>::iterator itrR = negBodyRules.begin();
         itrR != negBodyRules.end(); itrR++) {
      (*itrR)->BodyAtom(false, value, this);
      //	   this->print();
    }
  }

  for (vector<Rule *>::iterator itrR = headRules.begin();
       itrR != headRules.end(); itrR++) {

    if ((*itrR)->type == DISJUNCTIONRULE)
      (*itrR)->HeadAtom(value, this);
    else if (!value)
      (*itrR)->propagateHeadFalse();
  }
}
void Atom::propagateComputeTrueInNegBodies() { // go thru list of all rules
  // and sets Counters
  for (list<Rule *>::iterator itrR = negBodyRules.begin();
       itrR != negBodyRules.end(); itrR++) {
    (*itrR)->BodyAtom(false, true, this);
  }
  for (list<Rule *>::iterator itrR = posBodyRules.begin();
       itrR != posBodyRules.end(); itrR++) {
    (*itrR)->BodyAtom(true, true, this);
  }
  if (headof == 1) { // just one rule where the atoms that is computeTrue
    // is in the head, then we go through all of its
    // positive atoms and set them to compute true, and negative to neg
    int i = 0;
    for (vector<Rule *>::iterator itrR = headRules.begin();
         itrR != headRules.end(); itrR++) {

      if ((*itrR)->satUUn != UNSAT) {
        i++;
        assert(i == 1);
        (*itrR)->HeadInOneRule(this);
      }
    }
  }
}

CompareValues HeadRule::cmpHead(HeadRule *r) {
  if (head->id > r->head->id)
    return GTH;
  if (head->id < r->head->id)
    return LTH;
  return EQ;
}

// retruns SUBSET if second set is SUBSET
// retruns SUPERSET if second set is SUPERSET
// retruns EQ if sets are the same
// otherwise defines order
CompareValues Rule::subsumes(Atom **l1, Atom **l1end, Atom **l2, Atom **l2end) {
  if (l1end - l1 >= l2end - l2) {
    return subsumesGr(l1, l1end, l2, l2end);
  }
  CompareValues ret = subsumesGr(l2, l2end, l1, l1end);
  if (ret == LTH)
    return GTH;
  if (ret == GTH)
    return LTH;
  if (ret == EQ)
    return EQ;
  if (ret == SUBSET)
    return SUPERSET;
  else
    return SUBSET;
}

CompareValues Rule::subsumesGr(Atom **sup, Atom **supend, Atom **sub,
                               Atom **subend) {
  assert(supend - sup >=
         subend -
             sub); // size of super set is always greater than hypothetic subset
  Atom **a_sup = sup;
  for (Atom **a_sub = sub; a_sub < subend; a_sub++) {
    while (supend - a_sup >=
               subend - a_sub // super array is still larger than subarray
           && (*a_sub)->id > (*a_sup)->id) {
      a_sup++;
    }
    if (supend - a_sup >= subend - a_sub && (*a_sub)->id == (*a_sup)->id)
      a_sup++;
    else { // there may not be subsumption either it will not be enough atoms

      return equal(sup, supend, sub, subend);
    }
  }
  if (supend - sup == subend - sub)
    return EQ;
  else
    return SUBSET;
}

CompareValues FunctionalityRule::equal(Atom **sup, Atom **supend, Atom **sub,
                                       Atom **subend) {
  if (supend - sup > subend - sub)
    return GTH;
  else if (supend - sup < subend - sub)
    return LTH;
  if (supend - sup == 0)
    return EQ;
  Atom **a_sub = sub;
  Atom **a_sup = sup;
  for (; a_sub < subend; a_sub++, a_sup++) {
    if ((*a_sub)->id > (*a_sup)->id)
      return GTH;
    if ((*a_sub)->id < (*a_sup)->id)
      return LTH;
  }
  return EQ;
}
CompareValues Rule::bodySubsumed(Atom **nbody1, Atom **pbody1, Atom **nnbody1,
                                 Atom **end1, Atom **nbody2, Atom **pbody2,
                                 Atom **nnbody2, Atom **end2) {
  assert(nnbody1 == end1 && nnbody2 == end2);
  CompareValues nsub = subsumes(nbody1, pbody1, nbody2, pbody2);
  if (nsub == LTH || nsub == GTH)
    return nsub;
  CompareValues psub = subsumes(pbody1, end1, pbody2, end2);
  if (psub == LTH || psub == GTH)
    return psub;
  if (nsub == EQ)
    return psub;
  if (psub == EQ)
    return nsub;
  if (nsub == psub)
    return nsub;
  if (nsub == SUBSET)
    return GTH;
  else
    return LTH;
}
CompareValues FunctionalityRule::bodyEq(Atom **nbody1, Atom **pbody1,
                                        Atom **nnbody1, Atom **end1,
                                        Atom **nbody2, Atom **pbody2,
                                        Atom **nnbody2, Atom **end2) {
  CompareValues nsub = equal(nbody1, pbody1, nbody2, pbody2);
  if (nsub == LTH || nsub == GTH) {
    return nsub;
  }
  nsub = equal(pbody1, nnbody1, pbody2, nnbody2);
  if (nsub == LTH || nsub == GTH) {
    return nsub;
  }
  if (!(end1 - nnbody1 == 0 && end2 - nnbody2 == 0)) {

    return equal(nnbody1, end1, nnbody2, end2);
  }
  return EQ;
}

bool Rule::atomInList(Atom *a, Atom **la, Atom **laend) {
  for (la; la < laend; la++) {
    if ((*la) == a)
      return true;
  }
  return false;
}
CompareValues BasicRule::cmpRule(Rule *rcmp) {
  if (rcmp->type == CONSTRAINTRULE || rcmp->type == WEIGHTRULE)
    return LTH;
  if (rcmp->type == BASICRULE) { // as this rule
    BasicRule *r2cmp = (BasicRule *)rcmp;
    CompareValues cmp = cmpHead(r2cmp);
    if (cmp != EQ)
      return cmp;
    return bodySubsumed(nbody, pbody, end, end, r2cmp->nbody, r2cmp->pbody,
                        r2cmp->end, r2cmp->end);
  }
  if (rcmp->type == CHOICERULE) { // as this rule
    ChoiceRule *r2cmp = (ChoiceRule *)rcmp;
    if (r2cmp->head == head) {
      CompareValues cmp = bodySubsumed(nbody, pbody, end, end, r2cmp->nbody,
                                       r2cmp->pbody, r2cmp->end, r2cmp->end);
      if (cmp == EQ || cmp == SUPERSET)
        return SUPERSET;
    }
    return LTH;
  }
  if (rcmp->type ==
      DISJUNCTIONRULE) { // basic_rule is smaller than any other rule
    DisjunctionRule *r2cmp = (DisjunctionRule *)rcmp;
    if (atomInList(head, r2cmp->head, r2cmp->hend)) {
      CompareValues cmp = bodySubsumed(nbody, pbody, end, end, r2cmp->nbody,
                                       r2cmp->pbody, r2cmp->end, r2cmp->end);
      if (cmp == EQ || cmp == SUPERSET)
        return SUPERSET;
    }
    return LTH;
  }
}
CompareValues ChoiceRule::cmpRule(Rule *rcmp) {
  if (rcmp->type == CONSTRAINTRULE || rcmp->type == WEIGHTRULE ||
      rcmp->type == DISJUNCTIONRULE)
    return LTH;
  if (rcmp->type == BASICRULE) { // as this rule
    BasicRule *r2cmp = (BasicRule *)rcmp;
    if (r2cmp->cmpRule(this) == SUPERSET)
      return SUBSET;
    else
      return GTH;
  }
  ChoiceRule *r2cmp = (ChoiceRule *)rcmp;
  CompareValues cmp = cmpHead(r2cmp);
  if (cmp != EQ)
    return cmp;

  return bodySubsumed(nbody, pbody, end, end, r2cmp->nbody, r2cmp->pbody,
                      r2cmp->end, r2cmp->end);
}
CompareValues DisjunctionRule::cmpRule(Rule *rcmp) {
  if (rcmp->type == CONSTRAINTRULE || rcmp->type == WEIGHTRULE)
    return LTH;
  if (rcmp->type == CHOICERULE)
    return GTH;
  if (rcmp->type == BASICRULE) { // as this rule
    BasicRule *r2cmp = (BasicRule *)rcmp;
    if (r2cmp->cmpRule(this) == SUPERSET)
      return SUBSET;
    else
      return GTH;
  }
  DisjunctionRule *r2cmp = (DisjunctionRule *)rcmp;
  CompareValues hsub = subsumes(head, hend, r2cmp->head, r2cmp->hend);
  if (hsub == LTH || hsub == GTH)
    return hsub;
  CompareValues bsub = bodySubsumed(nbody, pbody, end, end, r2cmp->nbody,
                                    r2cmp->pbody, r2cmp->end, r2cmp->end);
  if (hsub == EQ)
    return bsub;
  if (bsub == EQ)
    return hsub;
  if (hsub == bsub)
    return hsub;
  if (hsub == SUBSET)
    return GTH;
  else
    return LTH;
}

CompareValues ConstraintRule::cmpRule(Rule *rcmp) {
  if (rcmp->type == WEIGHTRULE)
    return LTH;
  if (rcmp->type != CONSTRAINTRULE)
    return GTH;

  ConstraintRule *r2cmp = (ConstraintRule *)rcmp;
  CompareValues cmp = cmpHead(r2cmp);
  if (cmp != EQ)
    return cmp;

  if (atleast > r2cmp->atleast)
    return GTH;
  if (atleast < r2cmp->atleast)
    return LTH;

  return bodyEq(nbody, pbody, end, end, r2cmp->nbody, r2cmp->pbody, r2cmp->end,
                r2cmp->end);
}

CompareValues WeightRule::bodyEqWr(WeightRule *wr) {
  if (negBodyCount < wr->negBodyCount)
    return LTH;
  if (negBodyCount > wr->negBodyCount)
    return GTH;
  if (posBodyCount < wr->posBodyCount)
    return LTH;
  if (posBodyCount > wr->posBodyCount)
    return GTH;
  int i = 0;
  Atom **b = wr->body;
  for (Atom **a = body; a < end; a++, b++, i++) {
    if ((*a)->id != (*b)->id || aux[i].weight != wr->aux[i].weight) {
      if ((*a)->id < (*b)->id)
        return LTH;
      if ((*a)->id > (*b)->id)
        return GTH;
      if (aux[i].weight < wr->aux[i].weight)
        return LTH;
      else
        return GTH;
    }
  }
  return EQ;
}

CompareValues WeightRule::cmpRule(Rule *rcmp) {
  if (rcmp->type != WEIGHTRULE)
    return GTH;

  WeightRule *r2cmp = (WeightRule *)rcmp;
  CompareValues cmp = cmpHead(r2cmp);
  if (cmp != EQ)
    return cmp;
  return bodyEqWr(r2cmp);
}
CompareValues NestedRule::cmpBody(NestedRule *r2cmp) {
  NestedRule *rcmp = (NestedRule *)r2cmp;
  return bodyEq(nbody, pbody, nnbody, end, rcmp->nbody, rcmp->pbody,
                rcmp->nnbody, rcmp->end);
}

Atom **BasicRule::posbody() {
  // returns positive part of the body
  return pbody;
}
Atom **BasicRule::posendbody() {
  // returns positive part of the body
  return pend;
}
Atom **ChoiceRule::posbody() {
  // returns positive part of the body
  return pbody;
}
Atom **ChoiceRule::posendbody() {
  // returns positive part of the body
  return pend;
}
Atom **DisjunctionRule::posbody() {
  // returns positive part of the body
  return pbody;
}
Atom **DisjunctionRule::posendbody() {
  // returns positive part of the body
  return pend;
}
Atom **ConstraintRule::posbody() {
  // returns positive part of the body
  return 0;
}
Atom **ConstraintRule::posendbody() {
  // returns positive part of the body
  return 0;
}
Atom **WeightRule::posbody() {
  // returns positive part of the body
  return 0;
}
Atom **WeightRule::posendbody() {
  // returns positive part of the body
  return 0;
}

void BasicRule::propUpper(Atom *a) {
  upper--;
  if (upper == 0 && satUUn != UNSAT) {
    head->p->q.push(head);
  }
}
void DisjunctionRule::propUpper(Atom *a) {
  upper--;
  if (upper == 0 && satUUn != UNSAT) {
    for (Atom **h = head; h < hend; h++)
      (*h)->p->q.push(*h);
  }
}

void ChoiceRule::propUpper(Atom *a) {
  upper--;
  if (upper == 0 && satUUn != UNSAT) {
    head->p->q.push(head);
  }
}
void ConstraintRule::propUpper(Atom *a) {
  upper--;
  if (upper == 0 && satUUn != UNSAT) {
    head->p->q.push(head);
  }
}
void WeightRule::propUpper(Atom *a) {

  upper -= findAtWeightBody(a, true);
  if (satUUn != UNSAT && upper <= 0)
    head->p->q.push(head);
}

void BasicRule::HeadInOneRule(Atom *at) {
  Atom **a;
  for (a = nbody; a < nend; a++) {
    (*a)->setBFalse();
  }

  for (a = pbody; a < pend; a++) {

    (*a)->setComputeTrue();
  }
}

void ChoiceRule::HeadInOneRule(Atom *at) {
  Atom **a;
  for (a = nbody; a < nend; a++) {
    (*a)->setBFalse();
  }

  for (a = pbody; a < pend; a++) {
    (*a)->setComputeTrue();
  }
}
//
// Esra's bug June 2007
// WFM bug
// void ConstraintRule::HeadInOneRule(Atom* at)
// void     WeightRule::HeadInOneRule(Atom* at){
// some of the atoms could have been assigned values by now
// for instance a:-1{falseassigned1, notassigned}
// in such case only notassigned should be assigned computetrue
// Such bug could not appear in Basic, choice or disj rules
//

void ConstraintRule::HeadInOneRule(Atom *at) {
  Atom **a;
  if (atleastCount == posBodyCount + negBodyCount) {
    for (a = nbody; a < nend; a++) {
      if (!(*a)->Bpos && !(*a)->computeTrue)
        (*a)->setBFalse();
    }
    for (a = pbody; a < pend; a++) {
      if (!(*a)->Bneg && !(*a)->computeFalse)
        (*a)->setComputeTrue();
    }
  }
}
void WeightRule::HeadInOneRule(Atom *at) {
  Atom **a;
  if (atleastCount <= maxweightCount) {
    for (Atom **a = body; a < end; a++) {
      if (!(*a)->Bneg && !(*a)->computeFalse && !(*a)->Bpos &&
          !(*a)->computeTrue) {
        if (maxweightCount - aux[a - body].weight < atleastCount) {
          if (aux[a - body].positive)
            (*a)->setComputeTrue();
          else
            (*a)->setBFalse();
        }
      }
    }
  }
}
void DisjunctionRule::HeadInOneRule(Atom *at) {
  Atom **a;
  for (Atom **a = head; a < hend; a++) {
    if ((*a) != at) {
      (*a)->headof--;
      if ((*a)->headof == 0) {
        (*a)->setBFalse();
      }
    }
  }
  for (a = nbody; a < nend; a++) {
    (*a)->setBFalse();
  }

  for (a = pbody; a < pend; a++) {

    (*a)->setComputeTrue();
  }
}

void Atom::propagateConsInBodies() { // go thru list of pBodyRules
  // and sets pbodyCount
  for (list<NestedRule *>::iterator itrR = pBodyRules.begin();
       itrR != pBodyRules.end(); itrR++) {
    if (!(*itrR)->erased) {
      (*itrR)->pbodyCount--;
      if ((*itrR)->pbodyCount == 0) {
        (*itrR)->erased = true;
        (*itrR)->head[0]->setConsTrue();
      }
    }
  }
}

//
// Sets counters and active by propogating pos/neg atom in pos/neg part of the
// body
//
void BasicRule::BodyAtom(bool posBody, bool posAtom, Atom *at) {
  if (satUUn != UNKNOWN)
    return;
  if (posBody && posAtom) {
    posBodyCount--;
  } else if (!posBody && !posAtom) {
    negBodyCount--;
  } else { // rule is inactive
    //	this->print();

    satUUn = UNSAT;

    head->headof--;
    return;
  }
  /*  if(posBodyCount+negBodyCount==0){
        if(allAtomsBpos())
          head->setBTrue();
        else
          head->setComputeTrue();

  }
  */
}
bool BasicRule::allAtomsBpos() {
  for (Atom **a = pbody; a < pend; a++)
    if (!(*a)->Bpos) {
      return false;
    }
  return true;
}
bool BasicRule::pt() {
  if (head->Bpos)
    return true;
  Atom **a;
  for (a = pbody; a < pend; a++)
    if (!(*a)->Bpos) {
      return true;
    }

  for (a = nbody; a < nend; a++)
    if ((*a)->Bpos) {
      return true;
    }
  return false;
}
bool DisjunctionRule::pt() {
  Atom **a;

  for (a = head; a < hend; a++)
    if ((*a)->Bpos) {
      return true;
    }
  for (a = pbody; a < pend; a++)
    if (!(*a)->Bpos) {
      return true;
    }

  for (a = nbody; a < nend; a++)
    if ((*a)->Bpos) {
      return true;
    }
  return false;
}
bool ChoiceRule::pt() { return false; }
bool ConstraintRule::pt() { return false; }
bool WeightRule::pt() { return false; }
//
// Sets counters and active by propogating pos/neg atom in the head
//
void DisjunctionRule::HeadAtom(bool value, Atom *at) {
  if (!value) {
    negHeadCount++;
    assert(hend - head >= negHeadCount);
    if (hend - head == negHeadCount) { // we are at the situation of constraint
      propagateHeadFalse();
    }
    if (hend - head - negHeadCount == 1 && posBodyCount + negBodyCount == 0) {
      // we are at the situation when
      // All but one atom in disjunctive rule are negative
      // and body of a rule is satisfied, hence the atom which is left is
      // setToTrue
      for (Atom **a = head; a < hend; a++) {
        if (!(*a)->Bneg) {
          if (allAtomsBpos())
            (*a)->setBTrue();
          else
            (*a)->setComputeTrue();
          break;
        }
      }
    }
  } else { // if value then we go throu head atoms different from at
    // and make it headof --
    for (Atom **a = head; a < hend; a++) {
      if ((*a) != at) {
        (*a)->headof--;
        if ((*a)->headof == 0) {

          (*a)->setBFalse();
        }
      }
    }
  }
}
//
// Sets counters and active by propogating pos/neg atom in pos/neg part of the
// body
//
void DisjunctionRule::BodyAtom(bool posBody, bool posAtom, Atom *at) {
  if (satUUn != UNKNOWN)
    return;
  if (posBody && posAtom) {
    posBodyCount--;
  } else if (!posBody && !posAtom) {
    negBodyCount--;
  } else { // rule is inactive
    satUUn = UNSAT;
    for (Atom **a = head; a < hend; a++) {
      (*a)->headof--;
    }
  }
}
bool DisjunctionRule::allAtomsBpos() {
  for (Atom **a = pbody; a < pend; a++)
    if (!(*a)->Bpos) {
      return false;
    }
  return true;
}

//
// Sets counters and active by propogating pos/neg atom in pos/neg part of the
// body
//
void ChoiceRule::BodyAtom(bool posBody, bool posAtom, Atom *at) {

  if (satUUn != UNKNOWN)
    return;
  if (posBody && posAtom) {
    posBodyCount--;
  } else if (!posBody && !posAtom) {
    negBodyCount--;
  } else { // rule is inactive
    satUUn = UNSAT;
    //	for(Atom** a=head; a<hend; a++){
    head->headof--;
    //}
  }
}

//
// Sets counters and active by propogating pos/neg atom in pos/neg part of the
// body
//
void ConstraintRule::BodyAtom(bool posBody, bool posAtom, Atom *at) {
  if (satUUn != UNKNOWN)
    return;
  if (posBody && posAtom) {
    posBodyCount--;
    atleastCount--;
  } else if (!posBody && !posAtom) {
    negBodyCount--;
    atleastCount--;
  } else { // rule is inactive
    if (posBody)
      posBodyCount--;
    else
      negBodyCount--;
  }
  /*
  if(atleastCount<1){
        if(allAtomsBpos())
          head->setBTrue();
        else
          head->setComputeTrue();
  }
  */
  if (atleastCount > posBodyCount + negBodyCount) {
    satUUn = UNSAT;
    head->headof--;
  }
}
bool ConstraintRule::allAtomsBpos() {
  for (Atom **a = pbody; a < pend; a++) {
    if (!(*a)->Bpos)
      return false;
  }
  return true;
}
bool ChoiceRule::allAtomsBpos() { return false; }

Weight WeightRule::findAtWeightBody(Atom *at, bool posBody) {
  Atom **a;
  if (posBody) {
    for (a = body; a < end; a++) {
      if (aux[a - body].positive)
        if ((*a) == at)
          return aux[a - body].weight;
    }
  } else {
    for (a = body; a < end; a++) {
      if (!aux[a - body].positive) // nbody
        if ((*a) == at)
          return aux[a - body].weight;
    }
  }
  cerr << "Atom is not present in weightrule";
  exit(0);
}
//
// Sets counters and active by propogating pos/neg atom in pos/neg part of the
// body
//
void WeightRule::BodyAtom(bool posBody, bool posAtom, Atom *at) {
  if (satUUn != UNKNOWN)
    return;
  int ind = 0;
  Weight w = findAtWeightBody(at, posBody);

  if (posBody && posAtom) {
    posBodyCount--;
    atleastCount -= w;
  } else if (!posBody && !posAtom) {
    negBodyCount--;
    atleastCount -= w;
  } else { // rule is inactive
    if (posBody) {
      posBodyCount--;
    } else {
      negBodyCount--;
    }
  }
  maxweightCount -= w;
  if (atleastCount < 1) {
    if (allAtomsBpos())
      head->setBTrue();
    else
      head->setComputeTrue();
  }
  if (atleastCount > maxweightCount) {
    satUUn = UNSAT;
    head->headof--;
  }
}
bool WeightRule::allAtomsBpos() {
  for (Atom **a = body; a < end; a++) {
    if (aux[a - body].positive)
      if (!(*a)->Bpos)
        return false;
  }
  return true;
}
BasicRule::BasicRule() {
  erase = false;
  head = 0;
  nbody = 0;
  pbody = 0;
  nend = 0;
  pend = 0;
  end = 0;
  type = BASICRULE;
  satUUn = UNKNOWN;
  posBodyCount = 0;
  negBodyCount = 0;
}

BasicRule::~BasicRule() {
  delete[] nbody; // pbody is allocated after nbody
}

void BasicRule::propagateHeadFalse() {
  if (satUUn == UNSAT)
    return;

  // if only one guy in the body still needs an assignment
  if (!(head->Bneg || head->headof == 0))
    return;
  if (head->headof == 0 && !head->Bneg) {
    //	  if(strcmp(head->atom_name(),"has_box(5,5,15)")==0){

    head->setBFalse();
  }
  Atom **a;
  if (posBodyCount + negBodyCount == 1) {
    if (negBodyCount) {

      for (a = nbody; a < nend; a++) {
        if (!(*a)->Bneg) {
          (*a)->setComputeTrue();
          break;
        }
      }
    } else {
      for (a = pbody; a < pend; a++) {

        if (!(*a)->Bpos && !(*a)->computeTrue && !(*a)->computeTrue0) {

          (*a)->setBFalse();
          break;
        }
      }
    }
  }
}
void ChoiceRule::propagateHeadFalse() {
  // nothing is done as with choice rule this does not add us info
}
void ConstraintRule::propagateHeadFalse() {
  if (satUUn == UNSAT)
    return;

  // need to put more thinking in
  // for now nothing
  // if only one guy in the body still needs an assignment
  if (!(head->Bneg || head->headof == 0))
    return;
  if (head->headof == 0 && !head->Bneg) {
    head->setBFalse();
  }

  Atom **a;
  if (posBodyCount + negBodyCount == 1 && atleastCount == 1) {
    if (negBodyCount) {
      for (a = nbody; a < nend; a++) {
        if (!(*a)->Bneg) {
          (*a)->setComputeTrue();
        }
      }
    } else {
      for (a = pbody; a < pend; a++) {
        if (!(*a)->Bpos && !(*a)->computeTrue && !(*a)->computeTrue0) {
          (*a)->setBFalse();
          break;
        }
      }
    }
  }
}
void WeightRule::propagateHeadFalse() {
  // need to put more thinking in
  // for now nothing
}
void ChoiceRule::HeadAtom(bool v, Atom *at) {
  // nothing is done as with choice rule this does not add us info
}
void BasicRule::HeadAtom(bool v, Atom *at) {
  // nothing is done as with choice rule this does not add us info
}
void ConstraintRule::HeadAtom(bool v, Atom *at) {
  // need to put more thinking in
  // for now nothing
}
void WeightRule::HeadAtom(bool v, Atom *at) {
  // need to put more thinking in
  // for now nothing
}

void DisjunctionRule::propagateHeadFalse() {
  if (hend - head != negHeadCount)
    return;
  if (satUUn == UNSAT)
    return;
  Atom **a;
  if (posBodyCount + negBodyCount == 1) {
    if (negBodyCount) {
      for (a = nbody; a < nend; a++) {
        if (!(*a)->Bneg) {
          (*a)->setComputeTrue();
        }
      }
    } else {
      for (a = pbody; a < pend; a++) {
        if (!(*a)->Bpos && !(*a)->computeTrue && !(*a)->computeTrue0) {
          (*a)->setBFalse();
          break;
        }
      }
    }
  }
}

Result BasicRule::satUnsatUnknown() {
  if (satUUn == SAT || satUUn == UNSAT)
    return satUUn;
  if (posBodyCount + negBodyCount == 0) {
    satUUn = SAT;
    if (allAtomsBpos())
      head->setBTrue();
    else
      head->setComputeTrue();

    return satUUn;
  }

  return satUUn; // now it will be known at time of atom propogating if rule is
                 // unsat
}
Result ChoiceRule::satUnsatUnknown() {
  if (satUUn == SAT || satUUn == UNSAT)
    return satUUn;
  if (posBodyCount + negBodyCount == 0) {
    satUUn = SAT;
  }
  return satUUn;
}
Result DisjunctionRule::satUnsatUnknown() {
  if (satUUn == SAT || satUUn == UNSAT)
    return satUUn;
  if (posBodyCount + negBodyCount == 0 && hend - head - negHeadCount == 1) {
    satUUn = SAT;
    if (hend - head - negHeadCount == 1) {
      for (Atom **a = head; a < hend; a++) {
        if (!(*a)->Bneg) {
          if (allAtomsBpos())
            (*a)->setBTrue();
          else
            (*a)->setComputeTrue();
          break;
        }
      }
    }
    if (hend - head - negHeadCount == 0) { // the rule is constraint
      // we derive to a conflict
      head[0]->setBTrue(); // we set one of the guys true
      // to set the conflict on
    }
  }
  return satUUn;
}
Result ConstraintRule::satUnsatUnknown() {
  if (satUUn == SAT || satUUn == UNSAT)
    return satUUn;
  if (posBodyCount + negBodyCount < atleastCount) {
    satUUn = UNSAT;
    head->headof--;
    if (head->headof == 0) {
      head->setBFalse();
    }
  }
  if (atleastCount < 1) {
    satUUn = SAT;
    if (allAtomsBpos())
      head->setBTrue();
    else
      head->setComputeTrue();
  }
  return satUUn;
}

Result WeightRule::satUnsatUnknown() {

  if (satUUn == SAT || satUUn == UNSAT)
    return satUUn;
  if (atleastCount > maxweightCount) {
    satUUn = UNSAT;
    head->headof--;
    if (head->headof == 0) {
      head->setBFalse();
    }
  }

  if (atleastCount < 1) {
    satUUn = SAT;

    if (allAtomsBpos())
      head->setBTrue();
    else
      head->setComputeTrue();
  }
  return satUUn;
}

void BasicRule::initUpper() {
  upper = pend - pbody;
  if (upper == 0)
    head->p->q.push(head);
}
void ChoiceRule::initUpper() {
  upper = pend - pbody;
  if (upper == 0)
    head->p->q.push(head);
}
void DisjunctionRule::initUpper() {
  upper = pend - pbody;
  if (upper == 0) {
    for (Atom **h = head; h < hend; h++)
      (*h)->p->q.push(*h);
  }
}
void ConstraintRule::initUpper() {
  upper = atleast - (nend - nbody);
  if (upper <= 0)
    head->p->q.push(head);
  //  int c=0;
  //  for(Atom** a=nbody;a<nend;a++)
  //	if(a->computeTrue||a->Bpos)
  //	  c++;
  //  lower =atleast-c;
}
void WeightRule::initUpper() {
  upper = atleast - upper_c;
  if (upper <= 0)
    head->p->q.push(head);
}

void BasicRule::init(Init *init, int pos) {
  head = *init->head;
  long n = init->psize + init->nsize;
  upper = init->psize;
  if (n != 0)
    nbody = new Atom *[n];
  else
    nbody = 0;
  end = nbody + n;
  pend = end;
  for (long i = 0; i < init->nsize; i++) {
    nbody[i] = init->nbody[i];
    negBodyCount++;
  }
  nend = nbody + init->nsize;
  pbody = nend;

  for (long i = 0; i < init->psize; i++) {
    pbody[i] = init->pbody[i];
    posBodyCount++;
  }
}
void BasicRule::initHeadRuleLists4Sort() { head->headRules.push_back(this); }
void ChoiceRule::initHeadRuleLists4Sort() { head->headRules.push_back(this); }
void WeightRule::initHeadRuleLists4Sort() { head->headRules.push_back(this); }
void ConstraintRule::initHeadRuleLists4Sort() {
  head->headRules.push_back(this);
}
void DisjunctionRule::initHeadRuleLists4Sort() {
  for (Atom **a = head; a < hend; a++) {
    (*a)->headof++;
    (*a)->headRules.push_back(this);
  }
}
void BasicRule::initRuleLists4WF() {
  head->headof++;
  head->headRules.push_back(this);
  for (Atom **a = nbody; a < nend; a++)
    (*a)->negBodyRules.push_back(this);
  for (Atom **a = pbody; a < pend; a++)
    (*a)->posBodyRules.push_back(this);
}

void BasicRule::print() {
  head->print();

  if (nbody)
    cout << " :- ";
  Atom **a;
  int comma = 0;
  for (a = pbody; a != pend; a++) {
    if (comma)
      cout << ", ";
    (*a)->print();
    comma = 1;
  }
  for (a = nbody; a != nend; a++) {
    if (comma)
      cout << ", ";
    cout << "not ";
    (*a)->print();
    comma = 1;
  }
  cout << '.' << endl;
}

ConstraintRule::ConstraintRule() {
  erase = false;
  head = 0;
  //  lit = 0;
  upper = 0;
  // lower = 0;
  atleast = 0;
  nbody = 0;
  pbody = 0;
  nend = 0;
  pend = 0;
  end = 0;
  type = CONSTRAINTRULE;
  satUUn = UNKNOWN;

  posBodyCount = 0;
  negBodyCount = 0;
  atleastCount = 0;
}

ConstraintRule::~ConstraintRule() { delete[] nbody; }

void ConstraintRule::init(Init *init, int pos) {
  head = *init->head;
  long n = init->nsize + init->psize;
  if (n != 0)
    nbody = new Atom *[n];
  else
    nbody = 0;
  end = nbody + n;
  pend = end;
  atleast = init->atleast_body;
  upper = atleast - init->nsize;
  atleastCount = init->atleast_body;

  for (long i = 0; i < init->nsize; i++) {
    nbody[i] = init->nbody[i];
    negBodyCount++;
  }
  nend = nbody + init->nsize;
  pbody = nend;

  for (long i = 0; i < init->psize; i++) {
    pbody[i] = init->pbody[i];
    posBodyCount++;
  }
}
void ConstraintRule::initRuleLists4WF() {
  head->headof++;
  head->headRules.push_back(this);
  for (Atom **a = nbody; a < nend; a++)
    (*a)->negBodyRules.push_back(this);
  for (Atom **a = pbody; a < pend; a++)
    (*a)->posBodyRules.push_back(this);
}

void ConstraintRule::print() {
  head->print();

  cout << " :- ";
  Atom **a;
  int comma = 0;
  cout << atleast << " { ";
  for (a = pbody; a != pend; a++) {
    if (comma)
      cout << ", ";
    (*a)->print();

    comma = 1;
  }
  for (a = nbody; a != nend; a++) {
    if (comma)
      cout << ", ";
    cout << "not ";
    (*a)->print();
    comma = 1;
  }
  cout << " }";
  cout << '.' << endl;
}

ChoiceRule::ChoiceRule() {
  erase = false;
  head = 0;
  //  hend = 0;
  //  lit = 0;
  upper = 0;
  nbody = 0;
  pbody = 0;

  nend = 0;
  pend = 0;
  end = 0;
  // inactive = 0;
  // visited = false;
  type = CHOICERULE;
  satUUn = UNKNOWN;
  posBodyCount = 0;
  negBodyCount = 0;
}

ChoiceRule::~ChoiceRule() { delete[] nbody; }

void ChoiceRule::init(Init *init, int pos) {
  head = init->head[pos];
  long n = init->psize + init->nsize;
  upper = init->psize;
  if (n != 0)
    nbody = new Atom *[n];
  else
    nbody = 0;
  end = nbody + n;
  pend = end;
  for (long i = 0; i < init->nsize; i++) {
    nbody[i] = init->nbody[i];
    negBodyCount++;
  }
  nend = nbody + init->nsize;
  pbody = nend;

  for (long i = 0; i < init->psize; i++) {
    pbody[i] = init->pbody[i];
    posBodyCount++;
  }
}
void ChoiceRule::initRuleLists4WF() {
  head->headof++;
  head->headRules.push_back(this);
  for (Atom **a = nbody; a < nend; a++)
    (*a)->negBodyRules.push_back(this);
  for (Atom **a = pbody; a < pend; a++)
    (*a)->posBodyRules.push_back(this);
}
void ChoiceRule::print() {
  Atom **a;
  bool comma = false;
  cout << "{ ";
  head->print();
  /*
  for (a = head; a != hend; a++)
    {
      if (comma)
                cout << ", ";
          (*a)->print();
      comma = true;
    }
  */
  cout << " }";
  comma = false;
  if (pbody != pend || nbody != nend)
    cout << " :- ";
  for (a = pbody; a != pend; a++) {
    if (comma)
      cout << ", ";
    (*a)->print();
    comma = true;
  }
  for (a = nbody; a != nend; a++) {
    if (comma)
      cout << ", ";
    cout << "not ";
    (*a)->print();
    comma = true;
  }
  cout << '.' << endl;
}
DisjunctionRule::DisjunctionRule() {
  erase = false;
  head = 0;
  hend = 0;
  //  lit = 0;
  upper = 0;
  nbody = 0;
  pbody = 0;

  nend = 0;
  pend = 0;
  end = 0;
  // inactive = 0;
  //   visited = false;
  type = DISJUNCTIONRULE;
  satUUn = UNKNOWN;
  posBodyCount = 0;
  negBodyCount = 0;
  negHeadCount = 0;
}
DisjunctionRule::~DisjunctionRule() { delete[] head; }

void DisjunctionRule::print() {
  Atom **a;
  bool comma = false;
  cout << "{ ";
  for (a = head; a != hend; a++) {
    if (comma)
      cout << ", ";
    (*a)->print();
    comma = true;
  }
  cout << " }";
  comma = false;
  if (pbody != pend || nbody != nend)
    cout << " :- ";
  for (a = pbody; a != pend; a++) {
    if (comma)
      cout << ", ";
    (*a)->print();
    comma = true;
  }
  for (a = nbody; a != nend; a++) {
    if (comma)
      cout << ", ";
    cout << "not ";
    (*a)->print();
    comma = true;
  }
  cout << '.' << endl;
}

void DisjunctionRule::init(Init *init, int pos) {
  long n = init->hsize + init->psize + init->nsize;
  upper = init->psize;
  head = new Atom *[n];
  hend = head + init->hsize;
  end = head + n;
  pend = end;
  for (long i = 0; i < init->hsize; i++) {
    head[i] = init->head[i];
  }
  nbody = hend;
  // lit = init->psize + init->nsize;
  for (long i = 0; i < init->nsize; i++) {
    nbody[i] = init->nbody[i];
    negBodyCount++;
  }
  nend = nbody + init->nsize;
  pbody = nend;
  // upper = init->psize;
  for (long i = 0; i < init->psize; i++) {
    pbody[i] = init->pbody[i];
    posBodyCount++;
  }
}
void DisjunctionRule::initRuleLists4WF() {
  for (Atom **a = head; a < hend; a++) {
    (*a)->headof++;
    (*a)->headRules.push_back(this);
  }

  for (Atom **a = nbody; a < nend; a++)
    (*a)->negBodyRules.push_back(this);
  for (Atom **a = pbody; a < pend; a++)
    (*a)->posBodyRules.push_back(this);
}

WeightRule::WeightRule() {
  erase = false;
  head = 0;
  bend = 0;
  body = 0;
  end = 0;
  aux = 0;

  maxweight = 0;
  upper_c = 0;

  atleast = 0;
  max = 0;
  min = 0;
  max_shadow = 0;
  min_shadow = 0;
  //  visited = false;
  type = WEIGHTRULE;
  satUUn = UNKNOWN;
  posBodyCount = 0;
  negBodyCount = 0;
  atleastCount = 0;
  maxweightCount = 0;
}

WeightRule::~WeightRule() {
  delete[] body;
  delete[] aux;
  // delete[] reverse;
}

void WeightRule::init(Init *init, int pos) {
  head = *init->head;

  atleast = init->atleast_weight;
  atleastCount = init->atleast_weight;
  maxweight = 0;
  maxweightCount = 0;

  long n = init->psize + init->nsize;
  if (n != 0) {
    body = new Atom *[n];
    aux = new Auxiliary[n];
  } else {
    body = 0;
    aux = 0;
  }
  bend = body + n;
  //  nend = body+init->nsize;
  end = bend;
  max = body;
  min = body;
  max_shadow = body;
  min_shadow = body;
  long i;
  for (i = 0; i < init->nsize; i++) {
    body[i] = init->nbody[i];
    //  body[i]->negBodyRules.push_back(this);
    negBodyCount++;

    aux[i].a = body + i;
    aux[i].weight = init->nweight[i];
    maxweightCount += aux[i].weight;
    upper += aux[i].weight;
    upper_c += aux[i].weight;
    aux[i].positive = false;
    aux[i].in_loop = false;
  }
  for (long j = 0; j < init->psize; j++, i++) {
    body[i] = init->pbody[j];
    // body[i]->posBodyRules.push_back(this);
    posBodyCount++;

    aux[i].a = body + i;
    aux[i].weight = init->pweight[j];
    maxweightCount += aux[i].weight;
    aux[i].positive = true;
    aux[i].in_loop = true;
  }
}
void WeightRule::initRuleLists4WF() {
  head->headof++;
  head->headRules.push_back(this);
  int size = negBodyCount + posBodyCount;
  for (int i = 0; i < size; i++) {
    if (i < negBodyCount)
      body[i]->negBodyRules.push_back(this);
    else
      body[i]->posBodyRules.push_back(this);
  }
}

void WeightRule::print() {
  head->print();
  cout << " :- { ";
  int comma = 0;
  for (Atom **a = body; a != bend; a++)
    if (aux[a - body].positive) {
      if (comma)
        cout << ", ";
      (*a)->print();
      cout << " = " << aux[a - body].weight;
      comma = 1;
    } else {
      if (comma)
        cout << ", ";
      cout << "not ";
      (*a)->print();
      cout << " = " << aux[a - body].weight;
      comma = 1;
    }
  cout << "} >=" << atleast << '.' << endl;
}
Completion::Completion() {
  head = 0;
  nbody = 0;
  pbody = 0;
  nend = 0;
  pend = 0;
  end = 0;

  connector = OR;
  eq = EQUIV;
}

Completion::~Completion() {
  delete[] nbody; // pbody is allocated after nbody
}

void Completion::allocateNbody(long size) {
  nbody = new Atom *[size];
  end = nbody + size;
}

void Completion::initCompletionNbodyFromApi(Api *api) {

  int size = api->sizeBody();

  if (!nbody)
    delete[] nbody;
  allocateNbody(size);

  // set number of atoms in parts of the rules
  numNbody = api->sizeNbody();
  numPbody = api->sizePbody() + api->sizeNNbody();

  // set pointers to the beginning and end of the parts of the rules
  nend = nbody + numNbody;
  pbody = nend;
  pend = pbody + numPbody;
  // pend is the end of clause and must be the same
  // as an end of the clause
  assert(pend == end);

  for (int i = 0; i < numNbody; i++) {
    nbody[i] = api->nbodyAtom(i);
  }
  int i = 0;
  for (i; i < api->sizePbody(); i++) {
    pbody[i] = api->pbodyAtom(i);
  }
  for (int j = 0; j < api->sizeNNbody(); j++) {
    pbody[i] = api->nnbodyAtom(j);
    i++;
  }
}
void Completion::initCompletionNbodyFromCompApi(Api *api) {

  int size = api->sizeCompNbody() + api->sizeCompPbody();
  if (!nbody)
    delete[] nbody;
  allocateNbody(size);

  // set number of atoms in parts of the rules
  numNbody = api->sizeCompNbody();
  numPbody = api->sizeCompPbody();

  // set pointers to the beginning and end of the parts of the rules
  nend = nbody + numNbody;
  pbody = nend;
  pend = pbody + numPbody;
  // pend is the end of clause and must be the same
  // as an end of the clause
  assert(pend == end);

  for (int i = 0; i < numNbody; i++) {
    nbody[i] = api->compNbodyAtom(i);
  }
  for (int i = 0; i < numPbody; i++) {
    pbody[i] = api->compPbodyAtom(i);
  }
}

void Completion::print() {
  // cout <<"BOC"<< endl;;
  //   cout << head->atom_name () <<" " << head->id;
  head->print();

  if (eq == EQUIV) {
    if (nbody)
      cout << " <-> ";
    else
      cout << " <-> T";
  } else {
    if (nbody)
      cout << " -> ";
    else
      cout << " -> T";
  }
  Atom **a;
  int comma = 0;
  for (a = pbody; a != pend; a++) {
    if (connector == OR && comma)
      cout << " v ";
    else if (comma)
      cout << " & ";

    (*a)->print();
    comma = 1;
  }
  for (a = nbody; a != nend; a++) {
    if (connector == OR && comma)
      cout << " v ";
    else if (comma)
      cout << " & ";

    cout << " -";
    (*a)->print();
    comma = 1;
  }
  cout << "." << endl;
  //  cout <<"EOC";
}
void Completion::printBCircuit(FILE *file, char *gatename) {
  // head is not in any cycle in POSNEG dependency graph
  if (head->inLoop == -1) {
    head->printBCircuit(file);
    fprintf(file, ":= ");
  } else {
    fprintf(file, "%s := ", gatename);
    head->printBCircuit(file);
    if (eq == EQUIV) {
      if (nbody)
        fprintf(file, " == ");
      else
        fprintf(file, " == T ");
    } else {
      if (nbody)
        fprintf(file, " => ");
      else
        fprintf(file, " => T");
    }
  }
  Atom **a;
  int comma = 0;
  if (connector == OR)
    fprintf(file, " OR(");
  else
    fprintf(file, " AND(");
  for (a = pbody; a != pend; a++) {
    if (comma)
      fprintf(file, ",");

    (*a)->printBCircuit(file);
    comma = 1;
  }
  for (a = nbody; a != nend; a++) {
    if (comma)
      fprintf(file, ",");
    fprintf(file, "~");
    (*a)->printBCircuit(file);
    comma = 1;
  }
  fprintf(file, ");\n");
  // if the atom is in some cycle we should define the completion as a separate
  // clause
  if (head->inLoop != -1) {

    fprintf(file, "ASSIGN %s;\n", gatename);
  } else if (head->Bpos || head->computeTrue) {
    fprintf(file, "ASSIGN ");
    head->printBCircuit(file);
    fprintf(file, ";\n ");
    head->Bpos = false;
    head->computeTrue = false;
  }
}
//******* Nested Rule is for Cmodels use
// as all the rules are translated in the format of
// Nested Rules

NestedRule::NestedRule() {
  head = 0;
  hend = 0;
  nbody = 0;
  pbody = 0;
  nnbody = 0;
  nnend = 0;
  nend = 0;
  pend = 0;
  end = 0;

  numHead = 0;
  numPbody = 0;
  numNbody = 0;
  numNNbody = 0;

  type = NESTED;
  reprComp = 0;
  reprComp2 = 0;
  signReprComp = NOT_DEF;

  isRR = true;
  markedSCC = true;

  bodyACl = false;
  bodyAClVerification = 0;

  erased = false; // used in building reduct, rule is erased whenever it is not
                  // satisfied by the model
}

// returns true when rule is satisfied by the current model
//  otherwise returns false
//
//
bool NestedRule::bodySatisfied() {
  if (signReprComp != 0) {
    if (reprComp->inM && signReprComp == POS)
      return true;
    if (!reprComp->inM && signReprComp == NEG)
      return true;
    if (!reprComp->inM && signReprComp == POS)
      return false;
    if (reprComp->inM && signReprComp == NEG)
      return false;
  }
  for (Atom **a = nbody; a != nend; a++)
    if ((*a)->inM)
      return false;

  for (Atom **a = pbody; a != nnend; a++)
    if (!(*a)->inM)
      return false;

  return true;
}

// returns true if body is SAT, i.e. reprComp atom is inM
// and no atom in positive part of the body contains atom h->inLoop
// whether rules supports h externally of loop
bool NestedRule::support(Atom *h) {
  if (!reprComp->inM)
    return false;
  if (sizeHead() > 1)
    for (Atom **a = head; a != hend; a++)
      if ((*a) != h && (*a)->inM)
        return false;
  for (Atom **a = pbody; a != pend; a++)
    if ((*a)->inLoop == h->inLoop)
      return false;
  return true;
}

void NestedRule::finishRule() {
  assert(head);
  int size = sizeBody() + sizeHead();
  /*
  for (int i = 0; i < numHead; i++){

        head[i]->print();
        cout<<" ; ";
  }
  cout<<":-";
  for (int i = 0; i < numNbody; i++){
        cout<<"-";
        nbody[i]->print();
        cout<<" v ";
  }
  for (int i = 0; i < numPbody; i++){

        pbody[i]->print();
        cout<<" v ";

  }
  cout<<endl;
  */
  assert(size > 0);
  //  assert(reprComp);
}
void NestedRule::initRuleFromApi(Api *api, RuleType ruleType) {
  type = ruleType;

  int size =
      api->sizeNbody() + api->sizePbody() + api->sizeHead() + api->sizeNNbody();

  assert(size > 0); // size must be always greater than 0
  allocateHead(size);

  // set number of atoms in parts of the rules
  numHead = api->sizeHead();
  numNbody = api->sizeNbody();
  numPbody = api->sizePbody();
  numNNbody = api->sizeNNbody();
  // set pointers to the beginning and end of the parts of the rules
  hend = head + numHead;
  nbody = hend;
  nend = nbody + numNbody;
  pbody = nend;
  pend = pbody + numPbody;
  nnbody = pend;
  nnend = nnbody + numNNbody;

  // nnend is the end of nested part of the urle and must be the same
  // as an end of the rule
  assert(nnend == end);

  for (int i = 0; i < numHead; i++) {
    head[i] = api->headAtom(i);
  }
  for (int i = 0; i < numNbody; i++) {
    nbody[i] = api->nbodyAtom(i);
  }
  for (int i = 0; i < numPbody; i++) {
    pbody[i] = api->pbodyAtom(i);
  }
  for (int i = 0; i < numNNbody; i++) {
    nnbody[i] = api->nnbodyAtom(i);
  }
}
NestedRule::~NestedRule() { delete[] head; }

void NestedRule::print() {
  Atom **a;
  bool comma = false;
  cout << "{ ";
  for (a = head; a != hend; a++) {
    if (comma)
      cout << ", ";
    (*a)->print();

    comma = true;
  }
  cout << " }";
  comma = false;
  if (pbody != pend || nbody != nend || nnbody != nnend)
    cout << " :- ";
  for (a = pbody; a != pend; a++) {
    if (comma)
      cout << ", ";
    (*a)->print();
    comma = true;
  }
  for (a = nbody; a != nend; a++) {
    if (comma)
      cout << ", ";
    cout << "not "; // << (*a)->atom_name ();
    (*a)->print();
    comma = true;
  }
  for (a = nnbody; a != nnend; a++) {
    if (comma)
      cout << ", ";
    cout << "not not ";
    (*a)->print();
    comma = true;
  }
  cout << '.' << endl;
}

void NestedRule::printBCircuit(FILE *file, Atom *h) {
  Atom **a;
  bool comma = false;
  if (sizeHead() == 1 && sizeBody() == 0) {
    fprintf(file, "T");
    return;
  }
  if (sizeHead() != 1 || sizeBody() > 1)
    fprintf(file, "AND(");
  for (a = head; a != hend; a++) {
    if (h != (*a)) {
      if (comma)
        fprintf(file, ",");
      fprintf(file, "~");
      (*a)->printBCircuit(file);
      comma = true;
    }
  }
  for (a = pbody; a != nnend; a++) {
    if (comma)
      fprintf(file, ",");
    (*a)->printBCircuit(file);
    comma = true;
  }
  for (a = nbody; a != nend; a++) {
    if (comma)
      fprintf(file, ",");
    fprintf(file, "~");
    (*a)->printBCircuit(file);
    comma = true;
  }
  if (sizeHead() != 1 || sizeBody() > 1)
    fprintf(file, ")");
}
//******* Clauses are the ones stored by cmodels
//

Clause::Clause() {
  nbody = 0;
  pbody = 0;
  nend = 0;
  pend = 0;
  end = 0;
  numPbody = 0;
  numNbody = 0;
}
void Clause::finishClause() {
  int size = sizeCl();
  /*
  for (int i = 0; i < numNbody; i++){
        cout<<"-";
        nbody[i]->print();
        cout<<" v ";
  }
  for (int i = 0; i < numPbody; i++){

        pbody[i]->print();
        cout<<" v ";

  }
  cout<<endl;
  */
  assert(size > 0);
}

void Clause::initClauseFromApi(Api *api) {

  int size = api->sizeNbody() + api->sizePbody();

  allocate(size);

  // set number of atoms in parts of the rules
  numNbody = api->sizeNbody();
  numPbody = api->sizePbody();

  //  cout<<numNbody<<" "<<numPbody<<endl;
  // set pointers to the beginning and end of the parts of the rules
  nend = nbody + numNbody;
  pbody = nend;
  pend = pbody + numPbody;
  // pend is the end of clause and must be the same
  // as an end of the clause
  assert(pend == end);

  for (int i = 0; i < numNbody; i++) {
    nbody[i] = api->nbodyAtom(i);
  }
  for (int i = 0; i < numPbody; i++) {
    pbody[i] = api->pbodyAtom(i);
  }
}
Clause::~Clause() {
  if (nbody)
    delete[] nbody;
}

void Clause::print(bool useAtomNames) {
  Atom **a;
  bool comma = false;
  for (a = pbody; a != pend; a++) {
    if (comma) {
      cout << " v ";
    }

    if (useAtomNames) {
      cout << (*a)->atom_name();
    } else {
      (*a)->print();
    }

    comma = true;
  }

  for (a = nbody; a != nend; a++) {
    if (comma) {
      cout << " v ";
    }
    cout << "- ";
    if (useAtomNames) {
      cout << (*a)->atom_name();
    } else {
      (*a)->print();
    }

    comma = true;
  }
  cout << '.' << endl;
}

void Clause::printcnf(FILE *file) {
  Atom **a;
  int comma = 0;
  for (a = nbody; a != nend; a++) {
    if (comma)
      fprintf(file, " ");

    fprintf(file, " -%d", (*a)->id);
    comma = 1;
  }
  for (a = pbody; a != pend; a++) {
    if (comma)
      fprintf(file, " ");

    fprintf(file, " %d", (*a)->id);
    comma = 1;
  }
  fprintf(file, " 0\n");
}

void Clause::printsmtcnf(FILE *file) {
  Atom **a;
  int comma = 0;
  for (a = nbody; a != nend; a++) {
    if (comma)
      fprintf(file, " ");

    fprintf(file, "-");
    (*a)->printClean(file);
    comma = 1;
  }
  for (a = pbody; a != pend; a++) {
    if (comma)
      fprintf(file, " ");
    (*a)->printClean(file);
    comma = 1;
  }
  fprintf(file, " 0\n");
}

void Clause::printBCircuit(FILE *file, char *gatename) {
  // special case where we do not need to introduce a gate;
  // atom it self is a gate
  if (sizeCl() == 1) {
    if (sizePbody() == 1 &&
        ((*nbody)->Bpos == false && (*nbody)->computeTrue == false)) {
      // we skip the case
    } else {
      (*nbody)->printBCircuit(file);
      fprintf(file, ";\n ASSIGN ");
      if (sizeNbody() == 1)
        fprintf(file, "~");
      (*nbody)->printBCircuit(file);
      fprintf(file, ";\n");
      return;
    }
  }

  fprintf(file, "%s := OR(", gatename);
  Atom **a;
  int comma = 0;
  for (a = nbody; a != nend; a++) {
    if (comma)
      fprintf(file, ", ");
    fprintf(file, " ~");
    (*a)->printBCircuit(file);
    comma = 1;
  }
  for (a = pbody; a != pend; a++) {
    if (comma)
      fprintf(file, ", ");
    (*a)->printBCircuit(file);
    comma = 1;
  }
  fprintf(file, ");\nASSIGN %s;\n", gatename);
}

void Clause::translateToZchaffClause(int *clause, int &size) {
  //  print();
  Atom **a;
  size = 0;
  for (a = nbody; a != nend; a++) {
    clause[size] = (*a)->id * 2 + 1;
    size++;
  }
  for (a = pbody; a != pend; a++) {
    clause[size] = (*a)->id * 2;
    size++;
  }
}
//
// Minisat takes all the clasues
void Clause::translateToMinisatClause(int *clause, int &size) {
  Atom **a;
  size = 0;
  for (a = nbody; a != nend; a++) {
    clause[size] = -(*a)->id;
    size++;
  }
  for (a = pbody; a != pend; a++) {
    clause[size] = (*a)->id;
    size++;
  }
}
void Clause::translateToSimoReason(int *clause, int &size, const int &sizeCl) {

  Atom **a;
  for (long i = 0; i < sizeCl; i++)
    clause[i] = 0; // by default atom is not in reason
  size = 0;
  for (a = nbody; a != nend; a++) {
    clause[(*a)->id - 1] = 2;
    size++;
  }
  for (a = pbody; a != pend; a++) {
    clause[(*a)->id - 1] = 1;
    size++;
  }
}
void NestedRule::setType(RuleType ruleType) { type = ruleType; }

RuleType NestedRule::getType() { return type; }
long NestedRule::sizeHead() { return numHead; }
long NestedRule::sizePbody() { return numPbody; }
long NestedRule::sizeNbody() { return numNbody; }
long NestedRule::sizeBody() { return numNbody + numPbody + numNNbody; }
long NestedRule::sizeNNbody() { return numNNbody; }

Atom **NestedRule::getHead() { return head; }
Atom **NestedRule::getPbody() { return pbody; }
Atom **NestedRule::getNbody() { return nbody; }
Atom **NestedRule::getNNbody() { return nnbody; }

void NestedRule::allocateRule(long headSize, long nbodySize, long pbodySize,
                              long nnbodySize) {

  int size = headSize + pbodySize + nbodySize + nnbodySize;
  assert(size > 0); // size is always greater than 0
  allocateHead(size);

  // set number of atoms in parts of the rules
  numHead = headSize;
  numNbody = nbodySize;
  numPbody = pbodySize;
  numNNbody = nnbodySize;

  // set pointers to the beginning and end of the parts of the rules
  hend = head + numHead;
  nbody = hend;
  nend = nbody + numNbody;
  pbody = nend;
  pend = pbody + numPbody;
  nnbody = pend;
  nnend = nnbody + numNNbody;

  // nnend is the end of nested part of the urle and must be the same
  // as an end of the rule
  assert(nnend == end);
}
void NestedRule::addHead(long pos, Atom *atom) {
  assert(pos >= 0 && pos < numHead);
  head[pos] = atom;
}

void NestedRule::addNbody(long pos, Atom *atom) {
  assert(pos >= 0 && pos < numNbody);
  nbody[pos] = atom;
}

void NestedRule::addPbody(long pos, Atom *atom) {

  assert(pos >= 0 && pos < numPbody);
  pbody[pos] = atom;
}
void NestedRule::addNNbody(long pos, Atom *atom) {
  assert(pos >= 0 && pos < numNNbody);
  nnbody[pos] = atom;
}
void NestedRule::addBody(long pos, Atom *atom) {
  assert(pos >= 0 && pos < sizeBody());
  nbody[pos] = atom;
}

void NestedRule::allocateHead(long size) {
  head = new Atom *[size];
  end = head + size;
}

const char *Atom::atom_name() {
  if (name)
    return name;
  else
    return "#noname#";
}
long Clause::sizePbody() { return numPbody; }
long Clause::sizeCl() { return numPbody + numNbody; }
long Clause::sizeNbody() { return numNbody; }

Atom **Clause::getPbody() { return pbody; }
Atom **Clause::getNbody() { return nbody; }

void Clause::allocate(long size) {
  nbody = new Atom *[size];
  end = nbody + size;
}

void Clause::allocateClause(long nbodySize, long pbodySize) {

  int size = pbodySize + nbodySize;
  allocate(size);

  // set number of atoms in parts of the clause
  numNbody = nbodySize;
  numPbody = pbodySize;

  // set pointers to the beginning and end of the parts of the rules
  nend = nbody + numNbody;
  pbody = nend;
  pend = pbody + numPbody;

  // pend is the end of pos part of the clause and must be the same
  // as an end of the clause
  assert(pend == end);
}

void Clause::addNbody(long pos, Atom *atom) {
  assert(pos >= 0 && pos < numNbody);
  nbody[pos] = atom;
}

void Clause::addPbody(long pos, Atom *atom) {

  assert(pos >= 0 && pos < numPbody);
  pbody[pos] = atom;
}

void Clause::addBody(long pos, Atom *atom) {
  assert(pos >= 0 && pos < numPbody + numNbody);
  nbody[pos] = atom;
}

;
