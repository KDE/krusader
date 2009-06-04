/*****************************************************************************
 * Copyright (C) 2006 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2006 Rafi Yanai <yanai@users.sourceforge.net>               *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This package is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this package; if not, write to the Free Software               *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA *
 *****************************************************************************/

#ifndef USERACTIONPAGE_H
#define USERACTIONPAGE_H

#include <qwidget.h>

class UserActionListView;
class ActionProperty;
class QToolButton;

class UserActionPage : public QWidget {
Q_OBJECT
public:
   UserActionPage( QWidget* parent );
   ~UserActionPage();

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
    * applyes all changes by writing the actionfile and emits "applied"
    */
   void apply();

   //bool _modified; ///< true if the action-tree was changed (= changes were applied to an action)
   UserActionListView *actionTree;
   ActionProperty *actionProperties;
   QToolButton *importButton, *exportButton;
   QToolButton *copyButton, *pasteButton;
   QToolButton *removeButton, *newButton;

private slots:
   void slotChangeCurrent();	//loads a new action into the detail-view
   void slotUpdateAction();	//updates the action to the xml-file
   void slotNewAction();
   void slotRemoveAction();
   void slotImport();
   void slotExport();
   void slotToClip();
   void slotFromClip();
};

#endif
