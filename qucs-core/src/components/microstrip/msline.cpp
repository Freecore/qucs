/*
 * msline.cpp - microstrip transmission line class implementation
 *
 * Copyright (C) 2004 Stefan Jahn <stefan@lkcc.org>
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this package; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.  
 *
 * $Id: msline.cpp,v 1.5 2004-05-06 20:41:17 ela Exp $
 *
 */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "complex.h"
#include "object.h"
#include "node.h"
#include "circuit.h"
#include "component_id.h"
#include "substrate.h"
#include "constants.h"
#include "msline.h"

msline::msline () : circuit (2) {
  type = CIR_MSLINE;
}

void msline::calcSP (nr_double_t frequency) {

  /* how to get properties of this component, e.g. L, W */
  nr_double_t l = getPropertyDouble ("L");
  nr_double_t W = getPropertyDouble ("W");

  /* how to get properties of the substrate, e.g. Er, H */
  substrate * subst = getSubstrate ();
  nr_double_t er    = subst->getPropertyDouble ("er");
  nr_double_t h     = subst->getPropertyDouble ("h");
  nr_double_t t     = subst->getPropertyDouble ("t");
  nr_double_t tand  = subst->getPropertyDouble ("tand");

  /* local variables */
  complex s11, s21, n, g;
  nr_double_t z, y, a, b, ereff, ereffi, k0, Fi, Z0i, Z0, Z0eff, D, ds, Fsr;
  nr_double_t Rs, rho, Wi, Ki, ac, ad, Zi, l0;

  // quasi static effective er
  ereff = (er + 1) / 2 + (er - 1) / 2 / sqrt (1 + 10 * h / W);
  // Kirschning / Jansen
  Fi = 0;
  // Getsinger
  ereffi = er - (er - ereff) / (1 + Fi);

  Z0 = analyseZ0 (W, h, er);
  fprintf (stderr, "Z0 = %g\n", Z0);
  Z0eff = Z0;

  // dispersion: Bianco
  Z0i = Z0 - (Z0 - Z0eff) / (1 + Fi);

  // substrate surface roughness: Hammerstad / Bekkadal
  D   = 0;    // rms surface roughness
  ds  = 1e-6; // skin depth
  Fsr = 1 + M_2_PI * atan (1.4 * (D / ds) * (D / ds));
  // surface resistance
  Rs = sqrt (M_PI * frequency * MU0 * rho);
  // effective width and current distribution factor: Pucel / Gupta
  Wi = W;
  Ki = 0;
  // conductor losses
  ac = Rs / (Zi * Wi) * Ki * Fsr;

  l0 = C / frequency;
  // dielectric losses
  ad = M_PI * er / (er - 1) * (ereffi - 1) / sqrt (ereffi) * tand / l0;

  z = Z0i / z0;
  y = 1 / z;
  k0 = 2 * M_PI * frequency / C;
  a = ac + ad;
  b = sqrt (ereffi) * k0;
  g = rect (a, b);
  n = 2 * cosh (g * l) + (z + y) * sinh (g * l);
  s11 = (z - y) * sinh (g * l) / n;
  s21 = 2 / n;

  setS (1, 1, s11);
  setS (2, 2, s11);
  setS (1, 2, s21);
  setS (2, 1, s21);
}

/* This function returns the quasi static impedance of a microstrip
   line based on the given line width, substrate height and the
   substrate's relative dielectrical constant. */
nr_double_t msline::analyseZ0 (nr_double_t W, nr_double_t h, nr_double_t er) {

  nr_double_t x, xprev, xabs, xrel, d, f, c, z1, z2, z, Hs;

  // valid for Z0 >= 44 - 2 * er
  Hs = log (4 * h / W + sqrt (16 * h * h / W / W + 2));
  z1 = Hs - 0.5 * (er - 1) / (er + 1) * (log (M_PI_2) + log (M_PI_4) / er);
  z1 = z1 * 120 / sqrt (2 * (er + 1));

  // valid for Z0 < 44 - 2 * er
  x = 60 * M_PI * M_PI / (44 - 2 * er) / sqrt (er);
  do {
    xprev = x;
    c = W / h - (er - 1) / M_PI / er * (0.293 - 0.517 / er);
    d = 1 - 2 / (2 * x - 1) + (er - 1) / 2 / er / (x - 1);
    f = x - 1 - log (2 * x - 1) + (er - 1) / 2 / er * log (x - 1) - M_PI_2 * c;
    x = x - f / d;
    xabs = abs (x - xprev);
    xrel = xabs / x;
  } while (xabs > 1e-12 || xrel > 1e-6);
  z2 = 60 * M_PI * M_PI / x / sqrt (er);

  // finally decide
  if (z1 >= 44 - 2 * er && z2 >= 44 - 2 * er)
    z = z1;
  else
    z = z2;
  return z;
}
