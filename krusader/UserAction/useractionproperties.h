//
// C++ Interface: useraction
//
// Description: This manages all useractions
//
//
// Author: Shie Erlich and Rafi Yanai <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef USERACTIONPROPERTIES_H
#define USERACTIONPROPERTIES_H

#include <kshortcut.h>
#include <qstring.h>
#include <qstringlist.h>

/**
 * This holds all properties of a UserAction.
 * @author Jonas Bähr (http://www.jonas-baehr.de)
 */
class UserActionProperties {
public:
     enum ExecType { Terminal, CollectOutput, Normal };
     
    /**
     * This initiate every propertywith it's default
     */
    UserActionProperties(); 
    
    /**
     * Use this to dublicate properties.
     * @param from properties which should be copied
     */
    void copyFrom(UserActionProperties* from);
    
    /**
     * The name(id) has to be unique and is used to identify the action
     * @return The name
     */
    QString* name();
    void setName(const QString& name);
    /**
     * This title is displayd in menus and popups
     * @return The title
     */
    QString* title();
    void setTitle(const QString& title);
    /**
     * For an better overview all actions are grouped by thier category
     * @return The category
     */
    QString* category();
    void setCategory(const QString& category);
    /**
     * The icon of the action
     * @return The icon
     */
    QString* icon();
    void setIcon(const QString& icon);
    /**
     * The tooltip of the action
     * @return The tooltip
     */
    QString* tooltip();
    void setTooltip(const QString& tooltip);
    /**
     * A description of the action (used also as instant-help(waht's this; shift-F1))
     * @return The description
     */
    QString* description();
    void setDescription(const QString& description);
    /**
     * The command beenig executed (with Placeholder)
     * @return The command
     */
    QString* command();
    void setCommand(const QString& command);
    /**
     * A path where the action should be executed
     * @return The startpath
     */
    QString* startpath();
    void setStartpath(const QString& startpath);
    /**
     * Run the command as different user
     * @return The user under which the command should be executed
     */
    QString* user();
    void setUser(const QString& user);
    /**
     * A list of protocols for which the action should be available
     * @return The protocol-list
     */
    QStringList* showonlyProtocol();
    void setShowonlyProtocol(const QStringList& showonlyProtocol);
    /**
     * A list of paths for which the action should be available
     * @return The path-list
     */
    QStringList* showonlyPath();
    void setShowonlyPath(const QStringList& showonlyPath);
    /**
     * A list of mime-types for which the action should be available
     * @return The mime-list
     */
    QStringList* showonlyMime();
    void setShowonlyMime(const QStringList& showonlyMime);
    /**
     * A list of file-filters for which the action should be available
     * @return file-list
     */
    QStringList* showonlyFile();
    void setShowonlyFile(const QStringList& showonlyFile);
    /**
     * How the action should be executed
     * @return The exec-type
     */
    ExecType execType();
    void setExecType(const ExecType& execType);
    /**
     * @return true if the tooltip should be used as desctiption
     */
    bool descriptionUseTooltip();
    void setDescriptionUseTooltip(const bool& descriptionUseTooltip);
    /**
     * @return true if the output-collection should separate std. out from std. error
     */
    bool separateStderr();
    void setSeparateStderr(const bool& separateStderr);
    /**
     * @return true if the Placeholder should return local paths or URL's
     */
    bool acceptURLs();
    void setAcceptURLs(const bool& acceptURLs);
    /**
     * @return true if true the user has to confirm the execution. Here he also can edit the already expanded command
     */
    bool confirmExecution();
    void setConfirmExecution(const bool& confirmExecution);
    /**
     * Default shortcut for the action.
     * @return The shortcut
     */
    KShortcut* defaultShortcut();
    void setDefaultShortcut(const KShortcut& defaultShortcut);
  
private:
    QString _name;
    QString _title;
    QString _category;
    QString _icon;
    QString _tooltip;
    QString _description;
    QString _command;
    QString _startpath;
    QString _user;
    QStringList _showonlyProtocol;
    QStringList _showonlyPath;
    QStringList _showonlyMime;
    QStringList _showonlyFile;
    ExecType _execType;
    bool _descriptionUseTooltip;
    bool _separateStderr;
    bool _acceptURLs;
    bool _callEach;
    bool _confirmExecution;
    KShortcut _defaultShortcut;
};

#endif // ifndef USERACTIONPROPERTIES_H
