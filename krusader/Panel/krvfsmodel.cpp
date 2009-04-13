#include "krvfsmodel.h"
#include "../VFS/vfs.h"
#include "../VFS/vfile.h"
#include <klocale.h>
#include <QtDebug>
#include <QtAlgorithms>
#include "../VFS/krpermhandler.h"
#include "../defaults.h"
#include "../krusader.h"
#include "../krusaderview.h"
#include "listpanel.h"
#include "krcolorcache.h"

#define PERM_BITMASK (S_ISUID|S_ISGID|S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO)

class SortProps
{
public:
	SortProps( vfile *vf, int col, const KrViewProperties * props, bool isDummy, bool asc, int origNdx )
	{
		_col = col;
		_prop = props;
		_isdummy = isDummy;
		_ascending = asc;
		_vfile = vf;
		_index = origNdx;
		
		switch( _col )
		{
		case KrVfsModel::Extension:
		{
			if( vf->vfile_isDir() ) {
				_ext = "";
			} else {
				// check if the file has an extension
				const QString& vfName = vf->vfile_getName();
				int loc = vfName.lastIndexOf('.');
				if (loc>0) { // avoid mishandling of .bashrc and friend
					// check if it has one of the predefined 'atomic extensions'
					for (QStringList::const_iterator i = props->atomicExtensions.begin(); i != props->atomicExtensions.end(); ++i) {
						if (vfName.endsWith(*i) && vfName != *i ) {
							loc = vfName.length() - (*i).length();
							break;
						}
					}
					_ext = vfName.mid(loc);
				} else
					_ext = "";
			}
			break;
		}
		case KrVfsModel::Mime:
		{
			if( isDummy )
				_data = "";
			else
			{
				KMimeType::Ptr mt = KMimeType::mimeType(vf->vfile_getMime());
				if( mt )
					_data = mt->comment();
			}
			break;
		}
		case KrVfsModel::Permissions:
		{
			if( isDummy )
				_data = "";
			else
			{
				if (properties()->numericPermissions) {
					QString perm;
					_data = perm.sprintf("%.4o", vf->vfile_getMode() & PERM_BITMASK);
				} else
					_data = vf->vfile_getPerm();
			}
			break;
		}
		case KrVfsModel::KrPermissions:
		{
			if( isDummy )
				_data = "";
			else
			{
				_data = KrVfsModel::krPermissionString( vf );
			}
			break;
		}
		case KrVfsModel::Owner:
		{
			if( isDummy )
				_data = "";
			else
				_data = vf->vfile_getOwner();
		}
		case KrVfsModel::Group:
		{
			if( isDummy )
				_data = "";
			else
				_data = vf->vfile_getGroup();
		}
		default:
			break;
		}
	}
	
	inline int column() { return _col; }
	inline const KrViewProperties * properties() { return _prop; }
	inline bool isDummy() { return _isdummy; }
	inline bool isAscending() { return _ascending; }
	inline QString extension() { return _ext; }
	inline vfile * vf() { return _vfile; }
	inline int originalIndex() { return _index; }
	inline QString data() { return _data; }

private:
	int _col;
	const KrViewProperties * _prop;
	bool _isdummy;
	vfile * _vfile;
	bool _ascending;
	QString _ext;
	int _index;
	QString _data;
};

typedef bool(*LessThan)(SortProps *,SortProps *);

KrVfsModel::KrVfsModel( KrView * view ): QAbstractListModel(0), _extensionEnabled( true ), _view( view ),
                                         _lastSortOrder( KrVfsModel::Name ), _lastSortDir(Qt::AscendingOrder),
                                         _dummyVfile( 0 ), _ready( false ), _justForSizeHint( false ), 
                                         _alternatingTable( false )
{
	KConfigGroup grpSvr( krConfig, "Look&Feel" );
	_defaultFont = grpSvr.readEntry( "Filelist Font", *_FilelistFont );
	_fileIconSize = (grpSvr.readEntry("Filelist Icon Size",_FilelistIconSize)).toInt();
}

