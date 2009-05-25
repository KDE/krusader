/***************************************************************************
                                 krview.cpp
                            -------------------
   copyright            : (C) 2000-2002 by Shie Erlich & Rafi Yanai
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
#include "krview.h"
#include "../kicons.h"
#include "../krslots.h"
#include "../defaults.h"
#include "../VFS/krpermhandler.h"
#include "../Dialogs/krspecialwidgets.h"
#include "krviewitem.h"
#include "krselectionmode.h"
#include <qnamespace.h>
#include <qpixmapcache.h>
#include <qdir.h>
#include <qbitmap.h>
#include <qpainter.h>
#include <QPixmap>
#include <kmimetype.h>
#include <klocale.h>
#include <kinputdialog.h>


#define VF	getVfile()


// ----------------------------- operator
KrViewOperator::KrViewOperator(KrView *view, QWidget *widget): _view(view), _widget(widget), _massSelectionUpdate( false )
{
}

KrViewOperator::~KrViewOperator()
{
}

void KrViewOperator::startDrag()
{
	QStringList items;
	_view->getSelectedItems( &items );
	if ( items.empty() )
		return ; // don't drag an empty thing
	QPixmap px;
	if ( items.count() > 1 || _view->getCurrentKrViewItem() == 0 )
		px = FL_LOADICON( "queue" ); // how much are we dragging
	else
		px = _view->getCurrentKrViewItem() ->icon();
	emit letsDrag( items, px );
}

void KrViewOperator::setQuickSearch( KrQuickSearch *quickSearch ) {
	_quickSearch = quickSearch;
	
	connect( quickSearch, SIGNAL( textChanged( const QString& ) ), this, SLOT( quickSearch( const QString& ) ) );
	connect( quickSearch, SIGNAL( otherMatching( const QString&, int ) ), this, SLOT( quickSearch( const QString& , int ) ) );
	connect( quickSearch, SIGNAL( stop( QKeyEvent* ) ), this, SLOT( stopQuickSearch( QKeyEvent* ) ) );
	connect( quickSearch, SIGNAL( process( QKeyEvent* ) ), this, SLOT( handleQuickSearchEvent( QKeyEvent* ) ) );
}

void KrViewOperator::handleQuickSearchEvent( QKeyEvent * e ) {
	switch ( e->key() ) {
		case Qt::Key_Insert:
		{
			KrViewItem * item = _view->getCurrentKrViewItem();
			if( item )
			{
				item->setSelected( !item->isSelected() );
				quickSearch( _quickSearch->text(), 1 );
			}
			break;
		}
		case Qt::Key_Home:
		{
			KrViewItem * item = _view->getLast();
			if( item ) {
				_view->setCurrentKrViewItem( _view->getLast() );
				quickSearch( _quickSearch->text(), 1 );
			}
			break;
		}
		case Qt::Key_End:
		{
			KrViewItem * item = _view->getFirst();
			if( item ) {
				_view->setCurrentKrViewItem( _view->getFirst() );
				quickSearch( _quickSearch->text(), -1 );
			}
			break;
		}
	}
}

void KrViewOperator::quickSearch( const QString & str, int direction ) {
	KrViewItem * item = _view->getCurrentKrViewItem();
	if (!item) {
		_quickSearch->setMatch( false );
		return;
	}
	KConfigGroup grpSvr( _view->_config, "Look&Feel" );
	bool caseSensitive = grpSvr.readEntry( "Case Sensitive Quicksearch", _CaseSensitiveQuicksearch );
	if ( !direction ) {
		if ( caseSensitive ? item->name().startsWith( str ) : item->name().toLower().startsWith( str.toLower() ) ) {
			_quickSearch->setMatch( true );
			return ;
		}
		direction = 1;
	}
	KrViewItem * startItem = item;
	while ( true ) {
		item = ( direction > 0 ) ? _view->getNext( item ) : _view->getPrev( item );
		if ( !item )
			item = ( direction > 0 ) ? _view->getFirst() : _view->getLast();
		if ( item == startItem ) {
			_quickSearch->setMatch( false );
			return ;
		}
		if ( caseSensitive ? item->name().startsWith( str ) : item->name().toLower().startsWith( str.toLower() ) ) {
			_view->setCurrentKrViewItem( item );
			_view->makeItemVisible( item );
			_quickSearch->setMatch( true );
			return ;
		}
	}
}

void KrViewOperator::stopQuickSearch( QKeyEvent * e ) {
	if( _quickSearch ) {
		_quickSearch->hide();
		_quickSearch->clear();
		krDirUp->setEnabled( true );
		if ( e )
			_view->handleKeyEvent( e );
	}
}

void KrViewOperator::prepareForPassive() {
	if ( _quickSearch && !_quickSearch->isHidden() ) {
		stopQuickSearch( 0 );
	}
}

bool KrViewOperator::handleKeyEvent( QKeyEvent * e ) {
	if ( !_quickSearch->isHidden() ) {
		_quickSearch->myKeyPressEvent( e );
		return true;
	}
	return false;
}

void KrViewOperator::setMassSelectionUpdate( bool upd ) {
	_massSelectionUpdate = upd;
	if( !upd )
		emit selectionChanged();
}

// ----------------------------- krview

KrView::KrView( KConfig *cfg ) : _config( cfg ), _widget(0), _nameToMakeCurrent( QString() ), _nameToMakeCurrentIfAdded( QString() ),
		_numSelected( 0 ), _count( 0 ), _numDirs( 0 ), _countSize( 0 ), _selectedSize( 0 ), _properties(0), _focused( false ),
		_nameInKConfig(QString())
{
}

KrView::~KrView()
{
	if (_properties)
		qFatal("A class inheriting KrView didn't delete _properties!");
	if (_operator)
		qFatal("A class inheriting KrView didn't delete _operator!");
}

void KrView::init()
{
	// sanity checks:
	if (_nameInKConfig.isEmpty())
		qFatal("_nameInKConfig must be set during construction of KrView inheritors");
	if (!_widget)
		qFatal("_widget must be set during construction of KrView inheritors");
	// ok, continue
	initProperties();
	initOperator();
	setup();
}

void KrView::initProperties()
{
	_properties = createViewProperties();
	
	KConfigGroup grpSvr( _config, "Look&Feel" );
	_properties->displayIcons = grpSvr.readEntry( "With Icons", _WithIcons );
	bool dirsByNameAlways = grpSvr.readEntry("Always sort dirs by name", false);
	_properties->sortMode = static_cast<KrViewProperties::SortSpec>( KrViewProperties::Name |
			KrViewProperties::DirsFirst | 
			(dirsByNameAlways ? KrViewProperties::AlwaysSortDirsByName : 0) );
	_properties->numericPermissions = grpSvr.readEntry("Numeric permissions", _NumericPermissions);
	if ( !grpSvr.readEntry( "Case Sensative Sort", _CaseSensativeSort ) )
	_properties->sortMode = static_cast<KrViewProperties::SortSpec>( _properties->sortMode |
		KrViewProperties::IgnoreCase );
	_properties->sortMethod = static_cast<KrViewProperties::SortMethod>(
		grpSvr.readEntry("Sort method", (int) _DefaultSortMethod) );
	_properties->humanReadableSize = grpSvr.readEntry("Human Readable Size", _HumanReadableSize);
	_properties->localeAwareCompareIsCaseSensitive = QString( "a" ).localeAwareCompare( "B" ) > 0; // see KDE bug #40131
	QStringList defaultAtomicExtensions;
	defaultAtomicExtensions += ".tar.gz";
	defaultAtomicExtensions += ".tar.bz2";
	defaultAtomicExtensions += ".tar.lzma";
	defaultAtomicExtensions += ".moc.cpp";
	QStringList atomicExtensions = grpSvr.readEntry("Atomic Extensions", defaultAtomicExtensions);
	for (QStringList::iterator i = atomicExtensions.begin(); i != atomicExtensions.end(); )
	{
		QString & ext = *i;
		ext = ext.trimmed();
		if (!ext.length())
		{
			i = atomicExtensions.erase(i);
			continue;
		}
		if (!ext.startsWith("."))
			ext.insert(0, '.');
		++i;
	}
	_properties->atomicExtensions = atomicExtensions;
	
	KConfigGroup group( _config, nameInKConfig() );
	_properties->numberOfColumns = group.readEntry( "Number Of Brief Columns", _NumberOfBriefColumns );
	if( _properties->numberOfColumns < 1 )
		_properties->numberOfColumns = 1;
	else if( _properties->numberOfColumns > MAX_BRIEF_COLS )
		_properties->numberOfColumns = MAX_BRIEF_COLS;
}

QPixmap KrView::getIcon( vfile *vf /*, KRListItem::cmpColor color*/ )
{
	// KConfigGroup ag( krConfig, "Advanced");
	//////////////////////////////
	QPixmap icon;
	QString icon_name = vf->vfile_getIcon();
	//QPixmapCache::setCacheLimit( ag.readEntry("Icon Cache Size",_IconCacheSize) );

	if ( icon_name.isNull() )
		icon_name="";

	// first try the cache
	if ( !QPixmapCache::find( icon_name, icon ) ) {
		icon = FL_LOADICON( icon_name );
		// insert it into the cache
		QPixmapCache::insert( icon_name, icon );
	}
	// if it's a symlink - add an arrow overlay
	if ( vf->vfile_isSymLink() ) {
		QPixmap link( link_xpm );
		// bitBlt ( &icon, 0, icon.height() - 11, &link, 0, 21, 10, 11, Qt::CopyROP, false );
		QPainter painter( &icon );
		painter.drawPixmap( 0, icon.height() - 11, link, 0, 21, 10, 11 );
		//icon.setMask( icon.createHeuristicMask( false ) );
	}

	return icon;
}

