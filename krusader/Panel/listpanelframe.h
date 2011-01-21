/*****************************************************************************
 * Copyright (C) 2010 Jan Lepper <dehtris@yahoo.de>                          *
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

#ifndef __LISTPANELFRAME_H__
#define __LISTPANELFRAME_H__

#include <QFrame>
#include <kconfiggroup.h>

class QDragEnterEvent;
class QDropEvent;

class ListPanelFrame : public QFrame
{
    Q_OBJECT
public:
    ListPanelFrame(QWidget *parent, QString color);

signals:
    void dropped(QDropEvent*, QWidget*); /**< emitted when someone drops URL onto the frame */

protected slots:
    void colorsChanged();
    void refreshColors(bool active);

protected:
    QColor getColor(KConfigGroup &cg, QString name, const QColor &def, const QColor &kdedef);

    virtual void dropEvent(QDropEvent *e) {
        emit dropped(e, this);
    }
    virtual void dragEnterEvent(QDragEnterEvent*);

    QString color;
    QPalette palActive, palInactive;
};

#endif //__LISTPANELFRAME_H__
