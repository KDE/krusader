/*****************************************************************************
 * Copyright (C) 2002 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2002 Rafi Yanai <yanai@users.sourceforge.net>               *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This package is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this package; if not, write to the Free Software               *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA *
 *****************************************************************************/

#include "krvfsmodel.h"
#include "../VFS/vfs.h"
#include "../VFS/vfile.h"
#include <klocale.h>
#include <QtDebug>
#include <QtAlgorithms>
#include "../VFS/krpermhandler.h"
#include "../defaults.h"
#include "../krglobal.h"
#include "krpanel.h"
#include "krcolorcache.h"
#include "krsort.h"


KrVfsModel::KrVfsModel(KrInterView * view): QAbstractListModel(0), _extensionEnabled(true), _view(view),
        _dummyVfile(0), _ready(false), _justForSizeHint(false),
        _alternatingTable(false)
{
    KConfigGroup grpSvr(krConfig, "Look&Feel");
    _defaultFont = grpSvr.readEntry("Filelist Font", *_FilelistFont);
}

void KrVfsModel::populate(vfs* v, vfile *dummy)
{
    if(dummy)
        _vfiles.append(dummy);
    _dummyVfile = dummy;

    // this is fast, as QList data is implicitly shared
    QList<vfile*> list= v->vfiles();
    foreach(vfile *vf, list) {
        if (!_view->isFiltered(vf))
            _vfiles.append(vf);
    }

    _ready = true;
    // TODO: make a more efficient implementation that this dummy one :-)

    sort();
}

KrVfsModel::~KrVfsModel()
{
}

void KrVfsModel::clear()
{
    if(!_vfiles.count())
        return;
    emit layoutAboutToBeChanged();
    // clear persistent indexes
    QModelIndexList oldPersistentList = persistentIndexList();
    QModelIndexList newPersistentList;
    foreach(const QModelIndex &mndx, oldPersistentList)
        newPersistentList << QModelIndex();
    changePersistentIndexList(oldPersistentList, newPersistentList);

    _vfiles.clear();
    _vfileNdx.clear();
    _nameNdx.clear();
    _dummyVfile = 0;

    emit layoutChanged();
}

int KrVfsModel::rowCount(const QModelIndex& parent) const
{
    return _vfiles.count();
}


int KrVfsModel::columnCount(const QModelIndex &parent) const
{
    return KrViewProperties::MAX_COLUMNS;
}

