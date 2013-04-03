/*
 * ctline.h - ideal coupled transmission line class definitions
 *
 * Copyright (C) 2011 Michael Margraf <michael.margraf@alumni.tu-berlin.de>
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
 * the Free Software Foundation, Inc., 51 Franklin Street - Fifth Floor,
 * Boston, MA 02110-1301, USA.  
 *
 * $Id: ctline.h 1825 2011-03-11 20:42:14Z ela $
 *
 */

#ifndef __CTLINE_H__
#define __CTLINE_H__

class ctline : public circuit
{
 public:
  CREATOR (ctline);
  void calcSP (nr_double_t);
  void initDC (void);
  void initAC (void);
  void calcAC (nr_double_t);
  void calcNoiseAC (nr_double_t);
  void calcNoiseSP (nr_double_t);
};

#endif /* __CTLINE_H__ */
