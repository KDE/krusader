/*****************************************************************************
 * Copyright (C) 2002 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2002 Rafi Yanai <yanai@users.sourceforge.net>               *
 * Copyright (C) 2004-2018 Krusader Krew [https://krusader.org]              *
 *                                                                           *
 * This file is part of Krusader [https://krusader.org].                     *
 *                                                                           *
 * Krusader is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * Krusader is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with Krusader.  If not, see [http://www.gnu.org/licenses/].         *
 *****************************************************************************/

#include "krbookmark.h"
#include "../krglobal.h"
#include "../icon.h"
#include "../Archive/krarchandler.h"
#include "../FileSystem/krtrashhandler.h"
#include "../Panel/listpanelactions.h"

#include <KI18n/KLocalizedString>
#include <KXmlGui/KActionCollection>

#define BM_NAME(X)  (QString("Bookmark:")+X)

static const char* NAME_TRASH = I18N_NOOP("Trash bin");
static const char* NAME_VIRTUAL = I18N_NOOP("Virtual Filesystem");
static const char* NAME_LAN = I18N_NOOP("Local Network");

KrBookmark::KrBookmark(QString name, QUrl url, KActionCollection *parent, QString iconName, QString actionName) :
        QAction(parent), _url(url), _iconName(iconName), _folder(false), _separator(false), _autoDelete(true)
{
    QString actName = actionName.isNull() ? BM_NAME(name) : BM_NAME(actionName);
    setText(name);
    parent->addAction(actName, this);
    connect(this, SIGNAL(triggered()), this, SLOT(activatedProxy()));

    // do we have an icon?
    if (!iconName.isEmpty())
        setIcon(Icon(iconName));
    else {
        // what kind of a url is it?
        if (_url.isLocalFile()) {
            setIcon(Icon("folder"));
        } else { // is it an archive?
            if (KRarcHandler::isArchive(_url))
                setIcon(Icon("application-x-tar"));
            else setIcon(Icon("folder-html"));
        }
    }
}

KrBookmark::KrBookmark(QString name, QString iconName) :
        QAction(Icon(iconName), name, 0), _iconName(iconName), _folder(true), _separator(false), _autoDelete(false)
{
    setIcon(Icon(iconName == "" ? "folder" : iconName));
}

KrBookmark::~KrBookmark()
{
    if (_autoDelete) {
        QListIterator<KrBookmark *> it(_children);
        while (it.hasNext())
            delete it.next();
        _children.clear();
    }
}

KrBookmark * KrBookmark::getExistingBookmark(QString actionName, KActionCollection *collection)
{
    return static_cast<KrBookmark*>(collection->action(BM_NAME(actionName)));
}

KrBookmark * KrBookmark::trash(KActionCollection *collection)
{
    KrBookmark *bm = getExistingBookmark(i18n(NAME_TRASH), collection);
    if (!bm)
        bm = new KrBookmark(i18n(NAME_TRASH), QUrl("trash:/"), collection);

    bm->setIcon(Icon(KrTrashHandler::trashIconName()));
    return bm;
}

KrBookmark * KrBookmark::virt(KActionCollection *collection)
{
    KrBookmark *bm = getExistingBookmark(i18n(NAME_VIRTUAL), collection);
    if (!bm) {
        bm = new KrBookmark(i18n(NAME_VIRTUAL), QUrl("virt:/"), collection);
        bm->setIcon(Icon("document-open-remote"));
    }
    return bm;
}

KrBookmark * KrBookmark::lan(KActionCollection *collection)
{
    KrBookmark *bm = getExistingBookmark(i18n(NAME_LAN), collection);
    if (!bm) {
        bm = new KrBookmark(i18n(NAME_LAN), QUrl("remote:/"), collection);
        bm->setIcon(Icon("network-workgroup"));
    }
    return bm;
}

QAction * KrBookmark::jumpBackAction(KActionCollection *collection, bool isSetter, ListPanelActions *sourceActions)
{
    auto actionName = isSetter ? QString("setJumpBack") : QString("jumpBack");
    auto action = collection->action(actionName);
    if (action) {
        return action;
    }

    if (!sourceActions) {
        return nullptr;
    }

    // copy essential part of source action
    auto sourceAction = isSetter ? sourceActions->actSetJumpBack : sourceActions->actJumpBack;
    action = new QAction(sourceAction->icon(), sourceAction->text(), sourceAction);
    action->setShortcut(sourceAction->shortcut());
    action->setShortcutContext(Qt::WidgetShortcut);
    connect(action, &QAction::triggered, sourceAction, &QAction::trigger);
    // ensure there are no accelerator keys coming from another menu
    action->setText(KLocalizedString::removeAcceleratorMarker(action->text()));

    collection->addAction(actionName, action);
    return action;
}

KrBookmark * KrBookmark::separator()
{
    KrBookmark *bm = new KrBookmark("");
    bm->_separator = true;
    bm->_folder = false;
    return bm;
}


void KrBookmark::activatedProxy()
{
    emit activated(url());
}

