/*****************************************************************************
 * Copyright (C) 2003 Csaba Karai <krusader@users.sourceforge.net>           *
 * Copyright (C) 2004-2018 Krusader Krew [https://krusader.org]              *
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

#ifndef SYNCHRONIZERGUI_H
#define SYNCHRONIZERGUI_H

// QtCore
#include <QMap>
// QtGui
#include <QKeyEvent>
#include <QPixmap>
#include <QResizeEvent>
// QtWidgets
#include <QCheckBox>
#include <QDialog>
#include <QLabel>
#include <QSpinBox>
#include <QTabWidget>
#include <QTreeWidget>

#include <KCompletion/KComboBox>
#include <KCompletion/KHistoryComboBox>

#include "synchronizer.h"

class FilterTabs;
class GeneralFilter;
class KrTreeWidget;
class ProfileManager;

class SyncViewItem : public QTreeWidgetItem
{
private:
    SynchronizerFileItem *syncItemRef;
    SyncViewItem *lastItemRef;

public:
    SyncViewItem(SynchronizerFileItem *item, const QColor &txt, const QColor &base,
                 QTreeWidget *parent, QTreeWidgetItem *after, const QString &label1,
                 const QString &label2, const QString &label3, const QString &label4,
                 const QString &label5, const QString &label6, const QString &label7);

    SyncViewItem(SynchronizerFileItem *item, const QColor &txt, const QColor &base,
                 QTreeWidgetItem *parent, QTreeWidgetItem *after, const QString &label1,
                 const QString &label2, const QString &label3, const QString &label4,
                 const QString &label5, const QString &label6, const QString &label7);

    ~SyncViewItem() { syncItemRef->setViewItem(nullptr); }

    inline SynchronizerFileItem *synchronizerItemRef() { return syncItemRef; }
    inline SyncViewItem *lastItem() { return lastItemRef; }
    inline void setLastItem(SyncViewItem *s) { lastItemRef = s; }

    void setColors(const QColor &fore, const QColor &back);

private:
    void setColumns(const QString &label1, const QString &label2, const QString &label3,
                    const QString &label4, const QString &label5, const QString &label6,
                    const QString &label7, SynchronizerFileItem *item, const QColor &txt,
                    const QColor &base);
};

class SynchronizerGUI : public QDialog
{
    Q_OBJECT

public:
    // if rightDirectory is null, leftDirectory is actually the profile name to load
    SynchronizerGUI(QWidget *parent, QUrl leftDirectory, QUrl rightDirectory = QUrl(),
                    QStringList selList = QStringList());
    SynchronizerGUI(QWidget *parent, QString profile);
    ~SynchronizerGUI();

    inline bool wasSynchronization() { return wasSync; }

public slots:
    void rightMouseClicked(QTreeWidgetItem *, const QPoint &);
    void doubleClicked(QTreeWidgetItem *);
    void compare();
    void synchronize();
    void stop();
    void feedToListBox();
    void closeDialog();
    void refresh();
    void swapSides();
    void loadFromProfile(QString);
    void saveToProfile(QString);

protected slots:
    void reject();
    void addFile(SynchronizerFileItem *);
    void markChanged(SynchronizerFileItem *, bool);
    void setScrolling(bool);
    void statusInfo(QString);
    void subdirsChecked(bool);
    void setPanelLabels();
    void setCompletion();
    void checkExcludeURLValidity(QString &text, QString &error);
    void connectFilters(const QString &);

private:
    void initGUI(QString profile, QUrl leftURL, QUrl rightURL, QStringList selList);

    QString convertTime(time_t time) const;
    void setMarkFlags();
    void disableMarkButtons();
    void enableMarkButtons();
    void copyToClipboard(bool isLeft);

    int convertToSeconds(int time, int unit);
    void convertFromSeconds(int &time, int &unit, int second);

    static QPushButton *createButton(QWidget *parent, const QString &iconName, bool checked,
                                     const QKeySequence &shortCut, const QString &description,
                                     const QString &text = QString(), bool textAndIcon = false);

protected:
    virtual void keyPressEvent(QKeyEvent *) Q_DECL_OVERRIDE;
    virtual void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;
    virtual bool eventFilter(QObject *, QEvent *) Q_DECL_OVERRIDE;

    void executeOperation(SynchronizerFileItem *item, int op);

    ProfileManager *profileManager;
    FilterTabs     *filterTabs;
    GeneralFilter  *generalFilter;

    QTabWidget    *synchronizerTabs;

    KHistoryComboBox *leftLocation;
    KHistoryComboBox *rightLocation;
    KHistoryComboBox *fileFilter;

    KrTreeWidget  *syncList;
    Synchronizer   synchronizer;

    QCheckBox     *cbSubdirs;
    QCheckBox     *cbSymlinks;
    QCheckBox     *cbByContent;
    QCheckBox     *cbIgnoreDate;
    QCheckBox     *cbAsymmetric;
    QCheckBox     *cbIgnoreCase;

    QPushButton   *btnSwapSides;
    QPushButton   *btnCompareDirs;
    QPushButton   *btnStopComparing;
    QPushButton   *btnSynchronize;
    QPushButton   *btnFeedToListBox;
    QPushButton   *btnScrollResults;

    QPushButton   *btnLeftToRight;
    QPushButton   *btnEquals;
    QPushButton   *btnDifferents;
    QPushButton   *btnRightToLeft;
    QPushButton   *btnDeletable;
    QPushButton   *btnDuplicates;
    QPushButton   *btnSingles;

    QLabel        *statusLabel;
    QLabel        *leftDirLabel;
    QLabel        *rightDirLabel;

    QStringList    selectedFiles;

    QSpinBox      *parallelThreadsSpinBox;
    QSpinBox      *equalitySpinBox;
    QComboBox     *equalityUnitCombo;
    QSpinBox      *timeShiftSpinBox;
    QComboBox     *timeShiftUnitCombo;
    QCheckBox     *ignoreHiddenFilesCB;

private:
    static QString dirLabel(); // returns translated '<DIR>'
    static KHistoryComboBox *createHistoryComboBox(QWidget *parent, bool enabled);
    static QUrl getUrl(KHistoryComboBox *location);

    bool           isComparing;
    bool           wasClosed;
    bool           wasSync;
    bool           firstResize;
    bool           hasSelectedFiles;
    SyncViewItem  *lastItem;

    int            sizeX;
    int            sizeY;

    QColor         foreGrounds[ TT_MAX ];
    QColor         backGrounds[ TT_MAX ];
};

#endif /* __SYNCHRONIZERGUI_H__ */
