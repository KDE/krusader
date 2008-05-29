#include "krvfsmodel.h"
#include "../VFS/vfs.h"
#include "../VFS/vfile.h"
#include <QtDebug>

KrVfsModel::KrVfsModel(): QAbstractListModel(0), _vfs(0) {}

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
	return 3;
}

QVariant KrVfsModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid() || !_vfs)
		return QVariant();

	if (index.row() >= rowCount())
		return QVariant();

	if (role != Qt::DisplayRole)
		return QVariant();

	switch (index.column()) {
		case 0: return (_vfiles.at(index.row()))->vfile_getName();
		case 1: return ("<ext>");
		case 2: return (_vfiles.at(index.row()))->vfile_getSize();
		default: return QString();
	}
}

QVariant KrVfsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	// ignore anything that's not display, and not horizontal
	if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
		return QVariant();

	switch (section) {
		case 0: return "Name";
		case 1: return "Ext";
		case 2: return "Size";
	}
	return QString();
}