void KrVfsModel::setVfs(vfs* v, bool upDir)
{
	_dummyVfile = 0;
	if( upDir ) {
		_dummyVfile = new vfile( "..", 0, "drwxrwxrwx", 0, false, 0, 0, "", "", 0, -1);
		_dummyVfile->vfile_setIcon( "go-up" );
		_vfiles.append(_dummyVfile);
	}

	vfile *vf = v->vfs_getFirstFile();
	while (vf) {
			bool add = true;
			bool isDir = vf->vfile_isDir();
			if ( !isDir || ( isDir && ( properties()->filter & KrViewProperties::ApplyToDirs ) ) ) {
				switch ( properties()->filter ) {
				case KrViewProperties::All :
					break;
				case KrViewProperties::Custom :
					if ( !properties()->filterMask.match( vf ) )
						add = false;
					break;
				case KrViewProperties::Dirs:
					if ( !isDir )
						add = false;
					break;
				case KrViewProperties::Files:
					if ( isDir )
						add = false;
					break;
				default:
					break;
				}
			}
			if( add )
				_vfiles.append(vf);
			vf = v->vfs_getNextFile();
	}
	_ready = true;
	// TODO: connect all addedVfile/deleteVfile and friends signals
	// TODO: make a more efficient implementation that this dummy one :-)
	
	sort();
}

KrVfsModel::~KrVfsModel()
{
}

void KrVfsModel::clear()
{
	_vfiles.clear();
}

int KrVfsModel::rowCount(const QModelIndex& parent) const
{
	return _vfiles.count();
}


int KrVfsModel::columnCount(const QModelIndex &parent) const {
	return KrVfsModel::MAX_COLUMNS;
}

