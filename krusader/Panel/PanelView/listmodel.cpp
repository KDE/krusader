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

#include "listmodel.h"

#include "krinterview.h"
#include "krviewproperties.h"
#include "../FileSystem/fileitem.h"
#include "../defaults.h"
#include "../krcolorcache.h"
#include "../krglobal.h"
#include "../krpanel.h"

#include <KConfigCore/KSharedConfig>
#include <KI18n/KLocalizedString>

ListModel::ListModel(KrInterView *view)
    : QAbstractListModel(0), _extensionEnabled(true), _view(view), _dummyFileItem(0), _ready(false),
      _justForSizeHint(false), _alternatingTable(false)
{
    KConfigGroup grpSvr(krConfig, "Look&Feel");
    _defaultFont = grpSvr.readEntry("Filelist Font", _FilelistFont);
}

void ListModel::populate(const QList<FileItem *> &files, FileItem *dummy)
{
    _fileItems = files;
    _dummyFileItem = dummy;
    _ready = true;

    if(lastSortOrder() != KrViewProperties::NoColumn)
        sort();
    else {
        emit layoutAboutToBeChanged();
        for(int i = 0; i < _fileItems.count(); i++) {
            updateIndices(_fileItems[i], i);
        }
        emit layoutChanged();
    }
}

ListModel::~ListModel()
{
}

void ListModel::clear(bool emitLayoutChanged)
{
    if(!_fileItems.count())
        return;

    emit layoutAboutToBeChanged();
    // clear persistent indexes
    QModelIndexList oldPersistentList = persistentIndexList();
    QModelIndexList newPersistentList;
    newPersistentList.reserve(oldPersistentList.size());
    for (int i=0; i< oldPersistentList.size(); ++i)
        newPersistentList.append(QModelIndex());
    changePersistentIndexList(oldPersistentList, newPersistentList);

    _fileItems.clear();
    _fileItemNdx.clear();
    _nameNdx.clear();
    _urlNdx.clear();
    _dummyFileItem = 0;

    if (emitLayoutChanged)
        emit layoutChanged();
}

int ListModel::rowCount(const QModelIndex& /*parent*/) const
{
    return _fileItems.count();
}


int ListModel::columnCount(const QModelIndex& /*parent*/) const
{
    return KrViewProperties::MAX_COLUMNS;
}

