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

/* local includes */
#include "libtidal.h"

int libtidal_constituent(char *name, struct _libtidal_astronomical *astro, double offset, double amplitude, double lag, double *height) {
  struct _libtidal_factors factors;

  if (libtidal_constituents(name, astro, &factors) < 0)
    return -1;

  (*height) = factors.f * amplitude * cos(2.0 * M_PI * (factors.V + factors.F * offset + factors.u - lag));

  // done ...
  return 0;
}

static int libtidal_shallow(char *name, struct _libtidal_astronomical *astro, double offset, double amplitude, double lag, double *height) {
  struct _libtidal_factors factors;

  if (libtidal_shallows(name, astro, &factors) < 0)
    return -1;

  // the answer ...
  (*height) = factors.f * amplitude * cos(2.0 * M_PI * (factors.V + factors.F * offset + factors.u - lag));

  // done
  return 0;
}

int libtidal_height(int ntides, tidal_t *tides, double at, double lat, double zone, double *height) {
  int part;

  double _height = 0.0;
  tidal_t *tide = NULL;
  struct _libtidal_astronomical astro;

  // clear the decks ...
  (*height) = 0.0;

  // find the astronomical calculations
  libtidal_astronomical(&astro, at, lat, zone);

  // run through each constituent 
  for (part = 0, tide = tides; part < ntides; part++, tide++) {
    if (libtidal_constituent(tide->name, &astro, 0.0, tide->amplitude, tide->lag, &_height) == 0)
      (*height) += _height;
    else if (libtidal_shallow(tide->name, &astro, 0.0, tide->amplitude, tide->lag, &_height) == 0)
      (*height) += _height;
    else
      return -1;
  }

  // done
  return 0;
}

int libtidal_heights(int ntides, tidal_t *tides, double start, double lat, double zone, int nheights, double delta, double *heights) {
  int part, step;

  double _height = 0.0;
  tidal_t *tide = NULL;
  struct _libtidal_astronomical astro;

  /* find the astronomical calculation mid way .... */
  libtidal_astronomical(&astro, start + ((double) (nheights - 1) / 2.0) * delta, lat, zone);

  // run through each requested time
  for (step = 0; step < nheights; step++) {
    heights[step] = 0.0;
    // run through each constituent 
    for (part = 0, tide = tides; part < ntides; part++, tide++) {
      if (libtidal_constituent(tide->name, &astro, (((double) step) + ((double) (nheights - 1) / 2.0)) * delta, tide->amplitude, tide->lag, &_height) == 0)
        heights[step] += _height;
      else if (libtidal_shallow(tide->name, &astro, (((double) step) + ((double) (nheights - 1) / 2.0)) * delta, tide->amplitude, tide->lag, &_height) == 0)
        heights[step] += _height;
      else
        return -1;
    }
  }

  // done
  return 0;
}
