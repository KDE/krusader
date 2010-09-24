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

#ifndef KRINTERVIEWITEM_H
#define KRINTERVIEWITEM_H

#include <QAbstractItemView>

#include "krviewitem.h"
#include "krvfsmodel.h"


// dummy. remove this class when no longer needed
class KrInterViewItem: public KrViewItem
{
public:
    KrInterViewItem(KrInterView *parent, vfile *vf): KrViewItem(vf, parent->properties()) {
        _view = parent;
        if (parent->_model->dummyVfile() == vf)
            dummyVfile = true;
    }

    bool isSelected() const {
        return _view->isSelected(_vf);
    }
    void setSelected(bool s) {
        _view->setSelected(_vf, s);
        if(!_view->op()->isMassSelectionUpdate()) {
            redraw();
            _view->op()->emitSelectionChanged();
        }
    }
    QRect itemRect() const {
        return _view->itemRect(_vf);
    }
    void redraw() {
        _view->_itemView->viewport()->update(itemRect());
    }

private:
    KrInterView * _view;
};


#endif