/**
 * this function ADDs a list of selected item names into 'names'.
 * it assumes the list is ready and doesn't initialize it, or clears it
 */
void KrView::getItemsByMask( QString mask, QStringList* names, bool dirs, bool files )
{
	for ( KrViewItem * it = getFirst(); it != 0; it = getNext( it ) ) {
		if ( ( it->name() == ".." ) || !QDir::match( mask, it->name() ) ) continue;
		// if we got here, than the item fits the mask
		if ( it->getVfile()->vfile_isDir() && !dirs ) continue; // do we need to skip folders?
		if ( !it->getVfile()->vfile_isDir() && !files ) continue; // do we need to skip files
		names->append( it->name() );
	}
}

/**
 * this function ADDs a list of selected item names into 'names'.
 * it assumes the list is ready and doesn't initialize it, or clears it
 */
void KrView::getSelectedItems( QStringList *names )
{
	for ( KrViewItem * it = getFirst(); it != 0; it = getNext( it ) )
		if ( it->isSelected() && ( it->name() != ".." ) ) names->append( it->name() );

	// if all else fails, take the current item
	QString item = getCurrentItem();
	if ( names->empty() && item!=QString() && item!=".." ) names->append( item );
}

void KrView::getSelectedKrViewItems( KrViewItemList *items )
{
	for ( KrViewItem * it = getFirst(); it != 0; it = getNext( it ) )
		if ( it->isSelected() && ( it->name() != ".." ) ) items->append( it );

	// if all else fails, take the current item
	QString item = getCurrentItem();
	if ( items->empty() && item!=QString() && item!=".." && getCurrentKrViewItem() != 0 )
	  items->append( getCurrentKrViewItem() );
}

