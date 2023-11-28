//
// Added capability of previous constraint rule +new recursive function
//

/*
 * File api.cc
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

#include "api.h"
#include "atomrule.h"
#include "graphscc.h"
#include "program.h"
#include <assert.h>
#include <cstdio>
#include <cstdlib>
#include <float.h>
#include <fstream>
#include <iostream>
#include <limits.h>
#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <set>
#include <vector>

using namespace std;

/*****************************************************************************
 * Time values
 ****************************************************************************/
/*struct TimeValue {
  unsigned int sec;
  unsigned int usec;

  TimeValue() {
    sec = 0;
    usec= 0;
  }
  TimeValue& operator+=(const TimeValue& rhs) {
    sec += rhs.sec;
    usec += rhs.usec;
    if (usec > USECxSEC) {
      ++sec;
      usec = usec % USECxSEC;
    }
    return *this;
  }
  float asFloat() {
    return ((float)sec + ((float)usec / (float)1.0e6));
  }
}

*/

// If vertexName is not present, add it to vertexMap
// In either case, return the Vertex
Atom *WeightRuleMemory::findAtom(char *aName) {
  amap::iterator itr = atomMap.find(aName);
  //	cout<<aName<<endl;
  if (itr == atomMap.end()) {
    return NULL;
  }
  return (*itr).second;
}
void WeightRuleMemory::addAtom(Atom *a, char *atomName) {
  atomMap.insert(apair(atomName, a));
}
void WeightRuleMemory::clearAll() { atomMap.clear(); }

Api::listApi::listApi() {
  top = 0;
  size = 32;
  atoms = new Atom *[size];
  weights = new Weight[size];
  np = new bool[size];
}
void Api::listApi::print() {
  for (int i = 0; i < top; i++) {
    atoms[i]->print();
    cout << " ";
  }
  if (top > 0)
    cout << endl;
}

Api::listApi::~listApi() {
  delete[] atoms;
  delete[] weights;
  delete[] np;
}

void Api::listApi::push(Atom *a, Weight w) {
  if (top == size)
    grow();
  atoms[top] = a;
  weights[top] = w;
  top++;
}
void Api::listApi::push(Atom *a, Weight w, bool pos) {
  if (top == size)
    grow();
  atoms[top] = a;
  weights[top] = w;
  np[top] = pos;
  top++;
}
void Api::sort(listApi &l) {

  if (l.size > nnbody.size)
    nnbody.grow(l.size);
  //  cout<<"start "<<l.top<<" "<<l.size<<" "<<temp.size<<endl;
  //  l.print();

  for (unsigned m = 1; m <= l.top; m *= 2) {
    for (unsigned i = 0; i < l.top - m; i += m * 2) {
      inplace_merge(l, i, i + m, ((i + m * 2) > l.top) ? l.top : (i + m * 2));
    }
  }
  //  cout<<"end ";
  //  l.print();
  return;
}

void Api::copyListElement(listApi &l1, listApi &l2, int p1, int p2) {

  l1.atoms[p1] = l2.atoms[p2];
  l1.weights[p1] = l2.weights[p2];
}

void Api::inplace_merge(listApi &l, int first, int middle, int last) {

  int f = first;
  int m = middle;

  while (first != m && middle != last) {
    if (l.atoms[first]->id <= l.atoms[middle]->id) {

      copyListElement(nnbody, l, nnbody.top, first);
      first++;

    } else {
      copyListElement(nnbody, l, nnbody.top, middle);
      middle++;
    }
    nnbody.top++;
  }
  if (first < m) {
    for (first; first < m; first++) {
      copyListElement(nnbody, l, nnbody.top, first);

      nnbody.top++;
    }
  }
  if (middle < last) {

    for (first = middle; first < last; first++) {
      copyListElement(nnbody, l, nnbody.top, first);
      nnbody.top++;
    }
  }

  assert(last - f == nnbody.top);
  for (int i = 0; i < nnbody.top; i++) {

    copyListElement(l, nnbody, f, i);
    f++;
  }
  nnbody.reset();
}

bool Api::listApi::find(Atom *a) {
  for (int i = 0; i < top; i++)
    if (atoms[i] == a)
      return true;
  return false;
}
void Api::listApi::pop() { top--; }
void Api::listApi::reset() { top = 0; }

void Api::listApi::reset_repetition() {
  for (int i = 0; i < top; i++) {
    atoms[i]->inRule = UNDEF;
  }
  top = 0;
}

