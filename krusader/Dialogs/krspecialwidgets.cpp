/***************************************************************************
                                krspecialwidgets.cpp
                             -------------------
    copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
 ---------------------------------------------------------------------------
  Description
 ***************************************************************************

  A 

     db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
     88 ,8P' 88  `8D 88    88 88'  YP d8' `8b 88  `8D 88'     88  `8D
     88,8P   88oobY' 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY'
     88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
     88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
     YP   YD 88   YD ~Y8888P' `8888Y' YP   YP Y8888D' Y88888P 88   YD

                                                     S o u r c e    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/



#include "krspecialwidgets.h"
#include "krmaskchoice.h"
#include "newftpgui.h"
#include "../krusader.h"
#include "../MountMan/kmountman.h"
#include <math.h>
#include <kfileitem.h>
#include <klocale.h>
#include <klineedit.h>
#include <kdebug.h>

/////////////////////////////////////////////////////////////////////////////
/////////////////////// Pie related widgets /////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// The pie-related widgets use hard-coded coordinates to create the look.
// This is ok since the whole widget is fitted into an existing view and thus
// no re-alignments are needed.

#define LEFT       10
#define BOTTOM     150
#define WIDTH      120
#define HEIGHT     40
#define Z_HEIGHT   10
#define STARTANGLE 0
#define DEG(x)     (16*(x))

QColor KRPie::colors[12]={Qt::red,Qt::blue,Qt::green,Qt::cyan,Qt::magenta,Qt::gray,
                          Qt::black,Qt::white,Qt::darkRed,Qt::darkBlue,Qt::darkMagenta,
                          Qt::darkCyan};

//////////////////////////////////////////////////////////////////////////////
/////////////// KRFSDisplay - Filesystem / Freespace Display /////////////////
//////////////////////////////////////////////////////////////////////////////
// This is the full constructor: use it for a mounted filesystem
KRFSDisplay::KRFSDisplay(QWidget *parent, QString _alias, QString _realName,
    long _total, long _free) : QWidget(parent), totalSpace(_total),
    freeSpace(_free), alias(_alias), realName(_realName), mounted(true),
    empty(false), supermount(false) {
  resize(150,200);
  show();
}

// Use this one for an unmounted filesystem
KRFSDisplay::KRFSDisplay(QWidget *parent, QString _alias, QString _realName, bool sm) :
    QWidget(parent), alias(_alias), realName(_realName), mounted(false),
		empty(false), supermount(sm) {
  resize(150,200);
  show();
}

// This is used only when an empty widget needs to be displayed (for example:
// when filesystem statistics haven't been calculated yet)
KRFSDisplay::KRFSDisplay(QWidget *parent) : QWidget(parent), empty(true) {
  resize(150,200);
  show();
}
    

// The main painter!    
void KRFSDisplay::paintEvent(QPaintEvent *) {
  QPainter paint(this);
  if (!empty) {
    // create the text
    // first, name and location
    paint.setFont(QFont("helvetica",12,QFont::Bold));
    paint.drawText(10,20,alias);
    paint.setFont(QFont("helvetica",12,QFont::Normal));  
    paint.drawText(10,37,"("+realName+")");
    if (supermount) { // we don't support supermount anymore
      paint.setFont(QFont("helvetica",12,QFont::Bold));
      paint.drawText(10,60,i18n("Supermount device."));
      paint.setFont(QFont("helvetica",10,QFont::Bold));
      paint.drawText(10,80,i18n("Unable to get information"));
      paint.drawText(10,90,i18n("on a supermounted device."));
      paint.drawText(10,100,i18n("For details, please read"));
      paint.drawText(10,110,i18n("the Krusader FAQ."));
    } else
		if (mounted) {  // incase the filesystem is already mounted
      // second, the capacity
      paint.drawText(10,70,i18n("Capacity: ")+KIO::convertSizeFromKB(totalSpace));
      // third, the 2 boxes (used, free)
      QPen systemPen=paint.pen();
      paint.setPen(Qt::black);
      paint.drawRect(10,90,10,10);
      paint.fillRect(11,91,8,8,QBrush(Qt::gray));
      paint.drawRect(10,110,10,10);
      paint.fillRect(11,111,8,8,QBrush(Qt::white));
      // now, the text for the boxes
      paint.setPen(systemPen);
      paint.drawText(25,100,i18n("Used: ")+KIO::convertSizeFromKB(totalSpace-freeSpace));
      paint.drawText(25,120,i18n("Free: ")+KIO::convertSizeFromKB(freeSpace));
      // first, create the empty pie
      // bottom...             
      paint.setPen(Qt::black);
      paint.setBrush(Qt::white);
      paint.drawPie(LEFT,BOTTOM,WIDTH,HEIGHT,STARTANGLE,DEG(360));
      // body...
      paint.setPen(Qt::lightGray);
      for (int i=1; i<Z_HEIGHT; ++i) 
        paint.drawPie(LEFT,BOTTOM-i,WIDTH,HEIGHT,STARTANGLE,DEG(360));
      // side lines...
      paint.setPen(Qt::black);
      paint.drawLine(LEFT,BOTTOM+HEIGHT/2,LEFT,BOTTOM+HEIGHT/2-Z_HEIGHT);
      paint.drawLine(LEFT+WIDTH,BOTTOM+HEIGHT/2,LEFT+WIDTH,BOTTOM+HEIGHT/2-Z_HEIGHT);
      // top of the pie
      paint.drawPie(LEFT,BOTTOM-Z_HEIGHT,WIDTH,HEIGHT,STARTANGLE,DEG(360));
      // the "used space" slice
      float i=((float)(totalSpace-freeSpace)/(totalSpace)) * 360.0;
      paint.setBrush(Qt::gray);
      paint.drawPie(LEFT,BOTTOM-Z_HEIGHT,WIDTH,HEIGHT,STARTANGLE,(int)DEG(i));
      // if we need to draw a 3d stripe ...
      if (i>180.0) {
        for (int j=1; j<Z_HEIGHT; ++j) 
          paint.drawArc(LEFT,BOTTOM-j,WIDTH,HEIGHT,STARTANGLE-16*180,(int)(DEG(i-180.0)));
      }
    } else {  // if the filesystem is unmounted...
      paint.setFont(QFont("helvetica",12,QFont::Bold));  
      paint.drawText(10,60,i18n("Not mounted."));
    }
  } else {  // if the widget is in empty situation...
  
  }
}

////////////////////////////////////////////////////////////////////////////////
KRPie::KRPie(long _totalSize, QWidget *parent) : QWidget(parent,0), totalSize(_totalSize) {
  slices.setAutoDelete(true); // kill items when they are removed
  slices.append(new KRPieSlice(100,Qt::yellow,"DEFAULT"));
  sizeLeft=totalSize;
  resize(300,300);
}

void KRPie::paintEvent(QPaintEvent *) {
  QPainter paint( this );
  // now create the slices
  KRPieSlice *slice;
  float sAngle=STARTANGLE;
  for ( slice=slices.first(); slice != 0; slice=slices.next() ) {
    paint.setBrush(slice->getColor());
    paint.setPen(slice->getColor());
    // angles are negative to create a clock-wise drawing of slices
    float angle=-(slice->getPerct()/100*360)*16;
    for (int i=1; i<Z_HEIGHT; ++i) 
      paint.drawPie(LEFT,BOTTOM+i,WIDTH,HEIGHT,(int)sAngle,(int)angle);
    sAngle+=angle;
  }  
  paint.setPen(Qt::yellow);   // pen
  paint.setBrush(Qt::yellow); // fill
 // for (int i=1; i<Z_HEIGHT; ++i) 
  //  paint.drawPie(LEFT,BOTTOM+i,WIDTH,HEIGHT,sAngle,360*16-(STARTANGLE-sAngle));
  sAngle=STARTANGLE;
  for ( slice=slices.first(); slice != 0; slice=slices.next() ) {
    paint.setBrush(slice->getColor());
    paint.setPen(slice->getColor());
    // angles are negative to create a clock-wise drawing of slices
    float angle=-(slice->getPerct()/100*360)*16;
    paint.drawPie(LEFT,BOTTOM,WIDTH,HEIGHT,(int)sAngle,(int)angle);
    sAngle+=angle;
  }  


  paint.setPen(Qt::black);
  // the pie
//  paint.drawPie(LEFT,BOTTOM,WIDTH,HEIGHT,STARTANGLE,360*16);
  ///////////////////////// end of empty pie /////////////////////////
  // now, the pie is ready to draw slices on...
  // to make a good look on the perimiter, draw another black circle  
  paint.setPen(Qt::black);
  paint.drawArc(LEFT,BOTTOM,WIDTH,HEIGHT,STARTANGLE,360*16);

}

void KRPie::addSlice(long size,QString label) {
  int i=(slices.count() % 12);
  slices.removeLast();
  slices.append(new KRPieSlice(size*100/totalSize,colors[i],label));
  sizeLeft-=size;
  slices.append(new KRPieSlice(sizeLeft*100/totalSize,Qt::yellow,"DEFAULT"));  
}

////////////////////////////////////////////////////
/////////////////// KrQuickSearch  /////////////////
////////////////////////////////////////////////////
KrQuickSearch::KrQuickSearch(QWidget *parent, const char * name): KLineEdit(parent,name) {}

void KrQuickSearch::myKeyPressEvent(QKeyEvent *e) {
   switch (e->key()) {
      case Key_Up:
      case Key_Down:
      case Key_Home:
      case Key_End:
         emit stop(e);
         break;
      default:
         kdWarning()<<"got " << e->text() << endl;
         keyPressEvent(e);
   }
}


#include "krspecialwidgets.moc"
