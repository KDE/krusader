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

class UserActionProperties;
class AddPlaceholderPopup;

/**
 * Use this widget where ever you need to manipulate a UserAction
 * @author Jonas Bähr (http://www.jonas-baehr.de)
 */
class ActionProperty : public ActionPropertyBase {
	Q_OBJECT 
public:
	ActionProperty( QWidget *parent=0, const char *name=0, UserActionProperties *prop=0 );
	~ActionProperty();
	
	/**
	 * @return the current state of all properties editable within this widget
	 */
	UserActionProperties* properties();
	
	/**
	 * Use this to init the widget with some properties
	 * @param properties the properties which will be displayd
	 */
	void updateGUI( UserActionProperties *properties );
	
	/**
	 * @return true if all properties got valid values
	 */
	bool checkProperties();
	/**
	 * This checks is the name of the action is unique
	 * @param name the name which should be checked
	 * @return true if the name is valid
	 */
	bool checkName( const QString& name );
	
protected slots:
   /**
    * this generates a name for an action or checks if the current name is valid (==unique)
    */
   void proposeName();
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
   AddPlaceholderPopup *_popup;
   UserActionProperties *_properties;

private slots:
   /**
    * keeps the name in the internal _properties up to date.
    * !! don't forget to call them also at the end of updateGUI !!
    */
   void changedName();
   /**
    * keeps the category in the internal _properties up to date.
    * !! don't forget to call them also at the end of updateGUI !!
    */
   void changedCategory();
   /**
    * keeps the icon in the internal _properties up to date.
    * !! don't forget to call them also at the end of updateGUI !!
    */
   void changedIcon();
   /**
    * keeps the title in the internal _properties up to date.
    * !! don't forget to call them also at the end of updateGUI !!
    */
   void changedTitle();
   /**
    * keeps the tooltip in the internal _properties up to date.
    * !! don't forget to call them also at the end of updateGUI !!
    */
   void changedTooltip();
   /**
    * keeps the description in the internal _properties up to date.
    * !! don't forget to call them also at the end of updateGUI !!
    */
   void changedDescription();
   /**
    * keeps useTooltip in the internal _properties up to date.
    * !! don't forget to call them also at the end of updateGUI !!
    */
   void changedChkUseTooltip();
   /**
    * keeps the commandline in the internal _properties up to date.
    * !! don't forget to call them also at the end of updateGUI !!
    */
   void changedCommand();
   /**
    * keeps the startpath in the internal _properties up to date.
    * !! don't forget to call them also at the end of updateGUI !!
    */
   void changedStartpath();
   /**
    * keeps the execution-type in the internal _properties up to date.
    * !! don't forget to call them also at the end of updateGUI !!
    */
   void changedExecType();
   /**
    * keeps useUrl in the internal _properties up to date.
    * !! don't forget to call them also at the end of updateGUI !!
    */
   void changedAccept();
   /**
    * keeps callEach in the internal _properties up to date.
    * !! don't forget to call them also at the end of updateGUI !!
    */
   void changedCallEach();
   /**
    * keeps confirmExecution in the internal _properties up to date.
    * !! don't forget to call them also at the end of updateGUI !!
    */
   void changedConfirmExecution();
   /**
    * keeps the protocol-list in the internal _properties up to date.
    * !! don't forget to call them also at the end of updateGUI !!
    */
   void changedShowonlyProtocol();
   /**
    * keeps the path-list in the internal _properties up to date.
    * !! don't forget to call them also at the end of updateGUI !!
    */
   void changedShowonlyPath();
   /**
    * keeps the mime-type list in the internal _properties up to date.
    * !! don't forget to call them also at the end of updateGUI !!
    */
   void changedShowonlyMime();
   /**
    * keeps the file-filter list in the internal _properties up to date.
    * !! don't forget to call them also at the end of updateGUI !!
    */
   void changedShowonlyFile();
   /**
    * keeps the default shortcut in the internal _properties up to date.
    * !! don't forget to call them also at the end of updateGUI !!
    * @param shortcut the new shortcut
    */
   void changedShortcut(const KShortcut& shortcut);
};

#endif
