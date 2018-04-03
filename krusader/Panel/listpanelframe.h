/*****************************************************************************
 * Copyright (C) 2010 Jan Lepper <dehtris@yahoo.de>                          *
 * Copyright (C) 2010-2018 Krusader Krew [https://krusader.org]              *
 *                                                                           *
 * This file is part of Krusader [https://krusader.org].                     *
 *                                                                           *
 * Krusader is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * Krusader is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with Krusader.  If not, see [http://www.gnu.org/licenses/].         *
 *****************************************************************************/

#ifndef LISTPANELFRAME_H
#define LISTPANELFRAME_H

// QtGui
#include <QDropEvent>
// QtWidgets
#include <QFrame>

# include <KConfigCore/KConfigGroup>

class QDragEnterEvent;

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

    virtual void dropEvent(QDropEvent *e) Q_DECL_OVERRIDE {
        emit dropped(e, this);
    }
    virtual void dragEnterEvent(QDragEnterEvent*) Q_DECL_OVERRIDE;

    QString color;
    QPalette palActive, palInactive;
};

#endif // LISTPANELFRAME_H