QString KrView::statistics()
{
	_countSize = _numSelected = _selectedSize = 0;

	for ( KrViewItem * it = getFirst(); it != 0; it = getNext( it ) ) {
		if ( it->isSelected() ) {
			++_numSelected;
			_selectedSize += it->getVfile()->vfile_getSize();
		}
		if (it->getVfile()->vfile_getSize() > 0)
			_countSize += it->getVfile()->vfile_getSize();
	}
	QString tmp = QString(i18n("%1 out of %2, %3 (%4) out of %5 (%6)",
	                           _numSelected, _count, KIO::convertSize( _selectedSize ),
	                           KRpermHandler::parseSize(_selectedSize),
	                           KIO::convertSize( _countSize ),
	                           KRpermHandler::parseSize(_countSize) ) );
	// notify if we're running a filtered view
	if (filter() != KrViewProperties::All)
		tmp = ">> [ " + filterMask().nameFilter() + " ]  "+tmp;
	return tmp;
}

void KrView::changeSelection( const KRQuery& filter, bool select, bool includeDirs )
{
	if( op() ) op()->setMassSelectionUpdate( true );
	
	KConfigGroup grpSvr( _config, "Look&Feel" );
	bool markDirs = grpSvr.readEntry( "Mark Dirs", _MarkDirs ) || includeDirs;

	KrViewItem *temp = getCurrentKrViewItem();
	for ( KrViewItem * it = getFirst(); it != 0; it = getNext( it ) ) {
		if ( it->name() == ".." ) continue;
		if ( it->getVfile()->vfile_isDir() && !markDirs ) continue;

		vfile * file = it->getMutableVfile(); // filter::match calls getMimetype which isn't const
		if ( file == 0 ) continue;

		if ( filter.match( file ) ) {
			// we're increasing/decreasing the number of selected files
			if ( select ) {
				if ( !it->isSelected() ) {
					++_numSelected;
					_selectedSize += it->getVfile()->vfile_getSize();
				}
			} else {
				if ( it->isSelected() ) {
					--_numSelected;
					_selectedSize -= it->getVfile()->vfile_getSize();
				}
			}
			it->setSelected( select );
		}
	}
	
	if( op() ) op()->setMassSelectionUpdate( false );
	updateView();
	if( ensureVisibilityAfterSelect() && temp != 0 )
		makeItemVisible( temp );
}

