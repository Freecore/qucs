/***************************************************************************
                          rectdiagram.cpp  -  description
                             -------------------
    begin                : Thu Oct 2 2003
    copyright            : (C) 2003 by Michael Margraf
    email                : michael.margraf@alumni.tu-berlin.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "rectdiagram.h"
#include "main.h"

#include <math.h>

#include <qmessagebox.h>


RectDiagram::RectDiagram(int _cx, int _cy) : Diagram(_cx, _cy)
{
  x1 = 10;     // position of label text
  y1 = 32;
  x2 = 240;    // initial size of diagram
  y2 = 160;
  Name = "Rect";

  xAxis.log = ylAxis.log = yrAxis.log = false;
  calcDiagram();
}

RectDiagram::~RectDiagram()
{
}

// ------------------------------------------------------------
void RectDiagram::calcCoordinate(double x, double yr, double yi,
				 int *px, int *py)
{
  if(xAxis.log)
    *px = int(log10(x / xAxis.low)/log10(xAxis.up / xAxis.low)
		*double(x2) + 0.5);
  else  *px = int((x-xAxis.low)/(xAxis.up-xAxis.low)*double(x2) + 0.5);

  if(ylAxis.log)
    *py = int(log10(sqrt(yr*yr + yi*yi)/fabs(ylAxis.low)) /
		log10(ylAxis.up/ylAxis.low) * double(y2) + 0.5);
  else {
    if(fabs(yi) < 1e-250)  // preserve negative values if not complex number
      *py = int((yr-ylAxis.low)/(ylAxis.up-ylAxis.low)*double(y2) + 0.5);
    else   // calculate magnitude of complex number
      *py = int((sqrt(yr*yr + yi*yi)-ylAxis.low)/(ylAxis.up-ylAxis.low)
		*double(y2) + 0.5);
  }
}

// --------------------------------------------------------------
bool RectDiagram::calcDiagram()
{
  if(!Lines.isEmpty()) Lines.clear();
  if(!Texts.isEmpty()) Texts.clear();
  if(!Arcs.isEmpty()) Arcs.clear();

  if(fabs(xAxis.max-xAxis.min) < 1e-200) {
    xAxis.max += fabs(xAxis.max);
    xAxis.min -= fabs(xAxis.min);
  }
  if(fabs(ylAxis.max-ylAxis.min) < 1e-200) {
    ylAxis.max += fabs(ylAxis.max);
    ylAxis.min -= fabs(ylAxis.min);
  }
  if(ylAxis.max == 0) if(ylAxis.min == 0) {
    ylAxis.max = 1;
    ylAxis.min = -1;
  }
  if(xAxis.max == 0) if(xAxis.min == 0) {
    xAxis.max = 1;
    xAxis.min = -1;
  }
  xAxis.low  = xAxis.min;  xAxis.up  = xAxis.max;
  ylAxis.low = ylAxis.min; ylAxis.up = ylAxis.max;


  int z=0;
  double numGrids, Base, Expo, GridStep, corr, zD, zDstep, GridNum;

  QSize r;
  QString tmp;
  QFontMetrics  metrics(QucsSettings.font);
  int maxWidth = 0;
  bool set_log = false;

  // ====  x grid  =======================================================
if(xAxis.log) {
  if(xAxis.max*xAxis.min <= 0.0) goto Frame;  // invalid
  if(xAxis.max < 0.0) {
    corr = xAxis.min;
    xAxis.min = -xAxis.max;
    xAxis.max = -corr;
    corr = xAxis.low;
    xAxis.low = -xAxis.up;
    xAxis.up  = -corr;
    set_log = true;
  }

  Expo = floor(log10(xAxis.max));
  Base = xAxis.max/pow(10.0,Expo);
  if(Base > 3.0001) xAxis.up = pow(10.0,Expo+1.0);
  else  if(Base < 1.0001) xAxis.up = pow(10.0,Expo);
	else xAxis.up = 3.0 * pow(10.0,Expo);

  Expo = floor(log10(xAxis.min));
  Base = xAxis.min/pow(10.0,Expo);
  if(Base < 2.999) xAxis.low = pow(10.0,Expo);
  else  if(Base > 9.999) xAxis.low = pow(10.0,Expo+1.0);
	else xAxis.low = 3.0 * pow(10.0,Expo);

  corr = double(x2) / log10(xAxis.up / xAxis.low);

  zD = xAxis.low;
  zDstep = pow(10.0,Expo);
  if(set_log) z = x2;
  while((z <= x2) && (z >= 0)) {    // create all grid lines
    if(GridOn)  if(z < x2)  if(z > 0)
      Lines.append(new Line(z, y2, z, 0, GridPen));  // x grid

    if((zD < 1.5*zDstep) || (z == 0) || (z == x2)) {
      if(fabs(Expo) < 3.0)  tmp = StringNum(zD);
      else  tmp = StringNum(zD, 'e', 1);

      if(set_log)  Texts.append(new Text(z-10, -5, '-'+tmp));
      else  Texts.append(new Text(z-10, -5, tmp));

      Lines.append(new Line(z, 5, z, -5, QPen(QPen::black,0)));  // x marks
    }

    zD += zDstep;
    if(zD > 9.5*zDstep)  zDstep *= 10.0;
    z = int(corr*log10(zD / xAxis.low) + 0.5); // int(..) implies floor(..)
    if(set_log)  z = x2 - z;
  }
  if(set_log) {   // set back values ?
    corr = xAxis.min;
    xAxis.min = -xAxis.max;
    xAxis.max = -corr;
    corr = xAxis.low;
    xAxis.low = -xAxis.up;
    xAxis.up  = -corr;
  }
}
else {  // not logarithmical
  numGrids = floor(double(x2)/60.0);   // minimal grid is 60 pixel
  if(numGrids < 1.0) Base = xAxis.max-xAxis.min;
  else Base = (xAxis.max-xAxis.min)/numGrids;
  Expo = floor(log10(Base));
  Base = Base/pow(10.0,Expo);         // separate first significant digit
  if(Base < 3.5) {      // use only 1, 2 and 5, which ever is best fitted
    if(Base < 1.5) Base = 1.0;
    else Base = 2.0;
  }
  else {
    if(Base < 7.5) Base = 5.0;
    else { Base = 1.0; Expo++; }
  }
  GridStep = Base * pow(10.0,Expo); // grid distance (real coordinates)
  corr = floor((xAxis.max-xAxis.min)/GridStep - numGrids);
  if(corr < 0.0) corr++;
  numGrids += corr;     // correct rounding faults


  // upper x boundery ...........................
  zD = fabs(fmod(xAxis.max, GridStep)); // expand grid to upper diagram edge ?
  GridNum = zD/GridStep;
  if((1.0-GridNum) < 1e-10) GridNum = 0.0;  // fix rounding errors
  if(xAxis.max <= 0.0) {
    if(GridNum < 0.3) { xAxis.up += zD;  zD = 0.0; }
  }
  else  if(GridNum > 0.7)  xAxis.up += GridStep-zD;
        else if(GridNum < 0.1)
	       if(GridNum*double(x2) >= 1.0)// more than 1 pixel above ?
		 xAxis.up += 0.3*GridStep;  // beauty correction


  // lower x boundery ...........................
  zD = fabs(fmod(xAxis.min, GridStep)); // expand grid to lower diagram edge ?
  GridNum = zD/GridStep;
  if((1.0-GridNum) < 1e-10) zD = GridNum = 0.0;  // fix rounding errors
  if(xAxis.min <= 0.0) {
    if(GridNum > 0.7) { xAxis.low -= GridStep-zD;  zD = 0.0; }
    else if(GridNum < 0.1)
	   if(GridNum*double(x2) >= 1.0) { // more than 1 pixel above ?
	     xAxis.low -= 0.3*GridStep;    // beauty correction
	     zD += 0.3*GridStep;
	   }
  }
  else {
    if(GridNum > 0.3) {
      zD = GridStep-zD;
      if(GridNum > 0.9) {
	if((1.0-GridNum)*double(x2) >= 1.0) { // more than 1 pixel above ?
	  xAxis.low -= 0.3*GridStep;    // beauty correction
	  zD += 0.3*GridStep;
	}
      }
    }
    else { xAxis.low -= zD;  zD = 0.0; }
  }


  zDstep = GridStep/(xAxis.up-xAxis.low)*double(x2); // grid distance in pixel
  GridNum  = xAxis.low + zD;
  zD /= (xAxis.up-xAxis.low)/double(x2);

  zD += 0.5;     // perform rounding
  z = int(zD);   //  "int(...)" implies "floor(...)"
  while(z <= x2) {    // create all grid lines
    if(fabs(GridNum) < 0.01*pow(10.0, Expo)) GridNum = 0.0;// make 0 really 0
    if(fabs(Expo) < 3.0)
      Texts.append(new Text(z-10, -5, StringNum(GridNum)));
    else
      Texts.append(new Text(z-10, -5, StringNum(GridNum, 'e', 1)));
    GridNum += GridStep;

    if(GridOn)  if(z < x2)  if(z > 0)
      Lines.append(new Line(z, y2, z, 0, GridPen)); // x grid
    Lines.append(new Line(z, 5, z, -5, QPen(QPen::black,0)));   // x marks
    zD += zDstep;
    z = int(zD);
  }
} // of "if(xlog) ... else ..."



  // ====  y grid  =======================================================
  set_log = false;
if(ylAxis.log) {
  if(ylAxis.max*ylAxis.min <= 0.0) goto Frame;  // invalid
  if(ylAxis.max < 0.0) {
    corr = ylAxis.min;
    ylAxis.min = -ylAxis.max;
    ylAxis.max = -corr;
    corr = ylAxis.low;
    ylAxis.low = -ylAxis.up;
    ylAxis.up  = -corr;
    set_log = true;
  }

  Expo = floor(log10(ylAxis.max));
  Base = ylAxis.max/pow(10.0,Expo);
  if(Base > 3.0001) ylAxis.up = pow(10.0,Expo+1.0);
  else  if(Base < 1.0001) ylAxis.up = pow(10.0,Expo);
	else ylAxis.up = 3.0 * pow(10.0,Expo);

  Expo = floor(log10(ylAxis.min));
  Base = ylAxis.min/pow(10.0,Expo);
  if(Base < 2.999) ylAxis.low = pow(10.0,Expo);
  else  if(Base > 9.999) ylAxis.low = pow(10.0,Expo+1.0);
	else ylAxis.low = 3.0 * pow(10.0,Expo);

  corr = double(y2) / log10(ylAxis.up / ylAxis.low);

  z = 0;
  zD = ylAxis.low;
  zDstep = pow(10.0,Expo);
  if(set_log) z = y2;
  while((z <= y2) && (z >= 0)) {    // create all grid lines
    if(GridOn)  if(z < y2)  if(z > 0)
      Lines.append(new Line(0, z, x2, z, GridPen));  // y grid

    if((zD < 1.5*zDstep) || (z == 0)) {
      if(fabs(Expo) < 3.0)  tmp = StringNum(zD);
      else  tmp = StringNum(zD, 'e',1);
      if(set_log)  tmp = '-'+tmp;

      r = metrics.size(0, tmp);  // width of text
      if(maxWidth < r.width()) maxWidth = r.width();
      Texts.append(new Text(-r.width()-7, z+6, tmp));  // text aligned right

      Lines.append(new Line(-5, z, 5, z, QPen(QPen::black,0)));  // y marks
    }

    zD += zDstep;
    if(zD > 9.5*zDstep)  zDstep *= 10.0;
    z = int(corr*log10(zD / ylAxis.low) + 0.5); // int(..) implies floor(..)
    if(set_log)  z = y2 - z;
  }
  if(set_log) {   // set back values ?
    corr = ylAxis.min;
    ylAxis.min = -ylAxis.max;
    ylAxis.max = -corr;
    corr = ylAxis.low;
    ylAxis.low = -ylAxis.up;
    ylAxis.up  = -corr;
  }
}
else {  // not logarithmical
  numGrids = floor(double(y2)/60.0);   // minimal grid is 60 pixel
  if(numGrids < 1.0) Base = ylAxis.max-ylAxis.min;
  else Base = (ylAxis.max-ylAxis.min)/numGrids;
  Expo = floor(log10(Base));
  Base = Base/pow(10.0,Expo);        // separate first significant digit
  if(Base < 3.5) {     // use only 1, 2 and 5, which ever is best fitted
    if(Base < 1.5) Base = 1.0;
    else Base = 2.0;
  }
  else {
    if(Base < 7.5) Base = 5.0;
    else { Base = 1.0; Expo++; }
  }
  GridStep = Base * pow(10.0,Expo);   // grid distance in real coordinates
  corr = floor((ylAxis.max-ylAxis.min)/GridStep - numGrids);
  if(corr < 0.0) corr++;
  numGrids += corr;     // correct rounding faults


  // upper y boundery ...........................
  zD = fabs(fmod(ylAxis.max, GridStep));// expand grid to upper diagram edge ?
  GridNum = zD/GridStep;
  if((1.0-GridNum) < 1e-10) GridNum = 0.0;  // fix rounding errors
  if(ylAxis.max <= 0.0) {
    if(GridNum < 0.3) { ylAxis.up += zD;  zD = 0.0; }
  }
  else  if(GridNum > 0.7)  ylAxis.up += GridStep-zD;
        else if(GridNum < 0.1)
	       if(GridNum*double(y2) >= 1.0)// more than 1 pixel above ?
		 ylAxis.up += 0.3*GridStep;  // beauty correction


  // lower y boundery ...........................
  zD = fabs(fmod(ylAxis.min, GridStep));// expand grid to lower diagram edge ?
  GridNum = zD/GridStep;
  if((1.0-GridNum) < 1e-10) zD = GridNum = 0.0;  // fix rounding errors
  if(ylAxis.min <= 0.0) {
    if(GridNum > 0.7) { ylAxis.low -= GridStep-zD;  zD = 0.0; }
    else if(GridNum < 0.1)
	   if(GridNum*double(y2) >= 1.0) { // more than 1 pixel above ?
	     ylAxis.low -= 0.3*GridStep;   // beauty correction
	     zD += 0.3*GridStep;
	   }
  }
  else {
    if(GridNum > 0.3) {
      zD = GridStep-zD;
      if(GridNum > 0.9) {
	if((1.0-GridNum)*double(y2) >= 1.0) { // more than 1 pixel above ?
	  ylAxis.low -= 0.3*GridStep;    // beauty correction
	  zD += 0.3*GridStep;
	}
      }
    }
    else { ylAxis.low -= zD;  zD = 0.0; }
  }

  zDstep = GridStep/(ylAxis.up-ylAxis.low)*double(y2); // grid in pixel
  GridNum  = ylAxis.low + zD;
  zD /= (ylAxis.up-ylAxis.low)/double(y2);

  zD += 0.5;     // perform rounding
  z = int(zD);   //  "int(...)" implies "floor(...)"
  while(z <= y2) {    // create all grid lines
    if(fabs(GridNum) < 0.01*pow(10.0, Expo)) GridNum = 0.0;// make 0 really 0
    if(fabs(Expo) < 3.0)  tmp = StringNum(GridNum);
    else tmp = StringNum(GridNum, 'e',1);
    r = metrics.size(0, tmp);  // width of text
    if(maxWidth < r.width()) maxWidth = r.width();

    Texts.append(new Text(-r.width()-7, z+6, tmp));  // text aligned right
    GridNum += GridStep;

    if(GridOn)  if(z < y2)  if(z > 0)
      Lines.append(new Line(0, z, x2, z, GridPen));  // y grid
    Lines.append(new Line(-5, z, 5, z, QPen(QPen::black,0)));   // y marks
    zD += zDstep;
    z = int(zD);
  }
} // of "if(xlog) ... else ..."
  x1 = maxWidth+14;


  // outer frame
  Lines.append(new Line(0,  y2, x2, y2, QPen(QPen::black,0)));
  Lines.append(new Line(x2, y2, x2,  0, QPen(QPen::black,0)));
  Lines.append(new Line(0,   0, x2,  0, QPen(QPen::black,0)));
  Lines.append(new Line(0,  y2,  0,  0, QPen(QPen::black,0)));
  return true;

Frame:
  // outer frame
  Lines.append(new Line(0,  y2, x2, y2, QPen(QPen::red,0)));
  Lines.append(new Line(x2, y2, x2,  0, QPen(QPen::red,0)));
  Lines.append(new Line(0,   0, x2,  0, QPen(QPen::red,0)));
  Lines.append(new Line(0,  y2,  0,  0, QPen(QPen::red,0)));
  return false;
}

// ------------------------------------------------------------
RectDiagram* RectDiagram::newOne()
{
  return new RectDiagram();
}
