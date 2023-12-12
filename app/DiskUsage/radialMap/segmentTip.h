/*
    SPDX-FileCopyrightText: 2003-2004 Max Howell <max.howell@methylblue.com>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SEGMENTTIP_H
#define SEGMENTTIP_H

// QtCore
#include <QEvent>
// QtGui
#include <QPixmap>
// QtWidgets
#include <QWidget>

class File;
class Directory;

namespace RadialMap
{
class SegmentTip : public QWidget
{
public:
    explicit SegmentTip(uint);

    void updateTip(const File *, const Directory *);
    void moveto(QPoint, QWidget &, bool);

private:
    virtual bool eventFilter(QObject *, QEvent *) override;
    virtual bool event(QEvent *) override;

    uint m_cursorHeight;
    QPixmap m_pixmap;
    QString m_text;
};
}

#endif
