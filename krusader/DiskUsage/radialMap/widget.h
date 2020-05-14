/*****************************************************************************
 * Copyright (C) 2004 Max Howell <max.howell@methylblue.com>                 *
 * Copyright (C) 2004-2020 Krusader Krew [https://krusader.org]              *
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

#ifndef WIDGET_H
#define WIDGET_H

// QtCore
#include <QTimer>
#include <QUrl>
// QtGui
#include <QMouseEvent>
#include <QResizeEvent>
#include <QPaintEvent>

#include "segmentTip.h"

template <class T> class Chain;
class Directory;
class File;
class KJob;

namespace RadialMap
{
class Segment;

class Map : public QPixmap
{
public:
    Map();
    ~Map() override;

    void make(const Directory *, bool = false);
    bool resize(const QRect&);

    bool isNull() const {
        return (m_signature == nullptr);
    }
    void invalidate(const bool);

    friend class Builder;
    friend class Widget;

private:
    void paint(uint = 1);
    void aaPaint();
    void colorise();
    void setRingBreadth();

    Chain<Segment> *m_signature;

    QRect   m_rect;
    uint    m_ringBreadth;  ///ring breadth
    uint    m_innerRadius;  ///radius of inner circle
    uint    m_visibleDepth; ///visible level depth of system
    QString m_centerText;

    uint MAP_2MARGIN;
};

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget* = nullptr);

    QString path() const;
    QUrl url(File const * const = nullptr) const;

    bool isValid() const {
        return m_tree != nullptr;
    }

    friend struct Label; //FIXME badness

public slots:
    void zoomIn();
    void zoomOut();
    void create(const Directory*);
    void invalidate(const bool = true);
    void refresh(int);

private slots:
    void resizeTimeout();
    void sendFakeMouseEvent();
    void deleteJobFinished(KJob*);
    void createFromCache(const Directory*);

signals:
    void activated(const QUrl&);
    void invalidated(const QUrl&);
    void created(const Directory*);
    void mouseHover(const QString&);

protected:
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mousePressEvent(QMouseEvent*) override;

protected:
    const Segment *segmentAt(QPoint&) const;   //FIXME const reference for a library others can use
    const Segment *rootSegment() const {
        return m_rootSegment;
    } /// never == 0
    const Segment *focusSegment() const {
        return m_focus;
    } /// 0 == nothing in focus

private:
    void paintExplodedLabels(QPainter&) const;

    const Directory *m_tree;
    const Segment   *m_focus;
    QPoint           m_offset;
    QTimer           m_timer;
    Map              m_map;
    SegmentTip       m_tip;
    Segment         *m_rootSegment;
};
}

#endif
