/*
    SPDX-FileCopyrightText: 2000-2002 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2002 Rafi Yanai <yanai@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KRVIEWITEM_H
#define KRVIEWITEM_H

// QtCore
#include <QRect>
#include <QString>
// QtGui
#include <QPixmap>

#include <KIO/Global>

class FileItem;
class KrInterView;
class KrViewProperties;

/**
 * @brief A view item representing a file inside a KrView
 */
class KrViewItem
{
    friend class KrView;

public:
    KrViewItem(FileItem *fileitem, KrInterView *parentView);
    virtual ~KrViewItem()
    {
    }

    const QString &name(bool withExtension = true) const;
    inline bool hasExtension() const
    {
        return _hasExtension;
    }
    inline const QString &extension() const
    {
        return _extension;
    }
    /** Return description text for status bar. */
    QString description() const;

    QPixmap icon();

    bool isSelected() const;
    void setSelected(bool s);
    QRect itemRect() const;
    void redraw();

    // DON'T USE THOSE OUTSIDE THE VIEWS!!!
    inline const FileItem *getFileItem() const
    {
        return _fileitem;
    }
    inline void setFileItem(FileItem *fileitem)
    {
        _fileitem = fileitem;
    }
    inline FileItem *getMutableFileItem()
    {
        return _fileitem;
    }
    inline bool isDummy() const
    {
        return dummyFileItem;
    }
    inline bool isHidden() const
    {
        return _hidden;
    }

    // used INTERNALLY when calculation of dir size changes the displayed size of the item
    void setSize(KIO::filesize_t size);

protected:
    FileItem *_fileitem; // each view item holds a pointer to a corresponding file item for fast access
    KrInterView *_view; // the parent view this item belongs to
    bool dummyFileItem; // used in case our item represents the ".." (updir) item
    const KrViewProperties *_viewProperties;
    bool _hasExtension;
    bool _hidden;
    QString _name;
    QString _extension;
};

#endif
