/*
 * Copyright (c) 2009 Institute of Geological & Nuclear Sciences Ltd.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <stdio.h>
#include <math.h>
#include <time.h>

#include "libtidal.h"

static double factors[LIBTIDAL_FACTOR_LENGTH][LIBTIDAL_FACTOR_WIDTH] = {
  {279.696678, 0.9856473354, 0.00002267, 0.0},
  {270.434164, 13.1763965268, -0.000085, 0.000000039},
  {281.220833, 0.0000470684, 0.0000339, 0.00000007},
  {334.329556, 0.1114040803, -0.0007739, -0.00000026},
  {-259.183275, 0.0529539222, -0.0001557, -0.00000005},
};

static double leftover(double x) {
  return (double) (x - ((x < 0.0) ? ceil(x) : floor(x)));
}
static double residual(double x) {
  return leftover(x / 360.0);
}
static double calc(double *parts, double x) {
  return parts[0] + parts[1] * x + 1.0e-08 * parts[2] * x * x + 1.0e-12 * parts[3] * x * x * x;
}
static double dcalc(double *parts, double x) {
  return parts[1] + 2.0e-08 * parts[2] * x + 3.0e-12 * parts[3] * x * x;
}

void libtidal_astronomical(struct _libtidal_astronomical *a, double at, double latitude, double zone) {

  double days;

  // days since midday Dec 31, 1899 GMT
  days = at / (24.0 * 60.0 * 60.0) + 25567.5;

  // adjust for time zones ...
  days -= (zone / 24.0);

  // reference storage
  a->l = latitude;

  // astronimical calculations ...
  a->h = residual(calc(factors[LIBTIDAL_MEAN_LONG_SUN], days));
  a->s = residual(calc(factors[LIBTIDAL_MEAN_LONG_MOON], days));
  a->pp = residual(calc(factors[LIBTIDAL_MEAN_LONG_SOLAR_PERIGEE], days));
  a->p = residual(calc(factors[LIBTIDAL_MEAN_LONG_LUNAR_PERIGEE], days));
  a->np = residual(calc(factors[LIBTIDAL_MEAN_LONG_ASCENDING_NODE], days));

  // mumbo jumbo ...
  a->t = leftover(days - 0.5) + a->h - a->s;

  //  astronimical rates of change ...
  a->dh = dcalc(factors[LIBTIDAL_MEAN_LONG_SUN], days) / 360.0;
  a->ds = dcalc(factors[LIBTIDAL_MEAN_LONG_MOON], days) / 360.0;
  a->dpp = dcalc(factors[LIBTIDAL_MEAN_LONG_SOLAR_PERIGEE], days) / 360.0;
  a->dp = dcalc(factors[LIBTIDAL_MEAN_LONG_LUNAR_PERIGEE], days) / 360.0;
  a->dnp = dcalc(factors[LIBTIDAL_MEAN_LONG_ASCENDING_NODE], days) / 360.0;

  // time difference offset ...
  a->dt = 1.0 + a->dh - a->ds;
}
