//
// C++ Interface: useraction
//
// Description: This handles the useraction.xml
//
//
// Author: Jonas Bähr (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef USERACTIONXML_H
#define USERACTIONXML_H

class UserActionProperties;
class QDomDocument;
class QDomElement;
class QString;
class QStringList;

/**
 * This class provides all functions needed for the storing and manipulating stored UserAction.
 *
 * <b>structur of the xml-file</b>
 *
 * The !DOCTYPE  has do be "KrusaderUserActions" as well as the name of the root-element! @n
 * - @em KrusaderUserActions (root-element)
 *    - @em action Each of this elements represents an useraction. @n
 *       attribute @em name Name (id) of the action. (needed)
 *       - @em title The displayed title of the action (needed)
 *       - @em tooltip A tooltip for the action
 *       - @em icon An icon for the action
 *       - @em category A category for the action.
 *       - @em description A description of the action. Also used as instant-help (Shift-F1, "Waht's This")@n
 *          attribute @em same_as Only valid value: "tooltip". If set, the tooltip is used as description
 *       - @em command The commandline which should be executed. It may contain some placeholder, interpreted by the Expander (needed) @n
 *          attribute @em executionmode Valid values: "ternimal", "collect_output", "collect_output_separate_stderr" @n
 *          attribute @em accept Only valid value: "url" @n
 *          attribute @em onmultiplefiles Only valid value: "call_each" @n
 *          attribute @em confirmexecution Only valid value: "true"
 *       - @em startpath Execute the action at this location
 *       - @em availability
 *          - @em protocol If set, the action is only available there (if not, it's allways available)
 *             - @em show Each of this are protocols where the action should be available
 *             .
 *          - @em path If set, the action is only available there (if not, it's allways available)
 *             - @em show Each of this is a path where the action should be available
 *             .
 *          - @em mimetype If set, the action is only available there (if not, it's allways available)
 *             - @em show Each of this are mime-types for which the action should be available
 *             .
 *          - @em filename If set, the action is only available there (if not, it's allways available)
 *             - @em show Each of this are filenames for which the action should be available
 *             .
 *          .
 *       - @em defaultshortcut A default keyboard-shortcut for the action.
 *       .
 *    .
 * .
 *
 * @bug Krusader chrashes if the action-file hasn't got the right doctype/root-element
 * @author Jonas Bähr (http://www.jonas-baehr.de)
 */
class UserActionXML {
public:
  /**
   * This constructor reads the action-file in KDE's local or global data
   */
  UserActionXML();
  /**
   * This reads an actionfile from an given filename
   * @param filename Action-file to parese
   */
  UserActionXML( QString filename );

  /**
   * Write the actionfile to the original filename (given in the constructor)
   */
  void writeActionDom();
  /**
   * This writes the actionfile to a specific file.
   * @param filename File to write to.
   */
  void writeActionDom( QString filename );
  
  /**
   * Check if there already is an action with  this name
   * @param name Name to check
   * @return true is the name exists
   */
  bool nameExists( QString name );
  
  /**
   * Add a new action to the DOM
   * @param prop Properties of the new action
   */
  void addActionToDom( UserActionProperties *prop );
  
  /**
   * Remove the action with this name from the DOM
   */
  void removeAction( QString name );

  /**
   * Updates an action with new properties. The name have to remain the same!
   * @param prop New properties
   * @return 
   */
  bool updateAction( UserActionProperties *prop );
  /**
   * Updates a specific action with new properties
   * @param name Name of the old action
   * @param prop New Properties
   * @return 
   */
  bool updateAction( QString name, UserActionProperties *prop );
  
  /**
   * Read the properties of a specific action
   * @param name Name of the action which should be read
   * @return compleate properties of the action
   */
  UserActionProperties* readAction( QString name );
  
  /**
   * @return A list with the names of all existing actions.
   */
  QStringList getActionNames();
  /**
   * @return A list with all existing categories
   */
  QStringList getActionCategories();

protected:
  void getActionDom();
  QDomElement* findActionByName( QString name );
  QDomElement makeActionElement( UserActionProperties *prop );
  void removeAction( QDomElement *action );
  void updateAction( QDomElement *action, UserActionProperties *prop );
  UserActionProperties* readAction( QDomElement *action );
    
private:
  QDomDocument *_doc;
  QString _filename;
};

#endif // ifndef USERACTIONXML_H
