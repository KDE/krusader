//
// C++ Interface: useraction
//
// Description: This manages all useractions
//
//
// Author: Jonas Bähr (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef USERACTION_H
#define USERACTION_H

#include <qvaluelist.h>

class UserActionProperties;
class UserActionXML;
class QString;
class QStringList;
class KrAction;
class KURL;

/**
 * Useractions are Krusaders backend for user-defined actions on current/selected files in its panels and for krusader's internal actions which need some parameter. @n
 * There are several komponents:
 * - The xml-file (read by UserActionXML)
 * - The interface to KDE's action-system (the KrAction)
 * - The Expander, which parses the commandline for placeholders and calls the internal actions
 * - A widget to manipulate the UserActionProperties via GUI (ActionProperty)
 * .
 * @author Jonas Bähr (http://www.jonas-baehr.de)
 */

class UserAction {
public:

  typedef QValueList<KrAction*> KrActionList;
  
  /**
   * The constructor reands all useractions out of an xml-file in the users home-dir.
   * @todo: read also from global data for actions for all users
   */
  UserAction();
  ~UserAction();
  
  /**
   * Call this to register a new KrAction
   * @param prop Init the new KrAction with these values
   */
  void addKrAction(UserActionProperties* prop);
  
  /**
   * Searches in all regirsterd KrActions the one with name == name
   * @param name the name of the searched KrAction
   * @return a pointer to a KrAction
   */
  KrAction* action(const QString& name);
  
  /**
   * Use this to access the whole list of registerd KrActions.
   * currently only used to fill the usermenu with all available actions. This should change...
   * @return A pointer to the internal KrActionList
   */
  KrActionList* actionList();
  
  /**
   * updates only the KrAction with name=name with prop (not from the xml)
   * @param name Name of the KrAction to update
   * @param prop Update with this values
   */
  void updateKrAction(const QString& name, UserActionProperties *prop);
  /**
   * same as above
   * @internal
   */
  void updateKrAction(KrAction* action, UserActionProperties* prop);
  
  /**
   * removes a KrAction
   * @param name Name of the KrAction which should be removed
   */
  void removeKrAction(const QString& name);
  
  /**
   * check for each KrAction if it is available for the currend location / file and disables it if not
   */
  void setAvailability();
  /**
   * same as above but check for a specitic file
   * @param currentURL Check for this file
   */
  void setAvailability(const KURL& currentURL);
  
  /**
   * @return A pointer to the useraction-xml-handler
   */
  UserActionXML* xml();

private:
  KrActionList _actions;
  UserActionXML* _xml;
};

#endif // ifndef USERACTION_H
