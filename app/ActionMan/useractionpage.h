/*
    SPDX-FileCopyrightText: 2006 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2006 Rafi Yanai <yanai@users.sourceforge.net>
    SPDX-FileCopyrightText: 2006-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef USERACTIONPAGE_H
#define USERACTIONPAGE_H

// QtWidgets
#include <QWidget>

class UserActionListView;
class ActionProperty;
class QToolButton;

class UserActionPage : public QWidget
{
    Q_OBJECT
public:
    explicit UserActionPage(QWidget *parent);
    ~UserActionPage() override;

    /**
     * Be sure to call this function before you delete this page!!
     * @return true if this page can be closed
     */
    bool readyToQuit();

    void applyChanges();

signals:
    void changed(); ///< emitted on changes to an action (used to enable the apply-button)
    void applied(); ///< emitted when changes are applied to an action (used to disable the apply-button)

private:
    /**
     * If there are modifications in the property-widget, the user is asked
     * what to do. Apply, discard or continue editing. In the first case,
     * saving is done in this function.
     * @return true if a new action can be loaded in the property-widget.
     */
    bool continueInSpiteOfChanges();
    /**
     * applies all changes by writing the actionfile and emits "applied"
     */
    void apply();

    // bool _modified; ///< true if the action-tree was changed (= changes were applied to an action)
    UserActionListView *actionTree;
    ActionProperty *actionProperties;
    QToolButton *importButton, *exportButton;
    QToolButton *copyButton, *pasteButton;
    QToolButton *removeButton, *newButton;

private slots:
    void slotChangeCurrent(); // loads a new action into the detail-view
    void slotUpdateAction(); // updates the action to the xml-file
    void slotNewAction();
    void slotRemoveAction();
    void slotImport();
    void slotExport();
    void slotToClip();
    void slotFromClip();
};

#endif
