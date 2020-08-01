/*****************************************************************************
 * Copyright (C) 2000 Shie Erlich <krusader@users.sourceforge.net>           *
 * Copyright (C) 2000 Rafi Yanai <krusader@users.sourceforge.net>            *
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


#ifndef KRSPECIALWIDGETS_H
#define KRSPECIALWIDGETS_H

// QtCore
#include <QEvent>
// QtGui
#include <QPainter>
#include <QKeyEvent>
#include <QPaintEvent>
#include <QColor>
// QtWidgets
#include <QWidget>

#include <KCompletion/KLineEdit>
#include <KIO/Global>
#include <utility>

class KrPieSlice;

class KrPie : public QWidget
{
    Q_OBJECT
public:
    explicit KrPie(KIO::filesize_t _totalSize, QWidget *parent = nullptr);
    void addSlice(KIO::filesize_t size, QString label);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    QList<KrPieSlice> slices;
    KIO::filesize_t totalSize, sizeLeft;
    static QColor colors[ 12 ];
};

class KrFSDisplay : public QWidget
{
    Q_OBJECT
public:
    // this constructor is used for a mounted filesystem
    KrFSDisplay(QWidget *parent, QString _alias, QString _realName,
                KIO::filesize_t _total, KIO::filesize_t _free);
    // this one is for an unmounted/supermount file system
    KrFSDisplay(QWidget *parent, QString _alias, QString _realName, bool sm = false);
    // the last one is used inside MountMan(R), when no filesystem is selected
    explicit KrFSDisplay(QWidget *parent);
    inline void setTotalSpace(KIO::filesize_t t) {
        totalSpace = t;
    }
    inline void setFreeSpace(KIO::filesize_t t) {
        freeSpace = t;
    }
    inline void setAlias(QString a) {
        alias = std::move(a);
    }
    inline void setRealName(QString r) {
        realName = std::move(r);
    }
    inline void setMounted(bool m) {
        mounted = m;
    }
    inline void setEmpty(bool e) {
        empty = e;
    }
    inline void setSupermount(bool s) {
        supermount = s;
    }

protected:
    void paintEvent(QPaintEvent *) override;

private:
    KIO::filesize_t totalSpace, freeSpace;
    QString alias, realName;
    bool mounted, empty, supermount;
};

class KrPieSlice
{
public:
    KrPieSlice(long double _perct, QColor _color, QString _label) :
            perct(_perct), color(std::move(_color)), label(std::move(_label)) {}
    inline QColor getColor() {
        return color;
    }
    inline long double getPerct() {
        return perct;
    }
    inline QString getLabel() {
        return label;
    }
    inline void setPerct(float _perct) {
        perct = _perct;
    }
    inline void setLabel(QString _label) {
        label = std::move(_label);
    }

private:
    long double perct;
    QColor color;
    QString label;
};

#endif