QVariant KrVfsModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid() || index.row() >= rowCount())
		return QVariant();
	vfile *vf = _vfiles.at(index.row());
	if( vf == 0 )
		return QVariant();

	switch( role )
	{
		case Qt::FontRole:
			return _defaultFont;
		case Qt::EditRole:
		{
			if( index.column() == 0 )
			{
				return vf->vfile_getName();
			}
			return QVariant();
		}
		case Qt::UserRole:
		{
			if( index.column() == 0 )
			{
				return nameWithoutExtension( vf, false );
			}
			return QVariant();
		}
		case Qt::ToolTipRole:
		case Qt::DisplayRole:
		{
			switch (index.column()) {
				case KrVfsModel::Name:
				{
					return nameWithoutExtension( vf );
				}
				case KrVfsModel::Extension:
				{
					QString nameOnly = nameWithoutExtension( vf );
					const QString& vfName = vf->vfile_getName();
					return vfName.mid(nameOnly.length() + 1);
				}
				case KrVfsModel::Size:
				{
					if (vf->vfile_isDir() && vf->vfile_getSize() <= 0)
						return i18n("<DIR>");
	    				else 
						return ( properties()->humanReadableSize) ? 
							KIO::convertSize(vf->vfile_getSize())+"  " :
		 					KRpermHandler::parseSize(vf->vfile_getSize())+" ";
				}
				case KrVfsModel::Mime:
				{
					if( vf == _dummyVfile )
						return QVariant();
					KMimeType::Ptr mt = KMimeType::mimeType(vf->vfile_getMime());
					if( mt )
						return mt->comment();
					return QVariant();
				}
				case KrVfsModel::DateTime:
				{
					if( vf == _dummyVfile )
						return QVariant();
					time_t time = vf->vfile_getTime_t();
					struct tm* t=localtime((time_t *)&time);
					
					QDateTime tmp(QDate(t->tm_year+1900, t->tm_mon+1, t->tm_mday), QTime(t->tm_hour, t->tm_min));
					return KGlobal::locale()->formatDateTime(tmp);
				}
				case KrVfsModel::Permissions:
				{
					if( vf == _dummyVfile )
						return QVariant();
					if (properties()->numericPermissions) {
						QString perm;
						return perm.sprintf("%.4o", vf->vfile_getMode() & PERM_BITMASK);
					}
					return vf->vfile_getPerm();
				}
				case KrVfsModel::KrPermissions:
				{
					if( vf == _dummyVfile )
						return QVariant();
					return krPermissionString( vf );
				}
				case KrVfsModel::Owner:
				{
					if( vf == _dummyVfile )
						return QVariant();
					return vf->vfile_getOwner();
				}
				case KrVfsModel::Group:
				{
					if( vf == _dummyVfile )
						return QVariant();
					return vf->vfile_getGroup();
				}
				default: return QString();
			}
			return QVariant();
		}
		case Qt::DecorationRole:
		{
			switch (index.column() ) {
				case KrVfsModel::Name:
				{
					if( properties()->displayIcons ) {
						if( _justForSizeHint )
							return QPixmap( _fileIconSize, _fileIconSize );
						return KrView::getIcon( vf );
					}
					break;
				}
				default:
					break;
			}
			return QVariant();
		}
		case Qt::TextAlignmentRole:
		{
			switch (index.column() ) {
				case KrVfsModel::Size:
					return QVariant( Qt::AlignRight | Qt::AlignVCenter );
				default:
					return QVariant( Qt::AlignLeft | Qt::AlignVCenter );
			}
			return QVariant();
		}
		case Qt::BackgroundRole:
		case Qt::ForegroundRole:
		{
			KrColorItemType colorItemType;
			colorItemType.m_activePanel = (_view == ACTIVE_PANEL->view);
			int actRow = index.row();
			if( _alternatingTable )
			{
				int itemNum = _view->itemsPerPage();
				if( itemNum == 0 )
					itemNum++;
				if( ( itemNum & 1 ) == 0 )
					actRow += (actRow / itemNum );
			}
			colorItemType.m_alternateBackgroundColor = (actRow & 1);
			colorItemType.m_currentItem = _view->getCurrentIndex().row() == index.row();
			colorItemType.m_selectedItem = _view->isSelected( index );
			if (vf->vfile_isSymLink())
			{
				if (vf->vfile_getMime() == "Broken Link !" )
					colorItemType.m_fileType = KrColorItemType::InvalidSymlink;
				else
					colorItemType.m_fileType = KrColorItemType::Symlink;
			}
			else if (vf->vfile_isDir())
				colorItemType.m_fileType = KrColorItemType::Directory;
			else if (vf->vfile_isExecutable())
				colorItemType.m_fileType = KrColorItemType::Executable;
			else
				colorItemType.m_fileType = KrColorItemType::File;
			
			KrColorGroup cols;
			KrColorCache::getColorCache().getColors(cols, colorItemType);
			
			if( colorItemType.m_selectedItem )
			{
				if( role == Qt::ForegroundRole )
					return cols.highlightedText();
				else
					return cols.highlight();
			}
			if( role == Qt::ForegroundRole )
				return cols.text();
			else
				return cols.background();
		}
		default:
			return QVariant();
	}
}

bool KrVfsModel::setData ( const QModelIndex & index, const QVariant & value, int role )
{
	if( role == Qt::EditRole && index.isValid() )
	{
		if (index.row() < rowCount() && index.row() > 0 )
		{
			vfile *vf = _vfiles.at(index.row());
			if( vf == 0 )
				return false;
			_view->op()->emitRenameItem( vf->vfile_getName(), value.toString() );
		}
	}
	if( role == Qt::UserRole && index.isValid() )
	{
		_justForSizeHint = value.toBool();
	}
	return QAbstractListModel::setData( index, value, role );
}

// compares numbers within two strings
int compareNumbers(QString& aS1, int& aPos1, QString& aS2, int& aPos2)
{
   int res = 0;
   int start1 = aPos1;
   int start2 = aPos2;
   while ( aPos1 < aS1.length() && aS1.at( aPos1 ).isDigit() ) aPos1++;
   while ( aPos2 < aS2.length() && aS2.at( aPos2 ).isDigit() ) aPos2++;
   // the left-most difference determines what's bigger
   int i1 = aPos1 - 1;
   int i2 = aPos2 - 1;
   for ( ; i1 >= start1 || i2 >= start2; i1--, i2--)
   {
     int c1 = 0;
     int c2 = 0;
     if ( i1 >= start1 ) c1 = aS1.at( i1 ).digitValue();
     if ( i2 >= start2 ) c2 = aS2.at( i2 ).digitValue();
     if ( c1 < c2 ) res = -1;
     else if ( c1 > c2 ) res = 1;
   }
   return res;
}