QVariant ListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= rowCount())
        return QVariant();
    FileItem *fileitem = _fileItems.at(index.row());
    if (fileitem == 0)
        return QVariant();

    switch (role) {
    case Qt::FontRole:
        return _defaultFont;
    case Qt::EditRole: {
        if (index.column() == 0) {
            return fileitem->getName();
        }
        return QVariant();
    }
    case Qt::UserRole: {
        if (index.column() == 0) {
            return nameWithoutExtension(fileitem, false);
        }
        return QVariant();
    }
    case Qt::ToolTipRole: {
        if (index.column() == KrViewProperties::Name) {
            return fileitem == _dummyFileItem ? QVariant() : toolTipText(fileitem);
        }
        // breaktrough
    }
    case Qt::DisplayRole: {
        switch (index.column()) {
        case KrViewProperties::Name: {
            return nameWithoutExtension(fileitem);
        }
        case KrViewProperties::Ext: {
            QString nameOnly = nameWithoutExtension(fileitem);
            const QString& fileitemName = fileitem->getName();
            return fileitemName.mid(nameOnly.length() + 1);
        }
        case KrViewProperties::Size: {
            if (fileitem->getUISize() == (KIO::filesize_t)-1) {
                //HACK add <> brackets AFTER translating - otherwise KUIT thinks it's a tag
                static QString label = QString("<") +
                    i18nc("Show the string 'DIR' instead of file size in detailed view (for folders)", "DIR") + '>';
                return label;
            } else
                return KrView::sizeText(properties(), fileitem->getUISize());
        }
        case KrViewProperties::Type: {
            if (fileitem == _dummyFileItem)
                return QVariant();
            const QString mimeType = KrView::mimeTypeText(fileitem);
            return mimeType.isEmpty() ? QVariant() : mimeType;
        }
        case KrViewProperties::Modified: {
            return fileitem == _dummyFileItem ? QVariant() : dateText(fileitem->getTime_t());
        }
        case KrViewProperties::Changed: {
            return fileitem == _dummyFileItem ? QVariant() : dateText(fileitem->getChangedTime());
        }
        case KrViewProperties::Accessed: {
            return fileitem == _dummyFileItem ? QVariant() : dateText(fileitem->getAccessTime());
        }
        case KrViewProperties::Permissions: {
            if (fileitem == _dummyFileItem)
                return QVariant();
            return KrView::permissionsText(properties(), fileitem);
        }
        case KrViewProperties::KrPermissions: {
            if (fileitem == _dummyFileItem)
                return QVariant();
            return KrView::krPermissionText(fileitem);
        }
        case KrViewProperties::Owner: {
            if (fileitem == _dummyFileItem)
                return QVariant();
            return fileitem->getOwner();
        }
        case KrViewProperties::Group: {
            if (fileitem == _dummyFileItem)
                return QVariant();
            return fileitem->getGroup();
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
                return _view->getIcon(fileitem);
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
        if (fileitem->isSymLink()) {
            if (fileitem->isBrokenLink())
                colorItemType.m_fileType = KrColorItemType::InvalidSymlink;
            else
                colorItemType.m_fileType = KrColorItemType::Symlink;
        } else if (fileitem->isDir())
            colorItemType.m_fileType = KrColorItemType::Directory;
        else if (fileitem->isExecutable())
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

bool ListModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (role == Qt::EditRole && index.isValid()) {
        if (index.row() < rowCount() && index.row() >= 0) {
            FileItem *fileitem = _fileItems.at(index.row());
            if (fileitem == 0)
                return false;
            _view->op()->emitRenameItem(fileitem->getName(), value.toString());
        }
    }
    if (role == Qt::UserRole && index.isValid()) {
        _justForSizeHint = value.toBool();
    }
    return QAbstractListModel::setData(index, value, role);
}

void ListModel::sort(int column, Qt::SortOrder order)
{
    _view->sortModeUpdated(column, order);

    if(lastSortOrder() == KrViewProperties::NoColumn)
        return;

    emit layoutAboutToBeChanged();

    QModelIndexList oldPersistentList = persistentIndexList();

    KrSort::Sorter sorter(createSorter());
    sorter.sort();

    _fileItems.clear();
    _fileItemNdx.clear();
    _nameNdx.clear();
    _urlNdx.clear();

    bool sortOrderChanged = false;
    QHash<int, int> changeMap;
    for (int i = 0; i < sorter.items().count(); ++i) {
        const KrSort::SortProps *props = sorter.items()[i];
        _fileItems.append(props->fileitem());
        changeMap[ props->originalIndex() ] = i;
        if (i != props->originalIndex())
            sortOrderChanged = true;
        updateIndices(props->fileitem(), i);
    }

    QModelIndexList newPersistentList;
    foreach(const QModelIndex &mndx, oldPersistentList)
        newPersistentList << index(changeMap[ mndx.row()], mndx.column());

    changePersistentIndexList(oldPersistentList, newPersistentList);

    emit layoutChanged();
    if (sortOrderChanged)
        _view->makeItemVisible(_view->getCurrentKrViewItem());
}

QModelIndex ListModel::addItem(FileItem *fileitem)
{
    emit layoutAboutToBeChanged();

    if(lastSortOrder() == KrViewProperties::NoColumn) {
        int idx = _fileItems.count();
        _fileItems.append(fileitem);
        updateIndices(fileitem, idx);
        emit layoutChanged();
        return index(idx, 0);
    }

    QModelIndexList oldPersistentList = persistentIndexList();

    KrSort::Sorter sorter(createSorter());

    int insertIndex = sorter.insertIndex(fileitem, fileitem == _dummyFileItem, customSortData(fileitem));
    if (insertIndex != _fileItems.count())
        _fileItems.insert(insertIndex, fileitem);
    else
        _fileItems.append(fileitem);

    for (int i = insertIndex; i < _fileItems.count(); ++i) {
        updateIndices(_fileItems[i], i);
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

QModelIndex ListModel::removeItem(FileItem *fileitem)
{
    QModelIndex currIndex = _view->getCurrentIndex();
    int removeIdx = _fileItems.indexOf(fileitem);
    if(removeIdx < 0)
        return currIndex;

    emit layoutAboutToBeChanged();
    QModelIndexList oldPersistentList = persistentIndexList();
    QModelIndexList newPersistentList;

    _fileItems.removeAt(removeIdx);

    if (currIndex.row() == removeIdx) {
        if (_fileItems.count() == 0)
            currIndex = QModelIndex();
        else if (removeIdx >= _fileItems.count())
            currIndex = index(_fileItems.count() - 1, 0);
        else
            currIndex = index(removeIdx, 0);
    } else if (currIndex.row() > removeIdx) {
        currIndex = index(currIndex.row() - 1, 0);
    }

    _fileItemNdx.remove(fileitem);
    _nameNdx.remove(fileitem->getName());
    _urlNdx.remove(fileitem->getUrl());
    // update indices for fileItems following fileitem
    for (int i = removeIdx; i < _fileItems.count(); i++) {
        updateIndices(_fileItems[i], i);
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

QVariant ListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    // ignore anything that's not display, and not horizontal
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
        return QVariant();

    switch (section) {
    case KrViewProperties::Name: return i18nc("File property", "Name");
    case KrViewProperties::Ext: return i18nc("File property", "Ext");
    case KrViewProperties::Size: return i18nc("File property", "Size");
    case KrViewProperties::Type: return i18nc("File property", "Type");
    case KrViewProperties::Modified: return i18nc("File property", "Modified");
    case KrViewProperties::Changed: return i18nc("File property", "Changed");
    case KrViewProperties::Accessed: return i18nc("File property", "Accessed");
    case KrViewProperties::Permissions: return i18nc("File property", "Perms");
    case KrViewProperties::KrPermissions: return i18nc("File property", "rwx");
    case KrViewProperties::Owner: return i18nc("File property", "Owner");
    case KrViewProperties::Group: return i18nc("File property", "Group");
    }
    return QString();
}

const KrViewProperties *ListModel::properties() const
{
    return _view->properties();
}

FileItem *ListModel::fileItemAt(const QModelIndex &index)
{
    if (!index.isValid() || index.row() < 0 || index.row() >= _fileItems.count())
        return 0;
    return _fileItems[ index.row()];
}

const QModelIndex & ListModel::fileItemIndex(const FileItem *fileitem)
{
    return _fileItemNdx[ (FileItem *) fileitem ];
}

const QModelIndex & ListModel::nameIndex(const QString & st)
{
    return _nameNdx[ st ];
}

Qt::ItemFlags ListModel::flags(const QModelIndex & index) const
{
    Qt::ItemFlags flags = QAbstractListModel::flags(index);

    if (!index.isValid())
        return flags;

    if (index.row() >= rowCount())
        return flags;
    FileItem *fileitem = _fileItems.at(index.row());
    if (fileitem == _dummyFileItem) {
        flags = (flags & (~Qt::ItemIsSelectable)) | Qt::ItemIsDropEnabled;
    } else
        flags = flags | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
    return flags;
}

Qt::SortOrder ListModel::lastSortDir() const
{
    return (properties()->sortOptions & KrViewProperties::Descending) ?
                Qt::DescendingOrder : Qt::AscendingOrder;
}

int ListModel::lastSortOrder() const
{
    return properties()->sortColumn;
}

QString ListModel::nameWithoutExtension(const FileItem *fileItem, bool checkEnabled) const
{
    if ((checkEnabled && !_extensionEnabled) || fileItem->isDir())
        return fileItem->getName();
    // check if the file has an extension
    const QString& fileItemName = fileItem->getName();
    int loc = fileItemName.lastIndexOf('.');
    // avoid mishandling of .bashrc and friend
    // and virtfs / search result names like "/dir/.file" which whould become "/dir/"
    if (loc > 0 && fileItemName.lastIndexOf('/') < loc) {
        // check if it has one of the predefined 'atomic extensions'
        for (QStringList::const_iterator i = properties()->atomicExtensions.begin(); i != properties()->atomicExtensions.end(); ++i) {
            if (fileItemName.endsWith(*i) && fileItemName != *i) {
                loc = fileItemName.length() - (*i).length();
                break;
            }
        }
    } else
        return fileItemName;
    return fileItemName.left(loc);
}

const QModelIndex &ListModel::indexFromUrl(const QUrl &url)
{
    return _urlNdx[url];
}

KrSort::Sorter ListModel::createSorter()
{
    KrSort::Sorter sorter(_fileItems.count(), properties(), lessThanFunc(), greaterThanFunc());
    for(int i = 0; i < _fileItems.count(); i++)
        sorter.addItem(_fileItems[i], _fileItems[i] == _dummyFileItem, i, customSortData(_fileItems[i]));
    return sorter;
}

void ListModel::updateIndices(FileItem *file, int i)
{
    _fileItemNdx[file] = index(i, 0);
    _nameNdx[file->getName()] = index(i, 0);
    _urlNdx[file->getUrl()] = index(i, 0);
}

QString ListModel::toolTipText(FileItem *fileItem) const
{
    //"<p style='white-space:pre'>"; // disable automatic word-wrap
    QString text = "<b>" + fileItem->getName() + "</b><hr>";
    if (fileItem->getUISize() != (KIO::filesize_t)-1) {
        const QString size = KrView::sizeText(properties(), fileItem->getUISize());
        text += i18n("Size: %1", size) + "<br>";
    }
    text += i18nc("File property", "Type: %1", KrView::mimeTypeText(fileItem));
    text += "<br>" + i18nc("File property", "Modified: %1", dateText(fileItem->getTime_t()));
    text += "<br>" + i18nc("File property", "Changed: %1", dateText(fileItem->getChangedTime()));
    text += "<br>" + i18nc("File property", "Last Access: %1", dateText(fileItem->getAccessTime()));
    text += "<br>" + i18nc("File property", "Permissions: %1",
            KrView::permissionsText(properties(), fileItem));
    text += "<br>" + i18nc("File property", "Owner: %1", fileItem->getOwner());
    text += "<br>" + i18nc("File property", "Group: %1", fileItem->getGroup());
    if (fileItem->isSymLink()) {
        KLocalizedString ls;
        if (fileItem->isBrokenLink())
            ls = ki18nc("File property; broken symbolic link", "Link to: %1 - (broken)");
        else
            ls = ki18nc("File property", "Link to: %1");
        text += "<br>" + ls.subs(fileItem->getSymDest()).toString();
    }
    return text;
}

QString ListModel::dateText(time_t time)
{
    struct tm* t = localtime((time_t *) & time);

    const QDateTime dateTime(QDate(t->tm_year + 1900, t->tm_mon + 1, t->tm_mday),
                             QTime(t->tm_hour, t->tm_min));
    return QLocale().toString(dateTime, QLocale::ShortFormat);
}
