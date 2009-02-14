/***************************************************************************
                        krmousehandler.cpp  -  description
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

#include "krmousehandler.h"
#include "krview.h"

bool KrMouseHandler::mousePressEvent( QMouseEvent *e )
{
	if( !_view->isFocused() )
		_view->op()->emitNeedFocus();
	return true;
}