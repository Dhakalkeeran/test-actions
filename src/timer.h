/*
 * File timer.h
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
#ifndef TIMER_H
#define TIMER_H
#include <cstring>
#include <sys/time.h>
#include <time.h>
#ifndef __MINGW32__ // marcy
#include <sys/resource.h>
#endif
#include <signal.h>
#include <unistd.h>

class Timer {
public:
  float prevTime;
  long sec;
  long msec;
  clock_t timer;
  bool on;
  Timer();
  ~Timer(){};
  void start();
  void stop();
  void reset();
  char *print();
#ifdef __MINGW32__ // marcy
  float ru1, ru2, ru;
  int r;
#else
  struct rusage ru1, ru2, ru;
  struct rlimit r;
#endif
  float Epoch();
  void Start();
  float Elapsed();
  void SetTimeout(int seconds, void (*handler)(int));
};

#endif
