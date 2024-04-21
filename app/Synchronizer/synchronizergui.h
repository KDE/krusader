/*
    SPDX-FileCopyrightText: 2003 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
#include <QTabWidget>

#include <KComboBox>
#include <utility>

#include "../Filter/filtertabs.h"
#include "../Filter/generalfilter.h"
#include "../GUI/krtreewidget.h"
#include "../GUI/profilemanager.h"
#include "synchronizer.h"

class QSpinBox;

class SynchronizerGUI : public QDialog
{
    Q_OBJECT

public:
    class SyncViewItem : public QTreeWidgetItem
    {
    private:
        SynchronizerFileItem *syncItemRef;
        SyncViewItem *lastItemRef;

    public:
        SyncViewItem(SynchronizerFileItem *item,
                     QColor txt,
                     QColor base,
                     QTreeWidget *parent,
                     QTreeWidgetItem *after,
                     const QString &label1,
                     const QString &label2 = QString(),
                     const QString &label3 = QString(),
                     const QString &label4 = QString(),
                     const QString &label5 = QString(),
                     const QString &label6 = QString(),
                     const QString &label7 = QString(),
                     const QString &label8 = QString())
            : QTreeWidgetItem(parent, after)
            , syncItemRef(item)
            , lastItemRef(nullptr)
        {
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

        SyncViewItem(SynchronizerFileItem *item,
                     QColor txt,
                     QColor base,
                     QTreeWidgetItem *parent,
                     QTreeWidgetItem *after,
                     const QString &label1,
                     const QString &label2 = QString(),
                     const QString &label3 = QString(),
                     const QString &label4 = QString(),
                     const QString &label5 = QString(),
                     const QString &label6 = QString(),
                     const QString &label7 = QString(),
                     const QString &label8 = QString())
            : QTreeWidgetItem(parent, after)
            , syncItemRef(item)
            , lastItemRef(nullptr)
        {
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

        ~SyncViewItem() override
        {
            syncItemRef->setUserData(nullptr);
        }

        inline SynchronizerFileItem *synchronizerItemRef()
        {
            return syncItemRef;
        }
        inline SyncViewItem *lastItem()
        {
            return lastItemRef;
        }
        inline void setLastItem(SyncViewItem *s)
        {
            lastItemRef = s;
        }

        void setColors(const QColor &fore, const QColor &back)
        {
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
    SynchronizerGUI(QWidget *parent, QUrl leftDirectory, QUrl rightDirectory = QUrl(), QStringList selList = QStringList());
    SynchronizerGUI(QWidget *parent, QString profile);
    ~SynchronizerGUI() override;

    inline bool wasSynchronization()
    {
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
    void loadFromProfile(const QString &);
    void saveToProfile(const QString &);

protected slots:
    void reject() override;
    void addFile(SynchronizerFileItem *);
    void markChanged(SynchronizerFileItem *, bool);
    void setScrolling(bool);
    void statusInfo(const QString &);
    void subdirsChecked(bool);
    void setPanelLabels();
    void setCompletion();
    void checkExcludeURLValidity(QString &text, QString &error);
    void connectFilters(const QString &);

private:
    void initGUI(const QString &profile, QUrl leftURL, QUrl rightURL, QStringList selList);

    QString convertTime(time_t time) const;
    void setMarkFlags();
    void disableMarkButtons();
    void enableMarkButtons();
    void copyToClipboard(bool isLeft);

    int convertToSeconds(int time, int unit);
    void convertFromSeconds(int &time, int &unit, int second);

    static QPushButton *createButton(QWidget *parent,
                                     const QString &iconName,
                                     bool checked,
                                     const QKeySequence &shortCut,
                                     const QString &description,
                                     const QString &text = QString(),
                                     bool textAndIcon = false);

protected:
    void keyPressEvent(QKeyEvent *) override;
    bool eventFilter(QObject *, QEvent *) override;

    void executeOperation(SynchronizerFileItem *item, int op);

    ProfileManager *profileManager;
    FilterTabs *filterTabs;
    GeneralFilter *generalFilter;

    QTabWidget *synchronizerTabs;

    KrHistoryComboBox *leftLocation;
    KrHistoryComboBox *rightLocation;
    KrHistoryComboBox *fileFilter;

    KrTreeWidget *syncList;
    Synchronizer synchronizer;

    QCheckBox *cbSubdirs;
    QCheckBox *cbSymlinks;
    QCheckBox *cbByContent;
    QCheckBox *cbIgnoreDate;
    QCheckBox *cbAsymmetric;
    QCheckBox *cbIgnoreCase;

    QPushButton *btnSwapSides;
    QPushButton *btnCompareDirs;
    QPushButton *btnStopComparing;
    QPushButton *btnSynchronize;
    QPushButton *btnFeedToListBox;
    QPushButton *btnScrollResults;

    QPushButton *btnLeftToRight;
    QPushButton *btnEquals;
    QPushButton *btnDifferents;
    QPushButton *btnRightToLeft;
    QPushButton *btnDeletable;
    QPushButton *btnDuplicates;
    QPushButton *btnSingles;

    QLabel *statusLabel;
    QLabel *leftDirLabel;
    QLabel *rightDirLabel;

    QStringList selectedFiles;

    QSpinBox *parallelThreadsSpinBox;
    QSpinBox *equalitySpinBox;
    QComboBox *equalityUnitCombo;
    QSpinBox *timeShiftSpinBox;
    QComboBox *timeShiftUnitCombo;
    QCheckBox *ignoreHiddenFilesCB;

private:
    static QString dirLabel(); // returns translated '<DIR>'

    bool isComparing;
    bool wasClosed;
    bool wasSync;
    bool hasSelectedFiles;
    SyncViewItem *lastItem;

    QColor foreGrounds[TT_MAX];
    QColor backGrounds[TT_MAX];
};

#endif /* __SYNCHRONIZERGUI_H__ */
