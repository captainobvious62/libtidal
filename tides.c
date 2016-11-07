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

/* system includes */
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <time.h>
#include <math.h>

/* local includes */
#include "libtidal.h"

/*
 * tides: predict tidal heights ...
 *
 */

#define PROGRAM "tides"

#ifndef PACKAGE_VERSION
#define PACKAGE_VERSION "xxx"
#endif

/* program variables */
char *program_name = PROGRAM;
char *program_version = PROGRAM " (" PACKAGE_VERSION ") (c) GNS 2009 (m.chadwick@gns.cri.nz)";
char *program_usage = PROGRAM " [-hv][-u][-d <datum>][-l <lat>][-z <zone>][-c <name>/<amp>/<lag>][-f <from>][-t <to>][-s <step>]";

int main(int argc, char **argv) {
  int i;
  double dh;
  int sgn, last;

  int rc = 0;
  char *c = NULL;
  struct tm time_str;
  char timestr[256];

  int year, mon, mday;
  int hour, min, sec;

  double t;
  time_t at;

  char *from = NULL;
  char *to = NULL;

  double tide; /* calculated value */
  double start = 0.0; /* epoch time */
  double end = 0.0; /* epoch time */

  int high = 0; /* high/low tides */
  int verbose = 0; /* talkative */
  double zone = 0.0; /* time zone offset */
  double lat = 0.0; /* site latitude */
  double datum = 0.0; /* height offset */
  double step = 60.0; /* once a minute */

  int ntides = 0;
  tidal_t tides[LIBTIDAL_MAX_CONSTITUENTS];

  double *heights = NULL;

  while ((rc = getopt(argc, argv, "hvmd:l:z:c:f:t:i:")) != EOF) {
    switch(rc) {
    case '?':
      (void) fprintf(stderr, "usage: %s\n", program_usage);
      exit(-1); /*NOTREACHED*/
    case 'h':
      (void) fprintf(stderr, "\n%s: process MSEED tidal data\n\n", program_name);
      (void) fprintf(stderr, "usage:\n\t%s\n", program_usage);
      (void) fprintf(stderr, "version:\n\t%s\n", program_version);
      (void) fprintf(stderr, "options:\n");
      (void) fprintf(stderr, "\t-h\tcommand line help (this)\n");
      (void) fprintf(stderr, "\t-v\trun program in verbose mode\n");
      (void) fprintf(stderr, "\t-d\tprovide an alternative datum [%g m]\n", datum);
      (void) fprintf(stderr, "\t-l\tprovide a tidal constituent site latitude [%g]\n", lat);
      (void) fprintf(stderr, "\t-z\tprovide a tidal constituent time zone correction [%g]\n", zone);
      (void) fprintf(stderr, "\t-c\tprovide a tidal constituent [<name>/<amp>/<lag>]\n");
      (void) fprintf(stderr, "\t-f\tprovide a start date [%s]\n", (from) ? from : "<now>");
      (void) fprintf(stderr, "\t-t\tprovide an end date [%s]\n", (to) ? to : "<now>");
      (void) fprintf(stderr, "\t-i\toutput interval [%gs]\n", step);
      (void) fprintf(stderr, "\t-m\toutput high/low tides\n");
      exit(0); /*NOTREACHED*/
    case 'v':
      verbose++;
      break;
    case 'm':
      high++;
      break;
    case 'f':
      from = optarg;
      break;
    case 't':
      to = optarg;
      break;
    case 'd':
      datum = atof(optarg);
      break;
    case 'l':
      lat = atof(optarg);
      break;
    case 'z':
      zone = atof(optarg);
      break;
    case 'c':
      if (ntides < LIBTIDAL_MAX_CONSTITUENTS) {
        c = strtok(optarg, "/");
        memset(&tides[ntides], 0, sizeof(tidal_t));
        strncpy(tides[ntides].name, c, LIBTIDAL_CHARLEN - 1);
        tides[ntides].amplitude = atof(strtok(NULL, "/"));
        tides[ntides].lag = atof(strtok(NULL, "/")) / 360.0;
        ntides++;
      }
      break;
    case 'i':
      step = atof(optarg);
      break;
    }
  }

  /* perhaps send out the version information */
  if (verbose)
    fprintf(stderr, "%s\n", program_version);

  if (from != NULL) {
    rc = sscanf (from, "%d%*[-/:.]%d%*[-/:.]%d%*[-/:.T ]%d%*[-/:.]%d%*[- /:.]%d",
      &year, &mon, &mday, &hour, &min, &sec);
    memset(&time_str, 0, sizeof(struct tm));
    if (rc > 0)
      time_str.tm_year = year - 1900;
    if (rc > 1)
      time_str.tm_mon = mon - 1;
    if (rc > 2)
      time_str.tm_mday = mday;
    if (rc > 3)
      time_str.tm_hour = hour;
    if (rc > 4)
      time_str.tm_min = min;
    if (rc > 5)
      time_str.tm_sec = sec;
    time_str.tm_isdst = -1;

    start = (double) timegm(&time_str);
  }

  if (to != NULL) {
    rc = sscanf (to, "%d%*[-/:.]%d%*[-/:.]%d%*[-/:.T ]%d%*[-/:.]%d%*[- /:.]%d",
      &year, &mon, &mday, &hour, &min, &sec);
    memset(&time_str, 0, sizeof(struct tm));
    if (rc > 0)
      time_str.tm_year = year - 1900;
    if (rc > 1)
      time_str.tm_mon = mon - 1;
    if (rc > 2)
      time_str.tm_mday = mday;
    if (rc > 3)
      time_str.tm_hour = hour;
    if (rc > 4)
      time_str.tm_min = min;
    if (rc > 5)
      time_str.tm_sec = sec;
    time_str.tm_isdst = -1;

    end = (double) timegm(&time_str);
  }

  if ((from == NULL) && (to == NULL))
    start = end = (double) time((time_t *) 0);
  else if (from == NULL)
    start = (double) time((time_t *) 0);
  else if (to == NULL)
    end = (double) time((time_t *) 0);

  if (high) {
    if ((heights = (double *) malloc(sizeof(double) * (int) rint(step))) == NULL)
      return -1;

    for (t = start; (t < end) || ((t == start) && (start == end)); t += step) {
      if (libtidal_heights(ntides, tides, t, lat, zone, (int) rint(step), 1.0, heights) < 0) {
        at = (time_t) rint(t);
        strftime(timestr, sizeof(timestr), "%Y-%m-%dT%H:%M:%S", gmtime(&at));
        fprintf(stderr, "unable to predict tide: %s\n", timestr); continue;
      }
      for (i = 1; i < (int) rint(step); i++) {
        dh = (heights[i] - heights[i - 1]);
        sgn = ((dh < 0.0) ? -1 : ((dh >= 0.0) ? 1 : 0));
        if ((i > 1) && (sgn != 0) && (sgn != last)) {
          at = (time_t) rint(t) + (time_t) (i - 1);
          strftime(timestr, sizeof(timestr), "%Y-%m-%dT%H:%M:%S", gmtime(&at));
          fprintf(stdout, "%s %10.6f %s\n", timestr, datum + heights[i - 1], (sgn < 0) ? "H" : "L");
        }
        last = ((sgn != 0) ? sgn : last);
      }
    }
  
    free((char *)heights);
  }
  else {
    /* run through each requested time ... */
    for (t = start; (t < end) || ((t == start) && (start == end)); t += step) {
      at = (time_t) rint(t);
      if (libtidal_height(ntides, tides, t, lat, zone, &tide) < 0) {
        at = (time_t) rint(t);
        strftime(timestr, sizeof(timestr), "%Y-%m-%dT%H:%M:%S", gmtime(&at));
        fprintf(stderr, "unable to predict tide: %s\n", timestr); continue;
      }
      strftime(timestr, sizeof(timestr), "%Y-%m-%dT%H:%M:%S", gmtime(&at));
      fprintf(stdout, "%s %10.6f\n", timestr, datum + tide);
    }
  }

  return(0);
}
