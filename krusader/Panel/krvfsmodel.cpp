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


class SortProps
{
public:
	SortProps( vfile *vf, int col, const KrViewProperties * props, bool isDummy, bool asc )
	{
		_col = col;
		_prop = props;
		_isdummy = isDummy;
		_ascending = asc;
		_vfile = vf;
		if( _col == KrVfsModel::Extension )
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
		}
	}
	
	inline int column() { return _col; }
	inline const KrViewProperties * properties() { return _prop; }
	inline bool isDummy() { return _isdummy; }
	inline bool isAscending() { return _ascending; }
	inline QString extension() { return _ext; }
	inline vfile * vf() { return _vfile; }
	
private:
	int _col;
	const KrViewProperties * _prop;
	bool _isdummy;
	vfile * _vfile;
	bool _ascending;
	QString _ext;
};

typedef bool(*LessThan)(SortProps *,SortProps *);

KrVfsModel::KrVfsModel( KrView * view ): QAbstractListModel(0), _extensionEnabled( true ), _view( view ),
                                         _lastSortOrder( KrVfsModel::Name ), _lastSortDir(Qt::AscendingOrder),
                                         _dummyVfile( 0 ), _ready( false ) {}

void KrVfsModel::setVfs(vfs* v, bool upDir)
{
	emit layoutAboutToBeChanged();
	
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
	
	emit dataChanged(index(0, 0), index(_vfiles.count()-1, 0));
	emit layoutChanged();
	sort();
}

KrVfsModel::~KrVfsModel()
{
}

void KrVfsModel::clear()
{
	emit layoutAboutToBeChanged();
	_vfiles.clear();
	emit layoutChanged();
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
	if (!index.isValid())
		return QVariant();

	if (index.row() >= rowCount())
		return QVariant();
	vfile *vf = _vfiles.at(index.row());
	if( vf == 0 )
		return QVariant();

	switch( role )
	{
		case Qt::FontRole:
		{
			KConfigGroup grpSvr( krConfig, "Look&Feel" );
			return grpSvr.readEntry( "Filelist Font", *_FilelistFont );
		}
		case Qt::DisplayRole:
		{
			switch (index.column()) {
				case KrVfsModel::Name:
				{
					if( !_extensionEnabled || vf->vfile_isDir() )
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
				case KrVfsModel::Extension:
				{
					if( !_extensionEnabled || vf->vfile_isDir() )
						return QVariant();
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
						return QVariant();
					return vfName.mid(loc + 1);
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
				default: return QString();
			}
			return QVariant();
		}
		case Qt::DecorationRole:
		{
			switch (index.column() ) {
				case KrVfsModel::Name:
				{
					if( properties()->displayIcons )
						return KrView::getIcon( vf );
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
					return Qt::AlignRight;
				default:
					return Qt::AlignLeft;
			}
			return QVariant();
		}
		case Qt::BackgroundRole:
		case Qt::ForegroundRole:
		{
			KrColorItemType colorItemType;
			colorItemType.m_activePanel = (_view == ACTIVE_PANEL->view);
			colorItemType.m_alternateBackgroundColor = (index.row() & 1);
			colorItemType.m_currentItem = _view->getCurrentIndex() == index;
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

// compares numbers within two strings
bool compareNumbers(QString& aS1, int& aPos1, QString& aS2, int& aPos2)
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
         if( j != 0 ) return (j == -1);
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
         if( j != 0 ) return (j == -1);
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
	emit layoutAboutToBeChanged();
	
	
	QVector < SortProps * > sorting (_vfiles.count());
	for (int i = 0; i < _vfiles.count(); ++i)
		sorting[ i ] = new SortProps( _vfiles[ i ], column, properties(), _vfiles[ i ] == _dummyVfile, order == Qt::AscendingOrder );
	
	LessThan compare = (order == Qt::AscendingOrder ? &itemLessThan : &itemGreaterThan);
	qSort(sorting.begin(), sorting.end(), compare);
	
	_vfiles.clear();
	_vfileNdx.clear();
	for (int i = 0; i < sorting.count(); ++i) {
		_vfiles.append( sorting[ i ]->vf() );
		_vfileNdx[ sorting[ i ]->vf() ] = index( i, 0 );
		_nameNdx[ sorting[ i ]->vf()->vfile_getName() ] = index( i, 0 );
		delete sorting[ i ];
	}
	
	emit layoutChanged();
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
	}
	return QString();
}

vfile * KrVfsModel::vfileAt( const QModelIndex &index )
{
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

