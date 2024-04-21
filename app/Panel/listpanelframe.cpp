/*
    SPDX-FileCopyrightText: 2010 Jan Lepper <dehtris@yahoo.de>
    SPDX-FileCopyrightText: 2010-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "listpanelframe.h"

#include "../krglobal.h"
#include "krcolorcache.h"

// QtCore
#include <QUrl>
// QtGui
#include <QDragEnterEvent>
#include <QGuiApplication>

#include <KSharedConfig>
#include <KUrlMimeData>

ListPanelFrame::ListPanelFrame(QWidget *parent, const QString &color)
    : QFrame(parent)
    , color(color)
{
    if (!color.isEmpty()) {
        colorsChanged();
        refreshColors(false);
        setAutoFillBackground(true);
        connect(&KrColorCache::getColorCache(), &KrColorCache::colorsRefreshed, this, &ListPanelFrame::colorsChanged);
    }
}

void ListPanelFrame::dragEnterEvent(QDragEnterEvent *e)
{
    if (acceptDrops()) {
        QList<QUrl> URLs = KUrlMimeData::urlsFromMimeData(e->mimeData());
        e->setAccepted(!URLs.isEmpty());
    } else
        QFrame::dragEnterEvent(e);
}

void ListPanelFrame::colorsChanged()
{
    QPalette p = QGuiApplication::palette();
    const QColor &windowForeground = p.color(QPalette::Active, QPalette::WindowText);
    const QColor &windowBackground = p.color(QPalette::Active, QPalette::Window);

    KConfigGroup gc(krConfig, "Colors");
    QColor fgAct = getColor(gc, color + " Foreground Active", p.color(QPalette::Active, QPalette::HighlightedText), windowForeground);
    QColor bgAct = getColor(gc, color + " Background Active", p.color(QPalette::Active, QPalette::Highlight), windowBackground);
    QColor fgInact = getColor(gc, color + " Foreground Inactive", p.color(QPalette::Active, QPalette::Text), windowForeground);
    QColor bgInact = getColor(gc, color + " Background Inactive", p.color(QPalette::Active, QPalette::Base), windowBackground);

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
    if (color.isEmpty())
        return;
    setPalette(active ? palActive : palInactive);
}

QColor ListPanelFrame::getColor(KConfigGroup &cg, const QString &name, const QColor &def, const QColor &kdedef)
{
    if (cg.readEntry(name, QString()) == "KDE default")
        return kdedef;
    else if (!cg.readEntry(name, QString()).isEmpty())
        return cg.readEntry(name, def);
    return def;
}
