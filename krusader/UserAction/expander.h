//
// C++ Interface: expander
//
// Description: 
//
//
// Author: Jonas Bähr (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef EXPANDER_H
#define EXPANDER_H

// class QString;
#include <qstring.h>
class QStringList;
// #include <qstringlist.h>
class ListPanel;

/**
 * The Expander expands the command of an UserAction by replacing all placeholders by thier current values.@n
 * Each placeholder begins with a '%'-sign, followed by one char indicating the panel, followed by a command which may have some paramenter enclosed in brackets and also ends with a '%'-sign.
 * Examples are %aPath% or %rBookmark("/home/jonas/src/krusader_kde3", "yes")%.@n
 * The panel-indicator has to be either 'a' for the active, 'o' for the other, 'r' for the right, 'l' for the left or '_' for panel-independence.
 * 
 * Currently sopported are these commands can be ordered in three groups (childs are the parameter in the right order):
 * - Placeholders for Krusaders panel-data (panel-indicator has to be 'a', 'o', 'r' or 'l')
 *    - @em Path is replaced by the panel's path
 *    - @em Count is replaced by a nomber of
 *       -# Either "All", "Files", "Dirs", "Selected"
 *       .
 *    - @em Filter is preplaced by the panels filter-mask (ex: "*.cpp *.h")
 *    - @em Current is replaced by the current item or, in case of onmultiple="call_each", by each selected item.
 *       -# If "yes", only the filename (without path) is returned
 *       .
 *    - @em List isreplaced by a list of
 *       -# Either "All", "Files", "Dirs", "Selected"
 *       -# A seperator between the items (default: " " [one space])
 *       -# If "yes", only the filename (without path) is returned
 *       -# (for all but "Selected") a filter-mask (default: "*")
 *       .
 *   .
 * - Access to panel-dependent, krusader-internal, parameter-needed functions (panel-indicator has to be 'a', 'o', 'r' or 'l')
 *    - @em Select manipulates the selection of the panel
 *       -# A filter-mask (nessesary)
 *       -# Either "Add", "Remove", "Set" (default)
  *       .
 *    - @em Bookmark manipulates the selection of the panel
 *       -# A path or URL (nessesary)
 *       -# If "yes", the location is opend in a new tab
 *       .
 *   .
 * - Access to panel-independent, krusader-internal, parameter-needed functions (panel-indicator doesn't matter but should be set to '_')
 *    - @em Ask displays a lineedit and is replaced by its return
 *       -# The question (nessesary)
 *       -# A default answer
 *       -# A cation for the popup
 *       .
 *    - @em Clipboard manipulates the system-wide clipboard
 *       -# The string copied to clip (ex: "%aCurrent%") (nessesary)
 *       -# A separator. If set, parameter1 is append with this to the current clipboard content
 *       .
 *    .
 * .
 * Since all placeholders are expanded in the order they appear in the command, little one-line-scripts are possible
 *
 * @author Jonas Bähr (http://www.jonas-baehr.de), Shie Erlich
 */
class Expander {
public:
  /** an expander is a function that receives a QString input, expands
   * it and returns a new QString output containing the expanded expression.
   */
  typedef QString ( *EXPANDER ) ( const ListPanel*, const QStringList&, const bool&, const int& );

  /**
   * This holds informations about each parameter
   */
  typedef struct Parameter {
     QString description; ///< A description of the parameter
     QString preset; ///< the default of the parameter
     bool nessesary; ///< false if the parameter is optional
  };
  
  /**
   * a Placeholder is an entry containing the expression, its expanding function and Parameter
   */
  typedef struct Placeholder {
     QString expression; ///< The placeholder (without '%' or panel-prefix)
     QString description; ///< A description of the placeholder
     EXPANDER expFunc; ///< The function used to expand it
     Parameter parameter[4]; ///< All Parameter of this placeholder (WARNING: max 4; if needed more, change it in the header)
     int parameterCount; ///< How many parameter are realy defined
     bool needPanel; ///< true if the placeholder needs a panel to operate on
  };
  
