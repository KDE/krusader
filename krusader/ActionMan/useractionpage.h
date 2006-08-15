//
// C++ Interface: useractionpage
//
// Description: 
//
//
// Author: Shie Erlich and Rafi Yanai <>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef USERACTIONPAGE_H
#define USERACTIONPAGE_H

#include <qwidget.h>

class UserActionListView;
class ActionProperty;
class QToolButton;

/**
 * @author Jonas Bähr
*/
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

private:
   /**
    * If there are modifications in the property-widget, the user is asked
    * what to do. Apply, discard or continue editing. In the first case,
    * saving is done in this function.
    * @return true if a new action can be loaded in the property-widget.
    */
   bool continueInSpiteOfChanges();
   bool _modified;
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

#endif //USERACTIONPAGE_H
