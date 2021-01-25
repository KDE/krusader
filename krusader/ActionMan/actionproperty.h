/*****************************************************************************
 * Copyright (C) 2004-2007 Jonas Bähr <jonas.baehr@web.de>                   *
 * Copyright (C) 2004-2020 Krusader Krew [https://krusader.org]              *
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

#ifndef ACTIONPROPERTY_H
#define ACTIONPROPERTY_H

#include "ui_actionproperty.h"

class KrAction;

/**
 * Use this widget where ever you need to manipulate a UserAction
 */
class ActionProperty : public QWidget, public Ui::ActionProperty
{
    Q_OBJECT
public:
    explicit ActionProperty(QWidget *parent = nullptr, KrAction *action = nullptr);
    ~ActionProperty() override;

    /**
     * @return the currently displayed action
     */
    KrAction* action() {
        return _action;
    };

    /**
     * This inits the widget with the actions properties.
     * If no action is provided, the last used will be taken!
     * It also resets the changed() state.
     * @param action the action which should be displayed
     */
    void updateGUI(KrAction *action = nullptr);

    /**
     * This writes the displayed properties back into the action.
     * If no action is provided, the last used will be taken!
     * It also resets the changed() state.
     * @param action the action which should be manipulated
     */
    void updateAction(KrAction *action = nullptr);

    /**
     * clears everything
     */
    void clear();

    /**
     * @return true if all properties got valid values
     */
    bool validProperties();

    /**
     * @return true if any property got changed
     */
    bool isModified() {
        return _modified;
    };

signals:
    /**
     * emitted when any actionproperty changed. This signal is only emitted when
     * the _modified attribute changes to true. If there are changes made and
     * _modified is already true, no signal is emitted!
     */
    void changed();

protected slots:
    void setModified(bool m = true);
    /**
     * executes the AddPlaceholderPopup
     */
    void addPlaceholder();
    /**
     * asks for an existing path
     */
    void addStartpath();
    /**
     * (availability) asks for a new protocol
     */
    void newProtocol();
    /**
     * (availability) changes a protocol of the list
     */
    void editProtocol();
    /**
     * (availability) removes a protocol from the list
     */
    void removeProtocol();
    /**
     * (availability) asks for a new path
     */
    void addPath();
    /**
     * (availability) edits a path of the list
     */
    void editPath();
    /**
     * (availability) removes a path from the list
     */
    void removePath();
    /**
     * (availability) asks for a new mime-type
     */
    void addMime();
    /**
     * (availability) changes a mime-type of the list
     */
    void editMime();
    /**
     * (availability) removes a mime-type from the list
     */
    void removeMime();
    /**
     * (availability) asks for a new file-filter
     */
    void newFile();
    /**
     * (availability) edits a file-filter of the list
     */
    void editFile();
    /**
     * (availability) removes a file-filter from the list
     */
    void removeFile();

private:
    KrAction *_action;
    bool _modified;

private slots:
    /**
     * This updates the ShortcutButton
     * @internal
     */
    void changedShortcut(const QKeySequence& shortcut);
};

#endif
