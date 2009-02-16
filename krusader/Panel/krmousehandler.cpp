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
#include "krviewitem.h"
#include "krselectionmode.h"
#include "../krusader.h"
#include "../defaults.h"

KrMouseHandler::KrMouseHandler( KrView * view ) : _view( view ) {
	KConfigGroup grpSvr( krConfig, "Look&Feel" );
	// decide on single click/double click selection
	_singleClick = grpSvr.readEntry( "Single Click Selects", _SingleClickSelects ) && KGlobalSettings::singleClick();
}

bool KrMouseHandler::mousePressEvent( QMouseEvent *e )
{
	KrViewItem * item = _view->getKrViewItemAt( e->pos() );
	if( !_view->isFocused() )
		_view->op()->emitNeedFocus();
	if (e->button() == Qt::LeftButton)
	{
		KrViewItem * oldCurrent = _view->getCurrentKrViewItem();
		//dragStartPos = e->pos();
		if( e->modifiers() == Qt::NoModifier )
		{
			if( item )
			{
				if( KrSelectionMode::getSelectionHandler()->leftButtonSelects() )
				{
					if( KrSelectionMode::getSelectionHandler()->leftButtonPreservesSelection() )
						item->setSelected(!item->isSelected());
					else {
						// clear the current selection
						_view->changeSelection( KRQuery( "*" ), false, true);
						item->setSelected( true );
					}
				}
				_view->setCurrentKrViewItem( item );
			}
			e->accept();
			return true;
			// KrSelectionMode::getSelectionHandler()->shiftCtrlLeftButtonSelects()
		}
	}
	return false;
}

bool KrMouseHandler::mouseReleaseEvent( QMouseEvent *e )
{
	KrViewItem * item = _view->getKrViewItemAt( e->pos() );
	
	if( _singleClick && e->button() == Qt::LeftButton )
	{
		e->accept();
		if( item == 0 )
			return true;
		QString tmp = item->name();
		_view->op()->emitExecuted(tmp);
		return true;
	}
	if ( e->button() == Qt::MidButton && item != 0 )
	{
				e->accept();
		if( item == 0 )
			return true;
		_view->op()->emitMiddleButtonClicked( item );
		return true;
	}
	return false;
}

bool KrMouseHandler::mouseDoubleClickEvent( QMouseEvent *e )
{
	KrViewItem * i = _view->getKrViewItemAt( e->pos() );
	if( _singleClick )
		return false;
	if( e->button() == Qt::LeftButton )
	{
		e->accept();
		QString tmp = i->name();
		_view->op()->emitExecuted(tmp);
		return true;
	}
	return false;
}

bool KrMouseHandler::mouseMoveEvent( QMouseEvent *e )
{
	return false;
}

bool KrMouseHandler::wheelEvent ( QWheelEvent * )
{
	if( !_view->isFocused() )
		_view->op()->emitNeedFocus();
	return false;
}

void KrMouseHandler::handleContextMenu( KrViewItem * it, const QPoint & pos ) {
	if ( !_view->isFocused() )
		_view->op()->emitNeedFocus();
	if ( !it )
		return;
	if( it->isDummy() )
		return;
	int i = KrSelectionMode::getSelectionHandler()->showContextMenu();
	_contextMenuPoint = QPoint( pos.x(), pos.y() /*TODO: - header() ->height() */ );
	if (i < 0) {
		_view->setCurrentKrViewItem( it );
		_view->op()->emitContextMenu( _contextMenuPoint );
	}
	else if (i > 0) {
		contextMenuTimer.setSingleShot( true );
		contextMenuTimer.start(i);
	}
}

