/*
    SPDX-FileCopyrightText: 2004 Jonas Bähr <jonas.baehr@web.de>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef USERACTION_H
#define USERACTION_H

// QtCore
#include <QList>
#include <QSet>
#include <QString>

class QDomDocument;
class QDomElement;
class KrAction;
class QUrl;
class KActionMenu;

/**
 * Useractions are Krusaders backend for user-defined actions on
 * current/selected files in its panels and for krusader's internal actions
 * which need some parameter.
 *
 * There are several components:
 * - The UserAction class as a Manager
 * - The interface to KDE's action-system (the KrAction)
 * - The Expander, which parses the commandline for placeholders and calls
 *   the internal actions
 * - A widget to manipulate the UserAction's Properties via GUI
 *   (ActionProperty)
 *
 * The Useractions are stored in XML-files. Currently there are two main files.
 * The first is a global example-file which is read only (read after the other
 * actionfiles, duplicates are ignored) and a local file where the actions are
 * saved.
 * This class reads only the container and passes each action-tag to the new
 * KrAction, which reads it's data itself.
 */

class UserAction
{
public:
    typedef QList<KrAction *> KrActionList;

    enum ReadMode { renameDoublicated, ignoreDoublicated };

    /**
     * The constructor reads all useractions, see readAllFiles()
     */
    UserAction();
    ~UserAction();

    /**
     * adds an action to the collection.
     */
    void addKrAction(KrAction *action)
    {
        _actions.append(action);
    };

    /**
     * Use this to access the whole list of registered KrActions.
     * currently only used to fill the usermenu with all available actions. This should change...
     * @return A reference to the internal KrActionList
     */
    const KrActionList &actionList()
    {
        return _actions;
    };

    /**
     * @return how many useractions exist
     */
    qsizetype count() const
    {
        return _actions.count();
    };

    /**
     * removes a KrAction from the internal list but does not delete it.
     * @param action the KrAction which should be removed
     */
    void removeKrAction(KrAction *action);

    /**
     * check for each KrAction if it is available for the current location / file and disables it if not
     */
    void setAvailability();
    /**
     * same as above but check for a specitic file
     * @param currentURL Check for this file
     */
    void setAvailability(const QUrl &currentURL);

    /**
     * Fills a KActionMenu with all available UserActions in the list
     * @param menu popupmenu to populate
     * @param currentURL the current URL
     */
    void populateMenu(KActionMenu *menu, const QUrl *currentURL);

    QStringList allCategories();
    QStringList allNames();

    /**
     * reads all predefined useractionfiles.
     */
    void readAllFiles();
    /**
     * writes all actions to the local actionfile
     */
    bool writeActionFile();
    /**
     * Reads UserActions from a xml-file.
     * @param filename the XML file
     * @param mode the read mode
     * @param list If provided, all new actions will also be added to this list
     */
    void readFromFile(const QString &filename, ReadMode mode = renameDoublicated, KrActionList *list = nullptr);
    /**
     * Reads UserActions from a XML-Element.
     * @param element a container with action-elements
     * @param mode the read mode
     * @param list If provided, all new actions will also be added to this list
     */
    void readFromElement(const QDomElement &element, ReadMode mode = renameDoublicated, KrActionList *list = nullptr);

    /**
     * creates an empty QDomDocument for the UserActions
     */
    static QDomDocument createEmptyDoc();
    /**
     * Writes a QDomDocument to an UTF-8 encodes text-file
     * @param doc the XML-Tree
     * @param filename the filename where to save
     * @return true on success, false otherwise
     * @warning any existing file will get overwritten!
     */
    static bool writeToFile(const QDomDocument &doc, const QString &filename);

private:
    KrActionList _actions;
    QSet<QString> _defaultActions;
    QSet<QString> _deletedActions;
};

#define ACTION_XML "krusader/useractions.xml"
#define ACTION_XML_EXAMPLES "krusader/useraction_examples.xml"

#define ACTION_DOCTYPE "KrusaderUserActions"
// in well formed XML the root-element has to have the same name then the doctype:
#define ACTION_ROOT ACTION_DOCTYPE
#define ACTION_PROCESSINSTR "version=\"1.0\" encoding=\"UTF-8\" "

#endif // ifndef USERACTION_H
