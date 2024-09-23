/*
    SPDX-FileCopyrightText: 2003 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2003 Rafi Yanai <yanai@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PANELCONTEXTMENU_H
#define PANELCONTEXTMENU_H

// QtWidgets
#include <QMenu>

#include <KFileItemActions>
#include <KService>

class KrPanel;

/**
 * The right-click context menu for files and folders in the panel view.
 *
 * An instance is created each time a menu is shown.
 */
class PanelContextMenu : public QMenu
{
    Q_OBJECT
public:
    static PanelContextMenu *run(const QPoint &pos, KrPanel *panel);

private:
    explicit PanelContextMenu(KrPanel *thePanel, QWidget *parent = nullptr);

    void performAction(int id);
    void addEmptyMenuEntries(); // adds the choices for a menu without selected items
    void addCreateNewMenu(); // adds a "create new" submenu

    enum ID {
        OPEN_ID,
        BROWSE_ID,
        OPEN_TERM_ID,
        OPEN_TAB_ID,
        PREVIEW_ID,
        CHOOSE_ID,
        DELETE_ID,
        MOUNT_ID,
        UNMOUNT_ID,
        TRASH_ID,
        NEW_LINK_ID,
        NEW_SYMLINK_ID,
        REDIRECT_LINK_ID,
        EMPTY_TRASH_ID,
        RESTORE_TRASHED_FILE_ID,
        SYNC_SELECTED_ID,
        SEND_BY_EMAIL_ID,
        EJECT_ID,
        SERVICE_LIST_ID // ALWAYS KEEP THIS ONE LAST!!!
    };

private:
    KrPanel *const panel;
    KFileItemList _items;
    KService::List offers;
};

#endif
