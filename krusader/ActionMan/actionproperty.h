//
// C++ Interface: actionproperty
//
// Description: 
//
//
// Author: Jonas Bähr (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef ACTIONPROPERTY_H
#define ACTIONPROPERTY_H

#include "actionpropertybase.h"

class KrAction;
class KShortcut;

/**
 * Use this widget where ever you need to manipulate a UserAction
 * @author Jonas Bähr (http://www.jonas-baehr.de)
 */
class ActionProperty : public Ui_ActionPropertyBase {
	Q_OBJECT 
public:
	ActionProperty( QWidget *parent=0, const char *name=0, KrAction *action=0 );
	~ActionProperty();
	
	/**
	 * @return the currently displayed action
	 */
	KrAction* action() { return _action; };
	
	/**
	 * This inits the widget with the actions properties.
	 * If no action is provided, the last used will be taken!
	 * It also resets the changed() state.
	 * @param action the action which should be displayd
	 */
	void updateGUI( KrAction *action = 0 );

	/**
	 * This writes the displayed properties back into the action.
	 * If no action is provided, the last used will be taken!
	 * It also resets the changed() state.
	 * @param action the action which should be manipulated
	 */
	void updateAction( KrAction *action = 0 );
	
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
	bool isModified() { return _modified; };

signals:
	/**
	 * emited when any actionproperty changed. This signal is only emited when
	 * the _modified attribute changes to true. If there are changes made and
	 * _modified is already true, no signal is emited!
	 */
	void changed();
	
protected slots:
   void setModified( bool m = true );
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
    * (availability) removes a file-filter from the lsit
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
