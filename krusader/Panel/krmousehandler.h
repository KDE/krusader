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

class QMouseEvent;
class KrView;

class KrMouseHandler
{
public:
	KrMouseHandler( KrView * view ) : _view( view ) {}
	
	bool mousePressEvent( QMouseEvent *e );
	
protected:
	KrView * _view;
};

#endif /* __KR_MOUSE_HANDLER */