void Api::listApi::reset2() {
  top = 0;
  if (atoms)
    delete[] atoms;
  if (weights)
    delete[] weights;

  atoms = 0;
  weights = 0;
}
void Api::listApi::grow() {
  long sz = size * 2;
  Atom **atom_array = new Atom *[sz];
  Weight *weight_array = new Weight[sz];
  bool *np_array = new bool[sz];
  for (int i = 0; i < size; i++) {
    atom_array[i] = atoms[i];
    weight_array[i] = weights[i];
    np_array[i] = np[i];
  }
  size = sz;
  delete[] atoms;
  atoms = atom_array;
  delete[] weights;
  weights = weight_array;
  delete[] np;
  np = np_array;
}

void Api::listApi::grow(long sz) {

  size = sz;
  delete[] atoms;
  atoms = new Atom *[sz];
  delete[] weights;
  weights = new Weight[sz];
  delete[] np;
  np = new bool[sz];
}

Api::Api(Program *p) : program(p) { init = new Init; }

Api::~Api() { delete init; }

//
// if there are repertitions it rewrites a rule into Respective
// weightrule by summing up the weights
//
// FIXME This is wrong. I should rewrite it.
// void
// Api::checkRepretitions(){
//   assert(type==WEIGHTRULE);
//   int i;
//   //repetition
//   bool rep=false;
//   //negBody
//   for(i=0; i< size(nbody); i++){
// 	if(nbody.atoms[i]->nbInd.size()>1){//repetition
// 	  //if first time repetition we add it to cbody
// 	  rep=true;
// 	  if(i==nbody.atoms[i]->nbInd[0]){
// 		add_Cbody (nbody.atoms[i], false);
// 		int last= size(compNbody);
// 		compNbody.weights[last-1]=0;
// 		for (int j=nbody.atoms[i]->nbInd[0];
// j<nbody.atoms[i]->nbInd.size();j++){ compNbody.weights[last-1]
// += nbody.weights[j];
// 		}
// 	  }
// 	}
// 	else{//there is no repetition
// 	  add_Cbody (nbody.atoms[i], false);
// 	  int last= size(compNbody);
// 	  compNbody.weights[last-1] = nbody.weights[i];
// 	}
//   }

//   //posBody
//   for(i=0; i< size(pbody); i++){
// 	if(pbody.atoms[i]->pbInd.size()>1){//repetition
// 	  //if first time repetition we add it to cbody
// 	  rep=true;
// 	  if(i==pbody.atoms[i]->pbInd[0]){
// 		add_Cbody (pbody.atoms[i], true);
// 		int last= size(compPbody);
// 		compPbody.weights[last-1]=0;
// 		if(type==WEIGHTRULE){
// 		  for (int j=pbody.atoms[i]->pbInd[0];
// j<pbody.atoms[i]->pbInd.size();j++){
// if(type==WEIGHTRULE) 			  compPbody.weights[last-1] +=
// pbody.weights[j];
// 			//	if(type==CONSTRAINTRULE)
// 			//compPbody.weights[last-1] +=1;
// 		  }
// 		}
// 	  }
// 	}
// 	else{//there is no repetition
// 	  add_Cbody (pbody.atoms[i], true);
// 	  int last= size(compPbody);
// 	  if(type==WEIGHTRULE)
// 		compPbody.weights[last-1] = pbody.weights[i];

// 	}
//   }
//   if(!rep){
// 	comp_reset();

// 	return;
//   }

//   if(type==WEIGHTRULE)
// 	type=WEIGHTRULE;
//   pbody.reset();
//   nbody.reset();
//   for(i=0; i< size(compPbody); i++){
// 	add_body(compPbody.atoms[i],true);
// 	pbody.weights[i]=compPbody.weights[i];
//   }
//   for(i=0; i< size(compNbody); i++){
// 	add_body(compNbody.atoms[i],false);
// 	nbody.weights[i]=compNbody.weights[i];
//   }
//   comp_reset();
//   return;
// }

//
// returns true if init is set
// returns false if either weight or cardinality constraints had repetitions
// in such case we added the rule already here
//
void Api::set_init() {

  // if(type==WEIGHTRULE)
  //   checkRepretitions();
  sort(head);
  sort(nbody);
  sort(pbody);

  init->head = head.atoms;
  init->hsize = size(head);
  init->pbody = pbody.atoms;
  init->psize = size(pbody);
  init->pweight = pbody.weights;
  init->nbody = nbody.atoms;
  init->nweight = nbody.weights;
  init->nsize = size(nbody);
  init->atleast_weight = atleast_weight;
  init->atleast_body = atleast_body;
  init->atleast_head = atleast_head;
  init->maximize = maximize;
}

