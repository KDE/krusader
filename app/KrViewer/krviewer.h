/*
    SPDX-FileCopyrightText: 2002 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2002 Rafi Yanai <yanai@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KRVIEWER_H
#define KRVIEWER_H

// QtCore
#include <QEvent>
#include <QList>
#include <QPointer>
#include <QUrl>
// QtGui
#include <QFocusEvent>
#include <QKeyEvent>
// QtWidgets
#include <QAction>
#include <QMenu>
#include <QTabWidget>

#include <KParts/ReadOnlyPart>
#include <KParts/MainWindow>
#include <KParts/PartManager>

#include "../krglobal.h"
#include "viewertabbar.h"

class PanelViewerBase;

class KrViewer : public KParts::MainWindow
{
    Q_OBJECT
public:
    virtual ~KrViewer();

    enum Mode { Generic, Text, Hex, Lister, Default };

    static void view(QUrl url, QWidget *parent = krMainWindow);
    static void view(QUrl url, Mode mode, bool new_window, QWidget *parent = krMainWindow);
    static void edit(QUrl url, QWidget *parent);
    static void edit(const QUrl &url, Mode mode = Text, int new_window = -1, QWidget *parent = krMainWindow);
    static void configureDeps();

    virtual bool eventFilter(QObject *watched, QEvent *e) override;

public slots:
    void createGUI(KParts::Part *);
    void configureShortcuts();

    void viewGeneric();
    void viewText();
    void viewHex();
    void viewLister();
    void editText();

    void print();
    void copy();

    void tabChanged(int index);
    void tabURLChanged(PanelViewerBase *pvb, const QUrl &url);
    void tabCloseRequest(int index, bool force = false);
    void tabCloseRequest();

    void nextTab();
    void prevTab();
    void detachTab();

    void checkModified();

protected:
    virtual bool queryClose() override;
    virtual void changeEvent(QEvent *e) override;
    virtual void resizeEvent(QResizeEvent *e) override;
    virtual void focusInEvent(QFocusEvent *) override
    {
        if (viewers.removeAll(this))
            viewers.prepend(this);
    } // move to first

private slots:
    void openUrlFinished(PanelViewerBase *pvb, bool success);

private:
    explicit KrViewer(QWidget *parent = nullptr);
    void addTab(PanelViewerBase *pvb);
    void updateActions();
    void refreshTab(PanelViewerBase *pvb);
    void viewInternal(QUrl url, Mode mode, QWidget *parent = krMainWindow);
    void editInternal(QUrl url, Mode mode, QWidget *parent = krMainWindow);
    void addPart(KParts::ReadOnlyPart *part);
    void removePart(KParts::ReadOnlyPart *part);
    bool isPartAdded(KParts::Part *part);

    static void activateWindow(QWidget *window);
    static KrViewer *getViewer(bool new_window);
    static QString makeTabText(PanelViewerBase *pvb);
    static QString makeTabToolTip(PanelViewerBase *pvb);
    static QIcon makeTabIcon(PanelViewerBase *pvb);

    KParts::PartManager manager;
    QMenu *viewerMenu;
    ViewerTabWidget tabWidget;
    QPointer<QWidget> returnFocusTo;

    QAction *detachAction;
    QAction *printAction;
    QAction *copyAction;
    QAction *quitAction;

    QAction *configKeysAction;
    QAction *tabCloseAction;
    QAction *tabNextAction;
    QAction *tabPrevAction;

    static QList<KrViewer *> viewers; // the first viewer is the active one
    QList<QKeyCombination> reservedKeys; // the reserved key sequences
    QList<QAction *> reservedKeyActions; // the IDs of the reserved keys

    int sizeX;
    int sizeY;
};

class Invoker : public QObject
{
    Q_OBJECT

public:
    Invoker(QObject *recv, const char *slot)
    {
        connect(this, SIGNAL(invokeSignal()), recv, slot);
    }

    void invoke()
    {
        emit invokeSignal();
    }

signals:
    void invokeSignal();
};

#endif