void KrView::invertSelection()
{
	if( op() ) op()->setMassSelectionUpdate( true );
	KConfigGroup grpSvr( _config, "Look&Feel" );
	bool markDirs = grpSvr.readEntry( "Mark Dirs", _MarkDirs );

	KrViewItem *temp = getCurrentKrViewItem();
	for ( KrViewItem * it = getFirst(); it != 0; it = getNext( it ) ) {
		if ( it->name() == ".." ) continue;
		if ( it->getVfile()->vfile_isDir() && !markDirs && !it->isSelected() ) continue;
		if ( it->isSelected() ) {
			--_numSelected;
			_selectedSize -= it->getVfile()->vfile_getSize();
		} else {
			++_numSelected;
			_selectedSize += it->getVfile()->vfile_getSize();
		}
		it->setSelected( !it->isSelected() );
	}
	if( op() ) op()->setMassSelectionUpdate( false );
	updateView();
	if( ensureVisibilityAfterSelect() && temp != 0 )
		makeItemVisible( temp );
}

QString KrView::firstUnmarkedBelowCurrent()
{
	if( getCurrentKrViewItem() == 0 )
		return QString();
	
	KrViewItem * iterator = getNext( getCurrentKrViewItem() );
	while ( iterator && iterator->isSelected() )
		iterator = getNext( iterator );
	if ( !iterator ) {
		iterator = getPrev( getCurrentKrViewItem() );
		while ( iterator && iterator->isSelected() )
			iterator = getPrev( iterator );
	}
	if ( !iterator ) return QString();
	return iterator->name();
}

void KrView::delItem(const QString &name)
{
	QHash<QString, KrViewItem*>::iterator itr = _dict.find( name );
	if ( itr == _dict.end() ) {
		krOut << "got signal deletedVfile(" << name << ") but can't find KrViewItem" << endl;
		return;
	}

	KrViewItem * it = *itr;
	if (!preDelItem(it)) return; // do not delete this after all

	// remove from dict
	if (it->VF->vfile_isDir()) {
		--_numDirs;
	} else {
		_countSize -= it->VF->vfile_getSize();
	}
	--_count;
	_dict.remove( name );
	delete it;
	op()->emitSelectionChanged();
}