inline long Api::size(listApi &l) { return l.top; }

long Api::sizePbody() { return size(pbody); }
long Api::sizeNbody() { return size(nbody); }
long Api::sizeBody() { return size(nbody) + size(nnbody) + size(pbody); }
long Api::sizeNNbody() { return size(nnbody); }
long Api::sizeHead() {
  // head must always be greater than 0

  return size(head);
}

long Api::sizeCompPbody() { return size(compPbody); }
long Api::sizeCompNbody() { return size(compNbody); }

Weight Api::totalweight(listApi &l) {
  Weight ret = 0;
  for (int i = 0; i < l.top; i++)
    ret += l.weights[i];
  return ret;
}
Atom *Api::new_atom(int id) {
  program->number_of_atoms++;
  Atom *a = new Atom(program);
  a->set_lparse_id(id);
  program->atoms.push_back(a);
  return a;
}

void Api::set_compute(Atom *a, bool pos, bool read) {
  assert(a);
  // FIXME What is the difference between the 0 and non-0 version?
  if (pos) {
    if (read)
      a->setComputeTrue0();
    else
      a->setComputeTrue();
  } else
    a->setComputeFalse();
}

void Api::set_name(Atom *a, const char *s) {
  assert(a);
  if (a->name)
    delete[] a->name;
  if (s) {
    a->name = strcpy(new char[strlen(s) + 1], s);
  } else
    a->name = 0;
}

void Api::begin_rule(RuleType t) {
  type = t;
  atleast_weight = WEIGHT_MIN;
  atleast_body = 0;
  atleast_head = 1;
}

bool Api::headAtomInBody() {

  for (int ip = 0; ip < size(pbody); ip++) {
    for (int ih = 0; ih < size(head); ih++) {
      if (head.atoms[ih] == pbody.atoms[ip])
        return true;
    }
  }
  return false;
}

void Api::end_rule() {
  set_init();
  Rule *r = 0;
  switch (type) {
  case BASICRULE:
    assert(size(head) == 1);
    r = new BasicRule;
    break;
  case CONSTRAINTRULE:
    assert(size(head) == 1);
    r = new ConstraintRule;
    break;
  case DISJUNCTIONRULE:
    throw std::runtime_error(
        "Disjunction rules with multiple head atoms are not supported.");
  case WEIGHTRULE:
    assert(size(head) == 1);
    r = new WeightRule;
    break;
  default:
    break;
  }

  if (r && type != CHOICERULE) {
    program->rules.push_back(r);
    program->number_of_rules++;
    r->init(init);
  }

  if (type == CHOICERULE) {
    // NOTE: Split choices into separate rules
    assert(size(head) >= 1);
    for (int i = 0; i < size(head); i++) {
      if (!pbody.find(init->head[i])) {
        r = new ChoiceRule;
        program->rules.push_back(r);

        program->number_of_rules++;
        r->init(init, i);
      }
    }
  }

  rule_reset_repetition();
}

void Api::rule_reset() {
  pbody.reset();
  nbody.reset();
  nnbody.reset();
  head.reset();
}

void Api::rule_reset_repetition() {
  pbody.reset_repetition();
  nbody.reset_repetition();
  head.reset_repetition();
}

void Api::comp_reset() {
  compNbody.reset();
  compPbody.reset();
}

void Api::head_reset() { head.reset(); }
void Api::body_reset() {
  pbody.reset();
  nbody.reset();
  nnbody.reset();
}

void Api::add_head(Atom *a) {
  assert(a);
  head.push(a);
}

void Api::add_body(Atom *a, bool pos) {
  assert(a);
  if (pos) {
    pbody.push(a);
  } else {
    nbody.push(a);
  }
}
void Api::add_head_repetition(Atom *a) {
  assert(a);
  if (a->inRule != HEAD) {
    a->inRule = HEAD;
    head.push(a);
  }
}

// returns false if HEAD atom appears
// in the PBODY
// or if ATOM in NEG appears In POS
// in such case this rule should not be added to DB
bool Api::add_body_repetition(Atom *a, bool pos, RuleType type) {
  assert(a);

  if (pos) {
    if (a->inRule == HEAD && type != CHOICERULE && type != CONSTRAINTRULE) {
      rule_reset_repetition(); // we reset a rule here since it will not be
                               // added
      return false;
    }
    if (a->inRule == POSB || (a->inRule == HEAD && type == CONSTRAINTRULE))
      return true;
    a->inRule = POSB;
    pbody.push(a);
  } else {
    if (a->inRule == POSB && type != CONSTRAINTRULE) {
      rule_reset_repetition(); // we reset a rule here since it will not be
                               // added
      return false;
    }
    if (a->inRule == NEGB)
      return true;
    a->inRule = NEGB;
    nbody.push(a);
  }

  return true;
}
void Api::add_nnbody(Atom *a) {
  assert(a);
  nnbody.push(a);
}