bool compareTextsAlphabetical(QString& aS1, QString& aS2, const KrViewProperties * _viewProperties, bool aNumbers)
{
   int lPositionS1 = 0;
   int lPositionS2 = 0;
   // sometimes, localeAwareCompare is not case sensitive. in that case, we need to fallback to a simple string compare (KDE bug #40131)
   bool lUseLocaleAware = (_viewProperties->sortMode & KrViewProperties::IgnoreCase)
      || _viewProperties->localeAwareCompareIsCaseSensitive;
   int j = 0;
   QChar lchar1;
   QChar lchar2;
   while(true)
   {
      lchar1 = aS1[lPositionS1];
      lchar2 = aS2[lPositionS2];
      // detect numbers
      if(aNumbers && lchar1.isDigit() && lchar2.isDigit() )
      {
         int j = compareNumbers(aS1, lPositionS1, aS2, lPositionS2);
         if( j != 0 ) return j < 0;
      }
      else
      if( lUseLocaleAware
          && 
          ( ( lchar1 >= 128
              && ( (lchar2 >= 'A' && lchar2 <= 'Z') || (lchar2 >= 'a' && lchar2 <= 'z') || lchar2 >= 128 ) )
            ||
            ( lchar2 >= 128
              && ( (lchar1 >= 'A' && lchar1 <= 'Z') || (lchar1 >= 'a' && lchar1 <= 'z') || lchar1 >= 128 ) )
          )
        )
      {
         // use localeAwareCompare when a unicode character is encountered
         j = QString::localeAwareCompare(lchar1, lchar2);
         if(j != 0) return j < 0;
         lPositionS1++;
         lPositionS2++;
      }
      else
      {
         // if characters are latin or localeAwareCompare is not case sensitive then use simple characters compare is enough
         if(lchar1 < lchar2) return true;
         if(lchar1 > lchar2) return false;
         lPositionS1++;
         lPositionS2++;
      }
      // at this point strings are equal, check if ends of strings are reached
      if(lPositionS1 == aS1.length() && lPositionS2 == aS2.length()) return false;
      if(lPositionS1 == aS1.length() && lPositionS2 < aS2.length()) return true;
      if(lPositionS1 < aS1.length() && lPositionS2 == aS2.length()) return false;
   }
}

bool compareTextsCharacterCode(QString& aS1, QString& aS2, const KrViewProperties * _viewProperties, bool aNumbers)
{
   int lPositionS1 = 0;
   int lPositionS2 = 0;
   while(true)
   {
      // detect numbers
      if(aNumbers && aS1[lPositionS1].isDigit() && aS2[lPositionS2].isDigit())
      {
         int j = compareNumbers(aS1, lPositionS1, aS2, lPositionS2);
         if( j != 0 ) return j < 0;
      }
      else
      {
         if(aS1[lPositionS1] < aS2[lPositionS2]) return true;
         if(aS1[lPositionS1] > aS2[lPositionS2]) return false;
         lPositionS1++;
         lPositionS2++;
      }
      // at this point strings are equal, check if ends of strings are reached
      if(lPositionS1 == aS1.length() && lPositionS2 == aS2.length()) return false;
      if(lPositionS1 == aS1.length() && lPositionS2 < aS2.length()) return true;
      if(lPositionS1 < aS1.length() && lPositionS2 == aS2.length()) return false;
   }
}

bool compareTextsKrusader(QString& aS1, QString& aS2, const KrViewProperties * _viewProperties, bool asc, bool isName)
{
   // ensure "hidden" before others
   if( isName )
   {
      if( aS1[0] == '.' && aS2[0] != '.' ) return asc;
      if( aS1[0] != '.' && aS2[0] == '.' ) return !asc;
   }

   // sometimes, localeAwareCompare is not case sensitive. in that case, we need to fallback to a simple string compare (KDE bug #40131)
   bool lUseLocaleAware = (_viewProperties->sortMode & KrViewProperties::IgnoreCase)
      || _viewProperties->localeAwareCompareIsCaseSensitive;

   if( lUseLocaleAware )
      return QString::localeAwareCompare(aS1, aS2) < 0;
   else
      // if localeAwareCompare is not case sensitive then use simple compare is enough
      return QString::compare(aS1, aS2) < 0;
}

