/***************************************************************************
                              krspecialwidgets.h
                           -------------------
  copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
  e-mail               : krusader@users.sourceforge.net
  web site             : http://krusader.sourceforge.net
---------------------------------------------------------------------------
Description
***************************************************************************

A

   db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
   88 ,8P' 88  `8D 88    88 88'  YP d8' `8b 88  `8D 88'     88  `8D
   88,8P   88oobY' 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY'
   88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
   88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
   YP   YD 88   YD ~Y8888P' `8888Y' YP   YP Y8888D' Y88888P 88   YD

                                                   H e a d e r    F i l e

***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/


#ifndef KRSPECIALWIDGETS_H
#define KRSPECIALWIDGETS_H

#include <qwidget.h>
#include <QtGui/QPainter>
#include <qcolor.h>
#include <QKeyEvent>
#include <QPaintEvent>
#include <klineedit.h>
#include <QtCore/QEvent>
#include <kio/global.h>

class KRPieSlice;

class KRPie : public QWidget
{
    Q_OBJECT
public:
    KRPie(KIO::filesize_t _totalSize, QWidget *parent = 0);
    void addSlice(KIO::filesize_t size, QString label);

protected:
    void paintEvent(QPaintEvent *);

private:
    QList<KRPieSlice> slices;
    KIO::filesize_t totalSize, sizeLeft;
    static QColor colors[ 12 ];
};

class KRFSDisplay : public QWidget
{
    Q_OBJECT
public:
    // this constructor is used for a mounted filesystem
    KRFSDisplay(QWidget *parent, QString _alias, QString _realName,
                KIO::filesize_t _total, KIO::filesize_t _free);
    // this one is for an unmounted/supermount file system
    KRFSDisplay(QWidget *parent, QString _alias, QString _realName, bool sm = false);
    // the last one is used inside MountMan(R), when no filesystem is selected
    KRFSDisplay(QWidget *parent);
    inline void setTotalSpace(KIO::filesize_t t) {
        totalSpace = t;
    }
    inline void setFreeSpace(KIO::filesize_t t) {
        freeSpace = t;
    }
    inline void setAlias(QString a) {
        alias = a;
    }
    inline void setRealName(QString r) {
        realName = r;
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
    void paintEvent(QPaintEvent *);

private:
    KIO::filesize_t totalSpace, freeSpace;
    QString alias, realName;
    bool mounted, empty, supermount;
};

class KRPieSlice
{
public:
    KRPieSlice(float _perct, QColor _color, QString _label) :
            perct(_perct), color(_color), label(_label) {}
    inline QColor getColor() {
        return color;
    }
    inline float getPerct() {
        return perct;
    }
    inline QString getLabel() {
        return label;
    }
    inline void setPerct(float _perct) {
        perct = _perct;
    }
    inline void setLabel(QString _label) {
        label = _label;
    }

private:
    float perct;
    QColor color;
    QString label;
};

class KrQuickSearch: public KLineEdit
{
    Q_OBJECT
public:
    KrQuickSearch(QWidget *parent);
    void addText(const QString &str) {
        setText(text() + str);
    }
    bool shortcutOverride(QKeyEvent *e);
    void myKeyPressEvent(QKeyEvent *e);
    void setMatch(bool match);
    void myInputMethodEvent(QInputMethodEvent* e) {
        inputMethodEvent(e);
    }

signals:
    void stop(QKeyEvent *e);
    void process(QKeyEvent *e);
    void otherMatching(const QString &, int);
};

#endif