QVariant KrVfsModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= rowCount())
        return QVariant();
    vfile *vf = _vfiles.at(index.row());
    if (vf == 0)
        return QVariant();

    switch (role) {
    case Qt::FontRole:
        return _defaultFont;
    case Qt::EditRole: {
        if (index.column() == 0) {
            return vf->vfile_getName();
        }
        return QVariant();
    }
    case Qt::UserRole: {
        if (index.column() == 0) {
            return nameWithoutExtension(vf, false);
        }
        return QVariant();
    }
    case Qt::ToolTipRole:
    case Qt::DisplayRole: {
        switch (index.column()) {
        case KrViewProperties::Name: {
            return nameWithoutExtension(vf);
        }
        case KrViewProperties::Ext: {
            QString nameOnly = nameWithoutExtension(vf);
            const QString& vfName = vf->vfile_getName();
            return vfName.mid(nameOnly.length() + 1);
        }
        case KrViewProperties::Size: {
            if (vf->vfile_isDir() && vf->vfile_getSize() <= 0)
                return i18n("<DIR>");
            else
                return (properties()->humanReadableSize) ?
                       KIO::convertSize(vf->vfile_getSize()) + "  " :
                       KRpermHandler::parseSize(vf->vfile_getSize()) + ' ';
        }
        case KrViewProperties::Type: {
            if (vf == _dummyVfile)
                return QVariant();
            KMimeType::Ptr mt = KMimeType::mimeType(vf->vfile_getMime());
            if (mt)
                return mt->comment();
            return QVariant();
        }
        case KrViewProperties::Modified: {
            if (vf == _dummyVfile)
                return QVariant();
            time_t time = vf->vfile_getTime_t();
            struct tm* t = localtime((time_t *) & time);

            QDateTime tmp(QDate(t->tm_year + 1900, t->tm_mon + 1, t->tm_mday), QTime(t->tm_hour, t->tm_min));
            return KGlobal::locale()->formatDateTime(tmp);
        }
        case KrViewProperties::Permissions: {
            if (vf == _dummyVfile)
                return QVariant();
            if (properties()->numericPermissions) {
                QString perm;
                return perm.sprintf("%.4o", vf->vfile_getMode() & PERM_BITMASK);
            }
            return vf->vfile_getPerm();
        }
        case KrViewProperties::KrPermissions: {
            if (vf == _dummyVfile)
                return QVariant();
            return KrView::krPermissionString(vf);
        }
        case KrViewProperties::Owner: {
            if (vf == _dummyVfile)
                return QVariant();
            return vf->vfile_getOwner();
        }
        case KrViewProperties::Group: {
            if (vf == _dummyVfile)
                return QVariant();
            return vf->vfile_getGroup();
        }
        default: return QString();
        }
        return QVariant();
    }
    case Qt::DecorationRole: {
        switch (index.column()) {
        case KrViewProperties::Name: {
            if (properties()->displayIcons) {
                if (_justForSizeHint)
                    return QPixmap(_view->fileIconSize(), _view->fileIconSize());
                return _view->getIcon(vf);
            }
            break;
        }
        default:
            break;
        }
        return QVariant();
    }
    case Qt::TextAlignmentRole: {
        switch (index.column()) {
        case KrViewProperties::Size:
            return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        default:
            return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
        }
        return QVariant();
    }
    case Qt::BackgroundRole:
    case Qt::ForegroundRole: {
        KrColorItemType colorItemType;
        colorItemType.m_activePanel = _view->isFocused();
        int actRow = index.row();
        if (_alternatingTable) {
            int itemNum = _view->itemsPerPage();
            if (itemNum == 0)
                itemNum++;
            if ((itemNum & 1) == 0)
                actRow += (actRow / itemNum);
        }
        colorItemType.m_alternateBackgroundColor = (actRow & 1);
        colorItemType.m_currentItem = _view->getCurrentIndex().row() == index.row();
        colorItemType.m_selectedItem = _view->isSelected(index);
        if (vf->vfile_isSymLink()) {
            if (vf->vfile_getMime() == "Broken Link !")
                colorItemType.m_fileType = KrColorItemType::InvalidSymlink;
            else
                colorItemType.m_fileType = KrColorItemType::Symlink;
        } else if (vf->vfile_isDir())
            colorItemType.m_fileType = KrColorItemType::Directory;
        else if (vf->vfile_isExecutable())
            colorItemType.m_fileType = KrColorItemType::Executable;
        else
            colorItemType.m_fileType = KrColorItemType::File;

        KrColorGroup cols;
        KrColorCache::getColorCache().getColors(cols, colorItemType);

        if (colorItemType.m_selectedItem) {
            if (role == Qt::ForegroundRole)
                return cols.highlightedText();
            else
                return cols.highlight();
        }
        if (role == Qt::ForegroundRole)
            return cols.text();
        else
            return cols.background();
    }
    default:
        return QVariant();
    }
}

bool KrVfsModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (role == Qt::EditRole && index.isValid()) {
        if (index.row() < rowCount() && index.row() > 0) {
            vfile *vf = _vfiles.at(index.row());
            if (vf == 0)
                return false;
            _view->op()->emitRenameItem(vf->vfile_getName(), value.toString());
        }
    }
    if (role == Qt::UserRole && index.isValid()) {
        _justForSizeHint = value.toBool();
    }
    return QAbstractListModel::setData(index, value, role);
}

void KrVfsModel::sort(int column, Qt::SortOrder order)
{
    _view->sortModeUpdated(column, order);

    emit layoutAboutToBeChanged();

    QModelIndexList oldPersistentList = persistentIndexList();

    QVector<KrSort::SortProps*> sorting(_vfiles.count());
    for (int i = 0; i < _vfiles.count(); ++i)
        sorting[ i ] = new KrSort::SortProps(_vfiles[ i ], column, properties(), _vfiles[ i ] == _dummyVfile, order == Qt::AscendingOrder, i);

    KrSort::sort(sorting, order == Qt::DescendingOrder);

    _vfiles.clear();
    _vfileNdx.clear();
    _nameNdx.clear();

    bool sortOrderChanged = false;
    QHash<int, int> changeMap;
    for (int i = 0; i < sorting.count(); ++i) {
        _vfiles.append(sorting[ i ]->vf());
        changeMap[ sorting[ i ]->originalIndex()] = i;
        if (i != sorting[ i ]->originalIndex())
            sortOrderChanged = true;
        _vfileNdx[ sorting[ i ]->vf()] = index(i, 0);
        _nameNdx[ sorting[ i ]->vf()->vfile_getName()] = index(i, 0);
        delete sorting[ i ];
    }

    QModelIndexList newPersistentList;
    foreach(const QModelIndex &mndx, oldPersistentList)
        newPersistentList << index(changeMap[ mndx.row()], mndx.column());

    changePersistentIndexList(oldPersistentList, newPersistentList);

    emit layoutChanged();
    if (sortOrderChanged)
        _view->makeItemVisible(_view->getCurrentKrViewItem());
}

