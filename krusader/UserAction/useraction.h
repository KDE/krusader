//
// C++ Interface: useraction
//
// Description: This manages all useractions
//
//
// Author: Jonas B�r (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef USERACTION_H
#define USERACTION_H

#include <qptrlist.h>

class QDomDocument;
class QDomElement;
class QString;
class QStringList;
class KrAction;
class KURL;
class KPopupMenu;

/**
 * Useractions are Krusaders backend for user-defined actions on current/selected files in its panels and for krusader's internal actions which need some parameter. @n
 * There are several komponents:
 * - The xml-file (read by UserActionXML)
 * - The interface to KDE's action-system (the KrAction)
 * - The Expander, which parses the commandline for placeholders and calls the internal actions
 * - A widget to manipulate the UserActionProperties via GUI (ActionProperty)
 * .
 * @author Jonas B�r (http://www.jonas-baehr.de)
 */

class UserAction {
public:

  typedef QPtrList<KrAction> KrActionList;

  enum ReadMode { renameDoublicated, ignoreDoublicated };

  /**
   * The constructor reands all useractions out of an xml-file in the users home-dir.
   */
  UserAction();
  ~UserAction();
  
  /**
   * adds an action to the collection.
   */
  void addKrAction( KrAction* action ) { _actions.append( action ); };
  
  /**
   * Use this to access the whole list of registerd KrActions.
   * currently only used to fill the usermenu with all available actions. This should change...
   * @return A pointer to the internal KrActionList
   */
   const KrActionList actionList() { return _actions; };

  /**
   * @return how many useractions exist
   */
  int count() const { return _actions.count(); };

  /**
   * removes a KrAction and delete it!
   * @param action the KrAction which should be removed
   */
  void removeKrAction( KrAction* action ) { _actions.remove( action ); };
  
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
   * Fills a KPopupMenu with all available UserActions from the xml
   * @param  popupmenu to populate
   */
  void populateMenu(KPopupMenu* menu);

   QStringList allCategories();
   QStringList allNames();

   /**
    * reads all predefined useractionfiles
    */
   void readAllFiles();
   /**
    * writes all actions to the local actionfile
    */
   bool writeActionFile();
   /**
    * Reads UserActions from a xml-file.
    * @param list If provided, all new actions will also be added to this list
    */
   void readFromFile( const QString& filename, ReadMode mode = renameDoublicated, KrActionList* list = 0 );
   /**
    * Reads UserActions from a XML-Element.
    * @param element a container with action-elements
    * @param list If provided, all new actions will also be added to this list
    */
   void readFromElement( const QDomElement& element, ReadMode mode = renameDoublicated, KrActionList* list = 0 );

   /**
    * creates an empty QDomDocument for the UserActions
    */
   static QDomDocument createEmptyDoc();
   /**
    * creates an empty QDomDocument for the UserActions
    * @param doc the XML-Tree
    * @param filename the filename where to save
    * @return true on success, false otherwise
    * @warning any existing file will get overwritten!
    */
   static bool writeToFile( const QDomDocument& doc, const QString& filename );

private:
  KrActionList _actions;
};


#define ACTION_XML				"krusader/useractions.xml"
#define ACTION_XML_EXAMPLES	"krusader/useraction_examples.xml"

#define ACTION_DOCTYPE		"KrusaderUserActions"
// in well formed XML the root-element has to have the same name then the doctype:
#define ACTION_ROOT	ACTION_DOCTYPE
#define ACTION_PROCESSINSTR	"version=\"1.0\" encoding=\"UTF-8\" "



#endif // ifndef USERACTION_H