KrViewItem *KrView::addItem( vfile *vf )
{
	KrViewItem *item = preAddItem(vf);
	if (!item) return 0; // don't add it after all

	// add to dictionary
	_dict.insert( vf->vfile_getName(), item );
	if ( vf->vfile_isDir() )
		++_numDirs;
	else _countSize += vf->vfile_getSize();
	++_count;

	if (item->name() == nameToMakeCurrent() ) {
		setCurrentKrViewItem(item); // dictionary based - quick
		makeItemVisible( item );
	}
	if (item->name() == nameToMakeCurrentIfAdded() ) {
		setCurrentKrViewItem(item);
		setNameToMakeCurrentIfAdded(QString());
		makeItemVisible( item );
	}


	op()->emitSelectionChanged();
	return item;
}

void KrView::updateItem(vfile *vf)
{
	// since we're deleting the item, make sure we keep
	// it's properties first and repair it later
	QHash<QString, KrViewItem*>::iterator itr = _dict.find( vf->vfile_getName() );
	if ( itr == _dict.end() ) {
		krOut << "got signal updatedVfile(" << vf->vfile_getName() << ") but can't find KrViewItem" << endl;
	} else {
		KrViewItem * it = *itr;
		bool selected = it->isSelected();
		bool current = ( getCurrentKrViewItem() == it );
		delItem( vf->vfile_getName() );
		KrViewItem *updatedItem = addItem( vf );
		// restore settings
		( _dict[ vf->vfile_getName() ] ) ->setSelected( selected );
		if ( current ) {
			setCurrentItem( vf->vfile_getName() );
			makeItemVisible( updatedItem );
		}
	}
	op()->emitSelectionChanged();
}

void KrView::clear()
{
	_count = _numSelected = _numDirs = _selectedSize = _countSize = 0;
	_dict.clear();
}

// good old dialog box
void KrView::renameCurrentItem()
{
	QString newName, fileName;

	KrViewItem *it = getCurrentKrViewItem();
	if ( it ) fileName = it->name();
	else return ; // quit if no current item available

	// don't allow anyone to rename ..
	if ( fileName == ".." ) return ;

	bool ok = false;
	newName = KInputDialog::getText( i18n( "Rename" ), i18n( "Rename " ) + fileName + i18n( " to:" ),
	                                 fileName, &ok, krApp );
	// if the user canceled - quit
	if ( !ok || newName == fileName )
		return ;
	op()->emitRenameItem(it->name(), newName);
}

bool KrView::handleKeyEvent (QKeyEvent *e) {
	if( op()->handleKeyEvent( e ) )
		return true;
	bool res = handleKeyEventInt( e );
	
	// emit the new item description
	KrViewItem * current = getCurrentKrViewItem();
	if( current != 0 ) {
		QString desc = current->description();
		op()->emitItemDescription(desc);
	}
	
	return res;
}

