/*****************************************************************************
 * Copyright (C) 2002 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2002 Rafi Yanai <yanai@users.sourceforge.net>               *
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

#ifndef KRVIEWER_H
#define KRVIEWER_H

#include <QWidget>
#include <QEvent>
#include <QList>
#include <QKeyEvent>
#include <QFocusEvent>
#include <kparts/mainwindow.h>
#include <kparts/partmanager.h>
#include <kparts/browserextension.h>
#include <QtCore/QPointer>
#include <ktabwidget.h>
#include <ktemporaryfile.h>

#include "../krglobal.h"

class PanelViewerBase;

class KrViewer : public KParts::MainWindow
{
    Q_OBJECT
public:
    virtual ~KrViewer();

    enum Mode {Generic, Text, Hex, Lister};

    static void view(KUrl url, QWidget * parent = krMainWindow);
    static void view(KUrl url, Mode mode, bool new_window, QWidget * parent = krMainWindow);
    static void edit(KUrl url, QWidget * parent);
    static void edit(KUrl url, Mode mode = Text, int new_window = -1, QWidget * parent = krMainWindow);

    virtual bool eventFilter(QObject * watched, QEvent * e);

public slots:
    void keyPressEvent(QKeyEvent *e);
    void createGUI(KParts::Part*);

    void viewGeneric();
    void viewText();
    void viewHex();
    void viewLister();
    void editText();

    void print();
    void copy();

    void tabChanged(QWidget* w);
    void tabURLChanged(PanelViewerBase * pvb, const KUrl &url);
    void tabCloseRequest(QWidget *w);
    void tabCloseRequest();
    void partDestroyed(PanelViewerBase *);

    void nextTab();
    void prevTab();
    void detachTab();

    void checkModified();

protected:
    virtual bool queryClose();
    virtual bool queryExit();
    virtual void windowActivationChange(bool oldActive);
    virtual void resizeEvent(QResizeEvent *e);

    virtual void focusInEvent(QFocusEvent *) {
        if (viewers.removeAll(this)) viewers.prepend(this);
    } // move to first

private:
    KrViewer(QWidget *parent = 0);
    void addTab(PanelViewerBase* pvb, QString msg, QString iconName, KParts::Part* part);
    PanelViewerBase * getPanelViewerBase(KParts::Part* part);
    void updateActions(PanelViewerBase * base);
    bool isValidPart(KParts::Part* part);

    static KrViewer* getViewer(bool new_window);

    KParts::PartManager manager;
    QMenu* viewerMenu;
    KTemporaryFile tmpFile;
    KTabWidget tabBar;
    QPointer<QWidget> returnFocusTo;
    PanelViewerBase * returnFocusTab;

    QAction *detachAction;

    KAction *printAction;
    KAction *copyAction;

    QAction *tabClose;
    QAction *closeAct;

    static QList<KrViewer *> viewers; // the first viewer is the active one
    QList<int>    reservedKeys;   // the reserved key sequences
    QList<QAction *> reservedKeyActions; // the IDs of the reserved keys

    int sizeX;
    int sizeY;
};

class Invoker : public QObject
{
    Q_OBJECT

public:
    Invoker(QObject *recv, const char * slot) {
        connect(this, SIGNAL(invokeSignal()), recv, slot);
    }

    void invoke() {
        emit invokeSignal();
    }

signals:
    void invokeSignal();
};

#endif