QModelIndex KrVfsModel::addItem(vfile * vf)
{
    emit layoutAboutToBeChanged();

    QModelIndexList oldPersistentList = persistentIndexList();

    KrSort::SortProps insSort(vf, lastSortOrder(), properties(), vf == _dummyVfile, lastSortDir() == Qt::AscendingOrder, -1);

    QVector<KrSort::SortProps*> sorting(_vfiles.count());
    for (int i = 0; i < _vfiles.count(); ++i)
        sorting[ i ] = new KrSort::SortProps(_vfiles[ i ], lastSortOrder(), properties(), _vfiles[ i ] == _dummyVfile, lastSortDir() == Qt::AscendingOrder, i);

    QVector<KrSort::SortProps*>::iterator it =
            KrSort::lowerBound(sorting, &insSort, lastSortDir() == Qt::DescendingOrder);

    int insertIndex = _vfiles.count();
    if (it != sorting.end()) {
        insertIndex = (*it)->originalIndex();
        _vfiles.insert(insertIndex, vf);
    } else
        _vfiles.append(vf);

    for (int di = 0; di != sorting.count(); di++)
        delete sorting[ di ];

    for (int i = insertIndex; i < _vfiles.count(); ++i) {
        _vfileNdx[ _vfiles[ i ] ] = index(i, 0);
        _nameNdx[ _vfiles[ i ]->vfile_getName()] = index(i, 0);
    }

    QModelIndexList newPersistentList;
    foreach(const QModelIndex &mndx, oldPersistentList) {
        int newRow = mndx.row();
        if (newRow >= insertIndex)
            newRow++;
        newPersistentList << index(newRow, mndx.column());
    }

    changePersistentIndexList(oldPersistentList, newPersistentList);
    emit layoutChanged();
    _view->makeItemVisible(_view->getCurrentKrViewItem());

    return index(insertIndex, 0);
}

QModelIndex KrVfsModel::removeItem(vfile * vf)
{
    QModelIndex currIndex = _view->getCurrentIndex();
    int removeIdx = _vfiles.indexOf(vf);
    if(removeIdx < 0)
        return currIndex;

    emit layoutAboutToBeChanged();
    QModelIndexList oldPersistentList = persistentIndexList();
    QModelIndexList newPersistentList;

    _vfiles.remove(removeIdx);

    if (currIndex.row() == removeIdx) {
        if (_vfiles.count() == 0)
            currIndex = QModelIndex();
        else if (removeIdx >= _vfiles.count())
            currIndex = index(_vfiles.count() - 1, 0);
        else
            currIndex = index(removeIdx, 0);
    } else if (currIndex.row() > removeIdx) {
        currIndex = index(currIndex.row() - 1, 0);
    }

    _vfileNdx.remove(vf);
    _nameNdx.remove(vf->vfile_getName());
    // update model/name index for vfiles following vf
    for (int i = removeIdx; i < _vfiles.count(); i++) {
        _vfileNdx[ _vfiles[i] ] = index(i, 0);
        _nameNdx[ _vfiles[i]->vfile_getName() ] = index(i, 0);
    }

    foreach(const QModelIndex &mndx, oldPersistentList) {
        int newRow = mndx.row();
        if (newRow > removeIdx)
            newRow--;
        if (newRow != removeIdx)
            newPersistentList << index(newRow, mndx.column());
        else
            newPersistentList << QModelIndex();
    }
    changePersistentIndexList(oldPersistentList, newPersistentList);
    emit layoutChanged();
    _view->makeItemVisible(_view->getCurrentKrViewItem());

    return currIndex;
}

