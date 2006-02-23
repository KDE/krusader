/***************************************************************************
                       percentalsplitter.h  -  description
                             -------------------
    copyright            : (C) 2006 + by Csaba Karai
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

                                                     H e a d e r    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "percentalsplitter.h"
#include <qtooltip.h>
#include <qpainter.h>
#include <qapplication.h>

class PercentalSplitterToolTip : public QToolTip {
public:
  PercentalSplitterToolTip( QWidget * parent ) : QToolTip( parent ) {
  }
  
  virtual ~PercentalSplitterToolTip() {
    remove( parentWidget() );
  }
  
  void maybeTip( const QPoint & point ) {
    if( parentWidget()->inherits( "PercentalSplitter" ) ) {
      PercentalSplitter *splitter = (PercentalSplitter *)parentWidget();
      
      QString tipString = splitter->toolTipString();
      QRect rect = QRect( parentWidget()->rect() );
      rect.setX( splitter->sizes()[ 0 ] );
      rect.setWidth( splitter->handleWidth() );
      if( rect.contains( point ) )
        tip( rect, tipString );
    }
  }
};

PercentalSplitter::PercentalSplitter( QWidget * parent, const char * name ) : QSplitter( parent, name ), label( 0 ), opaqueOldPos( -1 ) {
  toolTip = new PercentalSplitterToolTip( this );
}  
  
PercentalSplitter::~PercentalSplitter() {
  delete toolTip;
}
  
QString PercentalSplitter::toolTipString( int p ) {
  QValueList<int> values = sizes();  
  if( values.count() == 2 && ( values[ 0 ] + values[ 1 ]  != 0 ) ) {
    if( p < 0 )
      p = values[ 0 ];
    int percent = (int)(((double)p / (double)( values[ 0 ] + values[ 1 ] )) * 10000. + 0.5);
    return QString( "%1.%2%3" ).arg( percent / 100 ).arg( ( percent / 10 )%10 ).arg( percent % 10 ) + "%";
  }
  return QString::null;
}
  
void PercentalSplitter::setRubberband ( int p ) {  
  QPainter paint( this );
  paint.setPen( gray );
  paint.setBrush( gray );
  paint.setRasterOp( XorROP );
  QRect r = contentsRect();
  const int rBord = 3; // customizable?
  int hw = handleWidth();
    
  if ( opaqueOldPos >= 0 ) {
    if( label == 0 )
      paint.drawRect( opaqueOldPos + hw / 2 - rBord, r.y(), 2 * rBord, r.height() );
    else {
      QPoint labelLoc = mapFromGlobal( labelLocation );
      if( labelLoc.y() > r.y() )
        paint.drawRect( opaqueOldPos + hw / 2 - rBord, r.y(), 2 * rBord, labelLoc.y() );
      if( labelLoc.y() + label->height() < r.height() )
        paint.drawRect( opaqueOldPos + hw / 2 - rBord, labelLoc.y() + label->height(), 2 * rBord, r.height() - labelLoc.y() - label->height() );
    }
  }

  if( p < 0 ) {
    if( label ) {
      delete label;
      label = 0;
    }
  }
  else {
    int scr = QApplication::desktop()->screenNumber( this );
      
    if( label == 0 ) {
      label = new QLabel( QApplication::desktop()->screen( scr ), "SplitterPercent", WStyle_StaysOnTop | 
                          WStyle_Customize | WStyle_NoBorder | WStyle_Tool | WX11BypassWM );
      label->setMargin(1);
      label->setAutoMask( FALSE );
      label->setFrameStyle( QFrame::Plain | QFrame::Box );
      label->setLineWidth( 1 );
      label->setAlignment( AlignAuto | AlignTop );
      label->setIndent(0);
      label->polish();
    }

    label->setText( toolTipString( p ) );
    label->adjustSize();
    labelLocation = mapToGlobal( QPoint( p - label->width()/2, r.y() + r.height()/2 ) );
    if( labelLocation.x() < 0 )
      labelLocation.setX( 0 );

#ifdef Q_WS_MAC
    QRect screen = QApplication::desktop()->availableGeometry( scr );
#else
    QRect screen = QApplication::desktop()->screenGeometry( scr );
#endif

    if( labelLocation.x() + label->width() > screen.width() )
      labelLocation.setX( screen.width() - label->width() );
    label->move( labelLocation );
    label->show();
    
    QPoint labelLoc = mapFromGlobal( labelLocation );
    if( labelLoc.y() > r.y() )
      paint.drawRect( p + hw / 2 - rBord, r.y(), 2 * rBord, labelLoc.y() );
    if( labelLoc.y() + label->height() < r.height() )
      paint.drawRect( p + hw / 2 - rBord, labelLoc.y() + label->height(), 2 * rBord, r.height() - labelLoc.y() - label->height() );
  }
  opaqueOldPos = p;
}

#include "percentalsplitter.moc"