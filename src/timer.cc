/*
 * File timer.cc
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
#include "timer.h"
#include <iomanip>
#include <limits.h>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
Timer::Timer() {
  prevTime = 0.0;
  sec = 0;
  msec = 0;
  on = false;
}

void Timer::start() {
  on = true;
  timer = clock();
}

void Timer::stop() {
  if (on) {
    on = false;
    clock_t ticks = clock() - timer;
    sec += ticks / CLOCKS_PER_SEC;
    if ((ticks - sec * CLOCKS_PER_SEC) > 0)
      msec += (ticks - sec * CLOCKS_PER_SEC) * 1000 / CLOCKS_PER_SEC;
    if (msec >= 1000) {
      msec -= 1000;
      sec++;
    }
  }
}

void Timer::reset() {
  prevTime = 0.0;
  sec = 0;
  msec = 0;
  on = false;
}

char *Timer::print() {
  static char str[256];
  strcpy(str, "\0");
  sprintf(str, "%d.%.2d", sec, msec);

  return str;
  // return "\0";
}

#ifdef _WIN32 // marcy
#include <ctime>
float Timer::Epoch() { return (ru = ((double)clock() / CLOCKS_PER_SEC)); }

void Timer::Start() {
  if (!on) {
    ru1 = Epoch();
    on = true;
  }
}

float Timer::Elapsed() {
  if (on) {
    static float t;
    t = Epoch() - ru1;
    prevTime += t;
    on = false;
    return t;
  } else
    return 0.0;
}

void Timer::SetTimeout(int seconds, void (*handler)(int)) {
  fprintf(stderr, "WARNING: SetTimeout NOT SUPPORTED UNDER WINDOWS!\n");
}

#else

float Timer::Epoch() {
  getrusage(RUSAGE_SELF, &ru);
  return (ru.ru_utime.tv_sec + ru.ru_utime.tv_usec / 1.0e6 +
          ru.ru_stime.tv_sec + ru.ru_stime.tv_usec / 1.0e6);
}

void Timer::Start() {
  if (!on) {
    getrusage(RUSAGE_SELF, &ru1);
    on = true;
  }
}

float Timer::Elapsed() {
  if (on) {
    static float t;
    getrusage(RUSAGE_SELF, &ru2);
    t = ru2.ru_utime.tv_usec / 1.0e6 - ru1.ru_utime.tv_usec / 1.0e6;
    t += ru2.ru_utime.tv_sec - ru1.ru_utime.tv_sec;
    prevTime += t;
    on = false;
    return t;
  } else
    return 0.0;
}

void Timer::SetTimeout(int seconds, void (*handler)(int)) {
  getrlimit(RLIMIT_CPU, &r);
  r.rlim_cur = seconds;
  setrlimit(RLIMIT_CPU, &r);
  signal(SIGXCPU, handler);
}

#endif
