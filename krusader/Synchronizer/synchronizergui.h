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
#include <QResizeEvent>
#include <QKeyEvent>
#include <QPixmap>
// QtWidgets
#include <QCheckBox>
#include <QDialog>
#include <QLabel>
#include <QTabWidget>

#include <KCompletion/KComboBox>
#include <utility>

#include "synchronizer.h"
#include "../GUI/profilemanager.h"
#include "../GUI/krtreewidget.h"
#include "../Filter/filtertabs.h"
#include "../Filter/generalfilter.h"

class QSpinBox;

class SynchronizerGUI : public QDialog
{
    Q_OBJECT

public:
    class SyncViewItem : public QTreeWidgetItem
    {
    private:
        SynchronizerFileItem *syncItemRef;
        SyncViewItem         *lastItemRef;

    public:
        SyncViewItem(SynchronizerFileItem *item, QColor txt, QColor base, QTreeWidget * parent, QTreeWidgetItem *after, const QString& label1,
                     const QString& label2 = QString(), const QString& label3 = QString(), const QString& label4 = QString(),
                     const QString& label5 = QString(), const QString& label6 = QString(),
                     const QString& label7 = QString(), const QString& label8 = QString()) :
                QTreeWidgetItem(parent, after), syncItemRef(item), lastItemRef(nullptr) {
            setText(0, label1);
            setText(1, label2);
            setText(2, label3);
            setText(3, label4);
            setText(4, label5);
            setText(5, label6);
            setText(6, label7);
            setText(7, label8);

            setTextAlignment(1, Qt::AlignRight);
            setTextAlignment(3, Qt::AlignHCenter);
            setTextAlignment(5, Qt::AlignRight);
            item->setUserData((void *)this);

            setColors(std::move(txt), std::move(base));
        }

        SyncViewItem(SynchronizerFileItem *item, QColor txt, QColor base, QTreeWidgetItem * parent, QTreeWidgetItem *after, const QString& label1,
                     const QString& label2 = QString(), const QString& label3 = QString(), const QString& label4 = QString(),
                     const QString& label5 = QString(), const QString& label6 = QString(),
                     const QString& label7 = QString(), const QString& label8 = QString()) :
                QTreeWidgetItem(parent, after), syncItemRef(item), lastItemRef(nullptr) {
            setText(0, label1);
            setText(1, label2);
            setText(2, label3);
            setText(3, label4);
            setText(4, label5);
            setText(5, label6);
            setText(6, label7);
            setText(7, label8);

            setTextAlignment(1, Qt::AlignRight);
            setTextAlignment(3, Qt::AlignHCenter);
            setTextAlignment(5, Qt::AlignRight);
            item->setUserData((void *)this);

            setColors(std::move(txt), std::move(base));
        }

        ~SyncViewItem() override {
            syncItemRef->setUserData(nullptr);
        }

        inline SynchronizerFileItem * synchronizerItemRef()       {
            return syncItemRef;
        }
        inline SyncViewItem         * lastItem()                  {
            return lastItemRef;
        }
        inline void                   setLastItem(SyncViewItem*s) {
            lastItemRef = s;
        }

        void setColors(const QColor& fore, const QColor& back) {
            QBrush textColor(fore);
            QBrush baseColor(back);

            for (int i = 0; i != columnCount(); i++) {
                if (back.isValid())
                    setBackground(i, baseColor);
                if (fore.isValid())
                    setForeground(i, textColor);
            }
        }
    };

public:
    // if rightDirectory is null, leftDirectory is actually the profile name to load
    SynchronizerGUI(QWidget* parent,  QUrl leftDirectory, QUrl rightDirectory = QUrl(), QStringList selList = QStringList());
    SynchronizerGUI(QWidget* parent,  QString profile);
    ~SynchronizerGUI() override;

    inline bool wasSynchronization()    {
        return wasSync;
    }

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
    void loadFromProfile(const QString&);
    void saveToProfile(const QString&);

protected slots:
    void reject() Q_DECL_OVERRIDE;
    void addFile(SynchronizerFileItem *);
    void markChanged(SynchronizerFileItem *, bool);
    void setScrolling(bool);
    void statusInfo(const QString&);
    void subdirsChecked(bool);
    void setPanelLabels();
    void setCompletion();
    void checkExcludeURLValidity(QString &text, QString &error);
    void connectFilters(const QString &);

private:
    void initGUI(const QString& profile, QUrl leftURL, QUrl rightURL, QStringList selList);

    QString convertTime(time_t time) const;
    void    setMarkFlags();
    void    disableMarkButtons();
    void    enableMarkButtons();
    void    copyToClipboard(bool isLeft);

    int     convertToSeconds(int time, int unit);
    void    convertFromSeconds(int &time, int &unit, int second);

    static QPushButton *createButton(QWidget *parent, const QString &iconName, bool checked,
                                     const QKeySequence &shortCut, const QString &description,
                                     const QString &text = QString(), bool textAndIcon = false);

protected:
    void keyPressEvent(QKeyEvent *) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *, QEvent *) Q_DECL_OVERRIDE;

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