bool compareTexts( QString aS1, QString aS2, const KrViewProperties * _viewProperties, bool asc, bool isName)
{
   //check empty strings
   if( aS1.length() == 0 ) {
     return false;
   } else {
     if( aS2.length() == 0 )
        return true;
   }
   
   if( isName )
   {
     if ( aS1 == ".." ) {
        return !asc;
     } else {
        if ( aS2 == ".." )
          return asc;
     }
   }

   if( _viewProperties->sortMode & KrViewProperties::IgnoreCase )
   {
      aS1 = aS1.toLower();
      aS2 = aS2.toLower();
   }

   switch(_viewProperties->sortMethod)
   {
      case KrViewProperties::Alphabetical:
         return compareTextsAlphabetical(aS1, aS2, _viewProperties, false);
      case KrViewProperties::AlphabeticalNumbers:
         return compareTextsAlphabetical(aS1, aS2, _viewProperties, true);
      case KrViewProperties::CharacterCode:
         return compareTextsCharacterCode(aS1, aS2, _viewProperties, false);
      case KrViewProperties::CharacterCodeNumbers:
         return compareTextsCharacterCode(aS1, aS2, _viewProperties, true);
      case KrViewProperties::Krusader:
      default:
         return compareTextsKrusader(aS1, aS2, _viewProperties, asc, isName);
   }
}

bool itemLessThan( SortProps *sp, SortProps *sp2 )
{
	vfile * file1 = sp->vf();
	vfile * file2 = sp2->vf();
	bool isdir1 = file1->vfile_isDir();
	bool isdir2 = file2->vfile_isDir();
	
	if( isdir1 && !isdir2 )
		return sp->isAscending();
	if( isdir2 && !isdir1 )
		return !sp->isAscending();
	
	if( sp->isDummy() )
		return sp->isAscending();
	if( sp2->isDummy() )
		return !sp->isAscending();
	
	bool alwaysSortDirsByName = (sp->properties()->sortMode & KrViewProperties::AlwaysSortDirsByName);
	int column = sp->column();
	if( alwaysSortDirsByName )
		column = KrVfsModel::Name;
	
	switch( sp->column() )
	{
		case KrVfsModel::Name:
			return compareTexts(file1->vfile_getName(), file2->vfile_getName(), sp->properties(), sp->isAscending(), true);
		case KrVfsModel::Extension:
			if( sp->extension() == sp2->extension() )
				return compareTexts(file1->vfile_getName(), file2->vfile_getName(), sp->properties(), sp->isAscending(), true);
			return compareTexts(sp->extension(), sp2->extension(), sp->properties(), sp->isAscending(), true);
		case KrVfsModel::Size:
			if( file1->vfile_getSize() == file2->vfile_getSize() )
				return compareTexts(file1->vfile_getName(), file2->vfile_getName(), sp->properties(), sp->isAscending(), true);
			return file1->vfile_getSize() < file2->vfile_getSize();
		case KrVfsModel::DateTime:
			if( file1->vfile_getTime_t() == file2->vfile_getTime_t() )
				return compareTexts(file1->vfile_getName(), file2->vfile_getName(), sp->properties(), sp->isAscending(), true);
			return file1->vfile_getTime_t() < file2->vfile_getTime_t();
		case KrVfsModel::Mime:
		case KrVfsModel::Permissions:
		case KrVfsModel::KrPermissions:
		case KrVfsModel::Owner:
		case KrVfsModel::Group:
			if( sp->data() == sp2->data() )
				return compareTexts(file1->vfile_getName(), file2->vfile_getName(), sp->properties(), sp->isAscending(), true);
			return compareTexts( sp->data(), sp2->data(), sp->properties(), sp->isAscending(), true );
	}
	file1->vfile_getName() < file2->vfile_getName();
}

bool itemGreaterThan( SortProps *sp, SortProps *sp2 )
{
	return !itemLessThan( sp, sp2 );
}

