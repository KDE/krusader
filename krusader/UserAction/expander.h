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
#include <qvaluelist.h>
// #include <qstringlist.h>
class ListPanel;

/**
 * This holds informations about each parameter
 */
class exp_parameter {
public:
   inline exp_parameter( QString desc, QString pre, bool ness)
      { _description = desc; _preset = pre; _nessesary = ness; }
   inline QString description() ///< A description of the parameter
      { return _description; }
   inline QString preset() ///< the default of the parameter
      { return _preset; }
   inline bool nessesary() ///< false if the parameter is optional
      { return _nessesary; }

private:
   QString _description;
   QString _preset;
   bool _nessesary;
};

#define EXP_FUNC  QString expFunc ( const ListPanel*, const QStringList&, const bool&, const int& )
/** 
  *  Abstract baseclass for all expander-functions (which replace placeholder).
  *  A Placeholder is an entry containing the expression, its expanding function and Parameter.
  *
  * @author Jonas Bähr (http://www.jonas-baehr.de)
  */
class exp_placeholder {
public:
   virtual inline ~exp_placeholder()
      { for ( int i = 0; i < parameterCount(); ++i ) delete parameter( i ); }
   inline QString expression()  ///< The placeholder (without '%' or panel-prefix)
      { return _expression; }
   inline QString description() ///< A description of the placeholder
      { return _description; }
   inline bool needPanel() ///< true if the placeholder needs a panel to operate on
      { return _needPanel; }
   inline void addParameter( exp_parameter* parameter ) ///< adds parameter to the placeholder
      { _parameter.append(parameter); }
   inline int parameterCount() ///< returns the number of placeholders
      { return _parameter.count(); }
   inline exp_parameter* parameter( int id ) ///< returns a specific parameter
      { return _parameter[ id ]; }

   virtual EXP_FUNC = 0;
protected:
   QString _expression;
   QString _description;
   QValueList <exp_parameter*> _parameter;
   bool _needPanel;
};

class exp_separator  : public exp_placeholder {
public:
   inline exp_separator( bool needPanel )
      { _needPanel = needPanel; }
   inline EXP_FUNC { return QString::null; }
};

/**
  * expands %_Path% ('_' is replaced by 'a', 'o', 'r' or 'l' to indicate the active, other, right or left panel) with the path of the specified panel
  */
class exp_Path : public exp_placeholder {
public:
   exp_Path();
   EXP_FUNC;
};

/**
  * expands %_Count% ('_' is replaced by 'a', 'o', 'r' or 'l' to indicate the active, other, right or left panel) with the number of items, which type is specified by the first Parameter
  */
class exp_Count : public exp_placeholder {
public:
   exp_Count();
   EXP_FUNC;
};

/**
  * expands %_Filter% ('_' is replaced by 'a', 'o', 'r' or 'l' to indicate the active, other, right or left panel) with the correspondend filter (ie: "*.cpp")
  */
class exp_Filter : public exp_placeholder {
public:
   exp_Filter();
   EXP_FUNC;
};

/**
  * expands %_Current% ('_' is replaced by 'a', 'o', 'r' or 'l' to indicate the active, other, right or left panel) with the current item ( != the selected onec)
  */
class exp_Current : public exp_placeholder {
public:
   exp_Current();
   EXP_FUNC;
};

/**
  * expands %_List% ('_' is replaced by 'a', 'o', 'r' or 'l' to indicate the active, other, right or left panel) with a list of items, which type is specified by the first Parameter
  */
class exp_List : public exp_placeholder {
public:
   exp_List();
   EXP_FUNC;
};
  
/**
  * expands %_Ask% ('_' is nessesary because there is no panel needed) with the return of an input-dialog
  */
class exp_Ask : public exp_placeholder {
public:
   exp_Ask();
   EXP_FUNC;
};
  
/**
  * This copies it's first Parameter to the clipboard
  */
class exp_Clipboard : public exp_placeholder {
public:
   exp_Clipboard();
   EXP_FUNC;
};
  
/**
  * This selects all items by the mask given with the first Parameter
  */
class exp_Select : public exp_placeholder {
public:
   exp_Select();
   EXP_FUNC;
};
  
/**
  * This changes the panel'spath to the value given with the first Parameter.
  */
class exp_Goto : public exp_placeholder {
public:
   exp_Goto();
   EXP_FUNC;
};

/**
  * This is equal to 'cp <first Parameter> <second Parameter>'.
  */
class exp_Copy : public exp_placeholder {
public:
   exp_Copy();
   EXP_FUNC;
};
  
/**
  * This opens the synchronizer with a given profile
  */
class exp_Sync : public exp_placeholder {
public:
   exp_Sync();
   EXP_FUNC;
};


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
  Expander();
  ~Expander();
  
   inline void addPlaceholder( exp_placeholder* placeholder ) ///< adds placeholder to the expander.
      { _placeholder.append(placeholder); }
   inline int placeholderCount() ///< returns the number of placeholders
      { return _placeholder.count(); }
   inline exp_placeholder* placeholder( int id )
      { return _placeholder[ id ]; }

   /**
     * This expands a whole commandline by calling (if callEach is true) expandCurrent for each selected item (else only once)
     * 
     * @param stringToExpand the commandline with the placeholder
     * @param useUrl true if the path's should be expanded to an URL instead of an local path
     * @param callEach true if %_Current% should be expanded once for every selected item
     * @return a list of all commands (one if callEach is false or one for every selectet item if true)
     */
   QStringList expand( const QString& stringToExpand, bool useUrl, bool callEach );

protected:
  /**
   * This expands a whole commandline by calling for each Placeholder the correspondend expander
   * 
   * @param stringToExpand the commandline with the placeholder
   * @param useUrl true if the path's should be expanded to an URL instead of an local path
   * @param currentItem the number of the current item (to expand once with every selectet item) or -1.
   * @return the expanded commanline for the current item
   */
  QString expandCurrent( const QString& stringToExpand, bool useUrl, int currentItem );
  /**
   * @param panelIndicator either '_' for panel-independent placeholders, 'a', 'o', 'r', or 'l' for the active, other (inactive), right or left panel
   * @return a pointer to the right panel or NULL if no panel is needed.
   */
  ListPanel* getPanel( const char& panelIndicator );
  /**
   *  This splits the parameter-string into separate parameter and expands each
   * @param exp the string holding all parameter
   * @param useUrl true if the path's should be expanded to an URL instead of an local path
   * @return a list of all parameter
   */
  QStringList separateParameter( QString* exp, bool useUrl );
  /**
   * This finds the end of a placeholder, taking care of the parameter
   * @return the position where the placeholder ends
   */
  int findEnd( const QString& str, int start );
  
private:
   QValueList <exp_placeholder*> _placeholder;
};

#endif // ifndef EXPANDER_H
