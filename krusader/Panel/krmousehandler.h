/***************************************************************************
                        krmousehandler.h  -  description
                             -------------------
    begin                : Sat Feb 14 2009
    copyright            : (C) 2009+ by Csaba Karai
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __KR_MOUSE_HANDLER__
#define __KR_MOUSE_HANDLER__

#include <QPoint>
#include <QTimer>

class QMouseEvent;
class QWheelEvent;
class KrView;
class KrViewItem;

class KrMouseHandler : QObject
{
	Q_OBJECT

public:
	KrMouseHandler( KrView * view );
	
	bool mousePressEvent( QMouseEvent *e );
	bool mouseReleaseEvent( QMouseEvent *e );
	bool mouseDoubleClickEvent( QMouseEvent *e );
	bool mouseMoveEvent ( QMouseEvent *e );
	bool wheelEvent ( QWheelEvent * );
	void handleContextMenu( KrViewItem * it, const QPoint & pos );
	
protected:
	KrView * _view;
	bool     _singleClick;
	QPoint   _contextMenuPoint;
	QTimer contextMenuTimer;
};

#endif /* __KR_MOUSE_HANDLER */