void KrVfsModel::sort ( int column, Qt::SortOrder order )
{
	_lastSortOrder = column;
	_lastSortDir = order;
	
	if( _view->properties() != 0 )
		_view->sortModeUpdated( convertSortOrderToKrViewProperties( _lastSortOrder, _lastSortDir ) );
	
	emit layoutAboutToBeChanged();
	
	QModelIndexList oldPersistentList = persistentIndexList();
	
	QVector < SortProps * > sorting (_vfiles.count());
	for (int i = 0; i < _vfiles.count(); ++i)
		sorting[ i ] = new SortProps( _vfiles[ i ], column, properties(), _vfiles[ i ] == _dummyVfile, order == Qt::AscendingOrder, i );
	
	LessThan compare = (order == Qt::AscendingOrder ? &itemLessThan : &itemGreaterThan);
	qSort(sorting.begin(), sorting.end(), compare);
	
	_vfiles.clear();
	_vfileNdx.clear();
	
	QHash<int, int> changeMap;
	for (int i = 0; i < sorting.count(); ++i) {
		_vfiles.append( sorting[ i ]->vf() );
		changeMap[ sorting[ i ]->originalIndex() ] = i;
		_vfileNdx[ sorting[ i ]->vf() ] = index( i, 0 );
		_nameNdx[ sorting[ i ]->vf()->vfile_getName() ] = index( i, 0 );
		delete sorting[ i ];
	}
	
	QModelIndexList newPersistentList;
	foreach( QModelIndex mndx, oldPersistentList )
		newPersistentList << index( changeMap[ mndx.row() ], mndx.column() );
	
	changePersistentIndexList(oldPersistentList, newPersistentList);
	
	emit layoutChanged();
	_view->makeItemVisible( _view->getCurrentKrViewItem() );
}

QModelIndex KrVfsModel::addItem( vfile * vf )
{
	bool isDir = vf->vfile_isDir();
	if ( !isDir || ( isDir && ( properties()->filter & KrViewProperties::ApplyToDirs ) ) ) {
		switch ( properties()->filter ) {
		case KrViewProperties::All :
			break;
		case KrViewProperties::Custom :
			if ( !properties()->filterMask.match( vf ) )
				return QModelIndex();
			break;
		case KrViewProperties::Dirs:
			if ( !isDir )
				return QModelIndex();
			break;
		case KrViewProperties::Files:
			if ( isDir )
				return QModelIndex();
			break;
		default:
			break;
		}
	}
	emit layoutAboutToBeChanged();
	
	QModelIndexList oldPersistentList = persistentIndexList();
	
	SortProps insSort( vf, _lastSortOrder, properties(), vf == _dummyVfile, _lastSortDir == Qt::AscendingOrder, -1 );
	
	QVector < SortProps * > sorting (_vfiles.count());
	for (int i = 0; i < _vfiles.count(); ++i)
		sorting[ i ] = new SortProps( _vfiles[ i ], _lastSortOrder, properties(), _vfiles[ i ] == _dummyVfile, _lastSortDir == Qt::AscendingOrder, i );
	
	LessThan compare = (_lastSortDir == Qt::AscendingOrder ? &itemLessThan : &itemGreaterThan);
	QVector<SortProps *>::iterator it = qLowerBound(sorting.begin(), sorting.end(), &insSort, compare);
	
	int insertIndex = _vfiles.count();
	if( it != sorting.end() ) {
		insertIndex = (*it)->originalIndex();
		_vfiles.insert( insertIndex, vf );
	} else
		_vfiles.append( vf );
	
	for (int di = 0; di != sorting.count(); di++ )
		delete sorting[ di ];
	
	for (int i = insertIndex; i < _vfiles.count(); ++i) {
		_vfileNdx[ _vfiles[ i ] ] = index( i, 0 );
		_nameNdx[ _vfiles[ i ]->vfile_getName() ] = index( i, 0 );
	}
	
	QModelIndexList newPersistentList;
	foreach( QModelIndex mndx, oldPersistentList ) {
		int newRow = mndx.row();
		if( newRow >= insertIndex )
			newRow++;
		newPersistentList << index( newRow, mndx.column() );
	}
	
	changePersistentIndexList(oldPersistentList, newPersistentList);
	emit layoutChanged();
	_view->makeItemVisible( _view->getCurrentKrViewItem() );
	
	return index( insertIndex, 0 );
}

