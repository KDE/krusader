/*
    SPDX-FileCopyrightText: 2004 Jonas Bähr <jonas.baehr@web.de>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "useraction.h"

// QtCore
#include <QDebug>
#include <QFile>
#include <QHash>
#include <QStandardPaths>
#include <QTextStream>
#include <QUrl>
// QtXml
#include <QDomDocument>
#include <QDomElement>
// QtWidgets
#include <QMenu>

#include <KActionCollection>
#include <KActionMenu>
#include <KLocalizedString>
#include <KMessageBox>

#include "../FileSystem/filesystem.h"
#include "../Panel/PanelView/krview.h"
#include "../Panel/krpanel.h"
#include "../Panel/panelfunc.h"
#include "../krusader.h"
#include "../krusaderview.h"
#include "kraction.h"

UserAction::UserAction()
{
    readAllFiles();
}

UserAction::~UserAction()
{
    // KrActions are deleted by Krusader's KActionCollection
}

void UserAction::removeKrAction(KrAction *action)
{
    _actions.removeAll(action);
    if (_defaultActions.contains(action->objectName()))
        _deletedActions.insert(action->objectName());
}

void UserAction::setAvailability()
{
    setAvailability(ACTIVE_FUNC->files()->getUrl(ACTIVE_PANEL->view->getCurrentItem()));
}

void UserAction::setAvailability(const QUrl &currentURL)
{
    // qDebug() << "UserAction::setAvailability currentFile: " << currentURL.url();
    //  disable the entries that should not appear in this folder
    QListIterator<KrAction *> it(_actions);
    while (it.hasNext()) {
        KrAction *action = it.next();
        action->setEnabled(action->isAvailable(currentURL));
    }
}

void UserAction::populateMenu(KActionMenu *menu, const QUrl *currentURL)
{
    // I have not found any method in Qt/KDE
    // for non-recursive searching of children by name ...
    QMap<QString, KActionMenu *> categoryMap;
    QList<KrAction *> uncategorised;

    for (KrAction *action : std::as_const(_actions)) {
        const QString category = action->category();
        if (!action->isEnabled())
            continue;
        if (currentURL != nullptr && !action->isAvailable(*currentURL))
            continue;
        if (category.isEmpty()) {
            uncategorised.append(action);
        } else {
            if (!categoryMap.contains(category)) {
                auto *categoryMenu = new KActionMenu(category, menu);
                categoryMenu->setObjectName(category);
                categoryMap.insert(category, categoryMenu);
            }
            KActionMenu *targetMenu = categoryMap.value(category);
            targetMenu->addAction(action);
        }
    }

    QMutableMapIterator<QString, KActionMenu *> mapIter(categoryMap);
    while (mapIter.hasNext()) {
        mapIter.next();
        menu->addAction(mapIter.value());
    }

    for (KrAction *action : std::as_const(uncategorised)) {
        menu->addAction(action);
    };
}

QStringList UserAction::allCategories()
{
    QStringList actionCategories;

    QListIterator<KrAction *> it(_actions);
    while (it.hasNext()) {
        KrAction *action = it.next();
        if (actionCategories.indexOf(action->category()) == -1)
            actionCategories.append(action->category());
    }

    return actionCategories;
}

QStringList UserAction::allNames()
{
    QStringList actionNames;

    QListIterator<KrAction *> it(_actions);
    while (it.hasNext()) {
        KrAction *action = it.next();
        actionNames.append(action->objectName());
    }

    return actionNames;
}

void UserAction::readAllFiles()
{
    QString filename = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                              ACTION_XML); // locate returns the local file if it exists, else the global one is retrieved.
    if (!filename.isEmpty())
        readFromFile(QStandardPaths::locate(QStandardPaths::GenericDataLocation, ACTION_XML), renameDoublicated);

    filename = QStandardPaths::locate(QStandardPaths::GenericDataLocation, ACTION_XML_EXAMPLES);
    if (!filename.isEmpty())
        readFromFile(QStandardPaths::locate(QStandardPaths::GenericDataLocation, ACTION_XML_EXAMPLES),
                     ignoreDoublicated); // ignore samples which are already in the normal file
}

void UserAction::readFromFile(const QString &filename, ReadMode mode, KrActionList *list)
{
    QDomDocument *doc = new QDomDocument(ACTION_DOCTYPE);
    QFile file(filename);
    if (file.open(QIODevice::ReadOnly)) {
        // qDebug() << "UserAction::readFromFile - " << filename << "could be opened";
        if (!doc->setContent(&file)) {
            // qDebug() << "UserAction::readFromFile - content set - failed";
            //  if the file doesn't exist till now, the content CAN be set but is empty.
            //  if the content can't be set, the file exists and is NOT an xml-file.
            file.close();
            delete doc;
            doc = nullptr;
            KMessageBox::error(MAIN_VIEW,
                               i18n("The file %1 does not contain valid UserActions.\n", filename), // text
                               i18n("UserActions - cannot read from file") // caption
            );
        }
        file.close();

        if (doc) {
            QDomElement root = doc->documentElement();
            // check if the file has got the right root-element (ACTION_ROOT)
            // this finds out if the xml-file read to the DOM is really a krusader
            // useraction-file
            if (root.tagName() != ACTION_ROOT) {
                KMessageBox::error(MAIN_VIEW,
                                   i18n("The actionfile's root element is not called %1, using %2", QString::fromLatin1(ACTION_ROOT), filename),
                                   i18n("UserActions - cannot read from file"));
                delete doc;
                doc = nullptr;
            }
            readFromElement(root, mode, list);
            delete doc;
        }

    } // if ( file.open( QIODevice::ReadOnly ) )
    else {
        KMessageBox::error(MAIN_VIEW, i18n("Unable to open actions file %1", filename), i18n("UserActions - cannot read from file"));
    }
}

void UserAction::readFromElement(const QDomElement &element, ReadMode mode, KrActionList *list)
{
    for (QDomNode node = element.firstChild(); !node.isNull(); node = node.nextSibling()) {
        QDomElement e = node.toElement();
        if (e.isNull())
            continue; // this should skip nodes which are not elements ( i.e. comments, <!-- -->, or text nodes)

        if (e.tagName() == "action") {
            QString name = e.attribute("name");
            if (name.isEmpty()) {
                KMessageBox::error(
                    MAIN_VIEW,
                    i18n("Action without name detected. This action will not be imported.\nThis is an error in the file, you may want to correct it."),
                    i18n("UserActions - invalid action"));
                continue;
            }

            if (mode == ignoreDoublicated) {
                _defaultActions.insert(name);
                if (krApp->actionCollection()->action(name) || _deletedActions.contains(name))
                    continue;
            }

            QString basename = name + "_%1";
            int i = 0;
            // append a counter till the name is unique... (this checks every action, not only useractions)
            while (krApp->actionCollection()->action(name))
                name = basename.arg(++i);

            KrAction *act = new KrAction(krApp->actionCollection(), name);
            if (act->xmlRead(e)) {
                _actions.append(act);
                if (list)
                    list->append(act);
            } else
                delete act;
        } else if (e.tagName() == "deletedAction") {
            QString name = e.attribute("name");
            if (name.isEmpty()) {
                qWarning() << "A deleted action without name detected! \nThis is an error in the file.";
                continue;
            }
            _deletedActions.insert(name);
        }
    } // for
}

QDomDocument UserAction::createEmptyDoc()
{
    QDomDocument doc = QDomDocument(ACTION_DOCTYPE);
    // adding: <?xml version="1.0" encoding="UTF-8" ?>
    doc.appendChild(doc.createProcessingInstruction("xml", ACTION_PROCESSINSTR));
    // adding root-element
    doc.appendChild(doc.createElement(ACTION_ROOT)); // create new actionfile by adding a root-element ACTION_ROOT
    return doc;
}

bool UserAction::writeActionFile()
{
    QString filename = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + ACTION_XML;

    QDomDocument doc = createEmptyDoc();
    QDomElement root = doc.documentElement();

    for (const QString &name : std::as_const(_deletedActions)) {
        QDomElement element = doc.createElement("deletedAction");
        element.setAttribute("name", name);
        root.appendChild(element);
    }

    QListIterator<KrAction *> it(_actions);
    while (it.hasNext()) {
        KrAction *action = it.next();
        root.appendChild(action->xmlDump(doc));
    }

    return writeToFile(doc, filename);
}

bool UserAction::writeToFile(const QDomDocument &doc, const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly))
        return false;

    /* // This is not needed, because each DomDocument created with UserAction::createEmptyDoc already contains the processinstruction
       if ( ! doc.firstChild().isProcessingInstruction() ) {
          // adding: <?xml version="1.0" encoding="UTF-8" ?> if not already present
          QDomProcessingInstruction instr = doc.createProcessingInstruction( "xml", ACTION_PROCESSINSTR );
          doc.insertBefore( instr, doc.firstChild() );
       }
    */

    //By default, UTF-8 is used for reading and writing
    QTextStream ts(&file);
    ts << doc.toString();

    file.close();
    return true;
}