bool KrView::handleKeyEventInt (QKeyEvent *e) {
	switch ( e->key() ) {
	case Qt::Key_Enter :
	case Qt::Key_Return :
	{
		if ( e->modifiers() & Qt::ControlModifier )         // let the panel handle it
			e->ignore();
		else {
			KrViewItem * i = getCurrentKrViewItem();
			if( i == 0 )
				return true;
			QString tmp = i->name();
			op()->emitExecuted(tmp);
		}
		return true;
	}
	case Qt::Key_QuoteLeft :          // Terminal Emulator bugfix
		if ( e->modifiers() == Qt::ControlModifier ) { // let the panel handle it
			e->ignore();
		} else {          // a normal click - do a lynx-like moving thing
			SLOTS->home(); // ask krusader to move up a directory
		}
		return true;
	case Qt::Key_Delete :                   // kill file
		SLOTS->deleteFiles( e->modifiers() == Qt::ShiftModifier || e->modifiers() == Qt::ControlModifier );
		return true;
	case Qt::Key_Insert:
	{
		KrViewItem * i = getCurrentKrViewItem();
		if( !i )
			return true;
		i->setSelected( !i->isSelected() );
		if (KrSelectionMode::getSelectionHandler()->insertMovesDown())
		{
			KrViewItem * next = getNext( i );
			if( next )
			{
				setCurrentKrViewItem( next );
				makeItemVisible( next );
			}
		}
		op()->emitSelectionChanged();
		return true;
	}
	case Qt::Key_Space:
	{
		KrViewItem * viewItem = getCurrentKrViewItem();
		if( viewItem != 0 )
		{
			viewItem->setSelected( !viewItem->isSelected() );
			if( viewItem->name() == ".." )
			{
				if (KrSelectionMode::getSelectionHandler()->spaceMovesDown())
				{
					KrViewItem * next = getNext( viewItem );
					if( next )
					{
						setCurrentKrViewItem( next );
						makeItemVisible( next );
					}
				}
				op()->emitSelectionChanged();
				return true;
			}
			if ( viewItem->getVfile()->vfile_isDir() && viewItem->getVfile()->vfile_getSize() <= 0 && 
			     KrSelectionMode::getSelectionHandler()->spaceCalculatesDiskSpace())
			{
				//
				// NOTE: this is buggy incase somewhere down in the folder we're calculating,
				// there's a folder we can't enter (permissions). in that case, the returned
				// size will not be correct.
				//
				KIO::filesize_t totalSize = 0;
				unsigned long totalFiles = 0, totalDirs = 0;
				QStringList items;
				items.push_back( viewItem->name() );
				if ( ACTIVE_PANEL->func->calcSpace( items, totalSize, totalFiles, totalDirs ) ) {
					// did we succeed to calcSpace? we'll fail if we don't have permissions
					if ( totalSize != 0 ) { // just mark it, and bail out
						viewItem->setSize( totalSize );
						viewItem->redraw();
					}
				}
			}
			if (KrSelectionMode::getSelectionHandler()->spaceMovesDown())
			{
				KrViewItem * next = getNext( viewItem );
				if( next )
				{
					setCurrentKrViewItem( next );
					makeItemVisible( next );
				}
			}
			op()->emitSelectionChanged();
		}
		return true;
	}
	case Qt::Key_Backspace :                         // Terminal Emulator bugfix
	case Qt::Key_Left :
		if ( e->modifiers() == Qt::ControlModifier || e->modifiers() == Qt::ShiftModifier ) { // let the panel handle it
			e->ignore();
		} else {          // a normal click - do a lynx-like moving thing
			SLOTS->dirUp(); // ask krusader to move up a directory
		}
		return true;         // safety
	case Qt::Key_Right :
		if ( e->modifiers() == Qt::ControlModifier || e->modifiers() == Qt::ShiftModifier ) { // let the panel handle it
			e->ignore();
		} else { // just a normal click - do a lynx-like moving thing
			KrViewItem *i = getCurrentKrViewItem();
			if( i )
				op()->emitGoInside( i->name() );
		}
		return true;
	case Qt::Key_Up :
		if ( e->modifiers() == Qt::ControlModifier ) { // let the panel handle it - jump to the Location Bar
			e->ignore();
		} else {
			KrViewItem *item = getCurrentKrViewItem();
			if( item )
			{
				if ( e->modifiers() == Qt::ShiftModifier ) {
					item->setSelected( !item->isSelected() );
					op()->emitSelectionChanged();
				}
				item = getPrev( item );
				if ( item ) {
					setCurrentKrViewItem( item );
					makeItemVisible( item );
				}
			}
		}
		return true;
	case Qt::Key_Down :
		if ( e->modifiers() == Qt::ControlModifier || e->modifiers() == ( Qt::ControlModifier | Qt::ShiftModifier ) ) { // let the panel handle it - jump to command line
			e->ignore();
		} else {
			KrViewItem *item = getCurrentKrViewItem();
			if( item )
			{
				if ( e->modifiers() == Qt::ShiftModifier ) {
					item->setSelected( !item->isSelected() );
					op()->emitSelectionChanged();
				}
				item = getNext( item );
				if ( item ) {
					setCurrentKrViewItem( item );
					makeItemVisible( item );
				}
			}
		}
		return true;
	case Qt::Key_Home:
		{
			if ( e->modifiers() & Qt::ShiftModifier )  /* Shift+Home */
			{
				bool select = true;
				KrViewItem *pos = getCurrentKrViewItem();
				if( pos == 0 )
					pos = getLast();
				KrViewItem *item = getFirst();
				op()->setMassSelectionUpdate( true );
				while( item ) {
					item->setSelected( select );
					if( item == pos )
						select = false;
					item = getNext( item );
				}
				op()->setMassSelectionUpdate( false );
			}
			KrViewItem * first = getFirst();
			if( first ) {
				setCurrentKrViewItem( first );
				makeItemVisible( first );
			}
		}
		return true;
	case Qt::Key_End:
		if( e->modifiers() & Qt::ShiftModifier ) {
			bool select = false;
			KrViewItem *pos = getCurrentKrViewItem();
			if( pos == 0 )
				pos = getFirst();
			op()->setMassSelectionUpdate( true );
			KrViewItem *item = getFirst();
			while( item ) {
				if( item == pos )
					select = true;
				item->setSelected( select );
				item = getNext( item );
			}
			op()->setMassSelectionUpdate( false );
		} else {
			KrViewItem *last = getLast();
			if( last ) {
				setCurrentKrViewItem( last );
				makeItemVisible( last );
			}
		}
		return true;
	case Qt::Key_PageDown:
	{
		KrViewItem * current = getCurrentKrViewItem();
		int downStep = itemsPerPage();
		while( downStep != 0 && current ) {
			KrViewItem * newCurrent = getNext( current );
			if( newCurrent == 0 )
				break;
			current = newCurrent;
			downStep--;
		}
		if( current ) {
			setCurrentKrViewItem( current );
			makeItemVisible( current );
		}
		return true;
	}
	case Qt::Key_PageUp:
	{
		KrViewItem * current = getCurrentKrViewItem();
		int upStep = itemsPerPage();
		while( upStep != 0 && current ) {
			KrViewItem * newCurrent = getPrev( current );
			if( newCurrent == 0 )
				break;
			current = newCurrent;
			upStep--;
		}
		if( current ) {
			setCurrentKrViewItem( current );
			makeItemVisible( current );
		}
		return true;
	}
	case Qt::Key_Escape:
		return true; // otherwise the selection gets lost??!??
	case Qt::Key_A :                 // mark all
		if ( e->modifiers() == Qt::ControlModifier ) {
			selectAllIncludingDirs();
			return true;
		}
		// default continues here !!!!!!!!!!!
	default:
		// if the key is A..Z or 1..0 do quick search otherwise...
		if ( e->text().length() > 0 && e->text() [ 0 ].isPrint() ) // better choice. Otherwise non-ascii characters like  can not be the first character of a filename
		// are we doing quicksearch? if not, send keys to panel
		//if ( _config->readBoolEntry( "Do Quicksearch", _DoQuicksearch ) ) {
		// are we using krusader's classic quicksearch, or wincmd style?
		{
			KConfigGroup grpSv( _config, "Look&Feel" );
			if ( !grpSv.readEntry( "New Style Quicksearch", _NewStyleQuicksearch ) )
				return false;
			else {
				// first, show the quicksearch if its hidden
				if ( op()->quickSearch()->isHidden() ) {
					op()->quickSearch()->show();
					// hack: if the pressed key requires a scroll down, the selected
					// item is "below" the quick search window, as the icon view will
					// realize its new size after the key processing. The following line
					// will resize the icon view immediately.
					ACTIVE_PANEL->layout->activate();
					// second, we need to disable the dirup action - hack!
					krDirUp->setEnabled( false );
				}
				// now, send the key to the quicksearch
				op()->quickSearch()->myKeyPressEvent( e );
				return true;
			}
		} else {
			if ( !op()->quickSearch()->isHidden() ) {
				op()->quickSearch()->hide();
				op()->quickSearch()->clear();
				krDirUp->setEnabled( true );
			}
		}
	}
	return false;
}