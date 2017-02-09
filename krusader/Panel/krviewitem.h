/*****************************************************************************
 * Copyright (C) 2000-2002 Shie Erlich <erlich@users.sourceforge.net>        *
 * Copyright (C) 2000-2002 Rafi Yanai <yanai@users.sourceforge.net>          *
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

#ifndef KRVIEWITEM_H
#define KRVIEWITEM_H

#include "../VFS/vfile.h"
#include "krview.h"

// QtCore
#include <QRect>
#include <QString>
// QtGui
#include <QPixmap>

#include <KIO/Global>

class KrInterView;

/**
 * @brief A view item representing a file inside a KrView
 */
class KrViewItem
{
    friend class KrView;
    friend class KrCalcSpaceDialog;

public:
    KrViewItem(vfile *vf, KrInterView *parentView);
    virtual ~KrViewItem() {}

    virtual const QString& name(bool withExtension = true) const;
    virtual inline bool hasExtension() const {
        return _hasExtension;
    }
    virtual inline const QString& extension() const {
        return _extension;
    }
    virtual QString description() const;

    virtual QPixmap icon();

    bool isSelected() const;
    void setSelected(bool s);
    QRect itemRect() const;
    void redraw();

    // DON'T USE THOSE OUTSIDE THE VIEWS!!!
    inline const vfile* getVfile() const {
        return _vf;
    }
    inline void setVfile(vfile *vf) {
        _vf = vf;
    }
    inline vfile* getMutableVfile() {
        return _vf;
    }
    inline bool isDummy() const {
        return dummyVfile;
    }
    inline bool isHidden() const {
        return _hidden;
    }

    // used INTERNALLY when calculation of dir size changes the displayed size of the item
    inline void setSize(KIO::filesize_t size) {
        _vf->vfile_setSize(size);
    }

protected:
    vfile* _vf;   // each view item holds a pointer to a corresponding vfile for fast access
    KrInterView * _view; // the parent view this item belongs to
    bool dummyVfile; // used in case our item represents the ".." (updir) item
    const KrViewProperties* _viewProperties;
    bool _hasExtension;
    bool _hidden;
    QString _name;
    QString _extension;
};

#endif
