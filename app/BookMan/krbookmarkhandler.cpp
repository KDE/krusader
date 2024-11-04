/*
    SPDX-FileCopyrightText: 2002 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2002 Rafi Yanai <yanai@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "krbookmarkhandler.h"
#include "kraddbookmarkdlg.h"

#include "../Dialogs/popularurls.h"
#include "../FileSystem/filesystem.h"
#include "../Panel/krpanel.h"
#include "../Panel/listpanelactions.h"
#include "../icon.h"
#include "../kractions.h"
#include "../krglobal.h"
#include "../krmainwindow.h"
#include "../krslots.h"

// QtCore
#include <QDebug>
#include <QEvent>
#include <QFile>
#include <QStandardPaths>
#include <QTextStream>
#include <QTimer>
// QtGui
#include <QCursor>
#include <QMouseEvent>

#include <KActionCollection>
#include <KBookmarkManager>
#include <KBookmarkMenu>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>
#include <utility>

#define SPECIAL_BOOKMARKS true

// ------------------------ for internal use
#define BOOKMARKS_FILE "krusader/krbookmarks.xml"
#define CONNECT_BM(X)                                                                                                                                          \
    {                                                                                                                                                          \
        disconnect(X, SIGNAL(activated(QUrl)), nullptr, nullptr);                                                                                              \
        connect(X, SIGNAL(activated(QUrl)), this, SLOT(slotActivated(QUrl)));                                                                                  \
    }

KrBookmarkHandler::KrBookmarkHandler(KrMainWindow *mainWindow)
    : QObject(mainWindow->widget())
    , _mainWindow(mainWindow)
    , _middleClick(false)
    , _mainBookmarkPopup(nullptr)
    , _quickSearchAction(nullptr)
    , _quickSearchBar(nullptr)
    , _quickSearchMenu(nullptr)
{
    // create our own action collection and make the shortcuts apply only to parent
    _privateCollection = new KActionCollection(this);
    _collection = _mainWindow->actions();

    // create _root: father of all bookmarks. it is a dummy bookmark and never shown
    _root = new KrBookmark(i18n("Bookmarks"));
    _root->setParent(this);

    // load bookmarks
    importFromFile();

    // create bookmark manager
    QString filename = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + BOOKMARKS_FILE;
    manager = new KBookmarkManager(filename, this);

    connect(manager, &KBookmarkManager::changed, this, &KrBookmarkHandler::bookmarksChanged);

    // create the quick search bar and action
    _quickSearchAction = new QWidgetAction(this);
    _quickSearchBar = new QLineEdit();
    _quickSearchBar->setPlaceholderText(i18n("Type to search..."));
    _quickSearchAction->setDefaultWidget(_quickSearchBar); // ownership of the bar is transferred to the action
    _quickSearchAction->setEnabled(false);
    _setQuickSearchText("");

    // fill a dummy menu to properly init actions (allows toolbar bookmark buttons to work properly)
    auto menu = new QMenu(mainWindow->widget());
    bookmarksMenu = new KBookmarkMenu(manager, nullptr, menu);

    populate(menu);
    menu->deleteLater();

}

KrBookmarkHandler::~KrBookmarkHandler()
{
    delete manager;
    delete _privateCollection;
}

void KrBookmarkHandler::bookmarkCurrent(QUrl url)
{
    QPointer<KrAddBookmarkDlg> dlg = new KrAddBookmarkDlg(_mainWindow->widget(), std::move(url));
    if (dlg->exec() == QDialog::Accepted) {
        KrBookmark *bm = new KrBookmark(dlg->name(), dlg->url(), _collection);
        addBookmark(bm, dlg->folder());
    }
    delete dlg;
}

void KrBookmarkHandler::addBookmark(KrBookmark *bm, KrBookmark *folder)
{
    if (folder == nullptr)
        folder = _root;

    // add to the list (bottom)
    folder->children().append(bm);

    exportToFile();
}

void KrBookmarkHandler::deleteBookmark(KrBookmark *bm)
{
    if (bm->isFolder())
        clearBookmarks(bm); // remove the child bookmarks
    removeReferences(_root, bm);

    const auto widgets = bm->associatedObjects();
    for (QObject *w : widgets)
        qobject_cast<QWidget *>(w)->removeAction(bm);
    delete bm;

    exportToFile();
}

void KrBookmarkHandler::removeReferences(KrBookmark *root, KrBookmark *bmToRemove)
{
    qsizetype index = root->children().indexOf(bmToRemove);
    if (index >= 0)
        root->children().removeAt(index);

    QListIterator<KrBookmark *> it(root->children());
    while (it.hasNext()) {
        KrBookmark *bm = it.next();
        if (bm->isFolder())
            removeReferences(bm, bmToRemove);
    }
}

void KrBookmarkHandler::exportToFileBookmark(QDomDocument &doc, QDomElement &where, KrBookmark *bm)
{
    if (bm->isSeparator()) {
        QDomElement bookmark = doc.createElement("separator");
        where.appendChild(bookmark);
    } else {
        QDomElement bookmark = doc.createElement("bookmark");
        // url
        bookmark.setAttribute("href", bm->url().toDisplayString());
        // icon
        bookmark.setAttribute("icon", bm->iconName());
        // title
        QDomElement title = doc.createElement("title");
        title.appendChild(doc.createTextNode(bm->text()));
        bookmark.appendChild(title);

        where.appendChild(bookmark);
    }
}

void KrBookmarkHandler::exportToFileFolder(QDomDocument &doc, QDomElement &parent, KrBookmark *folder)
{
    QListIterator<KrBookmark *> it(folder->children());
    while (it.hasNext()) {
        KrBookmark *bm = it.next();

        if (bm->isFolder()) {
            QDomElement newFolder = doc.createElement("folder");
            newFolder.setAttribute("icon", bm->iconName());
            parent.appendChild(newFolder);
            QDomElement title = doc.createElement("title");
            title.appendChild(doc.createTextNode(bm->text()));
            newFolder.appendChild(title);
            exportToFileFolder(doc, newFolder, bm);
        } else {
            exportToFileBookmark(doc, parent, bm);
        }
    }
}

// export to file using the xbel standard
//
//  <xbel>
//    <bookmark href="https://techbase.kde.org/"><title>Developer Web Site</title></bookmark>
//    <folder folded="no">
//      <title>Title of this folder</title>
//      <bookmark icon="kde" href="https://www.kde.org"><title>KDE Web Site</title></bookmark>
//      <folder toolbar="yes">
//        <title>My own bookmarks</title>
//        <bookmark href="https://www.calligra.org/"><title>Calligra Suite Web Site</title></bookmark>
//        <separator/>
//        <bookmark href="https://www.kdevelop.org/"><title>KDevelop Web Site</title></bookmark>
//      </folder>
//    </folder>
//  </xbel>
void KrBookmarkHandler::exportToFile()
{
    QDomDocument doc("xbel");
    QDomElement root = doc.createElement("xbel");
    doc.appendChild(root);

    exportToFileFolder(doc, root, _root);
    if (!doc.firstChild().isProcessingInstruction()) {
        // adding: <?xml version="1.0" encoding="UTF-8" ?> if not already present
        QDomProcessingInstruction instr = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\" ");
        doc.insertBefore(instr, doc.firstChild());
    }

    QString filename = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + BOOKMARKS_FILE;
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream stream(&file);
         // By default, UTF-8 is used for reading and writing in QTextStream
        stream << doc.toString();
        file.close();
    } else {
        KMessageBox::error(_mainWindow->widget(), i18n("Unable to write to %1", filename), i18n("Error"));
    }
}

bool KrBookmarkHandler::importFromFileBookmark(QDomElement &e, KrBookmark *parent, const QString &path, QString *errorMsg)
{
    QString url, name, iconName;
    // verify tag
    if (e.tagName() != "bookmark") {
        *errorMsg = i18n("%1 instead of %2", e.tagName(), QLatin1String("bookmark"));
        return false;
    }
    // verify href
    if (!e.hasAttribute("href")) {
        *errorMsg = i18n("missing tag %1", QLatin1String("href"));
        return false;
    } else
        url = e.attribute("href");
    // verify title
    QDomElement te = e.firstChild().toElement();
    if (te.tagName() != "title") {
        *errorMsg = i18n("missing tag %1", QLatin1String("title"));
        return false;
    } else
        name = te.text();
    // do we have an icon?
    if (e.hasAttribute("icon")) {
        iconName = e.attribute("icon");
    }
    // ok: got name and url, let's add a bookmark
    KrBookmark *bm = KrBookmark::getExistingBookmark(path + name, _collection);
    if (!bm) {
        bm = new KrBookmark(name, QUrl(url), _collection, iconName, path + name);
    } else {
        bm->setURL(QUrl(url));
        bm->setIconName(iconName);
    }
    parent->children().append(bm);

    return true;
}

bool KrBookmarkHandler::importFromFileFolder(QDomNode &first, KrBookmark *parent, const QString &path, QString *errorMsg)
{
    QString name;
    QDomNode n = first;
    while (!n.isNull()) {
        QDomElement e = n.toElement();
        if (e.tagName() == "bookmark") {
            if (!importFromFileBookmark(e, parent, path, errorMsg))
                return false;
        } else if (e.tagName() == "folder") {
            QString iconName = "";
            if (e.hasAttribute("icon"))
                iconName = e.attribute("icon");
            // the title is the first child of the folder
            QDomElement tmp = e.firstChild().toElement();
            if (tmp.tagName() != "title") {
                *errorMsg = i18n("missing tag %1", QLatin1String("title"));
                return false;
            } else
                name = tmp.text();
            KrBookmark *folder = new KrBookmark(name, iconName);
            parent->children().append(folder);

            QDomNode nextOne = tmp.nextSibling();
            if (!importFromFileFolder(nextOne, folder, path + name + '/', errorMsg))
                return false;
        } else if (e.tagName() == "separator") {
            parent->children().append(KrBookmark::separator());
        }
        n = n.nextSibling();
    }
    return true;
}

void KrBookmarkHandler::importFromFile()
{
    clearBookmarks(_root, false);

    QString filename = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + BOOKMARKS_FILE;
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
        return; // no bookmarks file

    QString errorMsg;
    QDomNode n;
    QDomElement e;
    QDomDocument doc("xbel");
    if (!doc.setContent(&file, &errorMsg)) {
        goto BM_ERROR;
    }
    // iterate through the document: first child should be "xbel" (skip all until we find it)
    n = doc.firstChild();
    while (!n.isNull() && n.toElement().tagName() != "xbel")
        n = n.nextSibling();

    if (n.isNull() || n.toElement().tagName() != "xbel") {
        errorMsg = i18n("%1 does not seem to be a valid bookmarks file", filename);
        goto BM_ERROR;
    } else
        n = n.firstChild(); // skip the xbel part
    importFromFileFolder(n, _root, "", &errorMsg);
    goto BM_SUCCESS;

BM_ERROR:
    KMessageBox::error(_mainWindow->widget(), i18n("Error reading bookmarks file: %1", errorMsg), i18n("Error"));

BM_SUCCESS:
    file.close();
}

void KrBookmarkHandler::_setQuickSearchText(const QString &text)
{
    bool isEmptyQuickSearchBarVisible = KConfigGroup(krConfig, "Look&Feel").readEntry("Always show search bar", true);

    _quickSearchBar->setText(text);

    auto length = text.length();
    bool isVisible = isEmptyQuickSearchBarVisible || length > 0;
    _quickSearchAction->setVisible(isVisible);
    _quickSearchBar->setVisible(isVisible);

    if (length == 0) {
        qDebug() << "Bookmark search: reset";
        _resetActionTextAndHighlighting();
    } else {
        qDebug() << "Bookmark search: query =" << text;
    }
}

QString KrBookmarkHandler::_quickSearchText() const
{
    return _quickSearchBar->text();
}

void KrBookmarkHandler::_highlightAction(QAction *action, bool isMatched)
{
    auto font = action->font();
    font.setBold(isMatched);
    action->setFont(font);
}

void KrBookmarkHandler::populate(QMenu *menu)
{
    // removing action from previous menu is necessary
    // otherwise it won't be displayed in the currently populating menu
    if (_mainBookmarkPopup) {
        _mainBookmarkPopup->removeAction(_quickSearchAction);
    }
    _mainBookmarkPopup = menu;
    menu->clear();
    _specialBookmarks.clear();
    buildMenu(_root, menu);
}

void KrBookmarkHandler::buildMenu(KrBookmark *parent, QMenu *menu, int depth)
{
    // add search bar widget to the top of the menu
    if (depth == 0) {
        menu->addAction(_quickSearchAction);
    }

    // run the loop twice, in order to put the folders on top. stupid but easy :-)
    // note: this code drops the separators put there by the user
    QListIterator<KrBookmark *> it(parent->children());
    while (it.hasNext()) {
        KrBookmark *bm = it.next();

        if (!bm->isFolder())
            continue;
        auto *newMenu = new QMenu(menu);
        newMenu->setIcon(Icon(bm->iconName()));
        newMenu->setTitle(bm->text());
        QAction *menuAction = menu->addMenu(newMenu);
        QVariant v;
        v.setValue(bm);
        menuAction->setData(v);

        buildMenu(bm, newMenu, depth + 1);
    }

    it.toFront();
    while (it.hasNext()) {
        KrBookmark *bm = it.next();
        if (bm->isFolder())
            continue;
        if (bm->isSeparator()) {
            menu->addSeparator();
            continue;
        }

        QUrl urlToSet = bm->url();
        if (!urlToSet.isEmpty() && urlToSet.isRelative()) {
            // Make it possible that the url can be used later by Krusader.
            // This avoids users seeing the effects described in the "Editing a local path in Bookmark
            // Manager breaks a bookmark" bug report (https://bugs.kde.org/show_bug.cgi?id=393320),
            // though it would be better to solve that upstream frameworks-kbookmarks bug
            bm->setURL(QUrl::fromUserInput(urlToSet.toString(), QString(), QUrl::AssumeLocalFile));
        }

        menu->addAction(bm);
        CONNECT_BM(bm);
    }

    if (depth == 0) {
        KConfigGroup group(krConfig, "Private");
        bool hasPopularURLs = group.readEntry("BM Popular URLs", true);
        bool hasTrash = group.readEntry("BM Trash", true);
        bool hasLan = group.readEntry("BM Lan", true);
        bool hasVirtualFS = group.readEntry("BM Virtual FS", true);
        bool hasJumpback = group.readEntry("BM Jumpback", true);

        if (hasPopularURLs) {
            menu->addSeparator();

            // add the popular links submenu
            auto *newMenu = new QMenu(menu);
            newMenu->setTitle(i18n("Popular URLs"));
            newMenu->setIcon(Icon("folder-bookmark"));
            QAction *bmfAct = menu->addMenu(newMenu);
            _specialBookmarks.append(bmfAct);
            // add the top 15 urls
#define MAX 15
            QList<QUrl> list = _mainWindow->popularUrls()->getMostPopularUrls(MAX);
            QList<QUrl>::Iterator it;
            for (it = list.begin(); it != list.end(); ++it) {
                QString name;
                if ((*it).isLocalFile())
                    name = (*it).path();
                else
                    name = (*it).toDisplayString();
                // note: these bookmark are put into the private collection
                // as to not spam the general collection
                KrBookmark *bm = KrBookmark::getExistingBookmark(name, _privateCollection);
                if (!bm)
                    bm = new KrBookmark(name, *it, _privateCollection);
                newMenu->addAction(bm);
                CONNECT_BM(bm);
            }

            newMenu->addSeparator();
            if (krPopularUrls != nullptr) {
                newMenu->addAction(krPopularUrls);
            }
            newMenu->installEventFilter(this);
        }

        // do we need to add special bookmarks?
        if (SPECIAL_BOOKMARKS) {
            if (hasTrash || hasLan || hasVirtualFS)
                menu->addSeparator();

            KrBookmark *bm;

            // note: special bookmarks are not kept inside the _bookmarks list and added ad-hoc
            if (hasTrash) {
                bm = KrBookmark::trash(_collection);
                menu->addAction(bm);
                _specialBookmarks.append(bm);
                CONNECT_BM(bm);
            }

            if (hasLan) {
                bm = KrBookmark::lan(_collection);
                menu->addAction(bm);
                _specialBookmarks.append(bm);
                CONNECT_BM(bm);
            }

            if (hasVirtualFS) {
                bm = KrBookmark::virt(_collection);
                menu->addAction(bm);
                _specialBookmarks.append(bm);
                CONNECT_BM(bm);
            }

            if (hasJumpback) {
                menu->addSeparator();

                ListPanelActions *actions = _mainWindow->listPanelActions();

                auto slotTriggered = [=] {
                    if (_mainBookmarkPopup && !_mainBookmarkPopup->isHidden()) {
                        _mainBookmarkPopup->close();
                    }
                };
                auto addJumpBackAction = [=](bool isSetter) {
                    auto action = KrBookmark::jumpBackAction(_privateCollection, isSetter, actions);
                    if (action) {
                        menu->addAction(action);
                        _specialBookmarks.append(action);

                        // disconnecting from this as a receiver is important:
                        // we don't want to break connections established by KrBookmark::jumpBackAction
                        disconnect(action, &QAction::triggered, this, nullptr);
                        connect(action, &QAction::triggered, this, slotTriggered);
                    }
                };

                addJumpBackAction(true);
                addJumpBackAction(false);
            }
        }

        menu->addSeparator();
        if (KrActions::actAddBookmark != nullptr) {
            menu->addAction(KrActions::actAddBookmark);
            _specialBookmarks.append(KrActions::actAddBookmark);
        }

        QAction *bmAct = bookmarksMenu->editBookmarksAction();
        menu->addAction(bmAct);
        _specialBookmarks.append(bmAct);

        // make sure the menu is connected to us
        disconnect(menu, SIGNAL(triggered(QAction *)), nullptr, nullptr);
    }

    menu->installEventFilter(this);
}

void KrBookmarkHandler::clearBookmarks(KrBookmark *root, bool removeBookmarks)
{
    for (auto it = root->children().begin(); it != root->children().end(); it = root->children().erase(it)) {
        KrBookmark *bm = *it;

        if (bm->isFolder()) {
            clearBookmarks(bm, removeBookmarks);
            delete bm;
        } else if (bm->isSeparator()) {
            delete bm;
        } else if (removeBookmarks) {
            const auto widgets = bm->associatedObjects();
            for (QObject *w : widgets) {
                qobject_cast<QWidget *>(w)->removeAction(bm);
            }
            delete bm;
        }
    }
}

void KrBookmarkHandler::bookmarksChanged(const QString &)
{
    importFromFile();
}

bool KrBookmarkHandler::eventFilter(QObject *obj, QEvent *ev)
{
    auto eventType = ev->type();
    auto *menu = qobject_cast<QMenu *>(obj);

    if (eventType == QEvent::Show && menu) {
        _setQuickSearchText("");
        _quickSearchMenu = menu;
        qDebug() << "Bookmark search: menu" << menu << "is shown";

        return QObject::eventFilter(obj, ev);
    }

    if (eventType == QEvent::Close && menu && _quickSearchMenu) {
        if (_quickSearchMenu == menu) {
            qDebug() << "Bookmark search: stopped on menu" << menu;
            _setQuickSearchText("");
            _quickSearchMenu = nullptr;
        } else {
            qDebug() << "Bookmark search: active action =" << _quickSearchMenu->activeAction();

            // fix automatic deactivation of current action due to spurious close event from submenu
            auto quickSearchMenu = _quickSearchMenu;
            auto activeAction = _quickSearchMenu->activeAction();
            QTimer::singleShot(0, this, [=]() {
                qDebug() << "Bookmark search: active action =" << quickSearchMenu->activeAction();
                if (!quickSearchMenu->activeAction() && activeAction) {
                    quickSearchMenu->setActiveAction(activeAction);
                    qDebug() << "Bookmark search: restored active action =" << quickSearchMenu->activeAction();
                }
            });
        }

        return QObject::eventFilter(obj, ev);
    }

    // Having it occur on keypress is consistent with other shortcuts,
    // such as Ctrl+W and accelerator keys
    if (eventType == QEvent::KeyPress && menu) {
        auto *kev = dynamic_cast<QKeyEvent *>(ev);
        const QList<QAction *> acts = menu->actions();
        bool quickSearchStarted = false;
        bool searchInSpecialItems = KConfigGroup(krConfig, "Look&Feel").readEntry("Search in special items", false);

        if (kev->key() == Qt::Key_Left && kev->modifiers() == Qt::NoModifier) {
            menu->close();
            return true;
        }

        if ((kev->modifiers() != Qt::ShiftModifier && kev->modifiers() != Qt::NoModifier) || kev->text().isEmpty() || kev->key() == Qt::Key_Delete
            || kev->key() == Qt::Key_Return || kev->key() == Qt::Key_Escape) {
            return QObject::eventFilter(obj, ev);
        }

        // update quick search text
        if (kev->key() == Qt::Key_Backspace) {
            auto newSearchText = _quickSearchText();
            newSearchText.chop(1);
            _setQuickSearchText(newSearchText);

            if (_quickSearchText().length() == 0) {
                return QObject::eventFilter(obj, ev);
            }
        } else {
            quickSearchStarted = _quickSearchText().length() == 0;
            _setQuickSearchText(_quickSearchText().append(kev->text()));
        }

        if (quickSearchStarted) {
            _quickSearchMenu = menu;
            qDebug() << "Bookmark search: started on menu" << menu;
        }

        // match actions
        QAction *matchedAction = nullptr;
        int nMatches = 0;
        const Qt::CaseSensitivity matchCase = _quickSearchText() == _quickSearchText().toLower() ? Qt::CaseInsensitive : Qt::CaseSensitive;
        for (auto act : acts) {
            if (act->isSeparator() || act->text().isEmpty()) {
                continue;
            }

            if (!searchInSpecialItems && _specialBookmarks.contains(act)) {
                continue;
            }

            if (quickSearchStarted) {
                // if the first key press is an accelerator key, let the accelerator handler process this event
                if (act->text().contains('&' + kev->text(), Qt::CaseInsensitive)) {
                    qDebug() << "Bookmark search: hit accelerator key of" << act;
                    _setQuickSearchText("");
                    return QObject::eventFilter(obj, ev);
                }

                // strip accelerator keys from actions so they don't interfere with the search key press events
                auto text = act->text();
                _quickSearchOriginalActionTitles.insert(act, text);
                act->setText(KLocalizedString::removeAcceleratorMarker(text));
            }

            // match prefix of the action text to the query
            if (act->text().left(_quickSearchText().length()).compare(_quickSearchText(), matchCase) == 0) {
                _highlightAction(act);
                if (!matchedAction || matchedAction->menu()) {
                    // Can't highlight menus (see comment below), hopefully pick something we can
                    matchedAction = act;
                }
                nMatches++;
            } else {
                _highlightAction(act, false);
            }
        }

        if (matchedAction) {
            qDebug() << "Bookmark search: primary match =" << matchedAction->text() << ", number of matches =" << nMatches;
        } else {
            qDebug() << "Bookmark search: no matches";
        }

        // trigger the matched menu item or set an active item accordingly
        if (nMatches == 1) {
            _setQuickSearchText("");
            if ((bool)matchedAction->menu()) {
                menu->setActiveAction(matchedAction);
            } else {
                matchedAction->activate(QAction::Trigger);
            }
        } else if (nMatches > 1) {
            // Because of a bug submenus cannot be highlighted
            // https://bugreports.qt.io/browse/QTBUG-939
            if (!matchedAction->menu()) {
                menu->setActiveAction(matchedAction);
            } else {
                menu->setActiveAction(nullptr);
            }
        } else {
            menu->setActiveAction(nullptr);
        }
        return true;
    }

    if (eventType == QEvent::MouseButtonRelease) {
        switch (dynamic_cast<QMouseEvent *>(ev)->button()) {
        case Qt::RightButton:
            _middleClick = false;
            if (obj->inherits("QMenu")) {
                auto *menu = dynamic_cast<QMenu *>(obj);
                QAction *act = menu->actionAt(dynamic_cast<QMouseEvent *>(ev)->pos());

                if (obj == _mainBookmarkPopup && _specialBookmarks.contains(act)) {
                    rightClickOnSpecialBookmark();
                    return true;
                }

                auto *bm = qobject_cast<KrBookmark *>(act);
                if (bm != nullptr) {
                    rightClicked(menu, bm);
                    return true;
                } else if (act && act->data().canConvert<KrBookmark *>()) {
                    auto *bm = act->data().value<KrBookmark *>();
                    rightClicked(menu, bm);
                }
            }
            break;
        case Qt::LeftButton:
            _middleClick = false;
            break;
        case Qt::MiddleButton:
            _middleClick = true;
            break;
        default:
            break;
        }
    }
    return QObject::eventFilter(obj, ev);
}

void KrBookmarkHandler::_resetActionTextAndHighlighting()
{
    for (QHash<QAction *, QString>::const_iterator i = _quickSearchOriginalActionTitles.constBegin(); i != _quickSearchOriginalActionTitles.constEnd(); ++i) {
        QAction *action = i.key();
        action->setText(i.value());
        _highlightAction(action, false);
    }

    _quickSearchOriginalActionTitles.clear();
}

#define POPULAR_URLS_ID 100100
#define TRASH_ID 100101
#define LAN_ID 100103
#define VIRTUAL_FS_ID 100102
#define JUMP_BACK_ID 100104

void KrBookmarkHandler::rightClickOnSpecialBookmark()
{
    KConfigGroup group(krConfig, "Private");
    bool hasPopularURLs = group.readEntry("BM Popular URLs", true);
    bool hasTrash = group.readEntry("BM Trash", true);
    bool hasLan = group.readEntry("BM Lan", true);
    bool hasVirtualFS = group.readEntry("BM Virtual FS", true);
    bool hasJumpback = group.readEntry("BM Jumpback", true);

    QMenu menu(_mainBookmarkPopup);
    menu.setTitle(i18n("Enable special bookmarks"));

    QAction *act;

    act = menu.addAction(i18n("Popular URLs"));
    act->setData(QVariant(POPULAR_URLS_ID));
    act->setCheckable(true);
    act->setChecked(hasPopularURLs);
    act = menu.addAction(i18n("Trash bin"));
    act->setData(QVariant(TRASH_ID));
    act->setCheckable(true);
    act->setChecked(hasTrash);
    act = menu.addAction(i18n("Local Network"));
    act->setData(QVariant(LAN_ID));
    act->setCheckable(true);
    act->setChecked(hasLan);
    act = menu.addAction(i18n("Virtual Filesystem"));
    act->setData(QVariant(VIRTUAL_FS_ID));
    act->setCheckable(true);
    act->setChecked(hasVirtualFS);
    act = menu.addAction(i18n("Jump back"));
    act->setData(QVariant(JUMP_BACK_ID));
    act->setCheckable(true);
    act->setChecked(hasJumpback);

    connect(_mainBookmarkPopup, SIGNAL(highlighted(int)), &menu, SLOT(close()));
    connect(_mainBookmarkPopup, SIGNAL(activated(int)), &menu, SLOT(close()));

    int result = -1;
    QAction *res = menu.exec(QCursor::pos());
    if (res && res->data().canConvert<int>())
        result = res->data().toInt();

    bool doCloseMain = true;

    switch (result) {
    case POPULAR_URLS_ID:
        group.writeEntry("BM Popular URLs", !hasPopularURLs);
        break;
    case TRASH_ID:
        group.writeEntry("BM Trash", !hasTrash);
        break;
    case LAN_ID:
        group.writeEntry("BM Lan", !hasLan);
        break;
    case VIRTUAL_FS_ID:
        group.writeEntry("BM Virtual FS", !hasVirtualFS);
        break;
    case JUMP_BACK_ID:
        group.writeEntry("BM Jumpback", !hasJumpback);
        break;
    default:
        doCloseMain = false;
        break;
    }

    menu.close();

    if (doCloseMain && _mainBookmarkPopup)
        _mainBookmarkPopup->close();
}

#define OPEN_ID 100200
#define OPEN_NEW_TAB_ID 100201
#define DELETE_ID 100202

void KrBookmarkHandler::rightClicked(QMenu *menu, KrBookmark *bm)
{
    QMenu popup(_mainBookmarkPopup);
    QAction *act;

    if (!bm->isFolder()) {
        act = popup.addAction(Icon("document-open"), i18n("Open"));
        act->setData(QVariant(OPEN_ID));
        act = popup.addAction(Icon("tab-new"), i18n("Open in a new tab"));
        act->setData(QVariant(OPEN_NEW_TAB_ID));
        popup.addSeparator();
    }
    act = popup.addAction(Icon("edit-delete"), i18n("Delete"));
    act->setData(QVariant(DELETE_ID));

    connect(menu, SIGNAL(highlighted(int)), &popup, SLOT(close()));
    connect(menu, SIGNAL(activated(int)), &popup, SLOT(close()));

    int result = -1;
    QAction *res = popup.exec(QCursor::pos());
    if (res && res->data().canConvert<int>())
        result = res->data().toInt();

    popup.close();
    if (_mainBookmarkPopup && result >= OPEN_ID && result <= DELETE_ID) {
        _mainBookmarkPopup->close();
    }

    switch (result) {
    case OPEN_ID:
        SLOTS->refresh(bm->url());
        break;
    case OPEN_NEW_TAB_ID:
        _mainWindow->activeManager()->newTab(bm->url());
        break;
    case DELETE_ID:
        deleteBookmark(bm);
        break;
    }
}

// used to monitor middle clicks. if mid is found, then the
// bookmark is opened in a new tab. ugly, but easier than overloading
// KAction and KActionCollection.
void KrBookmarkHandler::slotActivated(const QUrl &url)
{
    if (_mainBookmarkPopup && !_mainBookmarkPopup->isHidden())
        _mainBookmarkPopup->close();
    if (_middleClick)
        _mainWindow->activeManager()->newTab(url);
    else
        SLOTS->refresh(url);
}
