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
#include <QTime>

class QMouseEvent;
class QWheelEvent;
class KrView;
class KrViewItem;

class KrMouseHandler : public QObject
{
	Q_OBJECT

public:
	KrMouseHandler( KrView * view, int contextMenuShift );
	
	bool mousePressEvent( QMouseEvent *e );
	bool mouseReleaseEvent( QMouseEvent *e );
	bool mouseDoubleClickEvent( QMouseEvent *e );
	bool mouseMoveEvent ( QMouseEvent *e );
	bool wheelEvent ( QWheelEvent * );
	void handleContextMenu( KrViewItem * it, const QPoint & pos );
	void otherEvent( QEvent * e );
	void cancelTwoClickRename();
	
public slots:
	void showContextMenu();
	
signals:
	void renameCurrentItem();
	
protected:
	KrView     * _view;
	KrViewItem * _rightClickedItem;
	bool         _rightClickSelects;
	bool         _singleClick;
	QPoint       _contextMenuPoint;
	QTimer       _contextMenuTimer;
	int          _contextMenuShift;
	bool         _singleClicked;
	KrViewItem * _singleClickedItem;
	QTime        _singleClickTime;
	QTimer       _renameTimer;
};

#endif /* __KR_MOUSE_HANDLER */
