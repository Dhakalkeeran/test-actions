/*
 * File defines.h
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
#ifndef DEFINES_H
#define DEFINES_H

#define CMODELS_VERSION "1"

typedef enum { NOT_DEF, POS, NEG } InClause;
typedef enum { LTH, EQ, SUBSET, SUPERSET, GTH } CompareValues;

typedef enum { UNDEF, POSB, NEGB, HEAD } InRule;
typedef enum {
  ENDRULE,
  BASICRULE, // Ex: x :- y, not z.
  CONSTRAINTRULE,
  CHOICERULE, // Ex: { x } :- y.
  GENERATERULE,
  WEIGHTRULE,   // Ex:
  OPTIMIZERULE, // not supported
  SOMETHING,
  //
  DISJUNCTIONRULE, // Same as aspif disjunction, but with head > 1
  NESTED           // internal to cmodels
} RuleType;
typedef enum { AND, OR, EQUIV, IMPL } Connector;
typedef enum {
  //  RELSAT1, is no longer supporte
  RELSAT,
  SIMO,
  ZCHAFF,
  ASSAT_ZCHAFF,
  BCIRCUIT,
  MINISAT,
  MINISAT1,
  DIMACS_PRODUCE,
  CASP_DIMACS_PRODUCE,
  LEVEL_RANKING,
  SCC_LEVEL_RANKING,
  SCC_LEVEL_RANKING_STRONG,
  LEVEL_RANKING_STRONG
  // MCHAFF  is no longer supported
} SolverType;

typedef enum { CVC4, CVC5, Z3, YICES } SMTSolverCommand;

typedef enum { UNSAT = -1, UNKNOWN = 0, SAT = 1 } Result;
typedef enum {
  TIGHT = 1,
  NONDISJ = 2,
  MIN = 3,
  MINSCC = 4,
  HCF = 5
} VerifyMethod;

typedef enum {
  STANDARD = 0,
  ASPARAGUS = 1,
  ASPCOMP2 = 2,
  SILENT = 3
} OutputMethod;

//#ifdef USEDOUBLE
// typedef double Weight;
//#define WEIGHT_MAX DBL_MAX
//#define WEIGHT_MIN -DBL_MAX
//#else
typedef long Weight;
#define WEIGHT_MAX LONG_MAX
#define WEIGHT_MIN -LONG_MAX
//#endif

#endif