QModelIndex KrVfsModel::removeItem( vfile * vf )
{
	QModelIndex currIndex = _view->getCurrentIndex();
	for( int i=0; i != _vfiles.count(); i++ )
	{
		if( _vfiles[ i ] == vf )
		{
			emit layoutAboutToBeChanged();
			QModelIndexList oldPersistentList = persistentIndexList();
			QModelIndexList newPersistentList;
			
			_vfiles.remove( i );
			
			if( currIndex.row() == i )
			{
				if( _vfiles.count() == 0 )
					currIndex = QModelIndex();
				else if( i >= _vfiles.count() )
					currIndex = index( _vfiles.count() - 1, 0 );
				else
					currIndex = index( i, 0 );
			} else if( currIndex.row() > i )
			{
				currIndex = index( currIndex.row()-1, 0 );
			}
			
			for (int ri = i; ri < _vfiles.count(); ++ri) {
				_vfileNdx[ _vfiles[ ri ] ] = index( ri, 0 );
				_nameNdx[ _vfiles[ ri ]->vfile_getName() ] = index( ri, 0 );
			}
			
			foreach( QModelIndex mndx, oldPersistentList ) {
				int newRow = mndx.row();
				if( newRow > i )
					newRow--;
				if( newRow != i )
					newPersistentList << index( newRow, mndx.column() );
				else
					newPersistentList << QModelIndex();
			}
			changePersistentIndexList(oldPersistentList, newPersistentList);
			emit layoutChanged();
			_view->makeItemVisible( _view->getCurrentKrViewItem() );
			return currIndex;
		}
	}
	return currIndex;
}

void KrVfsModel::updateItem( vfile * vf )
{
	// TODO: make it faster
	sort();
}

QVariant KrVfsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	// ignore anything that's not display, and not horizontal
	if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
		return QVariant();

	switch (section) {
		case KrVfsModel::Name: return i18n( "Name" );
		case KrVfsModel::Extension: return i18n( "Ext" );
		case KrVfsModel::Size: return i18n( "Size" );
		case KrVfsModel::Mime: return i18n( "Type" );
		case KrVfsModel::DateTime: return i18n( "Modified" );
		case KrVfsModel::Permissions: return i18n( "Perms" );
		case KrVfsModel::KrPermissions: return i18n( "rwx" );
		case KrVfsModel::Owner: return i18n( "Owner" );
		case KrVfsModel::Group: return i18n( "Group" );
	}
	return QString();
}

vfile * KrVfsModel::vfileAt( const QModelIndex &index )
{
	if( index.row() < 0 || index.row() >= _vfiles.count() )
		return 0;
	return _vfiles[ index.row() ];
}

const QModelIndex & KrVfsModel::vfileIndex( vfile * vf )
{
	return _vfileNdx[ vf ];
}

const QModelIndex & KrVfsModel::nameIndex( const QString & st )
{
	return _nameNdx[ st ];
}

Qt::ItemFlags KrVfsModel::flags ( const QModelIndex & index ) const
{
	Qt::ItemFlags flags = QAbstractListModel::flags( index );
	
	if (!index.isValid())
		return flags;

	if (index.row() >= rowCount())
		return flags;
	vfile *vf = _vfiles.at(index.row());
	if( vf == _dummyVfile )
	{
		flags = (flags & (~Qt::ItemIsSelectable)) | Qt::ItemIsDropEnabled;
	} else
		flags = flags | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
	return flags;
}

QString KrVfsModel::nameWithoutExtension( const vfile * vf, bool checkEnabled ) const
{
	if( (checkEnabled && !_extensionEnabled ) || vf->vfile_isDir() )
		return vf->vfile_getName();
	// check if the file has an extension
	const QString& vfName = vf->vfile_getName();
	int loc = vfName.lastIndexOf('.');
	if (loc>0) { // avoid mishandling of .bashrc and friend
		// check if it has one of the predefined 'atomic extensions'
		for (QStringList::const_iterator i = properties()->atomicExtensions.begin(); i != properties()->atomicExtensions.end(); ++i) {
			if (vfName.endsWith(*i) && vfName != *i ) {
				loc = vfName.length() - (*i).length();
				break;
			}
		}
	} else
		return vfName;
	return vfName.left(loc);
}

