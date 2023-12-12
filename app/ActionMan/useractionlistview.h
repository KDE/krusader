/*
    SPDX-FileCopyrightText: 2006 Jonas Bähr <jonas.baehr@web.de>
    SPDX-FileCopyrightText: 2006-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef USERACTIONLISTVIEW_H
#define USERACTIONLISTVIEW_H

#include "../GUI/krtreewidget.h"

class KrAction;
class QString;
class UserActionListViewItem;
class QDomDocument;

class UserActionListView : public KrTreeWidget
{
    Q_OBJECT

public:
    explicit UserActionListView(QWidget *parent = nullptr);
    ~UserActionListView() override;
    QSize sizeHint() const override;

    void update();
    void update(KrAction *action);
    UserActionListViewItem *insertAction(KrAction *action);

    KrAction *currentAction() const;
    void setCurrentAction(const KrAction *);

    QDomDocument dumpSelectedActions(QDomDocument *mergeDoc = nullptr) const;

    void removeSelectedActions();

    /**
     * makes the first action in the list current
     */
    void setFirstActionCurrent();

    /**
     * makes @e item current and ensures its visibility
     */
protected slots:
    void slotCurrentItemChanged(QTreeWidgetItem *);

protected:
    QTreeWidgetItem *findCategoryItem(const QString &category);
    UserActionListViewItem *findActionItem(const KrAction *action);
};

class UserActionListViewItem : public QTreeWidgetItem
{
public:
    UserActionListViewItem(QTreeWidget *view, KrAction *action);
    UserActionListViewItem(QTreeWidgetItem *item, KrAction *action);
    ~UserActionListViewItem() override;

    void setAction(KrAction *action);
    KrAction *action() const;
    void update();

    /**
     * This reimplements qt's compare-function in order to have categories on the top of the list
     */
    bool operator<(const QTreeWidgetItem &other) const override;

private:
    KrAction *_action;
};

#endif
