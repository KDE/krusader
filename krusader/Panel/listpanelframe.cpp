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

#include "listpanelframe.h"

#include "krcolorcache.h"
#include "../krglobal.h"

#include <kcolorscheme.h>
#include <kurl.h>
#include <QDragEnterEvent>

ListPanelFrame::ListPanelFrame(QWidget *parent, QString color) : QFrame(parent), color(color)
{
    if(!color.isEmpty()) {
        colorsChanged();
        refreshColors(false);
        setAutoFillBackground(true);
        connect(&KrColorCache::getColorCache(), SIGNAL(colorsRefreshed()), SLOT(colorsChanged()));
    }
}

void ListPanelFrame::dragEnterEvent(QDragEnterEvent *e)
{
    if (acceptDrops()) {
        KUrl::List URLs = KUrl::List::fromMimeData(e->mimeData());
        e->setAccepted(!URLs.isEmpty());
    } else
        QFrame::dragEnterEvent(e);
}

void ListPanelFrame::colorsChanged()
{
    QColor windowForeground = KColorScheme(QPalette::Active, KColorScheme::Window).foreground().color();
    QColor windowBackground = KColorScheme(QPalette::Active, KColorScheme::Window).background().color();

    KConfigGroup gc(krConfig, "Colors");
    QColor fgAct = getColor(gc, color + " Foreground Active",
        KColorScheme(QPalette::Active, KColorScheme::Selection).foreground().color(), windowForeground);
    QColor bgAct = getColor(gc, color + " Background Active",
        KColorScheme(QPalette::Active, KColorScheme::Selection).background().color(), windowBackground);
    QColor fgInact = getColor(gc, color + " Foreground Inactive",
        KColorScheme(QPalette::Active, KColorScheme::View).foreground().color(), windowForeground);
    QColor bgInact = getColor(gc, color + " Background Inactive",
        KColorScheme(QPalette::Active, KColorScheme::View).background().color(), windowBackground);

    palActive = palInactive = palette();

    // --- active ---
    // foreground
    palActive.setColor(QPalette::WindowText, fgAct);
    palActive.setColor(QPalette::ButtonText, fgAct);
    // background
    palActive.setColor(QPalette::Window, bgAct);
    palActive.setColor(QPalette::Button, bgAct);

    // --- inactive ---
    // foreground
    palInactive.setColor(QPalette::WindowText, fgInact);
    palInactive.setColor(QPalette::ButtonText, fgInact);
    // background
    palInactive.setColor(QPalette::Window, bgInact);
    palInactive.setColor(QPalette::Button, bgInact);
}

void ListPanelFrame::refreshColors(bool active)
{
    if(color.isEmpty())
        return;
    setPalette(active ? palActive : palInactive);
}

QColor ListPanelFrame::getColor(KConfigGroup &cg, QString name, const QColor &def, const QColor &kdedef)
{
    if (cg.readEntry(name, QString()) == "KDE default")
        return kdedef;
    else if(!cg.readEntry(name, QString()).isEmpty())
        return cg.readEntry(name, def);
    return def;
}