QString KrVfsModel::krPermissionString( const vfile * vf )
{
	QString tmp;
	switch ( vf->vfile_isReadable() ){
		case ALLOWED_PERM: tmp+='r'; break;
		case UNKNOWN_PERM: tmp+='?'; break;
		case NO_PERM:      tmp+='-'; break;
	}
	switch ( vf->vfile_isWriteable()){
		case ALLOWED_PERM: tmp+='w'; break;
		case UNKNOWN_PERM: tmp+='?'; break;
		case NO_PERM:      tmp+='-'; break;
	}
	switch ( vf->vfile_isExecutable()){
		case ALLOWED_PERM: tmp+='x'; break;
		case UNKNOWN_PERM: tmp+='?'; break;
		case NO_PERM:      tmp+='-'; break;
	}
	return tmp;
}

int KrVfsModel::convertSortOrderFromKrViewProperties( KrViewProperties::SortSpec sortOrder, Qt::SortOrder & sortDir )
{
	sortDir = ( sortDir & KrViewProperties::Descending ) ? Qt::DescendingOrder : Qt::AscendingOrder;
	if ( sortOrder & KrViewProperties::Name )
		return KrVfsModel::Name;
	if ( sortOrder & KrViewProperties::Ext )
		return KrVfsModel::Extension;
	if ( sortOrder & KrViewProperties::Size )
		return KrVfsModel::Size;
	if ( sortOrder & KrViewProperties::Type )
		return KrVfsModel::Mime;
	if ( sortOrder & KrViewProperties::Modified )
		return KrVfsModel::DateTime;
	if ( sortOrder & KrViewProperties::Permissions )
		return KrVfsModel::Permissions;
	if ( sortOrder & KrViewProperties::KrPermissions )
		return KrVfsModel::KrPermissions;
	if ( sortOrder & KrViewProperties::Owner )
		return KrVfsModel::Owner;
	if ( sortOrder & KrViewProperties::Group )
		return KrVfsModel::Group;
	return -1;
}

KrViewProperties::SortSpec KrVfsModel::convertSortOrderToKrViewProperties( int sortOrder, Qt::SortOrder sortDir )
{
	KrViewProperties::SortSpec sp = KrViewProperties::Name;
	switch( sortOrder )
	{
	case KrVfsModel::Name:
		sp = KrViewProperties::Name;
	case KrVfsModel::Extension:
		sp = KrViewProperties::Ext;
	case KrVfsModel::Mime:
		sp = KrViewProperties::Type;
	case KrVfsModel::Size:
		sp = KrViewProperties::Size;
	case KrVfsModel::DateTime:
		sp = KrViewProperties::Modified;
	case KrVfsModel::Permissions:
		sp = KrViewProperties::Permissions;
	case KrVfsModel::KrPermissions:
		sp = KrViewProperties::KrPermissions;
	case KrVfsModel::Owner:
		sp = KrViewProperties::Owner;
	case KrVfsModel::Group:
		sp = KrViewProperties::Group;
	}
	
	int sortMode = _view->sortMode();
	
	if (sortMode & KrViewProperties::DirsFirst) 
		sp = static_cast<KrViewProperties::SortSpec>(sp | KrViewProperties::DirsFirst);
	if (sortMode & KrViewProperties::IgnoreCase)
		sp = static_cast<KrViewProperties::SortSpec>(sp | KrViewProperties::IgnoreCase);
	if (sortMode & KrViewProperties::AlwaysSortDirsByName)
		sp = static_cast<KrViewProperties::SortSpec>(sp | KrViewProperties::AlwaysSortDirsByName);
	if (sortMode & KrViewProperties::Descending)
		sp = static_cast<KrViewProperties::SortSpec>(sp | KrViewProperties::Descending);
	return sp;
}
