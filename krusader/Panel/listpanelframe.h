/*
    SPDX-FileCopyrightText: 2010 Jan Lepper <dehtris@yahoo.de>
    SPDX-FileCopyrightText: 2010-2020 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
    ListPanelFrame(QWidget *parent, const QString& color);

signals:
    void dropped(QDropEvent*, QWidget*); /**< emitted when someone drops URL onto the frame */

protected slots:
    void colorsChanged();

public slots:
    void refreshColors(bool active);

protected:
    QColor getColor(KConfigGroup &cg, const QString& name, const QColor &def, const QColor &kdedef);

    void dropEvent(QDropEvent *e) override {
        emit dropped(e, this);
    }
    void dragEnterEvent(QDragEnterEvent*) override;

    QString color;
    QPalette palActive, palInactive;
};

#endif // LISTPANELFRAME_H
