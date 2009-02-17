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

KrMouseHandler::KrMouseHandler( KrView * view, int contextMenuShift ) : _view( view ), _rightClickedItem( 0 ),
	_contextMenuShift( contextMenuShift )
{
	KConfigGroup grpSvr( krConfig, "Look&Feel" );
	// decide on single click/double click selection
	_singleClick = grpSvr.readEntry( "Single Click Selects", _SingleClickSelects ) && KGlobalSettings::singleClick();
	connect( &_contextMenuTimer, SIGNAL (timeout()), this, SLOT (showContextMenu()));
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
		}
		else if( e->modifiers() == Qt::ControlModifier )
		{
			if( item && (KrSelectionMode::getSelectionHandler()->shiftCtrlLeftButtonSelects() || 
			             KrSelectionMode::getSelectionHandler()->leftButtonSelects() ) )
			{
				item->setSelected(!item->isSelected());
			}
			if( item )
				_view->setCurrentKrViewItem( item );
			e->accept();
			return true;
		}
		else if( e->modifiers() == Qt::ShiftModifier )
		{
			if( item && (KrSelectionMode::getSelectionHandler()->shiftCtrlLeftButtonSelects() || 
			             KrSelectionMode::getSelectionHandler()->leftButtonSelects() ) )
			{
				KrViewItem * current = _view->getCurrentKrViewItem();
				if( current != 0 )
					_view->selectRegion( item, current, true );
			}
			if( item )
				_view->setCurrentKrViewItem( item );
			e->accept();
			return true;
		}
	}
	if (e->button() == Qt::RightButton)
	{
		KrViewItem * oldCurrent = _view->getCurrentKrViewItem();
		//dragStartPos = e->pos();
		if( e->modifiers() == Qt::NoModifier )
		{
			if( item )
			{
				if( KrSelectionMode::getSelectionHandler()->rightButtonSelects() )
				{
					if( KrSelectionMode::getSelectionHandler()->rightButtonPreservesSelection() ) {
						if (KrSelectionMode::getSelectionHandler()->showContextMenu() >= 0)
						{
							_rightClickSelects = !item->isSelected();
							_rightClickedItem = item;
						}
						item->setSelected(!item->isSelected());
					} else {
						// clear the current selection
						_view->changeSelection( KRQuery( "*" ), false, true);
						item->setSelected( true );
					}
				}
				_view->setCurrentKrViewItem( item );
				handleContextMenu( item, e->globalPos() );
			}
			e->accept();
			return true;
		}
		else if( e->modifiers() == Qt::ControlModifier )
		{
			if( item && (KrSelectionMode::getSelectionHandler()->shiftCtrlRightButtonSelects() || 
			             KrSelectionMode::getSelectionHandler()->rightButtonSelects() ) )
			{
				item->setSelected(!item->isSelected());
			}
			if( item )
				_view->setCurrentKrViewItem( item );
			e->accept();
			return true;
		}
		else if( e->modifiers() == Qt::ShiftModifier )
		{
			if( item && (KrSelectionMode::getSelectionHandler()->shiftCtrlRightButtonSelects() || 
			             KrSelectionMode::getSelectionHandler()->rightButtonSelects() ) )
			{
				KrViewItem * current = _view->getCurrentKrViewItem();
				if( current != 0 )
					_view->selectRegion( item, current, true );
			}
			if( item )
				_view->setCurrentKrViewItem( item );
			e->accept();
			return true;
		}
	}
	return false;
}

bool KrMouseHandler::mouseReleaseEvent( QMouseEvent *e )
{
	KrViewItem * item = _view->getKrViewItemAt( e->pos() );
	
	if( e->button() == Qt::RightButton ) {
		_rightClickedItem = 0;
		_contextMenuTimer.stop();
	}
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
	KrViewItem * item = _view->getKrViewItemAt( e->pos() );
	if( _singleClick )
		return false;
	if( e->button() == Qt::LeftButton && item != 0 )
	{
		e->accept();
		QString tmp = item->name();
		_view->op()->emitExecuted(tmp);
		return true;
	}
	return false;
}

bool KrMouseHandler::mouseMoveEvent( QMouseEvent *e )
{
	KrViewItem * item =  _view->getKrViewItemAt( e->pos() );
	if ( !item )
		return false;
	QString desc = item->description();
	_view->op()->emitItemDescription(desc);
	
	if (KrSelectionMode::getSelectionHandler()->rightButtonPreservesSelection() 
		&& KrSelectionMode::getSelectionHandler()->rightButtonSelects()
		&& KrSelectionMode::getSelectionHandler()->showContextMenu() >= 0 && e->buttons() == Qt::RightButton)
	{
		e->accept();
		if (item != _rightClickedItem && item && _rightClickedItem)
		{
			_view->selectRegion( item, _rightClickedItem, _rightClickSelects );
			_rightClickedItem = item;
			_view->setCurrentKrViewItem( item );
			_contextMenuTimer.stop();
		}
		return true;
	}
	return false;
}

bool KrMouseHandler::wheelEvent ( QWheelEvent * )
{
	if( !_view->isFocused() )
		_view->op()->emitNeedFocus();
	return false;
}

void KrMouseHandler::showContextMenu()
{
	if (_rightClickedItem)
		_rightClickedItem->setSelected(true);
	_view->op()->emitContextMenu( _contextMenuPoint );
}

void KrMouseHandler::handleContextMenu( KrViewItem * it, const QPoint & pos ) {
	if ( !_view->isFocused() )
		_view->op()->emitNeedFocus();
	if ( !it )
		return;
	if( it->isDummy() )
		return;
	int i = KrSelectionMode::getSelectionHandler()->showContextMenu();
	_contextMenuPoint = QPoint( pos.x(), pos.y() - _contextMenuShift );
	if (i < 0) {
		_view->setCurrentKrViewItem( it );
		_view->op()->emitContextMenu( _contextMenuPoint );
	}
	else if (i > 0) {
		_contextMenuTimer.setSingleShot( true );
		_contextMenuTimer.start(i);
	}
}