void KrVfsModel::updateItem(vfile * vf)
{
    QModelIndex lastIndex = vfileIndex(vf);

    if (!lastIndex.isValid())
        QModelIndex newIdx = addItem(vf);

    QVector<KrSort::SortProps*> sorting(_vfiles.count());
    int oldIndex = -1;
    for (int i = 0; i < _vfiles.count(); ++i) {
        sorting[ i ] = new KrSort::SortProps(_vfiles[ i ], lastSortOrder(), properties(), _vfiles[ i ] == _dummyVfile, lastSortDir() == Qt::AscendingOrder, i);
        if (_vfiles[ i ] == vf)
            oldIndex = i;
    }
    if (oldIndex < 0) {
        // internal error
        abort();
    }
    KrSort::SortProps *updateSort = sorting[ oldIndex ];
    sorting.remove(oldIndex);
    _vfiles.remove(oldIndex);

    emit layoutAboutToBeChanged();

    QModelIndexList oldPersistentList = persistentIndexList();

    QVector<KrSort::SortProps*>::iterator it =
        KrSort::lowerBound(sorting, updateSort, lastSortDir() == Qt::DescendingOrder);

    int newIndex = _vfiles.count();
    if (it != sorting.end()) {
        newIndex = (*it)->originalIndex();
        if (newIndex > oldIndex)
            newIndex--;
        _vfiles.insert(newIndex, vf);
    } else
        _vfiles.append(vf);

    for (int di = 0; di != sorting.count(); di++)
        delete sorting[ di ];
    delete updateSort;

    int i = newIndex;
    if (oldIndex < i)
        i = oldIndex;
    for (; i < _vfiles.count(); ++i) {
        _vfileNdx[ _vfiles[ i ] ] = index(i, 0);
        _nameNdx[ _vfiles[ i ]->vfile_getName()] = index(i, 0);
    }

    QModelIndexList newPersistentList;
    foreach(const QModelIndex &mndx, oldPersistentList) {
        int newRow = mndx.row();
        if (newRow == oldIndex)
            newRow = newIndex;
        else {
            if (newRow >= oldIndex)
                newRow--;
            if (mndx.row() > newIndex)
                newRow++;
        }
        newPersistentList << index(newRow, mndx.column());
    }

    changePersistentIndexList(oldPersistentList, newPersistentList);
    emit layoutChanged();
    if (newIndex != oldIndex)
        _view->makeItemVisible(_view->getCurrentKrViewItem());
}

QVariant KrVfsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    // ignore anything that's not display, and not horizontal
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
        return QVariant();

    switch (section) {
    case KrViewProperties::Name: return i18n("Name");
    case KrViewProperties::Ext: return i18n("Ext");
    case KrViewProperties::Size: return i18n("Size");
    case KrViewProperties::Type: return i18n("Type");
    case KrViewProperties::Modified: return i18n("Modified");
    case KrViewProperties::Permissions: return i18n("Perms");
    case KrViewProperties::KrPermissions: return i18n("rwx");
    case KrViewProperties::Owner: return i18n("Owner");
    case KrViewProperties::Group: return i18n("Group");
    }
    return QString();
}

vfile * KrVfsModel::vfileAt(const QModelIndex &index)
{
    if (index.row() < 0 || index.row() >= _vfiles.count())
        return 0;
    return _vfiles[ index.row()];
}

const QModelIndex & KrVfsModel::vfileIndex(const vfile * vf)
{
    return _vfileNdx[ (vfile*) vf ];
}

const QModelIndex & KrVfsModel::nameIndex(const QString & st)
{
    return _nameNdx[ st ];
}

Qt::ItemFlags KrVfsModel::flags(const QModelIndex & index) const
{
    Qt::ItemFlags flags = QAbstractListModel::flags(index);

    if (!index.isValid())
        return flags;

    if (index.row() >= rowCount())
        return flags;
    vfile *vf = _vfiles.at(index.row());
    if (vf == _dummyVfile) {
        flags = (flags & (~Qt::ItemIsSelectable)) | Qt::ItemIsDropEnabled;
    } else
        flags = flags | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
    return flags;
}

QString KrVfsModel::nameWithoutExtension(const vfile * vf, bool checkEnabled) const
{
    if ((checkEnabled && !_extensionEnabled) || vf->vfile_isDir())
        return vf->vfile_getName();
    // check if the file has an extension
    const QString& vfName = vf->vfile_getName();
    int loc = vfName.lastIndexOf('.');
    if (loc > 0) { // avoid mishandling of .bashrc and friend
        // check if it has one of the predefined 'atomic extensions'
        for (QStringList::const_iterator i = properties()->atomicExtensions.begin(); i != properties()->atomicExtensions.end(); ++i) {
            if (vfName.endsWith(*i) && vfName != *i) {
                loc = vfName.length() - (*i).length();
                break;
            }
        }
    } else
        return vfName;
    return vfName.left(loc);
}
