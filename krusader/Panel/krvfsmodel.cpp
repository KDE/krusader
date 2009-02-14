#include "krvfsmodel.h"
#include "../VFS/vfs.h"
#include "../VFS/vfile.h"
#include <klocale.h>
#include <QtDebug>
#include "../VFS/krpermhandler.h"
#include "../defaults.h"
#include "../krusader.h"
#include "../krusaderview.h"
#include "listpanel.h"
#include "krcolorcache.h"

KrVfsModel::KrVfsModel( KrView * view ): QAbstractListModel(0), _vfs(0), _extensionEnabled( true ), _view( view ) {}

void KrVfsModel::setVfs(vfs* v)
{
	emit layoutAboutToBeChanged();
	
	_vfs = v;
	
	vfile *vf = _vfs->vfs_getFirstFile();
	while (vf) {
			_vfiles.append(vf);
			vf = _vfs->vfs_getNextFile();
	}
	// TODO: connect all addedVfile/deleteVfile and friends signals
	// TODO: make a more efficient implementation that this dummy one :-)
	
	emit dataChanged(index(0, 0), index(_vfs->vfs_noOfFiles()-1, 0));
	emit layoutChanged();
}

KrVfsModel::~KrVfsModel()
{
}

int KrVfsModel::rowCount(const QModelIndex& parent) const
{
	if (!_vfs) return 0;
	
	// simply return the number of items in the vfs
	return _vfs->vfs_noOfFiles();
}


int KrVfsModel::columnCount(const QModelIndex &parent) const {
	return KrVfsModel::MAX_COLUMNS;
}

QVariant KrVfsModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid() || !_vfs)
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
					if( !_extensionEnabled )
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
					if( !_extensionEnabled )
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
