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

#ifndef _LIBTIDAL_H_
#define _LIBTIDAL_H_

#define LIBTIDAL_MEAN_LONG_SUN 0
#define LIBTIDAL_MEAN_LONG_MOON 1
#define LIBTIDAL_MEAN_LONG_SOLAR_PERIGEE 2
#define LIBTIDAL_MEAN_LONG_LUNAR_PERIGEE 3
#define LIBTIDAL_MEAN_LONG_ASCENDING_NODE 4

#define LIBTIDAL_FACTOR_LENGTH 5
#define LIBTIDAL_FACTOR_WIDTH 4

struct _libtidal_astronomical {
 double l; // local latitude (for reference)
 double t; // current phase
 double h; // mean longitude of the sum
 double pp; // mean longitude of the solar perigee
 double s; // mean longitude of the moon
 double p; // mean longitude of the lunar perigee
 double np; // negative of the longitude of the mean ascending node

 double dt; // rate of change of current phase
 double dh; // rate of change of mean longitude of the sum
 double dpp; // rate of change of mean longitude of the solar perigee
 double ds; // rate of change of mean longitude of the moon
 double dp; // rate of change of mean longitude of the lunar perigee
 double dnp; // rate of change of negative of the longitude of the mean ascending node
};

void libtidal_astronomical(struct _libtidal_astronomical *a, double at, double latitude, double zone);

#define LIBTIDAL_CHARLEN 32
#define LIBTIDAL_DOODLEN 6
#define LIBTIDAL_MAXSATS 32
#define LIBTIDAL_MAXPARTS 4

#define LIBTIDAL_CONSTITUENTS 45
#define LIBTIDAL_SHALLOWS 101

struct _libtidal_constituent {
  char name[LIBTIDAL_CHARLEN];
  int doodson[LIBTIDAL_DOODLEN];
  float semi;
  struct _tide_satellite {
    int doodson[LIBTIDAL_DOODLEN];
    float phase;
    float ratio;
    int corr;
  } satellites[LIBTIDAL_MAXSATS];
};

struct _libtidal_shallow {
  char name[LIBTIDAL_CHARLEN];
  int nparts;
  struct _tide_part {
    char name[LIBTIDAL_CHARLEN];
    float coef;
  } parts[LIBTIDAL_MAXPARTS];
};

struct _libtidal_factors {
	double V, F, u, f; /* astronomical calculations */
};

int libtidal_constituents(char *name, struct _libtidal_astronomical *astro, struct _libtidal_factors *factors);
int libtidal_shallows(char *name, struct _libtidal_astronomical *astro, struct _libtidal_factors *factors);

#ifndef LIBTIDAL_CHARLEN
#  define LIBTIDAL_CHARLEN 32
#endif /* LIBTIDAL_CHARLEN */

#ifndef LIBTIDAL_MAX_CONSTITUENTS
#define LIBTIDAL_MAX_CONSTITUENTS 146
#endif /* LIBTIDAL_MAX_CONSTITUENTS */

struct _tidal_ {
	char name[LIBTIDAL_CHARLEN];
	double amplitude;
	double lag;
};
typedef struct _tidal_ tidal_t;

int libtidal_height(int ntides, tidal_t *tides, double at, double lat, double zone, double *height);
int libtidal_heights(int ntides, tidal_t *tides, double start, double lat, double zone, int nheights, double delta, double *heights);

#endif /* _LIBTIDAL_H_ */
