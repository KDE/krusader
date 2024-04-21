/*
    SPDX-FileCopyrightText: 2004 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LOCATE_H
#define LOCATE_H

// QtGui
#include <QKeyEvent>
// QtWidgets
#include <QCheckBox>
#include <QDialog>

#include <KComboBox>

#include "../GUI/krhistorycombobox.h"

class KProcess;
class KrTreeWidget;
class QTreeWidgetItem;

class LocateDlg : public QDialog
{
    Q_OBJECT

public:
    explicit LocateDlg(QWidget *parent);
    ~LocateDlg() override;

    static LocateDlg *LocateDialog;

    virtual void feedToListBox();

    void reset();

public slots:
    void slotFeedStop();
    void slotUpdateDb();
    void slotLocate();

    void processStdout();
    void processStderr();
    void locateFinished();
    void locateError();
    void slotRightClick(QTreeWidgetItem *, const QPoint &);
    void slotDoubleClick(QTreeWidgetItem *);
    void updateFinished();

protected:
    void keyPressEvent(QKeyEvent *) override;

private:
    void operate(QTreeWidgetItem *item, int task);

    bool find();
    void nextLine();

    void updateButtons(bool locateIsRunning);

    bool dontSearchPath;
    bool onlyExist;
    bool isCs;

    bool isFeedToListBox;

    QString pattern;

    KrHistoryComboBox *locateSearchFor;
    KrTreeWidget *resultList;
    QString remaining;
    QTreeWidgetItem *lastItem;

    QString collectedErr;

    long findOptions;
    QString findPattern;
    QTreeWidgetItem *findCurrentItem;

    QCheckBox *dontSearchInPath;
    QCheckBox *existingFiles;
    QCheckBox *caseSensitive;

    QPushButton *feedStopButton;
    QPushButton *updateDbButton;
    QPushButton *locateButton;

    KProcess *locateProc;
    static KProcess *updateProcess;
};

#endif /* __LOCATE_H__ */