void Api::pop_body(bool pos) {
  if (pos)
    pbody.pop();
  else
    nbody.pop();
}
void Api::add_Cbody(Atom *a, bool pos) {
  assert(a);
  if (pos)
    compPbody.push(a);
  else
    compNbody.push(a);
}

void Api::add_body(Atom *a, bool pos, Weight w) {
#ifdef USEDOUBLE
  assert(w >= 0);
#endif
  assert(a);
  if (pos)
    pbody.push(a, w);
  else
    nbody.push(a, w);
}

void Api::add_body_LEGACY(Atom *a, Weight w, bool pos)
// FIXME Why does this exist?
{
#ifdef USEDOUBLE
  assert(w >= 0);
#endif
  assert(a);
  pbody.push(a, w, pos);
}

void Api::change_body(long i, bool pos, Weight w) {
#ifdef USEDOUBLE
  assert(w >= 0);
#endif
  if (pos) {
    assert(0 <= i && i < size(pbody));
    pbody.weights[i] = w;
  } else {
    assert(0 <= i && i < size(nbody));
    nbody.weights[i] = w;
  }
}

void Api::set_atleast_weight(Weight w) { atleast_weight = w; }

void Api::set_atleast_body(long n) { atleast_body = n; }

void Api::set_atleast_head(long n) { atleast_head = n; }

Atom *Api::headAtom(long pos) {
  // position must be in the range of an array
  assert(pos >= 0 && pos < size(head));
  return head.atoms[pos];
}

Atom *Api::pbodyAtom(long pos) {
  // position must be in the range of an array
  assert(pos >= 0 && pos < size(pbody));
  return pbody.atoms[pos];
}
Atom *Api::nbodyAtom(long pos) {
  // position must be in the range of an array
  assert(pos >= 0 && pos < size(nbody));
  return nbody.atoms[pos];
}
Atom *Api::nnbodyAtom(long pos) {
  // position must be in the range of an array
  assert(pos >= 0 && pos < size(nnbody));
  return nnbody.atoms[pos];
}

Atom *Api::compPbodyAtom(long pos) {
  // position must be in the range of an array
  assert(pos >= 0 && pos < size(compPbody));
  return compPbody.atoms[pos];
}
Atom *Api::compNbodyAtom(long pos) {
  // position must be in the range of an array
  assert(pos >= 0 && pos < size(compNbody));
  return compNbody.atoms[pos];
}

// sr is true if we sort by rules
// if we sort by bodies it is false
void Api::sortRuleList(vector<Rule *> &rules, long numRules) {
  /*
  CompareValues cmp;
  for(long i=0; i<numRules;i++){
        if(!rules[i]->erase){
          for(long j=i+1; j<numRules;j++){
                if(!rules[j]->erase){
                  cmp=rules[i]->cmpRule(rules[j]);
                  if(cmp==SUPERSET||cmp==EQ){
                        rules[j]->erase=true;
                  }
                  if(cmp==SUBSET){
                        rules[i]->erase=true;
                        break;
                  }

                }

          }
        }
  }
  */

  for (long m = 1; m <= numRules; m *= 2) {
    for (long i = 0; i < numRules - m; i += m * 2) {

      inplace_mergeRuleList(rules, i, i + m,
                            ((i + m * 2) > numRules) ? numRules : (i + m * 2));
    }
  }

  return;
}
// sr is true if we sort by rules
// if we sort by bodies it is false
void Api::sortBodyRuleList(vector<NestedRule *> &rules, long numRules) {

  for (long m = 1; m <= numRules; m *= 2) {
    for (long i = 0; i < numRules - m; i += m * 2) {
      inplace_mergeBodyRuleList(
          rules, i, i + m, ((i + m * 2) > numRules) ? numRules : (i + m * 2));
    }
  }

  return;
}

CompareValues Api::cmpRules(Rule *r1, Rule *r2) {
  if (r1->erase && r2->erase)
    return EQ;
  if (r1->erase)
    return LTH;
  if (r2->erase)
    return GTH;
  return r1->cmpRule(r2);
}

