/*****************************************************************************
 * Copyright (C) 2004 Csaba Karai <krusader@users.sourceforge.net>           *
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

#ifndef LOCATE_H
#define LOCATE_H

// QtGui
#include <QKeyEvent>
// QtWidgets
#include <QCheckBox>
#include <QDialog>

#include <KCompletion/KComboBox>
#include <KCompletion/KHistoryComboBox>

class KProcess;
class KrTreeWidget;
class QTreeWidgetItem;

class LocateDlg : public QDialog
{
    Q_OBJECT

public:
    explicit LocateDlg(QWidget *parent);

    static LocateDlg *LocateDialog;

    virtual void      feedToListBox();

    void              reset();

public slots:
    void              slotFeedStop();
    void              slotUpdateDb();
    void              slotLocate();

    void              processStdout();
    void              processStderr();
    void              locateFinished();
    void              locateError();
    void              slotRightClick(QTreeWidgetItem *, const QPoint &);
    void              slotDoubleClick(QTreeWidgetItem *);
    void              updateFinished();

protected:
    void              keyPressEvent(QKeyEvent *) Q_DECL_OVERRIDE;

private:
    void              operate(QTreeWidgetItem *item, int task);

    bool              find();
    void              nextLine();

    void              updateButtons(bool locateIsRunning);


    bool              dontSearchPath;
    bool              onlyExist;
    bool              isCs;

    bool              isFeedToListBox;

    QString           pattern;

    KHistoryComboBox *locateSearchFor;
    KrTreeWidget     *resultList;
    QString           remaining;
    QTreeWidgetItem  *lastItem;

    QString           collectedErr;

    long              findOptions;
    QString           findPattern;
    QTreeWidgetItem  *findCurrentItem;

    QCheckBox        *dontSearchInPath;
    QCheckBox        *existingFiles;
    QCheckBox        *caseSensitive;

    QPushButton      *feedStopButton;
    QPushButton      *updateDbButton;
    QPushButton      *locateButton;

    KProcess         *locateProc;
    static KProcess *updateProcess;
};

#endif /* __LOCATE_H__ */