  /**
   * This expands a whole commandline by calling (if callEach is true) expandCurrent for each selected item (else only once)
   * 
   * @param stringToExpand the commandline with the placeholder
   * @param useUrl true if the path's should be expanded to an URL instead of an local path
   * @param callEach true if %_Current% should be expanded once for every selected item
   * @return a list of all commands (one if callEach is false or one for every selectet item if true)
   */
  static QStringList expand( const QString& stringToExpand, bool useUrl, bool callEach );
  
  static const int numOfPlaceholder = 11; ///< How many Placeholder are realy defined
  /**
   * A List of all Placeholder and thier Parameter. This is used to generate the AddPlaceholderPopup as well as by the expand function
   */
  static Placeholder placeholder[ numOfPlaceholder ];

protected:
  /**
   * expands %_Path% ('_' is replaced by 'a', 'o', 'r' or 'l' to indicate the active, other, right or left panel) with the path of the specified panel
   */
  static QString exp_Path( const ListPanel* panel, const QStringList& parameter, const bool& useUrl, const int& currentItem );
  /**
   * expands %_Count% ('_' is replaced by 'a', 'o', 'r' or 'l' to indicate the active, other, right or left panel) with the number of items, which type is specified by the first Parameter
   */
  static QString exp_Count( const ListPanel* panel, const QStringList& parameter, const bool& useUrl, const int& currentItem );
  /**
   * expands %_Filter% ('_' is replaced by 'a', 'o', 'r' or 'l' to indicate the active, other, right or left panel) with the correspondend filter (ie: "*.cpp")
   */
  static QString exp_Filter( const ListPanel* panel, const QStringList& parameter, const bool& useUrl, const int& currentItem );
  /**
   * expands %_Current% ('_' is replaced by 'a', 'o', 'r' or 'l' to indicate the active, other, right or left panel) with the current item ( != the selected onec)
   */
  static QString exp_Current( const ListPanel* panel, const QStringList& parameter, const bool& useUrl, const int& currentItem );
  /**
   * expands %_List% ('_' is replaced by 'a', 'o', 'r' or 'l' to indicate the active, other, right or left panel) with a list of items, which type is specified by the first Parameter
   */
  static QString exp_List( const ListPanel* panel, const QStringList& parameter, const bool& useUrl, const int& currentItem );
  /**
   * expands %_Ask% ('_' is nessesary because there is no panel needed) with the return of an input-dialog
   */
  static QString exp_Ask( const ListPanel* panel, const QStringList& parameter, const bool& useUrl, const int& currentItem );
  /**
   * This copies it's first Parameter to the clipboard
   */
  static QString exp_Clipboard( const ListPanel* panel, const QStringList& parameter, const bool& useUrl, const int& currentItem );
  /**
   * This selects all items by the mask given with the first Parameter
   */
  static QString exp_Select( const ListPanel* panel, const QStringList& parameter, const bool& useUrl, const int& currentItem );
  /**
   * This changes the panel'spath to the value given with the first Parameter.
   */
  static QString exp_Bookmark( const ListPanel* panel, const QStringList& parameter, const bool& useUrl, const int& currentItem );
  
  /**
   * This expands a whole commandline by calling for each Placeholder the correspondend expander
   * 
   * @param stringToExpand the commandline with the placeholder
   * @param useUrl true if the path's should be expanded to an URL instead of an local path
   * @param currentItem the number of the current item (to expand once with every selectet item) or -1.
   * @return the expanded commanline for the current item
   */
  static QString expandCurrent( const QString& stringToExpand, bool useUrl, int currentItem );
  /**
   * @param panelIndicator either '_' for panel-independent placeholders, 'a', 'o', 'r', or 'l' for the active, other (inactive), right or left panel
   * @return a pointer to the right panel or NULL if no panel is needed.
   */
  static ListPanel* getPanel( const char& panelIndicator );
  /**
   *  This splits the parameter-string into separate parameter and expands each
   * @param exp the string holding all parameter
   * @param useUrl true if the path's should be expanded to an URL instead of an local path
   * @return a list of all parameter
   */
  static QStringList separateParameter( QString* exp, bool useUrl );
  /**
   * This finds the end of a placeholder, taking care of the parameter
   * @return the position where the placeholder ends
   */
  static int findEnd( const QString& str, int start );
  
};

#endif // ifndef EXPANDER_H