void Api::traverse2cmp(vector<Rule *> &rules, Rule *&rcmp, long from, long to) {

  if (rcmp->erase)
    return;
  CompareValues cmp;
  for (int i = from; i < to; i++) {
    if (!rules[i]->erase) {
      cmp = rcmp->cmpRule(rules[i]);
      if (cmp == SUPERSET || cmp == EQ) {
        rules[i]->erase = true;
      } else {
        if (cmp == SUBSET) {
          rcmp->erase = true;

          //		  assert(cmp!=SUBSET);
        }
        //		assert(cmp!=EQ);
      }
    }
  }
}
void Api::inplace_mergeRuleList(vector<Rule *> &rules, long first, long middle,
                                long last) {

  /*  cout<<"First to Middle List:"<<endl;
  for (int i=first; i<middle;i++){
        if(rules[i]!=0)
          rules[i]->print();
        else
          cout<<" EMPTY "<<endl;


  }
  cout<<"Middle to Last List:"<<endl;
  for (int i=middle; i<last;i++){

        if(rules[i]!=0)
          rules[i]->print();
        else
          cout<<" EMPTY "<<endl;

  }
  */

  int f = first;
  int m = middle;
  CompareValues cmp;
  while (first != m && middle != last) {
    /*	cout<<"Rule 1 ";
        if(rules[first]!=0)
          rules[first]->print();
        else
          cout<<" EMPTY_RULE ";

        cout<<"Rule 2 ";
        if(rules[middle]!=0)
          rules[middle]->print();
        else
          cout<<" EMPTY_RULE ";
*/
    cmp = cmpRules(rules[first], rules[middle]);
    /*	cout<<"Compare ";
        if(cmp==EQ)
                cout<<"EQ";
        if(cmp==SUBSET)
                cout<<"SUB";
        if(cmp==SUPERSET)
                cout<<"SUP";
        if(cmp==GTH)
                cout<<"GTH";
        if(cmp==LTH)
                cout<<"LTH";
        cout<<endl;
*/
    if (cmp == EQ || cmp == SUBSET) {
      if (!rules[first]->erase) {
        //		cout<<"DELETE: ";
        //		rules[first]->print();
        rules[first]->erase = true;
        cmp = LTH;
      }
    } else if (cmp == SUPERSET) {
      //	  cout<<"DELETE: ";
      //	  rules[middle]->print();
      rules[middle]->erase = true;
      cmp = GTH;
    }

    if (cmp != GTH) {
      tempRules.push_back(rules[first]);
      //	  traverse2cmp(rules,rules[first],middle,last);
      first++;
      //	  cout<<"While First ++"<<first;
    } else {
      tempRules.push_back(rules[middle]);
      //	  traverse2cmp(rules,rules[middle],first,m);
      middle++;
      // cout<<"While Middle ++"<<middle;
    }
  }

  if (first != m) {

    for (first; first != m; first++) {
      tempRules.push_back(rules[first]);
    }
  }
  if (middle != last) { // here we need to check is last element
    // of first list subsumes anybody here

    for (first = middle; first != last; first++) {
      tempRules.push_back(rules[first]);
    }
  }
  //  cout<<"Sorted List:"<<endl;
  for (list<Rule *>::iterator ir = tempRules.begin(); ir != tempRules.end();
       ++ir, f++) {
    rules[f] = (*ir);
    /*
        if(rules[f]!=0)
          rules[f]->print();
        else
          cout<<" EMPTY "<<endl;
        */
  }

  tempRules.clear();
}
void Api::inplace_mergeBodyRuleList(vector<NestedRule *> &rules, long first,
                                    long middle, long last) {

  int f = first;
  int m = middle;
  CompareValues cmp;
  while (first != m && middle != last) {
    cmp = rules[first]->cmpBody(rules[middle]);

    if (cmp != GTH) {
      tempNestedRules.push_back(rules[first]);
      first++;
      //	  cout<<"While First ++"<<first;
    } else {
      tempNestedRules.push_back(rules[middle]);
      middle++;
      // cout<<"While Middle ++"<<middle;
    }
  }

  if (first != m) {

    for (first; first != m; first++) {
      tempNestedRules.push_back(rules[first]);
    }
  }
  if (middle != last) { // here we need to check is last element
    // of first list subsumes anybody here

    for (first = middle; first != last; first++) {
      tempNestedRules.push_back(rules[first]);
    }
  }

  for (list<NestedRule *>::iterator ir = tempNestedRules.begin();
       ir != tempNestedRules.end(); ++ir, f++) {
    rules[f] = (*ir);
  }

  tempNestedRules.clear();
}

;
