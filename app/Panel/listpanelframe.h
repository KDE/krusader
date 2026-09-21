/*
    SPDX-FileCopyrightText: Krusader Krew <https://krusader.org>
    SPDX-FileCopyrightText: Jan Lepper <dehtris@yahoo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LISTPANELFRAME_H
#define LISTPANELFRAME_H

// QtGui
#include <QDropEvent>
// QtWidgets
#include <QFrame>

#include <KConfigGroup>

class QDragEnterEvent;

class ListPanelFrame : public QFrame
{
    Q_OBJECT
public:
    ListPanelFrame(QWidget *parent, const QString &color);

signals:
    void dropped(QDropEvent *); /**< emitted when someone drops URL onto the frame */

protected slots:
    void colorsChanged();

public slots:
    void refreshColors(bool active);

protected:
    QColor getColor(KConfigGroup &cg, const QString &name, const QColor &def, const QColor &kdedef);

    void dropEvent(QDropEvent *event) override
    {
        emit dropped(event);
    }
    void dragEnterEvent(QDragEnterEvent *) override;

    QString color;
    QPalette palActive, palInactive;
};

#endif // LISTPANELFRAME_H
